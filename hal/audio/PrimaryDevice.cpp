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


#include "StreamThread.h"
#include "StreamIn.h"
#include "StreamOut.h"
#include "PrimaryDevice.h"

auto __FUNC_WITH_CLASS__ = [](auto a, auto b) {return std::string(a).append(b).c_str();};
#define __FUNC__ (__FUNC_WITH_CLASS__("PrimaryDevice::", __FUNCTION__))

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

constexpr float kVolumeStepCoef = (1.f / 100.f);
constexpr size_t kInBufferDurationMs = 15;
constexpr size_t kOutBufferDurationMs = 21;

const std::array<uint32_t, 8> kSupportedRatesHz = {
    8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000
};

const std::array<hidl_bitfield<AudioChannelMask>, 4> kSupportedInChannelMask = {
    AudioChannelMask::IN_LEFT           | 0,
    AudioChannelMask::IN_RIGHT          | 0,
    AudioChannelMask::IN_FRONT          | 0,
    AudioChannelMask::IN_STEREO         | 0,
};

const std::array<hidl_bitfield<AudioChannelMask>, 3> kSupportedOutChannelMask = {
    AudioChannelMask::OUT_FRONT_LEFT    | 0,
    AudioChannelMask::OUT_FRONT_RIGHT   | 0,
    AudioChannelMask::OUT_STEREO        | 0,
};

const std::array<AudioFormat, 1> kSupportedAudioFormats = {
    AudioFormat::PCM_16_BIT,
};

bool PrimaryDevice::checkSampleRateHz(uint32_t value, uint32_t &suggest)
{
    for (const uint32_t supported : kSupportedRatesHz) {
        if (value <= supported) {
            suggest = supported;
            return (value == supported);
        }
    }

    suggest = kSupportedRatesHz.back();
    return false;
}

bool PrimaryDevice::checkAudioConfig(bool isOut, size_t duration_ms,
                      const AudioConfig &proposed, AudioConfig &suggested)
{
    bool valid = checkSampleRateHz(proposed.sampleRateHz, suggested.sampleRateHz);

    if (isOut) {
        if (std::find(kSupportedOutChannelMask.begin(),
                      kSupportedOutChannelMask.end(),
                      proposed.channelMask) == kSupportedOutChannelMask.end()) {
            suggested.channelMask = AudioChannelMask::OUT_STEREO | 0;
            valid = false;
        } else {
            suggested.channelMask = proposed.channelMask;
        }
    } else {
        if (std::find(kSupportedInChannelMask.begin(),
                      kSupportedInChannelMask.end(),
                      proposed.channelMask) == kSupportedInChannelMask.end()) {
            suggested.channelMask = AudioChannelMask::IN_STEREO | 0;
            valid = false;
        } else {
            suggested.channelMask = proposed.channelMask;
        }
    }

    if (std::find(kSupportedAudioFormats.begin(),
                  kSupportedAudioFormats.end(),
                  proposed.format) == kSupportedAudioFormats.end()) {
        suggested.format = AudioFormat::PCM_16_BIT;
        valid = false;
    } else {
        suggested.format = proposed.format;
    }

    suggested.offloadInfo = proposed.offloadInfo;

    suggested.frameCount = (proposed.frameCount == 0)
        ? getBufferSizeFrames(duration_ms, suggested.sampleRateHz)
        : proposed.frameCount;

    return valid;
}

static inline size_t align(size_t v, size_t a) {
    return (v + a - 1) / a * a;
}

size_t PrimaryDevice::getBufferSizeFrames(size_t duration_ms, uint32_t sample_rate)
{
    // AudioFlinger requires the buffer to be aligned by 16 frames
    return align(sample_rate * duration_ms / 1000, 16);
}

// Methods from ::android::hardware::audio::V4_0::IDevice follow.
Return<Result> PrimaryDevice::initCheck()
{
    ALOGV("%s:", __FUNC__);
    return Result::OK;
}

