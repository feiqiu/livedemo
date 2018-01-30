//
// Created by laoguo on 2016/8/13.
//
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <BasicUsageEnvironment.hh>
#include <RTSPServer.hh>
#include <GroupsockHelper.hh>

#include "LiveVideoSource.h"
#include "LiveVideoMediaSubsession.h"
#include "LiveAudioMediaSubsession.h"
#include "RtmpClient.h"


#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR,  x ,__VA_ARGS__)
#define LOGI(x, ...) __android_log_print(ANDROID_LOG_INFO,  x ,__VA_ARGS__)

#define TAG     "MediaServer"

extern "C" {

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName) {
    char* url = rtspServer->rtspURL(sms);
    //bug of live555, fun ourIPAddress() return the old ip when ip was changed (guess)
    //give up return the url by live555, use android api
    //UsageEnvironment& env = rtspServer->envir();
    //AddressString(ourIPAddress(env)).val()
    //LOGE(TAG, "Play this stream <%s> using the URL: %s", streamName, url);
    delete[] url;
}

static char errMessage[256];

class MediaServer {
public:
    MediaServer():mUrl (NULL), mPort(8554), mQuit(0) {
    }
    void pushVideo(uint8_t* data, size_t size, int64_t presentationTimeUs);
    void pushAudio(uint8_t* data, size_t size, int64_t presentationTimeUs);
    jint create(uint16_t port=8554);
    void destroy();
    const char* getUrl() {return mUrl;}
    void setSPS(int8_t* data, size_t size);
    void setPPS(int8_t* data, size_t size);
private:
    static void* running(void *arg);
    char volatile mQuit;

private:
    char*       mUrl;
    uint16_t    mPort;
    pthread_t   mThreadId;
    RTSPServer* mRtspServer;
    TaskScheduler*      mScheduler;
    UsageEnvironment*   mEnv;
    ServerMediaSession* mMediaSession;
    LiveVideoMediaSubsession* mVideoSession;
    LiveAudioMediaSubsession* mAudioSession;
};

jint MediaServer::create(uint16_t port) {
    const char* descriptionString = "Session streamed by liveScreen";
    const char* streamName = "live";

    if (mQuit != 0) return -1;

    mScheduler = BasicTaskScheduler::createNew();

    UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
    // To implement client access control to the RTSP server, do the following:
    authDB = new UserAuthenticationDatabase;
    authDB->addUserRecord("username1", "password1"); // replace these with real strings
    // Repeat the above with each <username>, <password> that you wish to allow
    // access to the server.
#endif
    // Create the RTSP server:
    mEnv = BasicUsageEnvironment::createNew(*mScheduler);
    mRtspServer = RTSPServer::createNew(*mEnv, port, authDB);
    if (mRtspServer == NULL) goto err;

    OutPacketBuffer::maxSize = MAX_VIDEO_SIZE;
    mMediaSession = ServerMediaSession::createNew(*mEnv, streamName, streamName, descriptionString);
    if (mMediaSession == NULL) goto err;
    mVideoSession = LiveVideoMediaSubsession::createNew(*mEnv);
    if (mVideoSession == NULL) goto err;
    mMediaSession->addSubsession(mVideoSession);
    mAudioSession = LiveAudioMediaSubsession::createNew(*mEnv);
    if (mAudioSession == NULL) goto err;
    mMediaSession->addSubsession(mAudioSession);
    mRtspServer->addServerMediaSession(mMediaSession);
    //mUrl not use, see announceStream
    mUrl = mRtspServer->rtspURL(mMediaSession);
    announceStream(mRtspServer, mMediaSession, streamName);

    if (mRtspServer->setUpTunnelingOverHTTP(80) || mRtspServer->setUpTunnelingOverHTTP(8000)
        || mRtspServer->setUpTunnelingOverHTTP(8080)) {
        LOGI(TAG, "We use port %d for optional RTSP-over-HTTP tunneling.", mRtspServer->httpServerPortNum());
    } else {
        LOGI(TAG, "RTSP-over-HTTP tunneling is not available.");
    }
    return pthread_create(&mThreadId, NULL, running, this);
err:
    strncpy(errMessage, mEnv->getResultMsg(), MIN(sizeof(errMessage)-1, strlen(mEnv->getResultMsg())));
    LOGE(TAG, "MediaServer create failed: %s ", mEnv->getResultMsg());
    destroy();
    return -1;
}

