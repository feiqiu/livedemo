//
// Created by laoguo on 2016/9/3.
//

#ifndef LIVESCREEN_LIVEAUDIOSOURCE_H
#define LIVESCREEN_LIVEAUDIOSOURCE_H


#include <FramedSource.hh>
#include <live555/groupsock/include/Groupsock.hh>

class LiveAudioSource : public FramedSource {
public:
    static LiveAudioSource *createNew(UsageEnvironment &env,
                                      unsigned preferredFrameSize,
                                      unsigned playTimePerFrame);

    LiveAudioSource(UsageEnvironment &env, unsigned preferredFrameSize, unsigned playTimePerFrame);
    ~LiveAudioSource();
    virtual unsigned maxFrameSize() const;

protected:
    static void getNextFrameData(void *clientData);
    void getFrameData();

private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
};
    
#endif //LIVESCREEN_LIVEAUDIOSOURCE_H
