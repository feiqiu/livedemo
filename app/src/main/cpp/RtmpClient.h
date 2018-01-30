//
// Created by Huixia on 2016/10/7.
//

#ifndef LIVESCREEN_RTMPCLIENT_H
#define LIVESCREEN_RTMPCLIENT_H
#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <unistd.h>
#include <pthread.h>
#include "rtmp.h"
#include "MediaBuffer.h"

#define MAX_URL_SIZE    250

class RtmpClient {
public:
    RtmpClient();
    ~RtmpClient();
    jint create(const char *url);
    void destroy();
    jint connect();
    jint setupUrl(const char *url);
    jint sendAVCSequenceHeader(uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len);
    jint sendAACSequenceHeader();
    jint sendH264Packet(uint8_t *data, size_t len, uint32_t timestamp);
    jint sendAACPacket(uint8_t *data, size_t len, uint32_t timestamp);
    void removeNALStart(uint8_t *&nal, size_t &len);

private:
    pthread_t   mSendVideoThreadId;
    pthread_t   mSendAudioThreadId;
    uint32_t    mVideoTimestampStart;
    uint32_t    mAudioTimestampStart;
    static int  procDisconnect(RtmpClient *client);
    static void* sendingVideo(void *arg);
    static void* sendingAudio(void *arg);
    static void  sleepMs(int millis);
    char volatile mQuit;
    bool volatile isConnected;
    pthread_cond_t  mCond;
    pthread_mutex_t mMutex;
    pthread_mutex_t mCondMutex;
    char mUrl[MAX_URL_SIZE];
    RTMP *mRtmp;
};
#endif //LIVESCREEN_RTMPCLIENT_H
