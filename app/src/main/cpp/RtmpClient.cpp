//
// Created by laoguo on 2016/10/5.
//


#include <sys/socket.h>
#include "stdlib.h"
#include "RtmpClient.h"
#include "TriggerStream.h"

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR,  x ,__VA_ARGS__)
#define LOGI(x, ...) __android_log_print(ANDROID_LOG_INFO,  x ,__VA_ARGS__)
#define TAG     "RtmpClient"

extern "C" {

RtmpClient::RtmpClient():mSendAudioThreadId(0),mSendVideoThreadId(0),isConnected(false) {
    mRtmp = RTMP_Alloc();
    pthread_mutex_init(&mMutex, NULL);
    pthread_mutex_init(&mCondMutex, NULL);
    pthread_cond_init(&mCond, NULL);
}

RtmpClient::~RtmpClient() {
    if (mRtmp) RTMP_Free(mRtmp);
    pthread_mutex_destroy(&mMutex);
    pthread_mutex_init(&mCondMutex, NULL);
    pthread_cond_destroy(&mCond);
}

jint RtmpClient::setupUrl(const char *url) {
    if (url != NULL) {
        strncpy(mUrl, url, MAX_URL_SIZE-1);
    }
}

    struct timeval tv;
void RtmpClient::sleepMs(int millis) {
    tv.tv_sec  = millis/1000;;
    tv.tv_usec = (millis%1000)*1000;
    select(0, NULL, NULL, NULL, &tv);
//    <= millis
//    int err;
//    do{
//        err=select(0, NULL, NULL, NULL, &tv);
//    } while(err<0 && errno==EINTR);
}

jint RtmpClient::connect() {
    MediaBuffer::getInstance().rtmpReset();

    RTMP_Init(mRtmp);

    if (!RTMP_SetupURL(mRtmp, mUrl)) {
        LOGE(TAG, "RTMP_SetupURL() failed!");
        goto err;
    }

    RTMP_EnableWrite(mRtmp);

    mRtmp->Link.timeout = 5;

    LOGE(TAG, "RtmpClient RTMP_Connect %s (timeout ：%d)", mUrl, mRtmp->Link.timeout);

    if (!RTMP_Connect(mRtmp, NULL)) {
        LOGE(TAG, "RTMP_Connect() failed!");
        goto err;
    }

    if (!RTMP_ConnectStream(mRtmp, 0)) {
        LOGE(TAG, "RTMP_ConnectStream() failed!");
        RTMP_Close(mRtmp);
        goto err;
    }
    LOGE(TAG, "RtmpClient connected %s", mUrl);
    isConnected = true;
    return 0;
err:
    return -1;
}

void RtmpClient::removeNALStart(uint8_t *&nal, size_t &len) {
    if(nal[2] == 0x00) {
        nal += 4;
        len -= 4;
    } else if (nal[2] == 0x01) {
        nal += 3;
        len -= 3;
    }
}

jint RtmpClient::sendAVCSequenceHeader(uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len) {
    RTMPPacket * packet;
    unsigned char * body;
    int i;
    LOGE(TAG, "sendAVCSequenceHeader");
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
    memset(packet, 0, RTMP_HEAD_SIZE);

    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    body = (unsigned char *)packet->m_body;

    removeNALStart(sps, sps_len);
    removeNALStart(pps, pps_len);

    i = 0;
    body[i++] = 0x17;
    body[i++] = 0x00;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;

    /*sps*/
    body[i++] = 0xe1;
    body[i++] = (unsigned char)((sps_len >> 8) & 0xff);
    body[i++] = (unsigned char)(sps_len & 0xff);
    memcpy(&body[i], sps, sps_len);
    i +=  sps_len;

    /*pps*/
    body[i++]   = 0x01;
    body[i++] = (unsigned char)((pps_len >> 8) & 0xff);
    body[i++] = (unsigned char)((pps_len) & 0xff);
    memcpy(&body[i], pps, pps_len);
    i +=  pps_len;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = i;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = mRtmp->m_stream_id;

    mVideoTimestampStart = 0;
    /*调用发送接口*/
    pthread_mutex_lock(&mMutex);
    RTMP_SendPacket(mRtmp, packet, TRUE);
    pthread_mutex_unlock(&mMutex);
    free(packet);
}

jint RtmpClient::sendH264Packet(uint8_t *data, size_t len, uint32_t timestamp) {
    RTMPPacket packet;

    if (mVideoTimestampStart == 0) {
        mVideoTimestampStart = timestamp;
    }
    removeNALStart(data, len);

    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, len+9);
    if ((data[0] & 0x1f) == 5) {
        packet.m_body[0] = 0x17;
    } else {
        packet.m_body[0] = 0x27;
    }

    packet.m_body[1] = 0x01;   /*nal unit*/
    packet.m_body[2] = 0x00;
    packet.m_body[3] = 0x00;
    packet.m_body[4] = 0x00;

    packet.m_body[5] = (uint8_t)((len >> 24) & 0xff);
    packet.m_body[6] = (uint8_t)((len >> 16) & 0xff);
    packet.m_body[7] = (uint8_t)((len >>  8) & 0xff);
    packet.m_body[8] = (uint8_t)((len ) & 0xff);
    memcpy(&packet.m_body[9], data, len);

    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nInfoField2 = mRtmp->m_stream_id;
    packet.m_nChannel = 0x04;
    packet.m_nBodySize = len+9;
    packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet.m_nTimeStamp = timestamp - mVideoTimestampStart;

    pthread_mutex_lock(&mMutex);
    RTMP_SendPacket(mRtmp, &packet, TRUE);
    pthread_mutex_unlock(&mMutex);
    RTMPPacket_Free(&packet);
}

