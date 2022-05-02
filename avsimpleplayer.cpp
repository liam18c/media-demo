#include "avsimpleplayer.h"

AVSimplePlayer::AVSimplePlayer()
{

}


void AVSimplePlayer::VideoOutput(QLabel* label) {
    AVDecoder* decoder = AVDecoder::GetInstance();
    while (1) {
        VideoFrame* frame = decoder->GetVideoFrame();
        double duration = 0;
        if (frame != nullptr) {
            duration = frame->duration;
            QImage image(frame->data, frame->width, frame->height, frame->width * 3, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(image);
            //pixmap = pixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            label->setAlignment(Qt::AlignCenter);
            label->setPixmap(pixmap);
            delete frame;
        }
        QThread::msleep(floor(duration * 1000));
    }
}
