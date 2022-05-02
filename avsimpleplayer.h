#ifndef AVSIMPLEPLAYER_H
#define AVSIMPLEPLAYER_H
#include <QWidget>
#include <QLabel>
#include <QAudioOutput>
#include <QAudioSink>
#include <QAudioFormat>

#include "avdecoder.h"

class AVSimplePlayer : public QObject
{
public:
    AVSimplePlayer();

public slots:
    void VideoOutput(QLabel* label);


private:
};

#endif // AVSIMPLEPLAYER_H
