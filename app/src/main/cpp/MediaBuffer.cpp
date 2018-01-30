//
// Created by laoguo on 2016-8-24.
//

#include <string.h>
#include <sys/time.h>
#include "MediaBuffer.h"

MediaBuffer::MediaBuffer():mVideoWriteIdx(0), mVideoReadIdx(0),
                           mAudioWriteIdx(0), mAudioReadIdx(0),
                           mRtmpVideoReadIdx(0),mRtmpAudioReadIdx(0),
                           mVideoTimeBase(0), mAudioTimeBase(0) {
    mVideoInfo.spsSize = 0;
    mVideoInfo.ppsSize = 0;
};

void MediaBuffer::videoReset() {
    mVideoWriteIdx = mVideoReadIdx = 0;
}

void MediaBuffer::AudioReset() {
    mAudioWriteIdx = mAudioReadIdx = 0;
}

void MediaBuffer::rtmpReset() {
    mRtmpVideoReadIdx = mRtmpAudioReadIdx = 0;
}

void MediaBuffer::videoSetSPS(int8_t* data, size_t size) {
    if (data == NULL || size < 5) return;
    mVideoInfo.spsSize = size;
    memcpy(mVideoInfo.sps, data, size);
    LOGI("MediaBuffer", "setSPS data[4]:%02x size:%d ", data[4], size);
}

size_t MediaBuffer::videoGetSPS(int8_t* data) {
    memcpy(data, mVideoInfo.sps,  mVideoInfo.spsSize);
    return mVideoInfo.spsSize;
}

void MediaBuffer::videoSetPPS(int8_t* data, size_t size) {
    if (data == NULL || size < 5) return;
    mVideoInfo.ppsSize = size;
    memcpy(mVideoInfo.pps, data, size);
    LOGI("MediaBuffer", "setPPS data[4]:%02x size:%d ", data[4], size);
}

size_t MediaBuffer::videoGetPPS(int8_t* data) {
    memcpy(data, mVideoInfo.pps,  mVideoInfo.ppsSize);
    return mVideoInfo.ppsSize;
}

VideoInfo MediaBuffer::videoGetInfo() {
    return mVideoInfo;
}

void MediaBuffer::videoWrite(uint8_t* data, size_t size, int64_t presentationTime) {
    mVideoBuffer[mVideoWriteIdx].size = MIN(size, MAX_VIDEO_SIZE);
    memcpy(mVideoBuffer[mVideoWriteIdx].buf, data, mVideoBuffer[mVideoWriteIdx].size);
    if (mVideoTimeBase == 0) {
        struct timeval tvBase;
        gettimeofday(&tvBase, NULL);
        mVideoTimeBase = (int64_t)tvBase.tv_sec*1000000 + tvBase.tv_usec - presentationTime;
    }
    mVideoBuffer[mVideoWriteIdx].presentationTime = mVideoTimeBase + presentationTime;
    if (++mVideoWriteIdx >= VIDEO_BUF_NUM) mVideoWriteIdx = 0;
}

size_t MediaBuffer::videoRead(uint8_t* data, size_t maxSize, timeval* presentationTime) {
    if (mVideoReadIdx == mVideoWriteIdx) return 0;

    size_t bufSize = 0;
    VideoBuf *pBuf = &mVideoBuffer[mVideoReadIdx];
    presentationTime->tv_sec = (long)(pBuf->presentationTime/1000000);
    presentationTime->tv_usec= (long)(pBuf->presentationTime%1000000);

    if (mVideoInfo.spsSize > 0 && mVideoInfo.ppsSize > 0 && (pBuf->buf[4] & 0x1f)== 0x05) {
        if (mVideoInfo.spsSize+mVideoInfo.ppsSize < maxSize) {
            memcpy(data, mVideoInfo.sps, mVideoInfo.spsSize);
            memcpy(data+mVideoInfo.spsSize, mVideoInfo.pps, mVideoInfo.ppsSize);
            memcpy(data+mVideoInfo.spsSize+mVideoInfo.ppsSize, pBuf->buf,
                   MIN(pBuf->size, maxSize-mVideoInfo.spsSize-mVideoInfo.ppsSize));
//            LOGW("MediaBuffer", "copy video info to fTo");
        }
        bufSize = pBuf->size+mVideoInfo.spsSize+mVideoInfo.ppsSize;
    } else {
        memcpy(data, pBuf->buf, MIN(maxSize, pBuf->size));
        bufSize = pBuf->size;
    }
    if (++mVideoReadIdx >= VIDEO_BUF_NUM) mVideoReadIdx = 0;
    return bufSize; //获取帧大小，非拷贝大小
}