void getAACSpecific(uint8_t *spec_info, uint8_t *spec_config) {
    uint16_t audioConfig = 0;

    spec_info[0] = 0xAE; //STEREO 0xAF
    spec_info[1] = 0x00;

    audioConfig |= ((2<<11) & 0xF800); //aac lc
    audioConfig |= ((4<<7) & 0x0780); //44k
    audioConfig |= ((1<<3) & 0x078); //1 channel
    audioConfig |= (0&0x07);

    spec_config[0] = (uint8_t)((audioConfig>>8)&0xff);
    spec_config[1] = (uint8_t)(audioConfig&0xff);

    LOGE(TAG, "getAACSpecific %02X %02X %02X %02X ", spec_info[0], spec_info[1], spec_config[0], spec_config[1]);
}

jint RtmpClient::sendAACSequenceHeader() {
    RTMPPacket * packet;
    unsigned char * body;

    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+16);
    memset(packet, 0, RTMP_HEAD_SIZE);

    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    body = (unsigned char *)packet->m_body;

    getAACSpecific(body, &body[2]);

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = 4;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = mRtmp->m_stream_id;

    mAudioTimestampStart = 0;
    /*调用发送接口*/
    pthread_mutex_lock(&mMutex);
    RTMP_SendPacket(mRtmp, packet, TRUE);
    pthread_mutex_unlock(&mMutex);
    free(packet);
}

jint RtmpClient::sendAACPacket(uint8_t *data, size_t len, uint32_t timestamp) {
    RTMPPacket packet;
    int ret = -1;
    if (mAudioTimestampStart == 0) {
        mAudioTimestampStart = timestamp;
    }

    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, len+2);

    packet.m_body[0] = (char)0xAF;
    packet.m_body[1] = 0x01;

    memcpy(&packet.m_body[2], data, len);

    packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet.m_nInfoField2 = mRtmp->m_stream_id;
    packet.m_nChannel = 0x04;
    packet.m_nBodySize = len+2;
    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_nTimeStamp = timestamp - mAudioTimestampStart;

    pthread_mutex_lock(&mMutex);
    ret = RTMP_SendPacket(mRtmp, &packet, TRUE);
    pthread_mutex_unlock(&mMutex);
    RTMPPacket_Free(&packet);

    return ret;
}

