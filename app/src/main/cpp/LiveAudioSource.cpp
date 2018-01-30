//
// Created by laoguo on 2016/9/3.
//

#include "LiveAudioSource.h"
#include "MediaBuffer.h"
#include "TriggerStream.h"

LiveAudioSource * LiveAudioSource::createNew(UsageEnvironment& env,
                                             unsigned preferredFrameSize,
                                             unsigned playTimePerFrame) {
    LOGI("LiveAudioSource", "LiveAudioSource()");
    triggerStream(true);
    return new LiveAudioSource(env, preferredFrameSize, playTimePerFrame);
}

LiveAudioSource::LiveAudioSource(UsageEnvironment& env,
                                 unsigned preferredFrameSize,
                                 unsigned playTimePerFrame) :
        FramedSource(env) {

}

LiveAudioSource::~LiveAudioSource() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    LOGI("LiveAudioSource", "~LiveAudioSource()");
    triggerStream(false);
}

unsigned LiveAudioSource::maxFrameSize() const {
    return MAX_AUDIO_SIZE;
}

void LiveAudioSource::doStopGettingFrames() {
    FramedSource::doStopGettingFrames();
}

void LiveAudioSource::getNextFrameData(void * clientData) {
    ((LiveAudioSource *)clientData)->getFrameData();
}

void LiveAudioSource::getFrameData() {
    fFrameSize = MediaBuffer::getInstance().audioRead(fTo, fMaxSize, &fPresentationTime);
    if (fFrameSize > 0) {
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
            LOGW("LiveAudioSource", "fFrameSize > fMaxSize %d", fNumTruncatedBytes);
        } else {
            fNumTruncatedBytes = 0;
        }
        afterGetting(this);
    } else {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(1500, getNextFrameData, this);
    }
}

void LiveAudioSource::doGetNextFrame() {
    if (!isCurrentlyAwaitingData()) return;
    //LOGW("AudioFramedSource", "doGetNextFrame fMaxSize:%d", fMaxSize);
    getFrameData();
}
