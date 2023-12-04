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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_PRIMARYDEVICE_H
#define ANDROID_HARDWARE_AUDIO_V5_0_PRIMARYDEVICE_H

#include <android/hardware/audio/5.0/IPrimaryDevice.h>
#include <android/hardware/audio/common/5.0/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

using namespace ::android;
using namespace ::android::hardware;
using namespace ::android::hardware::audio;
using namespace ::android::hardware::audio::V5_0;
using namespace ::android::hardware::audio::common::V5_0;

class StreamIn;
class StreamOut;

class PrimaryDevice : public IPrimaryDevice
{
public:
    ~PrimaryDevice() = default;

    // Methods from audio::V5_0::IDevice follow.
    Return<Result> initCheck() override;
    Return<Result> setMasterVolume(float volume) override;
    Return<void> getMasterVolume(getMasterVolume_cb _hidl_cb) override;
    Return<Result> setMicMute(bool mute) override;
    Return<void> getMicMute(getMicMute_cb _hidl_cb) override;
    Return<Result> setMasterMute(bool mute) override;
    Return<void> getMasterMute(getMasterMute_cb _hidl_cb) override;
    Return<void> getInputBufferSize(const AudioConfig& config, getInputBufferSize_cb _hidl_cb) override;

    Return<void> openOutputStream(int32_t ioHandle,
        const DeviceAddress& device, const AudioConfig& config,
        hidl_bitfield<AudioOutputFlag> flags, const SourceMetadata& sourceMetadata, openOutputStream_cb _hidl_cb) override;

    Return<void> openInputStream(int32_t ioHandle, const DeviceAddress& device, const AudioConfig& config,
        hidl_bitfield<AudioInputFlag> flags, const SinkMetadata& sinkMetadata, openInputStream_cb _hidl_cb) override;

    Return<bool> supportsAudioPatches() override;
    Return<void> createAudioPatch(const hidl_vec<AudioPortConfig>& sources,
                                  const hidl_vec<AudioPortConfig>& sinks,
                                  createAudioPatch_cb _hidl_cb) override;
    Return<Result> releaseAudioPatch(int32_t patch) override;

    Return<void> getAudioPort(const AudioPort& port, getAudioPort_cb _hidl_cb) override;
    Return<Result> setAudioPortConfig(const AudioPortConfig& config) override;
    Return<void> getHwAvSync(getHwAvSync_cb _hidl_cb) override;
    Return<Result> setScreenState(bool turnedOn) override;
    Return<void> getParameters(const hidl_vec<ParameterValue>& context, const hidl_vec<hidl_string>& keys, getParameters_cb _hidl_cb) override;
    Return<Result> setParameters(const hidl_vec<ParameterValue>& context, const hidl_vec<ParameterValue>& parameters) override;
    Return<void> getMicrophones(getMicrophones_cb _hidl_cb) override;
    Return<Result> setConnectedState(const DeviceAddress& address, bool connected) override;

    // Methods from audio::V4_0::IPrimaryDevice follow.
    Return<Result> setVoiceVolume(float volume) override;
    Return<Result> setMode(AudioMode mode) override;
    Return<Result> setBtScoHeadsetDebugName(const hidl_string& name) override;
    Return<void> getBtScoNrecEnabled(getBtScoNrecEnabled_cb _hidl_cb) override;
    Return<Result> setBtScoNrecEnabled(bool enabled) override;
    Return<void> getBtScoWidebandEnabled(getBtScoWidebandEnabled_cb _hidl_cb) override;
    Return<Result> setBtScoWidebandEnabled(bool enabled) override;
    Return<void> getBtHfpEnabled(getBtHfpEnabled_cb _hidl_cb) override;
    Return<Result> setBtHfpEnabled(bool enabled) override;
    Return<Result> setBtHfpSampleRate(uint32_t sampleRateHz) override;
    Return<Result> setBtHfpVolume(float volume) override;
    Return<void> getTtyMode(getTtyMode_cb _hidl_cb) override;
    Return<Result> setTtyMode(TtyMode mode) override;
    Return<void> getHacEnabled(getHacEnabled_cb _hidl_cb) override;
    Return<Result> setHacEnabled(bool enabled) override;
    Return<Result> updateRotation(IPrimaryDevice::Rotation rotation) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    friend StreamIn;
    friend StreamOut;

    size_t getBufferSizeFrames(size_t duration_ms, uint32_t sample_rate);
    bool checkSampleRateHz(uint32_t value, uint32_t &suggest);
    bool checkAudioConfig(bool isOut, size_t duration_ms,
        const AudioConfig &proposed, AudioConfig &suggested);

    /* */
    bool mMicMute = true;

    /* Patches */
    struct AudioPatch {
        AudioPortConfig source;
        AudioPortConfig sink;
    };

    AudioPatchHandle mNextAudioPatchHandle = 0;

    std::mutex mMutex;
    std::unordered_map<AudioPatchHandle, AudioPatch>    mAudioPatches;
    std::unordered_set<StreamIn *>                      mInputStreams;
    std::unordered_set<StreamOut *>                     mOutputStreams;

public:
    void removeStream(StreamIn *stream) {
        std::lock_guard<std::mutex> guard(mMutex);
        LOG_ALWAYS_FATAL_IF(mInputStreams.erase(stream) < 1);
    }

    void removeStream(StreamOut *stream) {
        std::lock_guard<std::mutex> guard(mMutex);
        LOG_ALWAYS_FATAL_IF(mOutputStreams.erase(stream) < 1);
    }
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_PRIMARYDEVICE_H
