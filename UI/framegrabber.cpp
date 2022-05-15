#include "framegrabber.h"

FrameGrabber::FrameGrabber(QObject *parent)
    : QObject{parent}
{

}

FrameGrabber::~FrameGrabber(){
    if(m_player_){
        m_player_->stop();
    }
}

void FrameGrabber::Stop(){
    m_player_->stop();
}

bool FrameGrabber::GrabFrame(QUrl url){
    m_player_ = new QMediaPlayer(this);
    m_player_->setSource(url);
    QMessageBox msgBox; // 弹窗提示
    msgBox.setIcon(QMessageBox::Warning);
    if(!m_player_->isAvailable()){
        msgBox.setText("Grab failed, please grap again!");
        msgBox.exec();
        return false;
    }
    else if(m_player_->mediaStatus() == QMediaPlayer::InvalidMedia){
        QMessageBox msgBox;
        msgBox.setText("Media Invalid, please use another!");
        msgBox.exec();
        return false;
    }

    this->BeginFetchFrame();
    return true;
}

void FrameGrabber::BeginFetchFrame(){
     QVideoSink* sink = new QVideoSink(m_player_);
     m_player_->setVideoSink(sink);
     m_player_->play();

     connect(m_player_, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus statu){
         if(statu == QMediaPlayer::InvalidMedia){
             QMessageBox msgBox;
             msgBox.setText("Invalid Media");
             msgBox.setIcon(QMessageBox::Warning);
             msgBox.exec();
             emit error();
         }
     });

     connect(sink, &QVideoSink::videoFrameChanged,this, [=](const QVideoFrame& frame){

         QMessageBox msgBox;  // 弹窗提示
         msgBox.setIcon(QMessageBox::Warning);
         if(frame.isValid()){
             if(!this->is_stop_){
             QVideoFrame targetframe(frame);
             if(targetframe.map(QVideoFrame::ReadOnly)){
                 QImage image = targetframe.toImage();
                 image = image.scaled(960,720);
                 emit framebeFetched(image);
                 targetframe.unmap();
                 is_stop_ = true;
                 this->Stop();
             }
             else {
                 msgBox.setText("frame can't be map, read failed");
                 msgBox.exec();
             }
             }
         }
         else{
             msgBox.setText("Invalid frame");
             msgBox.exec();
         }
     });
}