Return<Result> PrimaryDevice::setMasterVolume(float volume)
{
    if (isnan(volume) || volume < 0 || volume > 1.0) {
        return Result::INVALID_ARGUMENTS;
    }

	ALOGV("%s: volume=%0.2f", __FUNC__, volume);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getMasterVolume(getMasterVolume_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, 0);
    return Void();
}

Return<Result> PrimaryDevice::setMicMute(bool mute)
{
    mMicMute = mute;

    ALOGV("%s: mute=%d", __FUNC__, mMicMute);
    return Result::OK;
}

Return<void> PrimaryDevice::getMicMute(getMicMute_cb _hidl_cb)
{
    ALOGV("%s: mute=%d", __FUNC__, mMicMute);

    _hidl_cb(Result::OK, mMicMute);
    return Void();
}

Return<Result> PrimaryDevice::setMasterMute(bool mute)
{
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getMasterMute(getMasterMute_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, 0);
    return Void();
}

Return<void> PrimaryDevice::getInputBufferSize(const AudioConfig& /*config*/,
    getInputBufferSize_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, 0);
    return Void();
}

Return<void> PrimaryDevice::openOutputStream(int32_t ioHandle,
        const DeviceAddress& address, const AudioConfig& config,
        hidl_bitfield<AudioOutputFlag> flags,
        const SourceMetadata& sourceMetadata, openOutputStream_cb _hidl_cb)
{
    AudioConfig suggestedConfig;
    if (checkAudioConfig(true, kOutBufferDurationMs, config, suggestedConfig)) {
        auto stream = std::make_unique<StreamOut>(
            this, ioHandle, address, suggestedConfig, flags, sourceMetadata);

        {
            std::lock_guard<std::mutex> guard(mMutex);
            LOG_ALWAYS_FATAL_IF(!mOutputStreams.insert(stream.get()).second);
        }

        _hidl_cb(Result::OK, stream.release(), config);
    } else {
        ALOGE("%s: failed, invalid arguments", __FUNC__);
        _hidl_cb(Result::INVALID_ARGUMENTS, nullptr, suggestedConfig);
    }

    return Void();
}

Return<void> PrimaryDevice::openInputStream(int32_t ioHandle, const DeviceAddress& device,
        const AudioConfig& config, hidl_bitfield<AudioInputFlag> flags,
        const SinkMetadata& sinkMetadata, openInputStream_cb _hidl_cb)
{
    AudioConfig suggestedConfig;
    if (checkAudioConfig(false, kInBufferDurationMs, config, suggestedConfig)) {
        auto stream = std::make_unique<StreamIn>(
            this, ioHandle, device, suggestedConfig, flags, sinkMetadata);

        {
            std::lock_guard<std::mutex> guard(mMutex);
            LOG_ALWAYS_FATAL_IF(!mInputStreams.insert(stream.get()).second);
        }

        _hidl_cb(Result::OK, stream.release(), config);
    } else {
        ALOGE("%s: failed, invalid arguments", __FUNC__);
        _hidl_cb(Result::INVALID_ARGUMENTS, nullptr, suggestedConfig);
    }

    return Void();
}

Return<bool> PrimaryDevice::supportsAudioPatches()
{
    return true;
}

Return<void> PrimaryDevice::createAudioPatch(const hidl_vec<AudioPortConfig>& sources,
        const hidl_vec<AudioPortConfig>& sinks, createAudioPatch_cb _hidl_cb)
{
    if (sources.size() == 1 && sinks.size() == 1)
    {
        AudioPortConfig source_cfg = sources[0];
        AudioPortConfig sink_cfg = sinks[0];

        AudioPatchHandle handle;
        AudioPatch patch;
        patch.source = source_cfg;
        patch.sink = sink_cfg;

        while (true) {
            handle = mNextAudioPatchHandle;
            mNextAudioPatchHandle = std::max(handle + 1, 0);
            if (mAudioPatches.insert({handle, patch}).second) {
                break;
            }
        }

        ALOGV("%s: source:%d -> sink:%d (handle %d)", __FUNC__,
            patch.source.id, patch.sink.id, handle);

        _hidl_cb(Result::OK, handle);
    } else {
        _hidl_cb(Result::NOT_SUPPORTED, 0);
    }

    return Void();
}

