//
// Created by laoguo on 2016-8-23.
//

#include <H264VideoStreamDiscreteFramer.hh>
#include <H264VideoRTPSink.hh>
#include "LiveVideoMediaSubsession.h"
#include "TriggerStream.h"

LiveVideoMediaSubsession * LiveVideoMediaSubsession::createNew(UsageEnvironment& env) {
    MediaBuffer::getInstance().videoReset();
    return new LiveVideoMediaSubsession(env);
}

LiveVideoMediaSubsession::LiveVideoMediaSubsession(UsageEnvironment& env)
    : OnDemandServerMediaSubsession(env, true), fLiveSource(NULL) {
    LOGW("LiveVideoMediaSubsession", "LiveVideoMediaSubsession()");
}

LiveVideoMediaSubsession::~LiveVideoMediaSubsession() {
    LOGW("LiveVideoMediaSubsession", "~LiveVideoMediaSubsession()");
}

FramedSource* LiveVideoMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                              unsigned& estBitrate) {
    estBitrate = 4000;
    LOGW("LiveVideoMediaSubsession", "createNewStreamSource()");
    fLiveSource = LiveVideoSource::createNew(envir(), 0, 0);
    return H264VideoStreamDiscreteFramer::createNew(envir(), fLiveSource);
}

RTPSink* LiveVideoMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                    unsigned char rtpPayloadTypeIfDynamic,
                                                    FramedSource* inputSource) {
    LOGW("LiveVideoMediaSubsession", "createNewRTPSink()");
    RTPSink*rtpSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    return rtpSink;
}

void LiveVideoMediaSubsession::pushFrameData(uint8_t* data, size_t size, int64_t presentationTimeUs) {
    if (fLiveSource) MediaBuffer::getInstance().videoWrite(data, size, presentationTimeUs);
}

