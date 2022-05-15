#ifndef DECODEUTILS_H
#define DECODEUTILS_H

#include <QByteArray>
#include <QThread>

#include "sonic.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

class AVType{
public:
    const static uint32_t TYPENONE=1;
    const static uint32_t TYPEVIDEO=2;
    const static uint32_t TYPEAUDIO=4;
};

struct AVInfomation {
    //视频信息
    double duration = 0;
    uint32_t type=AVType::TYPENONE;

    //视频流信息
    int frame_rate = 0;
    int width = 0;
    int height = 0;

    //音频流信息
    int channels = 0;
    int sample_rate = 0;
    int sample_size = 0;
};

struct VideoFrame {
    uint8_t* data;
    int height;
    int width;
    double pos;//sec
    double duration;//sec

    VideoFrame() {
        data = nullptr;
    };
    VideoFrame(uint8_t* data, int width, int height) : data(data), width(width), height(height) {};

    ~VideoFrame() {
        delete data;
    }

    void SetTime(double pos, double duration) {
        this->pos = pos;
        this->duration = duration;
    }
};

struct AudioFrame {
    uint8_t* data;
    int samples_num;
    int out_buffer_size;
    int channels;
    double pos;//sec
    double duration;//sec

    AudioFrame() {
        data = nullptr;
    };
    AudioFrame(uint8_t* data, int samplesNum, int outBufferSize, int channels) : data(data), samples_num(samplesNum),
        out_buffer_size(outBufferSize), channels(channels) {};

    ~AudioFrame() {
        av_free(data);
    }

    void SetTime(double pos, double duration) {
        this->pos = pos;
        this->duration = duration;
    }

};

struct Thumbnail {
    uint8_t* data;
    int width;
    int height;
    int pos;

    Thumbnail();
    Thumbnail(uint8_t* data, int width, int height, int pos) : data(data), width(width), height(height), pos(pos){};

    ~Thumbnail() {
        delete data;
    }
};

uint8_t* yuvToRGB24(AVFrame* pFrame, int outWidth = 1280, int outHeight = 720, AVPixelFormat srcPixelFormat = AV_PIX_FMT_YUV420P);

uint8_t* yuvToBGRA (AVFrame* pFrame, int outWidth = 1280, int outHeight = 720, AVPixelFormat srcPixelFormat = AV_PIX_FMT_YUV420P);

inline double r2d(int num, int den) {
    return double(num) / den;
}

#endif // DECODEUTILS_H