Return<Result> PrimaryDevice::releaseAudioPatch(int32_t patchHandle)
{
    std::unordered_map<AudioPatchHandle, AudioPatch>::const_iterator patchIter =
        mAudioPatches.find(patchHandle);

    if(patchIter != mAudioPatches.end())
    {
        const AudioPatch &patch = patchIter->second;

        ALOGV("%s: source:%d -> sink:%d", __FUNC__,
            patch.source.id, patch.sink.id);
    }

    return (mAudioPatches.erase(patchHandle) == 1) ?
        Result::OK : Result::INVALID_ARGUMENTS;
}

Return<void> PrimaryDevice::getAudioPort(const AudioPort& port, getAudioPort_cb _hidl_cb)
{
    ALOGV("%s: id=%d", __FUNC__, port.id);

    _hidl_cb(Result::NOT_SUPPORTED, port);
    return Void();
}

Return<Result> PrimaryDevice::setAudioPortConfig(const AudioPortConfig& config)
{
    if(config.configMask & AudioPortConfigMask::SAMPLE_RATE) {
        ALOGD("%s id=%d sample rate=%dHz", __FUNC__, config.id, config.sampleRateHz);
    }

    if(config.configMask & AudioPortConfigMask::CHANNEL_MASK) {
        ALOGD("%s id=%d channelMask=0x%x", __FUNC__, config.id, config.channelMask);
    }

    if(config.configMask & AudioPortConfigMask::FORMAT) {
        ALOGD("%s id=%d format=0x%x", __FUNC__, config.id, config.format);
    }

    if(config.configMask & AudioPortConfigMask::GAIN) {
        const AudioGainConfig &gain = config.gain;

        ALOGD("%s id=%d gain: index=%d mode=0x%x channelMask=0x%x values=(%d %d %d %d)", __FUNC__,
            config.id, gain.index, gain.mode, gain.channelMask,
            gain.values[0], gain.values[1], gain.values[2], gain.values[3]);
    }

    return Result::OK;
}

Return<void> PrimaryDevice::getHwAvSync(getHwAvSync_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, {});
    return Void();
}

Return<Result> PrimaryDevice::setScreenState(bool turnedOn)
{
    ALOGV("%s: turnedOn=%d", __FUNC__, turnedOn);
    return Result::OK;
}

Return<void> PrimaryDevice::getParameters(const hidl_vec<ParameterValue>& context,
        const hidl_vec<hidl_string>& keys, getParameters_cb _hidl_cb)
{
    for (int i = 0; i < (int)context.size(); i++) {
        ALOGD("%s: context[%d] key=%s val=%s", __FUNC__, i,
            context[i].key.c_str(), context[i].value.c_str());
    }

    for (int i = 0; i < (int)keys.size(); i++) {
        ALOGD("%s: keys[%d] key=%s", __FUNC__, i,
            keys[i].c_str());
    }

    ALOGD("%s is not implemented", __FUNC__);
    _hidl_cb(Result::NOT_SUPPORTED, hidl_vec<ParameterValue>{});
    return Void();
}

Return<Result> PrimaryDevice::setParameters(const hidl_vec<ParameterValue>& context,
        const hidl_vec<ParameterValue>& parameters)
{
    for (int i = 0; i < (int)context.size(); i++) {
        ALOGD("%s: context[%d] key=%s val=%s", __FUNC__, i,
            context[i].key.c_str(), context[i].value.c_str());
    }

    for (int i = 0; i < (int)parameters.size(); i++) {
        ALOGD("%s: parameter[%d] key=%s val=%s", __FUNC__, i,
            parameters[i].key.c_str(), parameters[i].value.c_str());
    }

    return Result::OK;
}

