//
// Created by laoguo on 2016/9/3.
//

#ifndef LIVESCREEN_LIVEAUDIOMEDIASUBSESSION_H
#define LIVESCREEN_LIVEAUDIOMEDIASUBSESSION_H

#include <OnDemandServerMediaSubsession.hh>
#include "LiveAudioSource.h"

class LiveAudioMediaSubsession: public OnDemandServerMediaSubsession {
public:
    static LiveAudioMediaSubsession* createNew(UsageEnvironment& env);
    void pushFrameData(uint8_t* data, size_t size, int64_t presentationTimeUs);
protected:
    LiveAudioMediaSubsession(UsageEnvironment& env);
    ~LiveAudioMediaSubsession();
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
private:
    LiveAudioSource* fLiveSource;
};

#endif //LIVESCREEN_LIVEAUDIOMEDIASUBSESSION_H
