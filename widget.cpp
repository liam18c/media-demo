#include "widget.h"
#include "./ui_widget.h"

#include <QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    player=AVPlayer::getInstance();
    connect(player,&AVPlayer::PlayFinish,this,&Widget::playnext);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::playnext(){
    QString url("D:/Thunder/电影/阳光电影www.ygdy8.com.给洛杉矶的情书.2021.BD.1080P.中英双字.mkv");
    player->Start(url,(void*)ui->widget->winId());
}

void Widget::on_start_clicked()
{
    QString url("D:/Thunder/电影/e2fc2d8e937b99da033c85f78555fb79.mp4");
    player->Start(url,(void*)ui->widget->winId());
}

void Widget::on_stop_clicked()
{
    player->Stop();
}


void Widget::on_resume_clicked()
{
    player->Resume();
}




void Widget::on_close_clicked()
{
    player->Close();
}




void Widget::on_setspeed_clicked()
{
    player->SetPlaySpeed(2.5);
}


void Widget::on_setpos_clicked()
{
    player->SetPos(240);
}


void Widget::on_setmode_clicked()
{

    QString url("D:/Thunder/电影/阳光电影www.ygdy8.com.给洛杉矶的情书.2021.BD.1080P.中英双字.mkv");
    player->Start(url,(void*)ui->widget->winId());
//    player->SetPlayMode(-1);
}


void Widget::on_setVolume_clicked()
{
    QSize* size=new QSize();
    size->setWidth(750);
    size->setHeight(550);
    ui->widget->resize(*size);
    player->SetVolume(0.5);
}

