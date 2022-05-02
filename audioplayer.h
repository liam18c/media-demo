#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QLabel>
#include <QAudioOutput>
#include <QAudioSink>
#include <QAudioFormat>

#include "avdecoder.h"

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    AudioPlayer();

public slots:
    void AudioOutput();

private:

};

#endif // AUDIOPLAYER_H