Return<void> PrimaryDevice::getMicrophones(getMicrophones_cb _hidl_cb)
{
    ALOGV("%s:", __FUNC__);
    return Void();
}

Return<Result> PrimaryDevice::setConnectedState(const DeviceAddress& address, bool connected)
{
    ALOGV("%s: connected=%d", __FUNC__, connected);
    return Result::NOT_SUPPORTED;
}

// Methods from audio::V5_0::IPrimaryDevice follow.
Return<Result> PrimaryDevice::setVoiceVolume(float volume)
{
    if (isnan(volume) || volume < 0 || volume > 1.0) {
        return Result::INVALID_ARGUMENTS;
    }

    ALOGD("%s volume=%.2f", __FUNC__, volume);
    return Result::OK;
}

Return<Result> PrimaryDevice::setMode(AudioMode mode)
{
    switch (mode) {
        case AudioMode::NORMAL:
            ALOGD("%s AudioMode::NORMAL", __FUNC__);
            break;
        case AudioMode::RINGTONE:
            ALOGD("%s AudioMode::RINGTONE", __FUNC__);
            break;
        case AudioMode::IN_CALL:
            ALOGD("%s AudioMode::IN_CALL", __FUNC__);
            break;
        case AudioMode::IN_COMMUNICATION:
            ALOGD("%s AudioMode::IN_COMMUNICATION", __FUNC__);
            break;
        default:
            ALOGD("%s INVALID_ARGUMENTS", __FUNC__);
            return Result::INVALID_ARGUMENTS;
    };

    return Result::OK;
}

Return<Result> PrimaryDevice::setBtScoHeadsetDebugName(const hidl_string& name) {
    ALOGD("%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getBtScoNrecEnabled(getBtScoNrecEnabled_cb _hidl_cb) {
    ALOGD("%s", __FUNC__);
    return Void();
}

Return<Result> PrimaryDevice::setBtScoNrecEnabled(bool enabled) {
    ALOGD("%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getBtScoWidebandEnabled(getBtScoWidebandEnabled_cb _hidl_cb) {
    ALOGD("%s", __FUNC__);
    return Void();
}

Return<Result> PrimaryDevice::setBtScoWidebandEnabled(bool enabled) {
    ALOGD("%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getBtHfpEnabled(getBtHfpEnabled_cb _hidl_cb) {
    ALOGD("%s", __FUNC__);
    return Void();
}

Return<Result> PrimaryDevice::setBtHfpEnabled(bool enabled)
{
    ALOGD("%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> PrimaryDevice::setBtHfpSampleRate(uint32_t sampleRateHz)
{
    ALOGD("%s: sampleRateHz=%d", __FUNC__, sampleRateHz);
    return Result::NOT_SUPPORTED;
}

Return<Result> PrimaryDevice::setBtHfpVolume(float volume)
{
    ALOGD("%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getTtyMode(getTtyMode_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, TtyMode::OFF);
    return Void();
}

Return<Result> PrimaryDevice::setTtyMode(IPrimaryDevice::TtyMode mode)
{
    ALOGD("%s mode=%d", __FUNC__, mode);
    return Result::NOT_SUPPORTED;
}

Return<void> PrimaryDevice::getHacEnabled(getHacEnabled_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, false);
    return Void();
}

Return<Result> PrimaryDevice::setHacEnabled(bool enabled)
{
    ALOGD("%s: enabled=%d", __FUNC__, enabled);
    return Result::NOT_SUPPORTED;
}

Return<Result> PrimaryDevice::updateRotation(IPrimaryDevice::Rotation /*rotation*/)
{
    return Result::NOT_SUPPORTED;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
