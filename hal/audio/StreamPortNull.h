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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_NULL_H
#define ANDROID_HARDWARE_AUDIO_V5_0_NULL_H

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

class StreamSinkNull;
class StreamSourceNull;

/* */
class StreamNull
{
public:
    StreamNull(const AudioConfig &config, bool isOut);
    virtual ~StreamNull(void);

private:
    friend StreamSinkNull;
    friend StreamSourceNull;

    const nsecs_t   mStartNs;
    const unsigned  mSampleRateHz;
    const unsigned  mChannels;
    const bool      mIsOut;
};

/* */
class StreamSinkNull :
    public StreamPortSinkBase,
    public StreamNull
{
public:
    StreamSinkNull(const AudioConfig &config, uint64_t &frames);

    virtual Result getPresentationPosition(uint64_t &frames, TimeSpec &ts);
    virtual size_t write(const void* buffer, size_t bytesToWrite);

    uint64_t getPresentationFrames(const nsecs_t nowNs) const;
    uint64_t getAvailableFrames(const nsecs_t nowNs) const;
    uint64_t getAvailableFramesNow() const;

    static std::unique_ptr<StreamSinkNull> create(const AudioConfig &config, uint64_t &frames) {
        return std::make_unique<StreamSinkNull>(config, frames);
    }

private:
    uint64_t &mFrames;
    uint64_t mPreviousFrames = 0;
    uint64_t mReceivedFrames = 0;
    uint64_t mFrameSize = 0;
};

/* */
class StreamSourceNull :
    public StreamPortSourceBase,
    public StreamNull
{
public:
    StreamSourceNull(const AudioConfig &config, uint64_t &frames);

private:
    uint64_t &mFrames;
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_NULL_H
