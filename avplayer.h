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

    void Start(const QString&);
    void Resume();
    void Stop();
    void Close();
    void SetOrder(bool);
    void SetSpeed(float);
    void SeekPos(float);

    void* operator new(size_t)=delete;

public slots:
    void GetDecoderReady();

private:
    ~AVPlayer();

private:
    AVPlayerState m_state;
    bool m_order;
    float m_speed;

    QMutex mutex;
    VideoPlayerThread* m_video_player_thread = nullptr;  
    AVDecoder* m_decoder=nullptr;
};

#endif // AVPLAYER_H
