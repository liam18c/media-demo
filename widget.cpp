#include "widget.h"
#include "./ui_widget.h"

#include <QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


    video_player.moveToThread(&video_output_thread);
    video_output_thread.start();
    connect(this, &Widget::startVideo, &video_player, &AVSimplePlayer::VideoOutput);

    audio_player.moveToThread(&audio_output_thread);
    audio_output_thread.start();
    connect(this, &Widget::startAudio, &audio_player, &AudioPlayer::AudioOutput);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString url("../MediaPlayer/Resources/apex.mp4");
    AVDecoder::GetInstance()->Open(url);
}


void Widget::on_exit_clicked()
{
    AVDecoder::GetInstance()->Close();
}


void Widget::on_resume_clicked()
{

}


void Widget::Start() {

}


void Widget::on_show_clicked()
{
    emit startVideo(ui->label);
    emit startAudio();
}


void Widget::on_pushButton_2_clicked()
{
    QString str = ui->lineEdit->text();
    float sec = str.toFloat();
    AVDecoder::GetInstance()->SeekPos(sec, 0);
}

