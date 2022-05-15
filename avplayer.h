#ifndef AVPLAYER_H
#define AVPLAYER_H


#include "avdecoder.h"
#include "videoplayerthread.h"
#include "audioplayer.h"

class AVPlayer : public QObject
{
    Q_OBJECT

public:
    enum AVPlayerState{
        START,STOP,CLOSE
    };

    AVPlayer();
    static AVPlayer*getInstance(){
        static AVPlayer instance;
        return &instance;
    }

    //指定播放的文件地址与绑定的label的winId
    void Start(const QString& url,void*winId);
    //当前位置播放
    void Resume();
    //当前位置暂停
    void Stop();
    //主动关闭，视频播放完成则自动结束，不需主动关闭
    void Close();
    //播放模式，1代表顺序播放，-1代表倒放
    void SetPlayMode(int flag);
    //设置播放速度：0~4
    void SetPlaySpeed(double speed);
    //快进或后退
    void SetPos(double sec);
    //设置音量 0~1
    void SetVolume(double volume);
    //获取音视频信息
    AVInfomation* GetAVInformation();
    //获取当前播放帧信息
    VideoFrame* GetCurrentFrame();

    AVPlayer::AVPlayerState GetPlayState() const;
    void* operator new(size_t)=delete;

signals:
    void PlayFinish();
    void PlayStart();
    void PlayStateChange(AVPlayer::AVPlayerState);
    void VideoPositionChange(qint64);
    void urlError();

public slots:
    //接收到解码初始化环境成功的信号
    void GetDecoderReady();
    void GetPlayFinish();

private:
    ~AVPlayer();

private:
    AVPlayerState m_state;
    uint32_t m_av_type;
    int m_play_mode;
    double m_play_speed;

    QMutex mutex;
    void* m_winId;
    VideoPlayerThread* m_video_player_thread = nullptr;  
    AudioPlayer* m_audio_player=nullptr;
    AVDecoder* m_decoder=nullptr;
};

#endif // AVPLAYER_H
