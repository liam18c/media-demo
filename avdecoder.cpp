#include "avdecoder.h"

AVDecoder::AVDecoder()
{
    m_decode_thread = new DecodeThread();

    m_video_frame_queue = new FrameQueue<VideoFrame>();
    m_audio_frame_queue = new FrameQueue<AudioFrame>();

    connect(m_decode_thread, &DecodeThread::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
    connect(m_decode_thread, &DecodeThread::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
    connect(m_decode_thread, &DecodeThread::initSucceed, this, &AVDecoder::onInit);

    connect(this, &AVDecoder::stop, m_decode_thread, &DecodeThread::Stop);
    connect(this, &AVDecoder::resume, m_decode_thread, &DecodeThread::Resume);
    connect(this, &AVDecoder::exit, m_decode_thread, &DecodeThread::Exit);

}

AVDecoder::~AVDecoder() {

}


void AVDecoder::Open(QString url) {
    if (m_state == AVState::START) return;
    m_state = AVState::START;
    m_url = url;
    m_decode_thread->Init(m_url);
}

void AVDecoder::Close() {
    qDebug("exit");
    if (m_state == AVState::CLOSE) return;
    emit exit();
    m_state = AVState::CLOSE;
    QThread::msleep(1);
    m_video_frame_queue->clear();
    m_audio_frame_queue->clear();
}

void AVDecoder::SeekPos(float sec) {
    m_decode_thread->SeekToPos(sec);
}

VideoFrame* AVDecoder::GetVideoFrame() {
    VideoFrame* frame = m_video_frame_queue->pop();
    if (m_video_frame_queue->size() <= m_video_buffer_lower_size) emit resume();
    if (frame != nullptr) return frame;
    else return nullptr;
}

AudioFrame* AVDecoder::GetAudioFrame() {
    AudioFrame* frame = m_audio_frame_queue->pop();
    if (frame != nullptr) return frame;
    else return nullptr;
}

bool AVDecoder::HasFrame(){return m_video_frame_queue->size()>0;}
//
//Slots
//
void AVDecoder::receiveVideoFrame(VideoFrame* frame) {
    if (m_video_frame_queue->size() >= m_video_buffer_upper_size) emit stop();
    if (m_state == AVDecoder::START) m_video_frame_queue->push(frame);
    else delete frame;
    //qDebug("%d ", m_video_frame_queue->size());
}

void AVDecoder::receiveAudioFrame(AudioFrame* frame) {
    if (m_state == AVDecoder::START) m_audio_frame_queue->push(frame);
    else delete frame;
    //qDebug("%d", m_audio_frame_queue->size());
}

void AVDecoder::onInit() {
    m_decode_thread->start();
    qDebug("start to play");
    //可捕获该信号进行播放
    emit Ready();
}
