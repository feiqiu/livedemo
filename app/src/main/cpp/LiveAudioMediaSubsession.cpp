//
// Created by laoguo on 2016/9/3.
//

#include <MPEG4LATMAudioRTPSink.hh>
#include "LiveAudioMediaSubsession.h"
#include "MediaBuffer.h"

LiveAudioMediaSubsession * LiveAudioMediaSubsession::createNew(UsageEnvironment& env) {
    MediaBuffer::getInstance().AudioReset();
    LOGI("LiveAudioMediaSubsession", "LiveAudioMediaSubsession()");
    return new LiveAudioMediaSubsession(env);
}

LiveAudioMediaSubsession::LiveAudioMediaSubsession(UsageEnvironment& env)
        : OnDemandServerMediaSubsession(env, true), fLiveSource(NULL) {

}

LiveAudioMediaSubsession::~LiveAudioMediaSubsession() {
    LOGI("LiveAudioMediaSubsession", "~LiveAudioMediaSubsession()");
}

FramedSource* LiveAudioMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                              unsigned& estBitrate) {
    estBitrate = 128;
    fLiveSource = LiveAudioSource::createNew(envir(), 0, 0);
    return fLiveSource;
}

RTPSink* LiveAudioMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                    unsigned char rtpPayloadTypeIfDynamic,
                                                    FramedSource* inputSource) {
    uint8_t profile = 2; // AAC LC
    uint8_t freqIdx = 4; // 44.1KHz
    uint8_t chanCfg = 1; // CPE
    unsigned char audioSpecificConfig[2];
    char fConfigStr[16];
    uint8_t audioObjectType = (uint8_t)(profile + 1);
    audioSpecificConfig[0] = (audioObjectType<<3) | (freqIdx>>1);
    audioSpecificConfig[1] = (freqIdx<<7) | (chanCfg<<3);
    sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
    RTPSink*rtpSink = MPEG4LATMAudioRTPSink::createNew(envir(), rtpGroupsock,rtpPayloadTypeIfDynamic, 44100,  fConfigStr, 1, False);
    return rtpSink;
}

void LiveAudioMediaSubsession::pushFrameData(uint8_t* data, size_t size, int64_t presentationTimeUs) {
    if (fLiveSource) MediaBuffer::getInstance().audioWrite(data, size, presentationTimeUs);
}