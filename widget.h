#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QAudioOutput>
#include <QAudioSink>
#include <QAudioFormat>

#include "avdecoder.h"
#include "audioplayer.h"
#include "avsimpleplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void Start();

signals:
    void startVideo(QLabel* label);
    void startAudio();

private slots:
    void on_pushButton_clicked();

    void on_exit_clicked();

    void on_resume_clicked();

    void on_show_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::Widget *ui;
    AVSimplePlayer video_player;
    AudioPlayer audio_player;

    QThread video_output_thread;
    QThread audio_output_thread;
};
#endif // WIDGET_H
