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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_STREAM_PORT_H
#define ANDROID_HARDWARE_AUDIO_V5_0_STREAM_PORT_H

#include <android/hardware/audio/5.0/IStream.h>

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

using namespace ::android;
using namespace ::android::hardware;
using namespace ::android::hardware::audio;
using namespace ::android::hardware::audio::V5_0;
using namespace ::android::hardware::audio::common::V5_0;

class StreamPortSinkBase
{
public:
    virtual ~StreamPortSinkBase() {}
    virtual Result getPresentationPosition(uint64_t &frames, TimeSpec &ts) = 0;
    virtual size_t write(const void* buffer, size_t bytesToWrite) = 0;

    static std::unique_ptr<StreamPortSinkBase> create(const DeviceAddress &address,
        const AudioConfig &config, const hidl_bitfield<AudioOutputFlag> &, uint64_t &frames);
};

class StreamPortSourceBase
{
public:
    virtual ~StreamPortSourceBase() {}
    virtual Result getCapturePosition(uint64_t &frames, uint64_t &time) = 0;
    virtual size_t read(void* buffer, size_t bytesToRead) = 0;

    static std::unique_ptr<StreamPortSourceBase> create(const DeviceAddress &address,
        const AudioConfig &config, const hidl_bitfield<AudioOutputFlag> &, uint64_t &frames);
};

class StreamPortControlBase
{
public:
    virtual ~StreamPortControlBase() {}

    /* */
    static std::unique_ptr<StreamPortControlBase> create(const DeviceAddress &address);
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_STREAM_PORT_H
