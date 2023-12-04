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

#include <log/log.h>
#include <system/audio.h>

#include "StreamPortNull.h"

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

StreamNull::StreamNull(const AudioConfig &config, bool isOut)
    : mStartNs(systemTime(SYSTEM_TIME_MONOTONIC)),
      mSampleRateHz(config.sampleRateHz),
      mChannels(__builtin_popcount(config.channelMask)),
      mIsOut(isOut)
{
    ALOGV("%s", __FUNCTION__);
}

StreamNull::~StreamNull(void)
{
    ALOGV("%s", __FUNCTION__);
}

/* ------------------------------------------------------------------ */
StreamSinkNull::StreamSinkNull(const AudioConfig &config, uint64_t &frames)
    : StreamNull(config, true /*OUT*/),
      mFrames(frames)
{
    mFrameSize = __builtin_popcount(config.channelMask) *
        audio_bytes_per_sample(static_cast<audio_format_t>(config.format));
}

uint64_t StreamSinkNull::getPresentationFrames(const nsecs_t nowNs) const
{
    return uint64_t(mSampleRateHz) * ns2us(nowNs - mStartNs) / 1000000;
}

uint64_t StreamSinkNull::getAvailableFrames(const nsecs_t nowNs) const
{
    return getPresentationFrames(nowNs) - mReceivedFrames;
}

uint64_t StreamSinkNull::getAvailableFramesNow() const
{
    return getAvailableFrames(systemTime(SYSTEM_TIME_MONOTONIC));
}

size_t StreamSinkNull::write(const void* /*buffer*/, size_t bytesToWrite)
{
    size_t out_frames = bytesToWrite / mFrameSize;

    mFrames += out_frames;

    usleep((int64_t)bytesToWrite * 1000000 / mFrameSize / mSampleRateHz);

    return bytesToWrite;
}

Result StreamSinkNull::getPresentationPosition(uint64_t &frames, TimeSpec &ts)
{
    const nsecs_t nowNs = systemTime(SYSTEM_TIME_MONOTONIC);
    const uint64_t nowFrames = getPresentationFrames(nowNs);
    mFrames += (nowFrames - mPreviousFrames);
    mPreviousFrames = nowFrames;

    frames = mFrames;
    ts.tvSec = ns2s(nowNs);
    ts.tvNSec = nowNs - s2ns(ts.tvSec);
    return Result::OK;
}
/*
Result StreamSinkNull::start(void)
{
    return Result::OK;
}

Result StreamSinkNull::stop(void)
{
    return Result::OK;
}
*/
}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