size_t MediaBuffer::rtmpVideoRead(uint8_t* &data, uint32_t &presentationTime) {
    if (mRtmpVideoReadIdx == mVideoWriteIdx) return 0;

    VideoBuf *pBuf = &mVideoBuffer[mRtmpVideoReadIdx];
    presentationTime = (uint32_t)(pBuf->presentationTime/1000);
    data = pBuf->buf;
    if (++mRtmpVideoReadIdx >= VIDEO_BUF_NUM) mRtmpVideoReadIdx = 0;
    return  pBuf->size; //获取帧大小，非拷贝大小
}

void MediaBuffer::audioWrite(uint8_t* data, size_t size, int64_t presentationTime) {
    mAudioBuffer[mAudioWriteIdx].size = MIN(size, MAX_AUDIO_SIZE);
    memcpy(mAudioBuffer[mAudioWriteIdx].buf, data, mAudioBuffer[mAudioWriteIdx].size);
    if (mAudioTimeBase == 0) {
        struct timeval tvBase;
        gettimeofday(&tvBase, NULL);
        mAudioTimeBase = (int64_t)tvBase.tv_sec*1000000 + tvBase.tv_usec - presentationTime;
    }
    mAudioBuffer[mAudioWriteIdx].presentationTime = mAudioTimeBase + presentationTime;
    if (++mAudioWriteIdx >= AUDIO_BUF_NUM) mAudioWriteIdx = 0;
}

size_t MediaBuffer::audioRead(uint8_t* data, size_t maxSize, timeval* presentationTime) {
    if (mAudioReadIdx == mAudioWriteIdx) return 0;

    size_t bufSize = 0;
    size_t header_size;
    AudioBuf *pBuf = &mAudioBuffer[mAudioReadIdx];
    presentationTime->tv_sec = (long)(pBuf->presentationTime/1000000);
    presentationTime->tv_usec= (long)(pBuf->presentationTime%1000000);

    header_size = pBuf->size/0xFF + 1;
    memset(data, 0xFF, header_size-1);
    data[header_size-1] = (uint8_t)(pBuf->size % 0xFF);
    memcpy(data+header_size, pBuf->buf, MIN(maxSize, pBuf->size));
    bufSize = pBuf->size+header_size;

    if (++mAudioReadIdx >= AUDIO_BUF_NUM) mAudioReadIdx = 0;
    return bufSize; //获取帧大小，非拷贝大小
}

size_t MediaBuffer::rtmpAudioRead(uint8_t* &data, uint32_t &presentationTime) {
    if (mRtmpAudioReadIdx == mAudioWriteIdx) return 0;

    AudioBuf *pBuf = &mAudioBuffer[mRtmpAudioReadIdx];
    presentationTime = (uint32_t)(pBuf->presentationTime/1000);
    data = pBuf->buf;
    if (++mRtmpAudioReadIdx >= AUDIO_BUF_NUM) mRtmpAudioReadIdx = 0;
    return  pBuf->size;
}


void MediaBuffer::buildAacHeader(uint8_t* header, size_t packetSize) {
    int profile = 2; // AAC LC
    int freqIdx = 4; // 44.1KHz
    int chanCfg = 2; // CPE

    header[0] = 0xFF;
    header[1] = 0xF9;
    header[2] = (uint8_t)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    header[3] = (uint8_t)(((chanCfg & 3) << 6) + (packetSize >> 11));
    header[4] = (uint8_t)((packetSize & 0x7FF) >> 3);
    header[5] = (uint8_t)(((packetSize & 7) << 5) + 0x1F);
    header[6] = 0xFC;
}
