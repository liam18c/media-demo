#ifndef AUDIOPLAYERTHREAD_H
#define AUDIOPLAYERTHREAD_H

#include <QThread>
#include <avdecoder.h>

class AudioPlayerThread : public QThread
{
    Q_OBJECT
public:
    AudioPlayerThread();
    void Init(AVDecoder*);

    void Start();
    void Resume();
    void Stop();
    void Close();
    void SetSpeed(float);

protected:
    void run() override;

private:
    void release();

private:

    QMutex stop_mutex;
    std::atomic_bool m_exit = false;

    AVDecoder* m_decoder;
};

#endif // AUDIOPLAYERTHREAD_H