void MediaServer::destroy() {
    mQuit = 1;
    if (mThreadId) {
        pthread_join(mThreadId, NULL);
        mThreadId = 0;
    }

    if (mRtspServer != NULL) {
        mRtspServer->closeAllClientSessionsForServerMediaSession(mMediaSession);
        mRtspServer->removeServerMediaSession(mMediaSession);
        Medium::close(mRtspServer);
        mVideoSession = NULL;
        mMediaSession = NULL;
        mRtspServer   = NULL;
    }

    if (mEnv != NULL){
        mEnv->reclaim();
        mEnv = NULL;
    }

    if (mScheduler != NULL) {
        delete mScheduler;
        mScheduler = NULL;
    }

    if (mUrl != NULL) {
        delete mUrl;
        mUrl = NULL;
    }
    ReceivingInterfaceAddr = INADDR_ANY;
    mQuit = 0;
}

void* MediaServer::running(void *arg) {
    MediaServer *server = (MediaServer *)arg;
    server->mEnv->taskScheduler().doEventLoop(&(server->mQuit));
    LOGE(TAG, "MediaServer running finished ");
}

void MediaServer::setSPS(int8_t* data, size_t size) {
    MediaBuffer::getInstance().videoSetSPS(data, size);
}

void MediaServer::setPPS(int8_t* data, size_t size) {
    MediaBuffer::getInstance().videoSetPPS(data, size);
}

void MediaServer::pushVideo(uint8_t* data, size_t size, int64_t presentationTimeUs) {
    MediaBuffer::getInstance().videoWrite(data, size, presentationTimeUs);
}

void MediaServer::pushAudio(uint8_t* data, size_t size, int64_t presentationTimeUs) {
    MediaBuffer::getInstance().audioWrite(data, size, presentationTimeUs);
}

static MediaServer mediaServer;
static RtmpClient rtmpClient;

jstring Java_org_laoguo_livescreen_MediaServer_rtspCreate(JNIEnv *env, jclass clazz, jint port) {
    int errCode = mediaServer.create((uint16_t)port);
    if (0 == errCode && mediaServer.getUrl()!= NULL) {
        return nullptr;//env->NewStringUTF(mediaServer.getUrl());
    } else {
        char errMsg[128]={0};
        snprintf(errMsg, sizeof(errMsg), "Error(%d): %s", errCode, errMessage);
        return env->NewStringUTF(errMsg);
    }
}

jstring Java_org_laoguo_livescreen_MediaServer_rtspDestroy(JNIEnv *env, jclass clazz) {
    mediaServer.destroy();
}

void Java_org_laoguo_livescreen_MediaServer_setSPS(JNIEnv* env, jclass clazz,
                                                   jbyteArray buf, jint size) {
    jboolean b = JNI_FALSE;
    jbyte *data = env->GetByteArrayElements(buf, &b);
    mediaServer.setSPS(data, (size_t) size);
    env->ReleaseByteArrayElements(buf, data, 0);
}

void Java_org_laoguo_livescreen_MediaServer_setPPS(JNIEnv* env, jclass clazz,
                                                   jbyteArray buf, jint size) {
    jboolean b = JNI_FALSE;
    jbyte *data = env->GetByteArrayElements(buf, &b);
    mediaServer.setPPS(data, (size_t) size);
    env->ReleaseByteArrayElements(buf, data, 0);
}

void Java_org_laoguo_livescreen_MediaServer_playVideoFrame(JNIEnv* env, jclass clazz,
                                                           jobject buf, jint size, jlong presentationTimeUs) {
    uint8_t* data = (uint8_t*)env->GetDirectBufferAddress(buf);
    mediaServer.pushVideo(data, (size_t) size, presentationTimeUs);
}

void Java_org_laoguo_livescreen_MediaServer_playAudioFrame(JNIEnv* env, jclass clazz,
                                                           jobject buf, jint size, jlong presentationTimeUs) {
    uint8_t* data = (uint8_t*)env->GetDirectBufferAddress(buf);
    mediaServer.pushAudio(data, (size_t) size, presentationTimeUs);
}


jint Java_org_laoguo_livescreen_MediaServer_rtmpCreate(JNIEnv *env, jclass clazz, jstring url) {
    const char *nativeUrl = env->GetStringUTFChars(url, 0);
    rtmpClient.create(nativeUrl);
    env->ReleaseStringUTFChars( url, nativeUrl);
}

jstring Java_org_laoguo_livescreen_MediaServer_rtmpDestroy(JNIEnv *env, jclass clazz) {
    rtmpClient.destroy();
}

} //end extern "C"
