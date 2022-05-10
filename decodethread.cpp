#include "decodethread.h"

#define ERROR_BUF_SIZE 1024

DecodeThread::DecodeThread() {}

void DecodeThread::Init(const QString& url) {
    int ret = 0;
    QThread::msleep(1);
    std::unique_lock lock(m_mutex);
    m_close = false;
    m_exit.store(false);
    m_stop.store(false);
    //qDebug("start init");
    m_fmt_ctx = avformat_alloc_context();
    m_avInfomation = new AVInfomation();
    ret = avformat_open_input(&m_fmt_ctx, url.toStdString().c_str(), 0, 0);
    if (ret < 0) {
        av_strerror(ret, m_errbuf, ERROR_BUF_SIZE);
        printf("Open file failed:");
        printf("%s\n", m_errbuf);
        return;
    }
    ret = avformat_find_stream_info(m_fmt_ctx, 0);
    if (ret < 0) return;

    for (int i = 0; i < m_fmt_ctx->nb_streams; i++) {
        AVStream* in_stream = m_fmt_ctx->streams[i];
        const AVCodecParameters* codec_paras = m_fmt_ctx->streams[i]->codecpar;
        if (codec_paras->codec_type == AVMEDIA_TYPE_VIDEO) {
            //填充视频信息
            m_video_stream = i;
            m_avInfomation->frame_rate = in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den;
            //qDebug("%d", m_avInfomation->frame_rate);

            //打开视频解码器
            m_video_codec = avcodec_find_decoder(codec_paras->codec_id);
            if (m_video_codec == NULL) {
                printf("Find video decoder failed!\n");
                return;
            }
            m_video_codec_ctx = avcodec_alloc_context3(m_video_codec);
            avcodec_parameters_to_context(m_video_codec_ctx, codec_paras);
            if (m_video_codec_ctx == NULL) {
                printf("Allocate video codec_ctx failed!\n");
                return;
            }
            ret = avcodec_open2(m_video_codec_ctx, m_video_codec, 0);
            if (ret < 0) {
                av_strerror(ret, m_errbuf, ERROR_BUF_SIZE);
                printf("Open video codec failed!\n");
                return;
            }
        } else if (codec_paras->codec_type == AVMEDIA_TYPE_AUDIO) {
            //打开音频信息
            m_audio_stream = i;
            m_audio_codec = avcodec_find_decoder(codec_paras->codec_id);
            if (m_audio_codec == NULL) {
                printf("Find audio decoder failed!\n");
                return;
            }
            m_audio_codec_ctx = avcodec_alloc_context3(m_audio_codec);
            avcodec_parameters_to_context(m_audio_codec_ctx, codec_paras);
            if (m_audio_codec_ctx == NULL) {
                printf("Allocate audio codec_ctx failed!\n");
                return;
            }
            ret = avcodec_open2(m_audio_codec_ctx, m_audio_codec, 0);
            if (ret < 0) {
                av_strerror(ret, m_errbuf, ERROR_BUF_SIZE);
                printf("Open audio codec failed!\n");
                return;
            }

            AVChannelLayout out_ch_layout;
            out_ch_layout.order = AV_CHANNEL_ORDER_NATIVE;
            out_ch_layout.nb_channels = 2;
            out_ch_layout.u.mask = AV_CH_LAYOUT_STEREO;
            swr_alloc_set_opts2(&m_swr_ctx,
                                &out_ch_layout, AV_SAMPLE_FMT_S16, m_audio_codec_ctx->sample_rate,
                                &m_audio_codec_ctx->ch_layout, m_audio_codec_ctx->sample_fmt, m_audio_codec_ctx->sample_rate,
                                0, 0);
            swr_init(m_swr_ctx);
            if (swr_is_initialized(m_swr_ctx) > 0) {
                printf("Audio resample context init succeed!\n");
            } else {
                printf("Audio resample context init failed!\n");
                return;
            }
        }
    }
    m_pkt = av_packet_alloc();
    m_frame = av_frame_alloc();
    //填充音视频数据
    fillInfomation();
    lock.unlock();
    //生成视频缩略图
    //genThumbnails();
    emit initSucceed();
}

