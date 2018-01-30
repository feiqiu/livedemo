//
// Created by laoguo on 2016-8-23.
//

#ifndef LIVESCREEN_LIVEVIDEOMEDIASUBSESSION_H
#define LIVESCREEN_LIVEVIDEOMEDIASUBSESSION_H

#include <OnDemandServerMediaSubsession.hh>
#include <StreamReplicator.hh>
#include "LiveVideoSource.h"

class LiveVideoMediaSubsession: public OnDemandServerMediaSubsession {
public:
    static LiveVideoMediaSubsession* createNew(UsageEnvironment& env);
    void pushFrameData(uint8_t* data, size_t size, int64_t presentationTimeUs);
protected:
    LiveVideoMediaSubsession(UsageEnvironment& env);
    ~LiveVideoMediaSubsession();
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
private:
    LiveVideoSource* fLiveSource;
};

#endif //LIVESCREEN_LIVEVIDEOMEDIASUBSESSION_H
