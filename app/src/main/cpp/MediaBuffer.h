//
// Created by laoguo on 2016-8-24.
//

#ifndef LIVESCREEN_RINGBUFFER_H
#define LIVESCREEN_RINGBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <linux/time.h>
#include <android/log.h>
#define MAX_VIDEO_SIZE      (250*1024)
#define MAX_AUDIO_SIZE      (10*1024)
#define MAX_SPS_SIZE        (100)
#define MAX_PPS_SIZE        (100)
#define VIDEO_BUF_NUM       16
#define AUDIO_BUF_NUM       64
#define MIN(x, y)           (x > y ? y : x)
#define LOGW(x, ...)    __android_log_print(ANDROID_LOG_WARN,  x ,__VA_ARGS__)
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO,  x ,__VA_ARGS__)

typedef struct {
    uint8_t buf[MAX_VIDEO_SIZE];
    size_t  size;
    int64_t presentationTime;
} VideoBuf;

typedef struct {
    uint8_t buf[MAX_AUDIO_SIZE];
    size_t  size;
    int64_t presentationTime;
} AudioBuf;

typedef struct {
    int8_t  sps[MAX_SPS_SIZE];
    int8_t  pps[MAX_PPS_SIZE];
    size_t  spsSize;
    size_t  ppsSize;
} VideoInfo;

typedef enum  {
    MEDIA_VIDEO,
    MEDIA_AUDIO
} MediaType;

class MediaBuffer {
public:
    static MediaBuffer & getInstance() {
        static MediaBuffer instance;
        return instance;
    }

public:
    void   videoWrite(uint8_t* data, size_t size, int64_t presentationTime);
    size_t videoRead(uint8_t* data, size_t maxSize, timeval* presentationTime);
    size_t rtmpVideoRead(uint8_t* &data,  uint32_t &presentationTime);
    void   audioWrite(uint8_t* data, size_t size, int64_t presentationTime);
    size_t audioRead(uint8_t* data, size_t maxSize, timeval* presentationTime);
    size_t rtmpAudioRead(uint8_t* &data, uint32_t &presentationTime);
    void   videoSetSPS(int8_t* data, size_t size);
    void   videoSetPPS(int8_t* data, size_t size);
    VideoInfo videoGetInfo();
    size_t videoGetSPS(int8_t* data);
    size_t videoGetPPS(int8_t* data);
    void   videoReset(); //for rtsp
    void   AudioReset(); //for rtsp
    void   rtmpReset();
private:
    MediaBuffer();
    MediaBuffer(MediaBuffer const&);
    void operator=(MediaBuffer const&);
    void buildAacHeader(uint8_t* header, size_t packetSize);
private:
    VideoInfo mVideoInfo;
    VideoBuf  mVideoBuffer[VIDEO_BUF_NUM];
    AudioBuf  mAudioBuffer[AUDIO_BUF_NUM];
    size_t  mVideoWriteIdx;
    size_t  mVideoReadIdx;
    size_t  mAudioWriteIdx;
    size_t  mAudioReadIdx;
    size_t  mRtmpVideoReadIdx;
    size_t  mRtmpAudioReadIdx;
    int64_t mVideoTimeBase;
    int64_t mAudioTimeBase;
};
#endif //LIVESCREEN_RINGBUFFER_H
