//
// Created by laoguo on 2016-8-23.
//

#ifndef LIVESCREEN_LIVEVIDEOSOURCE_H
#define LIVESCREEN_LIVEVIDEOSOURCE_H

#include "UsageEnvironment.hh"
#include "FramedSource.hh"
#include "MediaBuffer.h"

class LiveVideoSource : public FramedSource {
public:
    static LiveVideoSource* createNew(UsageEnvironment& env,
                                      unsigned preferredFrameSize,
                                      unsigned playTimePerFrame);
    LiveVideoSource(UsageEnvironment& env, unsigned preferredFrameSize, unsigned playTimePerFrame);
    ~LiveVideoSource();

    virtual unsigned maxFrameSize() const;

protected:
    static void getNextFrameData(void * clientData);
    void getFrameData();
private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
};

#endif //LIVESCREEN_LIVEVIDEOSOURCE_H
