#include "avdecoder.h"

AVDecoder::AVDecoder()
{
    m_decode_thread = new DecodeThread();
    m_inverter = new Inverter();

    m_video_frame_queue = new FrameQueue<VideoFrame>();
    m_audio_frame_queue = new FrameQueue<AudioFrame>();

    connect(m_decode_thread, &DecodeThread::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
    connect(m_decode_thread, &DecodeThread::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
    connect(m_decode_thread, &DecodeThread::initSucceed, this, &AVDecoder::onInit);

    connect(m_inverter, &Inverter::seekToPos, m_decode_thread, &DecodeThread::SeekToPos);
    connect(this, &AVDecoder::seekToPos, m_decode_thread, &DecodeThread::SeekToPos);

    connect(this, &AVDecoder::stop, m_decode_thread, &DecodeThread::Stop);
    connect(this, &AVDecoder::resume, m_decode_thread, &DecodeThread::Resume);
    connect(this, &AVDecoder::exit, m_decode_thread, &DecodeThread::Exit);

    connect(this, &AVDecoder::stopInverter, m_inverter, &Inverter::Stop);
    connect(this, &AVDecoder::resumeInverter, m_inverter, &Inverter::Resume);
    connect(m_decode_thread, &DecodeThread::urlError, this, [&](){
        emit this->urlError();
    });
}

AVDecoder::~AVDecoder() {

}


void AVDecoder::Open(QString url) {
    if (m_state == AVState::START) return;
    m_state = AVState::START;
    m_url = url;
    m_decode_thread->Init(m_url);
    m_sample_rate = m_decode_thread->GetAVInfomation()->sample_rate;
    m_sonic_stream = sonicCreateStream(m_sample_rate, 2);

}

void AVDecoder::Close() {
    if (m_state == AVState::CLOSE) return;
    m_state = AVState::CLOSE;
    emit exit();
    emit stopInverter();
    m_play_mode = 1;
    m_play_speed = 1;
    m_seek_pos = 0;
    m_video_clock = 0;
    m_last_video_delay = 0;
    m_audio_clock = 0;
    m_last_audio_delay = 0;
    QThread::msleep(10);
    std::thread t(&AVDecoder::reset, this);
    t.detach();
    if (m_sonic_stream) {
        sonicDestroyStream(m_sonic_stream);
        m_sonic_stream = nullptr;
    }
}

void AVDecoder::SetPos(double sec, int flag) {
    m_state = AVDecoder::STOP;
    int mode = m_play_mode.load();
    m_seek_pos = sec;
    if (mode == 1) {
        emit stop();
        emit seekToPos(sec, flag);
    } else {
        m_inverter->Stop();
        emit resumeInverter(sec);
    }
    emit resume();
    std::thread t(&AVDecoder::reset, this);
    t.detach();
    reset();
    m_state = AVDecoder::START;
}

VideoFrame* AVDecoder::GetVideoFrame() {
    VideoFrame* frame = m_video_frame_queue->pop();
    //倒放则将阈值提高
    int threshold = m_play_mode.load() == 1 ? m_video_buffer_lower_size : 60;
    if (m_video_frame_queue->size() == threshold) {
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
        double diff = m_play_mode * (m_video_clock - m_audio_clock + m_play_mode * m_last_audio_delay / 2);
        if (diff <= -sync_threshold) {
            frame->duration = std::max(AV_FRAMEDOWN_TIME, frame->duration + diff);
        } else if (diff >= sync_threshold) {
            frame->duration = std::min(AV_FRAMEUP_TIME, frame->duration + diff);
        }
        m_last_video_delay = frame->duration;
        if (m_play_speed != 1) frame->duration /= m_play_speed;
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

        scaleAudioSpeed(frame);
        return frame;
    }
    else return nullptr;
}

Thumbnail* AVDecoder::GetThumbnail(double pos) {
    return m_decode_thread->GetThumbnail(pos);
}

AVInfomation* AVDecoder::GetAVInfomation() {
    return m_decode_thread->GetAVInfomation();
}

void AVDecoder::SetPlayMode(int flag) {
    if (flag != 1 && flag != -1) return;
    if (flag == m_play_mode.load()) return;
    m_play_mode.store(flag);
    if (flag == 1) {
        connect(m_decode_thread, &DecodeThread::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
        connect(m_decode_thread, &DecodeThread::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
        disconnect(m_decode_thread, &DecodeThread::sendVideoFrame, m_inverter, &Inverter::receiveVideoFrame);
        disconnect(m_decode_thread, &DecodeThread::sendAudioFrame, m_inverter, &Inverter::receiveAudioFrame);
        disconnect(m_inverter, &Inverter::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
        disconnect(m_inverter, &Inverter::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
        disconnect(m_inverter, &Inverter::stop, m_decode_thread, &DecodeThread::Stop);
    } else {
        disconnect(m_decode_thread, &DecodeThread::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
        disconnect(m_decode_thread, &DecodeThread::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
        connect(m_decode_thread, &DecodeThread::sendVideoFrame, m_inverter, &Inverter::receiveVideoFrame);
        connect(m_decode_thread, &DecodeThread::sendAudioFrame, m_inverter, &Inverter::receiveAudioFrame);
        connect(m_inverter, &Inverter::sendVideoFrame, this, &AVDecoder::receiveVideoFrame);
        connect(m_inverter, &Inverter::sendAudioFrame, this, &AVDecoder::receiveAudioFrame);
        connect(m_inverter, &Inverter::stop, m_decode_thread, &DecodeThread::Stop);
    }
}

void AVDecoder::SetPlaySpeed(double speed) {
    m_mutex.lock();
    if (speed > 0 && speed <= 4) m_play_speed = speed;
    if (m_play_mode.load() == -1) m_play_speed = std::min(m_play_speed, 2.0);
    m_mutex.unlock();
}

void AVDecoder::reset() {
    m_mutex.lock();
    m_audio_clock = -1;
    m_video_clock = -1;
    m_last_audio_delay = 0;
    m_last_video_delay = 0;
    m_video_frame_queue->clear();
    m_audio_frame_queue->clear();
    m_mutex.unlock();
}

void AVDecoder::scaleAudioSpeed(AudioFrame* frame) {
    m_mutex.lock();
    float speed = m_play_speed;
    m_mutex.unlock();
    if (speed != 1) {
        sonicSetSpeed(m_sonic_stream, speed);

        short* buf = (short*)frame->data;
        int ret = sonicWriteShortToStream(m_sonic_stream, buf, frame->samples_num);
        if (ret) {
            int max_nb_samples = 2 * frame->samples_num / speed;
            if (speed < 1) {
                int size = max_nb_samples * 2 * 2;
                av_free(frame->data);
                av_samples_alloc(&frame->data, NULL, 2, size, AV_SAMPLE_FMT_S16, 0);
                buf = (short*)frame->data;
            }
            int new_nb_samples = sonicReadShortFromStream(m_sonic_stream, buf, max_nb_samples);
            int new_out_buffer_size = new_nb_samples * 2 * 2;
            frame->out_buffer_size = new_out_buffer_size;

            frame->duration = double(new_nb_samples) / m_sample_rate;
        }
    }
}


//
//Slots
//
void AVDecoder::receiveVideoFrame(VideoFrame* frame) {
    if (m_state == AVDecoder::START) {
        m_video_frame_queue->push(frame);
        //倒放则将阈值提高
        int threshold = m_play_mode.load() == 1 ? m_video_buffer_upper_size : 75;
        if (m_video_frame_queue->size() == threshold) {
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

