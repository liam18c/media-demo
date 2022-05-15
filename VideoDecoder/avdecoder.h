#ifndef AVDECODER_H
#define AVDECODER_H

#include <QQueue>
#include <QStack>
#include <QTimerEvent>
#include <cmath>

#include "decodethread.h"

#define AV_SYNC_THRESHOLD_MIN 0.01 //sec
#define AV_SYNC_THRESHOLD_MAX 0.03 //sec
#define AV_FRAMEDOWN_TIME 0.01 //sec
#define AV_FRAMEUP_TIME 0.08 //sec
#define AV_END_THRESHOLD 100 //msec

class Inverter;
template <typename T>
class FrameQueue;

class AVDecoder : public QObject
{
    Q_OBJECT

    enum AVState {START = 0, STOP = 1, CLOSE = 2};
public:
    AVDecoder();
    //单例模式
    static AVDecoder* GetInstance() {
        static AVDecoder instance;
        return &instance;
    }

    void Open(QString url);
    //0代表seek，1代表快进或后退，无论哪种模式，sec都表示视频开始到目标时间点的时间总和
    void SetPos(double sec, int flag);
    //1代表顺序播放，-1代表倒放
    void SetPlayMode(int flag);
    //设置播放速度
    void SetPlaySpeed(double speed);
    void Close();

    //目前视频帧固定为rgb24的格式
    VideoFrame* GetVideoFrame();
    //目前音频帧全部重采样为44100，16，2格式
    AudioFrame* GetAudioFrame();
    Thumbnail* GetThumbnail(double pos);
    AVInfomation* GetAVInfomation();

    void* operator new(size_t) = delete;

signals:
    //解码环境初始化成功发送该信号，可捕获
    void Ready();
    void urlError();

private slots:
    void receiveVideoFrame(VideoFrame* frame);
    void receiveAudioFrame(AudioFrame* frame);

    void onInit();

signals:
    void exit();
    void stop();
    void resume();
    void stopInverter();
    void resumeInverter(double sec);
    void seekToPos(double sec, int flag);

private:
    void reset();
    void scaleAudioSpeed(AudioFrame* frame);

    ~AVDecoder();

private: 
    std::atomic_int m_state = AVState::CLOSE;
    std::atomic_int m_play_mode = 1;
    int m_sample_rate = 44100;
    double m_play_speed = 1;

    QString m_url;
    DecodeThread* m_decode_thread = nullptr;
    Inverter* m_inverter = nullptr;

    const int m_video_buffer_upper_size = 30;
    const int m_video_buffer_lower_size = 15;

    std::mutex m_mutex;

    double m_seek_pos = 0;//sec
    double m_video_clock = 0;//sec
    double m_last_video_delay = 0;//sec
    double m_audio_clock = 0;//sec
    double m_last_audio_delay = 0;//sec

    sonicStream m_sonic_stream;

    FrameQueue<VideoFrame>* m_video_frame_queue;
    FrameQueue<AudioFrame>* m_audio_frame_queue;
};


class Inverter : public QThread {
    Q_OBJECT
public:
    Inverter() {

    }

public slots:

    void Resume(double sec) {
        //播放位置大于0才能倒放
        if (sec > 0) {
            reset(sec);
            m_stop.store(false);
            emit seekToPos(std::max(sec - 1, 0.0), 0);
            start();
        }
    }
    void Stop() {
        m_stop.store(true);
        emit stop();
        reset(-1);
    }