void DecodeThread::run() {
    SeekToPos(0, 0);
    int ret = 0;
    while (!m_exit.load()) {
        if (!m_stop.load()) {
            std::unique_lock lock(m_mutex);
            ret = av_read_frame(m_fmt_ctx, m_pkt);
            if (ret >= 0) {
                //解码视频帧
                if (m_pkt->stream_index == m_video_stream) {
                    ret = avcodec_send_packet(m_video_codec_ctx, m_pkt);

                    while (ret >= 0) {
                        ret = avcodec_receive_frame(m_video_codec_ctx, m_frame);
                        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;

                        VideoFrame* frame = decodeVideoFrame(m_frame);
                        //qDebug("%f", frame->pos);
                        if (frame != nullptr) emit sendVideoFrame(frame);
                        av_frame_unref(m_frame);
                    }
                } else if (m_pkt->stream_index == m_audio_stream) {
                    //解码音频帧
                    ret = avcodec_send_packet(m_audio_codec_ctx, m_pkt);
                    while (ret >= 0) {
                        ret = avcodec_receive_frame(m_audio_codec_ctx, m_frame);
                        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;

                        AudioFrame* frame = decodeAudioFrame(m_frame);
                        if (frame != nullptr) emit sendAudioFrame(frame);

                        av_frame_unref(m_frame);
                    }
                }
                av_packet_unref(m_pkt);
            } else {
                av_strerror(ret, m_errbuf, 1024);
                //qDebug("%s", m_errbuf);
                m_stop.store(true);
            }
        } else {
            QThread::msleep(1);
        }
    }
    release();
}

void DecodeThread::SeekToPos(double sec, int flag) {
     if (m_exit.load()) return;
     if (sec < 0 || sec > m_avInfomation->duration) return;
     std::unique_lock lock(m_mutex);
     m_seek_pos = flag ? sec : 0;
     int ret = 0;
     if (m_video_stream != -1) {
         int video_timestamp = floor(sec / av_q2d(m_fmt_ctx->streams[m_video_stream]->time_base));
         ret = av_seek_frame(m_fmt_ctx, m_video_stream, video_timestamp, AVSEEK_FLAG_BACKWARD);
     } else {
         int audio_timestamp = floor(sec / av_q2d(m_fmt_ctx->streams[m_audio_stream]->time_base));
         ret = av_seek_frame(m_fmt_ctx, m_audio_stream, audio_timestamp, AVSEEK_FLAG_ANY);
     }
     if (ret < 0) {
         av_strerror(ret, m_errbuf, 1024);
         qDebug("%s", m_errbuf);
         return;
     }
     if (m_video_codec_ctx) avcodec_flush_buffers(m_video_codec_ctx);
     if (m_audio_codec_ctx) avcodec_flush_buffers(m_audio_codec_ctx);
}

Thumbnail* DecodeThread::GetThumbnail(double pos) {
    if (pos < 0 || pos > m_avInfomation->duration) return nullptr;
    for (int i = 0; i < m_thumbnails.size(); i++) {
        if (i == m_thumbnails.size() - 1) return m_thumbnails[i];
        if (pos < m_thumbnails[i]->pos && i > 0) return m_thumbnails[i - 1];
    }
    return nullptr;
}

void DecodeThread::fillInfomation() {
    m_avInfomation->duration = m_fmt_ctx->duration / AV_TIME_BASE;
    if (m_video_codec_ctx) {
        m_avInfomation->width = m_video_codec_ctx->width;
        m_avInfomation->height = m_video_codec_ctx->height;
    }
    if (m_audio_codec_ctx) {
        m_avInfomation->sample_rate = m_audio_codec_ctx->sample_rate;
    }
    m_avInfomation->channels = 2;
    m_avInfomation->sample_size = 16;
}

