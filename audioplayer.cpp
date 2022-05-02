#include "audioplayer.h"

AudioPlayer::AudioPlayer()
{

}


void AudioPlayer::AudioOutput() {
    QAudioFormat fmt;
    fmt.setSampleRate(44100);
    fmt.setSampleFormat(QAudioFormat::Int16);
    fmt.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    QAudioSink* output = new QAudioSink(fmt);
    QIODevice* io = output->start();
    AVDecoder* decoder = AVDecoder::GetInstance();
    int num = 0;
    while (1) {
        AudioFrame* frame = decoder->GetAudioFrame();
        int msec = 0;
        if (frame != nullptr) {
            msec = floor(frame->duration * 1000);
            char* out = (char*) frame->data;
            io->write(out, frame->out_buffer_size);
            delete frame;
        }

        QThread::msleep(msec);
    }
}
