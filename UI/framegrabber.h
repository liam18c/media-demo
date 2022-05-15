#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMessageBox>

class FrameGrabber : public QObject
{
    Q_OBJECT
public:
    explicit FrameGrabber(QObject *parent = nullptr);
    ~FrameGrabber();
    bool GrabFrame(QUrl url);
    void Stop();

signals:
    void error();
    void framebeFetched(const QImage& image);

private:
    void BeginFetchFrame();

private:
    QMediaPlayer* m_player_;
    bool is_stop_=false;
};

#endif // FRAMEGRABBER_H
