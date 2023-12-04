/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_TINYALSA_H
#define ANDROID_HARDWARE_AUDIO_V5_0_TINYALSA_H

#include <memory>
#include <tinyalsa/asoundlib.h>
#include <utils/Timers.h>

#include "StreamPort.h"

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

using namespace ::android;
using namespace ::android::hardware;
using namespace ::android::hardware::audio;
using namespace ::android::hardware::audio::V5_0;
using namespace ::android::hardware::audio::common::V5_0;

/* */
class TinyalsaMixControl;
class StreamSinkTinyalsa;
class StreamSourceTinyalsa;

/* */
class StreamTinyalsa
{
public:
    StreamTinyalsa(unsigned pcmCard, unsigned pcmDevice,
                 const AudioConfig &config, bool isOut);
    virtual ~StreamTinyalsa(void);

private:
    friend StreamSinkTinyalsa;
    friend StreamSourceTinyalsa;

    const nsecs_t   mStartNs;
    const unsigned  mSampleRateHz;
    const unsigned  mFrameSize;
    const unsigned  mWriteSizeFrames;
    const bool      mIsOut;

    struct pcm * mPCM = nullptr;

    /* For propper start/stop handling */
    std::atomic<uint32_t> mStartedCount;
};

/* */
class StreamSinkTinyalsa :
    public StreamPortSinkBase,
    public StreamTinyalsa
{
public:
    StreamSinkTinyalsa(unsigned pcmCard, unsigned pcmDevice,
                 const AudioConfig &config, uint64_t &frames);

    virtual Result getPresentationPosition(uint64_t &frames, TimeSpec &ts) override;
    virtual size_t write(const void* buffer, size_t bytesToWrite) override;

    uint64_t getPresentationFrames(const nsecs_t nowNs) const;
    uint64_t getAvailableFrames(const nsecs_t nowNs) const;
    uint64_t getAvailableFramesNow() const;

    static std::unique_ptr<StreamSinkTinyalsa> create(unsigned pcmCard, unsigned pcmDevice,
                 const AudioConfig &config, uint64_t &frames)
    {
        return std::make_unique<StreamSinkTinyalsa>(pcmCard, pcmDevice, config, frames);
    }

private:
    int MmapWrite(const void *buffer, unsigned int bytes);

private:
    uint64_t &mFrames;
    uint64_t mPreviousFrames = 0;
    uint64_t mReceivedFrames = 0;
};

/* */
class StreamSourceTinyalsa :
    public StreamPortSourceBase,
    public StreamTinyalsa
{
public:
    StreamSourceTinyalsa(unsigned pcmCard, unsigned pcmDevice,
                   const AudioConfig &config, uint64_t &frames);

private:
    uint64_t &mFrames;
};

class TinyalsaMixer :
    public StreamPortControlBase
{
public:
    class MixControlBase {
    public:
        const std::string mName;
        struct mixer_ctl * const mCtl;
        int mType, mNumValues;
    public:
        virtual ~MixControlBase() = default;
        MixControlBase(struct mixer *mix, const char *name);
        bool Set(bool value, int nIdx = 0);
        bool Set(int value, int nIdx = 0);
        bool Get(bool &value, int nIdx = 0);
        bool Get(int &value, int nIdx = 0);

        struct mixer_ctl * getCtl(void) const {
            return mCtl;
        }
    };

//    std::unique_ptr<MixControlBase> mMasterVolume;
//    std::unique_ptr<MixControlBase> mMasterMute;

public:
    friend StreamPortControlBase;

    TinyalsaMixer(unsigned pcmCard);
    ~TinyalsaMixer(void);

    static std::unique_ptr<TinyalsaMixer> create(unsigned pcmCard) {
        return std::make_unique<TinyalsaMixer>(pcmCard);
    }

private:
    struct mixer * mMixer = nullptr;
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_PRIMARYDEVICE_H
