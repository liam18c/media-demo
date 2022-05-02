#ifndef DECODEUTILS_H
#define DECODEUTILS_H

#include <QByteArray>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

struct VideoFrame {
    uint8_t* data;
    int height;
    int width;
    double duration;
    float pos;

    VideoFrame() {
        data = nullptr;
    };
    VideoFrame(uint8_t* data, int width, int height) : data(data), width(width), height(height) {};

    ~VideoFrame() {
        delete data;
    }
};

struct AudioFrame {
    uint8_t* data;
    int sample_size;
    int sample_rate;
    int out_buffer_size;
    int channels;
    float pos;
    AudioFrame() {
        data = nullptr;
    };
    AudioFrame(uint8_t* data, int sampleSize, int sampleRate, int outBufferSize, int channels) : data(data), sample_size(sampleSize)
      , sample_rate(sampleRate), out_buffer_size(outBufferSize), channels(channels) {};

    ~AudioFrame() {
        //delete data;
    }

};

uint8_t* yuvToRGB24(AVFrame* pFrame, int outWidth = 1280, int outHeight = 720, AVPixelFormat srcPixelFormat = AV_PIX_FMT_YUV420P);

uint8_t* yuvToBGRA (AVFrame* pFrame, int outWidth = 1280, int outHeight = 720, AVPixelFormat srcPixelFormat = AV_PIX_FMT_YUV420P);

#endif // DECODEUTILS_H
