#include "widget.h"
#include "./ui_widget.h"

#include <QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    player=AVPlayer::getInstance();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString url("../media-demo-Cen/Resources/TestVideo.mp4");
    player->Start(url);
}


void Widget::on_exit_clicked()
{

}


void Widget::on_resume_clicked()
{
    player->Stop();
}


void Widget::on_show_clicked()
{
    player->Resume();
//    VideoFrame* frame = player->m_decoder->GetVideoFrame();
//    QImage image(frame->data, frame->width, frame->height, frame->width * 3, QImage::Format_RGB888);
//    QPixmap pixmap = QPixmap::fromImage(image);
//    pixmap = pixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    ui->label->setAlignment(Qt::AlignCenter);
//    ui->label->setPixmap(pixmap);
}