    void receiveVideoFrame(VideoFrame* frame) {
        if (frame == nullptr) return;
        std::unique_lock lock(m_mutex);
        if (!m_stop.load() && frame->pos <= m_seek_pos) {
            while (!m_video_frame_stack.empty() && std::abs(frame->pos - m_video_frame_stack.top()->pos) >= 0.1) {
                delete m_video_frame_stack.top();
                m_video_frame_stack.pop();
                m_cur_pos = frame->pos;
            }

            m_cur_pos = std::max(0.0, std::min(m_cur_pos, frame->pos));
            m_video_frame_stack.push(frame);
        } else {
            delete frame;
        }
    }
    void receiveAudioFrame(AudioFrame* frame) {
        if (frame == nullptr) return;
        std::unique_lock lock(m_mutex);
        if (frame->pos <= m_seek_pos) {
            while (!m_audio_frame_stack.empty() && std::abs(frame->pos - m_audio_frame_stack.top()->pos) >= 0.1) {
                delete m_audio_frame_stack.top();
                m_audio_frame_stack.pop();
            }
            m_audio_frame_stack.push(frame);
        } else {
            delete frame;
        }
    }


signals:
    void stop();
    void seekToPos(double sec, int flag);
    void sendVideoFrame(VideoFrame* frame);
    void sendAudioFrame(AudioFrame* frame);

protected:
    void run() override {
        while (!m_stop.load()) {
            m_mutex.lock();
            if (m_seek_pos <= 0) {
                emit stop();
                m_mutex.unlock();
                break;
            }
            if (!m_video_frame_loaded && !m_video_frame_stack.empty() &&
                m_video_frame_stack.top()->pos + m_video_frame_stack.top()->duration >= m_seek_pos) {
                m_video_frame_loaded = true;
            }
            if (!m_audio_frame_loaded && !m_audio_frame_stack.empty() &&
                m_audio_frame_stack.top()->pos + m_audio_frame_stack.top()->duration >= m_seek_pos) {
                m_audio_frame_loaded = true;
            }
            if (m_video_frame_loaded || m_audio_frame_loaded) {
                while (!m_video_frame_stack.empty()) {
                    emit sendVideoFrame(m_video_frame_stack.top());
                    m_video_frame_stack.pop();
                }
                while (!m_audio_frame_stack.empty()) {
                    emit sendAudioFrame(m_audio_frame_stack.top());
                    m_audio_frame_stack.pop();
                }
                m_video_frame_loaded = false;
                m_audio_frame_loaded = false;
                m_seek_pos = m_cur_pos;
                if (m_cur_pos > 0) m_cur_pos = std::max(0.0, m_cur_pos - 1);
                emit seekToPos(m_cur_pos, 0);
            }
            m_mutex.unlock();
        }
    }

private:
    void reset(double sec) {
        m_mutex.lock();
        m_seek_pos = sec;
        m_cur_pos = sec;
        //QThread::msleep(1);
        while (!m_video_frame_stack.empty()) {
            delete m_video_frame_stack.top();
            m_video_frame_stack.pop();
        }
        while (!m_audio_frame_stack.empty()) {
            delete m_audio_frame_stack.top();
            m_audio_frame_stack.pop();
        }
        m_video_frame_loaded = false;
        m_audio_frame_loaded = false;
        m_mutex.unlock();
    }

private:
    std::mutex m_mutex;
    QStack<VideoFrame*> m_video_frame_stack;
    QStack<AudioFrame*> m_audio_frame_stack;

    std::atomic_bool m_stop = false;
    bool m_video_frame_loaded = false;
    bool m_audio_frame_loaded = false;
    double m_seek_pos = 0;
    double m_cur_pos = 0;
};

template <typename T>
class FrameQueue {
public:
    void push(T* frame) {
        mutex.lock();
        data.enqueue(frame);
        mutex.unlock();
    }

    T* pop() {
        mutex.lock();
        if (data.size() == 0) {
            mutex.unlock();
            return nullptr;
        }
        T* frame = data.dequeue();
        mutex.unlock();
        return frame;
    }

    void clear() {
        mutex.lock();
        while (!data.empty()) {
            delete data.dequeue();
        }
        mutex.unlock();
    }

    int size() {
        mutex.lock();
        int temp = data.size();
        mutex.unlock();
        return temp;
    }

private:
    std::mutex mutex;
    QQueue<T*> data;
};

#endif // AVDECODER_H
