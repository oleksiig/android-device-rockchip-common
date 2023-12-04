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
#include "StreamIn.h"
#include "StreamThread.h"
//#include "StreamThreadRead.h"
#include "PrimaryDevice.h"

auto __FUNC_WITH_CLASS__ = [](auto a, auto b) {return std::string(a).append(b).c_str();};
#define __FUNC__ (__FUNC_WITH_CLASS__("StreamIn::", __FUNCTION__))

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

// Methods from ::android::hardware::audio::V4_0::IStream follow.
Return<uint64_t> StreamIn::getFrameSize()
{
    return (__builtin_popcount(mConfig.channelMask) *
            audio_bytes_per_sample(static_cast<audio_format_t>(mConfig.format)));
}

Return<uint64_t> StreamIn::getFrameCount()
{
    return mConfig.frameCount;
}

Return<uint64_t> StreamIn::getBufferSize()
{
    return this->getFrameSize() * this->getFrameCount();
}

Return<uint32_t> StreamIn::getSampleRate()
{
    return mConfig.sampleRateHz;
}

Return<void> StreamIn::getSupportedSampleRates(AudioFormat format, getSupportedSampleRates_cb _hidl_cb)
{
    if (mConfig.format == format) {
        _hidl_cb(Result::OK, {mConfig.sampleRateHz});
    } else {
        _hidl_cb(Result::OK, {});
    }
    return Void();
}

Return<Result> StreamIn::setSampleRate(uint32_t /*sampleRateHz*/)
{
    return Result::NOT_SUPPORTED;
}

Return<hidl_bitfield<AudioChannelMask>> StreamIn::getChannelMask()
{
    return mConfig.channelMask;
}

Return<void> StreamIn::getSupportedChannelMasks(AudioFormat format, getSupportedChannelMasks_cb _hidl_cb)
{
    if (mConfig.format == format) {
        _hidl_cb(Result::OK, {mConfig.channelMask});
    } else {
        _hidl_cb(Result::OK, {});
    }
    return Void();
}

Return<Result> StreamIn::setChannelMask(hidl_bitfield<AudioChannelMask> /*mask*/)
{
    return Result::NOT_SUPPORTED;
}

Return<AudioFormat> StreamIn::getFormat()
{
    return mConfig.format;
}

Return<void> StreamIn::getSupportedFormats(getSupportedFormats_cb _hidl_cb)
{
    _hidl_cb({mConfig.format});
    return Void();
}

Return<Result> StreamIn::setFormat(AudioFormat format)
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamIn::getAudioProperties(getAudioProperties_cb _hidl_cb)
{
    _hidl_cb(mConfig.sampleRateHz, mConfig.channelMask, mConfig.format);
    return Void();
}

Return<Result> StreamIn::addEffect(uint64_t effectId)
{
    ALOGV("%s:", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::removeEffect(uint64_t effectId)
{
    ALOGV("%s:", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::standby()
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

  //  if (mWriteThread) {
  //      LOG_ALWAYS_FATAL_IF(!mWriteThread->standby());
  //  }
    return Result::OK;
}

Return<void> StreamIn::getDevices(getDevices_cb _hidl_cb)
{
    _hidl_cb(Result::OK, {mDeviceAddr});
    return Void();
}

Return<Result> StreamIn::setDevices(const hidl_vec<DeviceAddress>& /*devices*/)
{
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::setHwAvSync(uint32_t /*hwAvSync*/)
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamIn::getParameters(const hidl_vec<ParameterValue>& context,
                                     const hidl_vec<hidl_string>& keys,
                                     getParameters_cb _hidl_cb)
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

    for (int i = 0; i < (int)context.size(); i++) {
        ALOGD("%s: context[%d] key=%s val=%s", __FUNC__, i,
            context[i].key.c_str(), context[i].value.c_str());
    }

    for (int i = 0; i < (int)keys.size(); i++) {
        ALOGD("%s: keys[%d] key=%s", __FUNC__, i,
            keys[i].c_str());
    }

    _hidl_cb(Result::NOT_SUPPORTED, hidl_vec<ParameterValue>{});
    return Void();
}

Return<Result> StreamIn::setParameters(const hidl_vec<ParameterValue>& context,
                                       const hidl_vec<ParameterValue>& parameters)
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

    for (int i = 0; i < (int)context.size(); i++) {
        ALOGD("%s: context[%d] key=%s val=%s", __FUNC__, i,
            context[i].key.c_str(), context[i].value.c_str());
    }

    for (int i = 0; i < (int)parameters.size(); i++) {
        ALOGD("%s: parameter[%d] key=%s val=%s", __FUNC__, i,
            parameters[i].key.c_str(), parameters[i].value.c_str());
    }

    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::start()
{
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::stop()
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamIn::createMmapBuffer(int32_t /*minSizeFrames*/, createMmapBuffer_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, {});
    return Void();
}

Return<void> StreamIn::getMmapPosition(getMmapPosition_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, {});
    return Void();
}

Return<Result> StreamIn::closeEx(const bool fromDctor)
{
    if (mDevice) {
        //mWriteThread.reset();
         mDevice->removeStream(this);
         return Result::OK;
    }
    else if (fromDctor)
    {
        return Result::OK;
    }
    return Result::INVALID_STATE;
}

Return<Result> StreamIn::close()
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);
    return closeEx(false);
}

// Methods from audio::V4_0::IStreamIn follow.
Return<void> StreamIn::getAudioSource(getAudioSource_cb _hidl_cb) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Void();
}

Return<Result> StreamIn::setGain(float gain) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<void> StreamIn::updateSinkMetadata(const SinkMetadata& sinkMetadata)
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

    for (int i = 0; i < (int)sinkMetadata.tracks.size(); i++)
    {
        const RecordTrackMetadata &track = sinkMetadata.tracks[i];
        ALOGD("%s: track[%d]: source=%d, gain=%.2f", __FUNC__, i,
            track.source, track.gain);
    }
    return Void();
}

Return<void> StreamIn::prepareForReading(uint32_t frameSize, uint32_t framesCount, prepareForReading_cb _hidl_cb) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Void();
}

Return<uint32_t> StreamIn::getInputFramesLost()
{
    return 0;
}

Return<void> StreamIn::getCapturePosition(getCapturePosition_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, {}, {});
    return Void();
}

Return<void> StreamIn::getActiveMicrophones(getActiveMicrophones_cb _hidl_cb) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Void();
}

Return<void> StreamIn::setMicMute(bool mute)
{
    ALOGV("%s: dev:0x%x: mute=%d", __FUNC__, mDeviceAddr.device, mute);
    return Void();
}

Return<Result> StreamIn::setMicrophoneDirection(MicrophoneDirection direction) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamIn::setMicrophoneFieldDimension(float zoom) {
    ALOGD("StreamIn::%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
