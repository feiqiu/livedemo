//
// Created by laoguo on 2016-8-23.
//

#include "LiveVideoSource.h"
#include "MediaBuffer.h"
#include "TriggerStream.h"

LiveVideoSource * LiveVideoSource::createNew(UsageEnvironment& env,
                                             unsigned preferredFrameSize,
                                             unsigned playTimePerFrame) {
    LOGI("LiveVideoSource", "LiveVideoSource()");
    triggerStream(true);
    return new LiveVideoSource(env, preferredFrameSize, playTimePerFrame);
}

LiveVideoSource::LiveVideoSource(UsageEnvironment& env,
                                 unsigned preferredFrameSize,
                                 unsigned playTimePerFrame) :
        FramedSource(env) {
}

LiveVideoSource::~LiveVideoSource() {
    LOGI("LiveVideoSource", "~LiveVideoSource()");
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    triggerStream(false);
}

unsigned LiveVideoSource::maxFrameSize() const {
    return MAX_VIDEO_SIZE;
}

void LiveVideoSource::doStopGettingFrames() {
    FramedSource::doStopGettingFrames();
}

void LiveVideoSource::getNextFrameData(void * clientData) {
    ((LiveVideoSource *)clientData)->getFrameData();
}

void LiveVideoSource::getFrameData() {
    fFrameSize = MediaBuffer::getInstance().videoRead(fTo, fMaxSize, &fPresentationTime);
    if (fFrameSize > 0) {
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
            LOGW("LiveVideoSource", "fFrameSize > fMaxSize %d", fNumTruncatedBytes);
        } else {
            fNumTruncatedBytes = 0;
        }
        afterGetting(this);
    } else {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(2500, getNextFrameData, this);
    }
}

void LiveVideoSource::doGetNextFrame() {
    if (!isCurrentlyAwaitingData()) return;
    getFrameData();
}