void DecodeThread::release() {
    m_mutex.lock();
    if (m_avInfomation) {
        delete m_avInfomation;
        m_avInfomation = nullptr;
    }
    if (m_swr_ctx) {
        swr_free(&m_swr_ctx);
        m_swr_ctx = nullptr;
    }
    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    if (m_pkt) {
        av_packet_free(&m_pkt);
        m_pkt = nullptr;
    }
    if (m_audio_codec_ctx) {
        avcodec_free_context(&m_audio_codec_ctx);
        m_audio_codec_ctx = nullptr;
    }
    if (m_video_codec_ctx) {
        avcodec_free_context(&m_video_codec_ctx);
        m_video_codec_ctx = nullptr;
    }
    if (m_fmt_ctx) {
        avformat_close_input(&m_fmt_ctx);
        avformat_free_context(m_fmt_ctx);
        m_fmt_ctx = nullptr;
    }
    for (int i = 0; i < m_thumbnails.size(); i++) {
        delete m_thumbnails[i];
    }
    m_thumbnails.clear();
    m_close = true;
    m_video_stream = -1;
    m_audio_stream = -1;
    m_mutex.unlock();
}

VideoFrame* DecodeThread::decodeVideoFrame(AVFrame* frame) {
    double pos = frame->best_effort_timestamp * av_q2d(m_fmt_ctx->streams[m_video_stream]->time_base);
    if (pos >= m_seek_pos) {
        uint8_t* data = yuvToRGB24(frame, frame->width, frame->height);
        VideoFrame* vframe = new VideoFrame(data, frame->width, frame->height);
        double pos = frame->pts * av_q2d(m_fmt_ctx->streams[m_video_stream]->time_base);
        double duration = r2d(1, m_avInfomation->frame_rate);
        duration += frame->repeat_pict / (2 * m_avInfomation->frame_rate);
        vframe->SetTime(pos, duration);
        return vframe;
    } else {
        return nullptr;
    }
}

AudioFrame* DecodeThread::decodeAudioFrame(AVFrame* frame) {
    double pos = frame->pts * av_q2d(m_fmt_ctx->streams[m_audio_stream]->time_base);
    if (pos >= m_seek_pos) {

        uint8_t* data;
        int out_samples = av_rescale_rnd(frame->nb_samples, m_avInfomation->sample_rate, frame->sample_rate, AV_ROUND_UP);
        av_samples_alloc(&data, NULL, 2, out_samples, AV_SAMPLE_FMT_S16, 1);
        out_samples = swr_convert(m_swr_ctx, &data, out_samples, (const uint8_t**)frame->data, frame->nb_samples);

        int out_buffer_size = av_samples_get_buffer_size(0, 2, out_samples, AV_SAMPLE_FMT_S16, 1);
        //int out_buffer_size = 2 * 2 * out_samples;
        AudioFrame* aframe = new AudioFrame(data, out_samples, out_buffer_size, 2);
        double duration = double(out_samples) / m_avInfomation->sample_rate;
        aframe->SetTime(pos, duration);
        return aframe;
    } else {
        return nullptr;
    }
}

void DecodeThread::genThumbnails() {
    int pos = 0;
    int ret = 0;
    int last_keyframe_pts = -1;
    while (!m_exit.load() && pos <= m_avInfomation->duration) {
        SeekToPos(pos, 0);
        while (av_read_frame(m_fmt_ctx, m_pkt) >= 0) {
            if (m_pkt->stream_index == m_video_stream) {
                ret = avcodec_send_packet(m_video_codec_ctx, m_pkt);
                if (ret >= 0) {
                    ret = avcodec_receive_frame(m_video_codec_ctx, m_frame);
                    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) continue;
                    if (m_frame->key_frame && m_frame->pts != last_keyframe_pts) {
                        last_keyframe_pts = m_frame->pts;
                        uint8_t* rgb = yuvToRGB24(m_frame, 200, 120);
                        Thumbnail* thumbnail = new Thumbnail(rgb, 200, 120, pos);
                        m_thumbnails.push_back(thumbnail);
                        break;
                    }
                }
                av_frame_unref(m_frame);
            }
        }
        av_packet_unref(m_pkt);
        pos += 5;
    }
}

//
//Slots
//
void DecodeThread::Exit() {
    m_stop.store(true);
    m_exit.store(true);
}

void DecodeThread::Stop() {
    m_stop.store(true);
}

void DecodeThread::Resume() {
    m_stop.store(false);
}

