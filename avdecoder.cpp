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
    //qDebug("exit");
    if (m_state == AVState::CLOSE) return;
    emit exit();
    m_state = AVState::CLOSE;
    QThread::msleep(10);
    m_video_frame_queue->clear();
    m_audio_frame_queue->clear();
    resetClock();
}

void AVDecoder::SeekPos(double sec, int flag) {
    //float pos = flag == 1 ? m_clock
    m_state = AVDecoder::STOP;
    emit stop();
    m_seek_pos = sec;
    m_decode_thread->SeekToPos(sec);
    m_video_frame_queue->clear();
    m_audio_frame_queue->clear();
    resetClock();
    emit resume();
    m_state = AVDecoder::START;
}

VideoFrame* AVDecoder::GetVideoFrame() {
    VideoFrame* frame = m_video_frame_queue->pop();
    if (m_video_frame_queue->size() == m_video_buffer_lower_size) {
        emit resume();
    }

    if (frame != nullptr) {
        //错误帧
        if (frame->pos < 0) {
            delete frame;
            return nullptr;
        }
        m_mutex.lock();

        m_video_clock = frame->pos;

        //修正视频帧的渲染时间
        double sync_threshold = std::max(AV_SYNC_THRESHOLD_MIN, std::min(AV_SYNC_THRESHOLD_MAX, m_last_video_delay));
        double diff = m_video_clock - m_audio_clock + m_last_audio_delay / 2;
        if (diff <= -sync_threshold) {
            frame->duration = std::max(AV_FRAMEDOWN_TIME, frame->duration + diff);
        } else if (diff >= sync_threshold) {
            frame->duration = std::min(AV_FRAMEUP_TIME, frame->duration + diff / 2);
        }
        m_last_video_delay = frame->duration;
        m_mutex.unlock();
        return frame;
    }
    else return nullptr;
}

AudioFrame* AVDecoder::GetAudioFrame() {
    AudioFrame* frame = m_audio_frame_queue->pop();

    if (frame != nullptr) {
        //错误帧
        if (frame->pos < 0) {
            delete frame;
            return nullptr;
        }

        m_mutex.lock();
        m_audio_clock = frame->pos;
        m_last_audio_delay = frame->duration;
        m_mutex.unlock();

        return frame;
    }
    else return nullptr;
}

void AVDecoder::SetPlayMode(int flag) {
    m_play_mode = flag;
}

void AVDecoder::resetClock() {
    m_mutex.lock();
    m_audio_clock = -1;
    m_video_clock = -1;
    m_last_audio_delay = 0;
    m_last_video_delay = 0;
    m_mutex.unlock();
}


//
//Slots
//
void AVDecoder::receiveVideoFrame(VideoFrame* frame) {
    if (m_state == AVDecoder::START) {
        m_video_frame_queue->push(frame);
        if (m_video_frame_queue->size() == m_video_buffer_upper_size) {
            emit stop();
        }
    }
    else delete frame;
}

void AVDecoder::receiveAudioFrame(AudioFrame* frame) {
    if (m_state == AVDecoder::START) m_audio_frame_queue->push(frame);
    else delete frame;
}

void AVDecoder::onInit() {
    m_decode_thread->start();
    //可捕获该信号进行播放
    emit Ready();
}
