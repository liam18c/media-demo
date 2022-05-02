#ifndef AVDECODER_H
#define AVDECODER_H

#include <QQueue>

#include "decodethread.h"

template <typename T>
class FrameQueue;

class AVDecoder : public QObject
{
    Q_OBJECT

public:
    enum AVState {START, CLOSE};

    AVDecoder();
    static AVDecoder* GetInstance() {
        static AVDecoder instance;
        return &instance;
    }

    void Open(QString url);
    void SeekPos(float sec);
    void Close();

    VideoFrame* GetVideoFrame();
    AudioFrame* GetAudioFrame();

    void* operator new(size_t) = delete;
    bool HasFrame();
public slots:

signals:
    void Ready();

private slots:
    void receiveVideoFrame(VideoFrame* frame);
    void receiveAudioFrame(AudioFrame* frame);

    void onInit();

signals:
    void exit();
    void stop();
    void resume();

private:
    ~AVDecoder();

private: 
    volatile AVState m_state = AVState::CLOSE;

    //QMutex m_mutex;
    QString m_url;
    DecodeThread* m_decode_thread = nullptr;

    int m_video_buffer_upper_size = 10;
    int m_video_buffer_lower_size = 5;

    float m_seek_pos = 0;

    FrameQueue<VideoFrame>* m_video_frame_queue;
    FrameQueue<AudioFrame>* m_audio_frame_queue;
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
    QMutex mutex;
    QQueue<T*> data;
};

#endif // AVDECODER_H
