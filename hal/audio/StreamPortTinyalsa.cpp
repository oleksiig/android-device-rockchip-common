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
#include <stdlib.h>

#include "StreamPortTinyalsa.h"

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

auto __FUNC_WITH_CLASS__ = [](auto a, auto b) {return std::string(a).append(b).c_str();};
#define __FUNC__ (__FUNC_WITH_CLASS__("Tinyalsa::", __FUNCTION__))

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) ((size_t)(sizeof(a) / sizeof(*(a))))
#endif

StreamTinyalsa::StreamTinyalsa(unsigned pcmCard, unsigned pcmDevice,
                               const AudioConfig &config, bool isOut)
    : mStartNs(systemTime(SYSTEM_TIME_MONOTONIC)),
      mSampleRateHz(config.sampleRateHz),
      mFrameSize(__builtin_popcount(config.channelMask) * audio_bytes_per_sample(static_cast<audio_format_t>(config.format))),
      mWriteSizeFrames(config.frameCount),
      mIsOut(isOut),
      mStartedCount(0)
{
    struct pcm_config pcm_config;
    memset(&pcm_config, 0, sizeof(pcm_config));

    ALOGV("%s: card:%d device:%d", __FUNC__, pcmCard, pcmDevice);

    pcm_config.channels         = __builtin_popcount(config.channelMask);
    pcm_config.rate             = mSampleRateHz;
    //pcm_config.avail_min        = mWriteSizeFrames;
    pcm_config.period_size      = mWriteSizeFrames;
    pcm_config.period_count     = 4; /* 4 - 2048 */
    pcm_config.format           = PCM_FORMAT_S16_LE;
    pcm_config.start_threshold  = 0;//2;
    pcm_config.stop_threshold   = 0;//INT_MAX;//isOut ? 0 : INT_MAX;

    mPCM = ::pcm_open(pcmCard, pcmDevice, (isOut ? PCM_OUT : PCM_IN) |
        PCM_MONOTONIC | PCM_MMAP | PCM_NOIRQ, &pcm_config);

    if(!::pcm_is_ready(mPCM))
    {
        ALOGE("%s: pcm_open failed for nChannels=%u sampleRateHz=%u "
              "frameCount=%u isOut=%d, error: %s", __FUNC__,
                pcm_config.channels, pcm_config.rate,
                pcm_config.period_size, isOut, ::pcm_get_error(mPCM));
        return;
    }

    ALOGV("%s", __FUNC__);
}

StreamTinyalsa::~StreamTinyalsa(void)
{
    ::pcm_close(mPCM);

    ALOGV("%s", __FUNC__);
}

/* ------------------------------------------------------------------ */
StreamSinkTinyalsa::StreamSinkTinyalsa(unsigned pcmCard,unsigned pcmDevice,
                 const AudioConfig &config, uint64_t &frames)
    : StreamTinyalsa(pcmCard, pcmDevice, config, true /*OUT*/),
      mFrames(frames)
{

}

uint64_t StreamSinkTinyalsa::getPresentationFrames(const nsecs_t nowNs) const
{
    return uint64_t(mSampleRateHz) * ns2us(nowNs - mStartNs) / 1000000;
}

uint64_t StreamSinkTinyalsa::getAvailableFrames(const nsecs_t nowNs) const
{
    return getPresentationFrames(nowNs) - mReceivedFrames;
}

uint64_t StreamSinkTinyalsa::getAvailableFramesNow() const
{
    return getAvailableFrames(systemTime(SYSTEM_TIME_MONOTONIC));
}

size_t StreamSinkTinyalsa::write(const void* buffer, size_t bytesToWrite)
{
    size_t out_frames = bytesToWrite / mFrameSize;

    int res = ::pcm_mmap_write(mPCM, buffer, bytesToWrite);
    if (res < 0) {
        ALOGW("%s: MmapWrite failed, result %d", __FUNC__, res);
    }

    mFrames += out_frames;
    return bytesToWrite;
}

Result StreamSinkTinyalsa::getPresentationPosition(uint64_t &frames, TimeSpec &ts)
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
Result StreamSinkTinyalsa::start(void)
{
    return Result::OK;
}

Result StreamSinkTinyalsa::stop(void)
{
    return Result::OK;
}
*/

/* ------------------------------------------------------------------ */
template<typename T, int count>
class MixControl : public TinyalsaMixer::MixControlBase
{
private:
    T values[count];
public:
    MixControl(struct mixer *mix, const char *name)
        : MixControlBase(mix, name) { }
};

