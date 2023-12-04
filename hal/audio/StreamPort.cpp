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

#include "StreamPort.h"
#include "StreamPortTinyalsa.h"
#include "StreamPortNull.h"

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

constexpr unsigned int kPrimaryPcmCard      = 0;
constexpr unsigned int kTelephonyPcmCard    = 1;

/* */
std::unique_ptr<StreamPortSinkBase> StreamPortSinkBase::create(const DeviceAddress &address,
        const AudioConfig &config, const hidl_bitfield<AudioOutputFlag> &, uint64_t &frames)
{
    if (config.format != AudioFormat::PCM_16_BIT) {
        ALOGE("%s: Only PCM_16_BIT is supported", __FUNCTION__);
        return nullptr;
    }

    if (address.device == AudioDevice::OUT_SPEAKER ||
        address.device == AudioDevice::OUT_BUS)
    {
        auto sinkptr = StreamSinkTinyalsa::create(kPrimaryPcmCard, 0, config, frames);
        if (sinkptr != nullptr)
            return sinkptr;
    }
    else if(address.device == AudioDevice::OUT_TELEPHONY_TX)
    {
        ALOGW("%s: OUT_TELEPHONY_TX", __FUNCTION__);
    }

    ALOGW("%s: failed to create alsa sink; created nullsink instead.", __FUNCTION__);
    return StreamSinkNull::create(config, frames);
}

std::unique_ptr<StreamPortSourceBase> StreamPortSourceBase::create(const DeviceAddress &address,
        const AudioConfig &config, const hidl_bitfield<AudioOutputFlag> &, uint64_t &frames)
{
    if (config.format != AudioFormat::PCM_16_BIT) {
        ALOGE("%s: Only PCM_16_BIT is supported", __FUNCTION__);
        return nullptr;
    }

    if (address.device == AudioDevice::IN_BUILTIN_MIC)
    {
        ALOGW("%s: IN_BUILTIN_MIC", __FUNCTION__);
    }
    else if(address.device == AudioDevice::IN_FM_TUNER)
    {
        ALOGW("%s: IN_FM_TUNER", __FUNCTION__);
    }
    else if(address.device == AudioDevice::IN_TV_TUNER)
    {
        ALOGW("%s: IN_TV_TUNER", __FUNCTION__);
    }

    return nullptr;
}

std::unique_ptr<StreamPortControlBase> StreamPortControlBase::create(const DeviceAddress &address)
{
    if (address.device == AudioDevice::OUT_SPEAKER || address.device == AudioDevice::OUT_BUS) {
        return TinyalsaMixer::create(kPrimaryPcmCard);
    }

    ALOGW("%s: TinyAlsa mixer is not supported for device 0x%08x", __FUNCTION__, address.device);
    return nullptr;
}

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
