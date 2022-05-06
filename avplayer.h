#ifndef AVPLAYER_H
#define AVPLAYER_H


#include "avdecoder.h"
#include "videoplayerthread.h"

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

    void Start(const QString& url,void*winId);
    void Resume();
    void Stop();
    void Close();
    void SetPlayMode(int flag);
    void SetPlaySpeed(double speed);
    void SetPos(double sec,int flag);
    AVInfomation* GetAVInformation();

    void* operator new(size_t)=delete;

signals:
    void PlayFinish();
    void PlayStart();

public slots:
    //接收到解码初始化环境成功的信号
    void GetDecoderReady();
    void GetPlayFinish();

private:
    ~AVPlayer();

private:
    AVPlayerState m_state;
    int m_play_mode;
    double m_play_speed;

    QMutex mutex;
    void* m_winId;
    VideoPlayerThread* m_video_player_thread = nullptr;  
    AVDecoder* m_decoder=nullptr;
};

#endif // AVPLAYER_H