TinyalsaMixer::MixControlBase::MixControlBase(struct mixer *mix, const char *name)
    : mName(name),
      mCtl(::mixer_get_ctl_by_name(mix, name))

{
    if(mCtl == nullptr) {
        ALOGW("%s: Mixer control '%s' not found", __FUNC__, name);
        return;
    }
    mType = ::mixer_ctl_get_type(mCtl);
    mNumValues = ::mixer_ctl_get_num_values(mCtl);
}

bool TinyalsaMixer::MixControlBase::Set(bool value, int nIdx)
{
    if(mCtl == nullptr) {
        ALOGW("%s: Control '%s' is NULL", __FUNC__, mName.c_str());
        return false;
    }

    if(mType != MIXER_CTL_TYPE_BOOL) {
        ALOGW("%s: Control '%s' accepts only BOOL type",
            __FUNC__, mName.c_str());
        return false;
    }

    int status = ::mixer_ctl_set_value(mCtl, nIdx, (int)value);
    return (status >= 0);
}

bool TinyalsaMixer::MixControlBase::Get(bool &value, int nIdx)
{
    if(mCtl == nullptr) {
        ALOGW("%s: Control '%s' is NULL", __FUNC__, mName.c_str());
        return false;
    }

    if(mType != MIXER_CTL_TYPE_BOOL) {
        ALOGW("%s: Control '%s' returns only BOOL type",
            __FUNC__, mName.c_str());
        return false;
    }

    int boolVal = ::mixer_ctl_get_value(mCtl, nIdx);
    if(boolVal < 0 )
        return false;

    value = boolVal;
    return true;
}

bool TinyalsaMixer::MixControlBase::Set(int value, int nIdx)
{
    if(mCtl == nullptr) {
        ALOGW("%s: Control '%s' is NULL", __FUNC__, mName.c_str());
        return false;
    }

    if(mType != MIXER_CTL_TYPE_ENUM && mType != MIXER_CTL_TYPE_INT) {
        ALOGW("%s: Control '%s' accepts only INT(ENUM) type",
            __FUNC__, mName.c_str());
        return false;
    }

    int status = ::mixer_ctl_set_value(mCtl, nIdx, value);
    return (status >= 0);
}

bool TinyalsaMixer::MixControlBase::Get(int &value, int nIdx)
{
    if(mCtl == nullptr) {
        ALOGW("%s: Control '%s' is NULL", __FUNC__, mName.c_str());
        return false;
    }

    if(mType != MIXER_CTL_TYPE_ENUM && mType != MIXER_CTL_TYPE_INT) {
        ALOGW("%s: Control '%s' returns only INT(ENUM) type",
            __FUNC__, mName.c_str());
        return false;
    }

    int intVal = ::mixer_ctl_get_value(mCtl, nIdx);
    if(intVal < 0 )
        return false;

    value = intVal;
    return true;
}

/* ------------------------------------------------------------------ */
TinyalsaMixer::TinyalsaMixer(unsigned pcmCard) :
    mMixer(::mixer_open(pcmCard))
{
    if(nullptr == mMixer) {
        ALOGW("%s: No mixer for CARD %u", __FUNC__, pcmCard);
        return;
    }

    ALOGD("%s: Mixer name: '%s'\n",
        __FUNC__, ::mixer_get_name(mMixer));

//    mMasterVolume       = std::make_unique< MixControl<int, 1> >    (mMixer, "Master Volume");
//    mMasterMute         = std::make_unique< MixControl<bool, 1> >   (mMixer, "Master Mute");
}

TinyalsaMixer::~TinyalsaMixer(void)
{
    if(mMixer) ::mixer_close(mMixer);

    ALOGV("%s", __FUNC__);
}
/*
int TinyalsaMixer::getMasterVolume(void)
{
    int volume = -1;
    int min = mixer_ctl_get_range_min(mMasterVolume->getCtl());
    int max = mixer_ctl_get_range_max(mMasterVolume->getCtl());
    int range = (max - min);

    mMasterVolume->Get(volume);

    return ((volume - min) * 100) / range;
}

void TinyalsaMixer::setMasterVolume(int volume)
{
    int min = mixer_ctl_get_range_min(mMasterVolume->getCtl());
    int max = mixer_ctl_get_range_max(mMasterVolume->getCtl());
    int range = (max - min);
    int value = min + (range * volume) / 100;

 //   ALOGE("%s: raw volume %d, percents %d%%", __FUNC__, value, volume);

    mMasterVolume->Set(value);
}
*/
}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
