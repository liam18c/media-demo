
#include "decodeutils.h"

uint8_t* yuvToRGB24(AVFrame* pFrame, int outWidth, int outHeight, AVPixelFormat srcPixelFormat) {
    int srcW = pFrame->width;
    int srcH = pFrame->height;
    SwsContext* sws_ctx = nullptr;
    sws_ctx = sws_getCachedContext(sws_ctx, srcW, srcH, srcPixelFormat, outWidth, outHeight, AV_PIX_FMT_RGB24, SWS_BICUBIC, 0, 0, 0);
    uint8_t* data[AV_NUM_DATA_POINTERS] = {0};
    uint8_t* dst = new uint8_t[outWidth * outHeight* 3];
    //uint8_t* dst = new uint8_t[srcW * srcH * 3];
    data[0] = dst;
    int dst_linesize[AV_NUM_DATA_POINTERS] = {0};
    dst_linesize[0] = outWidth * 3;
    sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, srcH, data, dst_linesize);
    sws_freeContext(sws_ctx);
    return dst;
}

uint8_t* yuvToBGRA(AVFrame* pFrame, int outWidth, int outHeight, AVPixelFormat srcPixelFormat) {
    int srcW = pFrame->width;
    int srcH = pFrame->height;
    SwsContext* sws_ctx = nullptr;
    sws_ctx = sws_getCachedContext(sws_ctx, srcW, srcH, srcPixelFormat, outWidth, outHeight, AV_PIX_FMT_BGRA, SWS_BICUBIC, 0, 0, 0);
    uint8_t* data[AV_NUM_DATA_POINTERS] = {0};
    uint8_t* dst = new uint8_t[outWidth * outHeight* 4];
    //uint8_t* dst = new uint8_t[srcW * srcH * 3];
    data[0] = dst;
    int dst_linesize[AV_NUM_DATA_POINTERS] = {0};
    dst_linesize[0] = outWidth * 4;
    sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, srcH, data, dst_linesize);
    return dst;
}