void* RtmpClient::sendingVideo(void *arg) {
    RtmpClient *client = (RtmpClient *)arg;

    VideoInfo vInfo;
    pthread_mutex_lock(&client->mCondMutex);
    pthread_cond_wait(&client->mCond, &client->mCondMutex);
    pthread_mutex_unlock(&client->mCondMutex);

    if (!RTMP_IsConnected(client->mRtmp)) return NULL;

    do {
        vInfo = MediaBuffer::getInstance().videoGetInfo();
        if (vInfo.spsSize > 0 && vInfo.ppsSize > 0) {
            client->sendAVCSequenceHeader((uint8_t *)vInfo.sps, vInfo.spsSize, (uint8_t *)vInfo.pps, vInfo.ppsSize);
            break;
        }
        sleepMs(10);
    } while (!client->mQuit);

    while (!client->mQuit) {
        uint8_t* data;
        uint32_t presentationTime;
        size_t len = MediaBuffer::getInstance().rtmpVideoRead(data, presentationTime);
        if (len > 0) {
            client->sendH264Packet(data, len, presentationTime);
        }
        sleepMs(5);
    }
}

int RtmpClient::procDisconnect(RtmpClient *client) {
    if (!RTMP_IsConnected(client->mRtmp)) {
        RTMP_Close(client->mRtmp);
        client->isConnected = false;
        LOGE(TAG, "procDisconnect connecting");
        client->connect();
        if (!RTMP_IsConnected(client->mRtmp)) {
            return -1;
        } else {
            LOGE(TAG, "procDisconnect connect ok");
            client->sendAACSequenceHeader();
            VideoInfo vInfo = MediaBuffer::getInstance().videoGetInfo();
            if (vInfo.spsSize > 0 && vInfo.ppsSize > 0) {
                client->sendAVCSequenceHeader((uint8_t *)vInfo.sps, vInfo.spsSize, (uint8_t *)vInfo.pps, vInfo.ppsSize);
            }
        }
    }
    return 0;
}

void* RtmpClient::sendingAudio(void *arg) {
    RtmpClient *client = (RtmpClient *)arg;

//    while (!client->mQuit) {
//        client->connect();
//        if (!RTMP_IsConnected(client->mRtmp)) {
//            sleepMs(1000);
//        } else {
//            break;
//        }
//    }
    do {
        client->connect();
        if (!RTMP_IsConnected(client->mRtmp)) {
            sleepMs(1000);
        } else {
            pthread_cond_signal(&client->mCond);
            triggerStream(true);
            client->sendAACSequenceHeader();
            break;
        }
    } while (!client->mQuit);

    while (!client->mQuit) {
        uint8_t* data;
        uint32_t presentationTime;
        if (0 != procDisconnect(client)) {
            sleepMs(1000);
            continue;
        };
        size_t len = MediaBuffer::getInstance().rtmpAudioRead(data, presentationTime);
        if (len > 0) {
            client->sendAACPacket(data, len, presentationTime);
        }
        sleepMs(2);
    }
}

jint RtmpClient::create(const char *url) {
    int ret = 0;
    setupUrl(url);
    mQuit = 0;

    ret |= pthread_create(&mSendAudioThreadId, NULL, sendingAudio, this);
    ret |= pthread_create(&mSendVideoThreadId, NULL, sendingVideo, this);

    return ret;
}

void RtmpClient::destroy() {
    mQuit = 1;
    pthread_cond_signal(&mCond);
    if (mSendAudioThreadId) {
        pthread_join(mSendAudioThreadId, NULL);
        mSendAudioThreadId = 0;
    }
    if (mSendVideoThreadId) {
        pthread_join(mSendVideoThreadId, NULL);
        mSendVideoThreadId = 0;
    }
    RTMP_Close(mRtmp);
    isConnected = false;
    LOGE(TAG, "RtmpClient destroy");
}

}