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
#include "StreamOut.h"
#include "StreamThread.h"
#include "StreamThreadWrite.h"
#include "PrimaryDevice.h"

auto __FUNC_WITH_CLASS__ = [](auto a, auto b) {return std::string(a).append(b).c_str();};
#define __FUNC__ (__FUNC_WITH_CLASS__("StreamOut::", __FUNCTION__))

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

// Methods from ::android::hardware::audio::V4_0::IStream follow.
Return<uint64_t> StreamOut::getFrameSize()
{
    return (__builtin_popcount(mConfig.channelMask) *
            audio_bytes_per_sample(static_cast<audio_format_t>(mConfig.format)));
}

Return<uint64_t> StreamOut::getFrameCount()
{
    return mConfig.frameCount;
}

Return<uint64_t> StreamOut::getBufferSize()
{
    return this->getFrameSize() * this->getFrameCount();
}

Return<uint32_t> StreamOut::getSampleRate()
{
    return mConfig.sampleRateHz;
}

Return<void> StreamOut::getSupportedSampleRates(AudioFormat format, getSupportedSampleRates_cb _hidl_cb)
{
    if (mConfig.format == format) {
        _hidl_cb(Result::OK, {mConfig.sampleRateHz});
    } else {
        _hidl_cb(Result::OK, {});
    }
    return Void();
}

Return<Result> StreamOut::setSampleRate(uint32_t /*sampleRateHz*/)
{
    return Result::NOT_SUPPORTED;
}

Return<hidl_bitfield<AudioChannelMask>> StreamOut::getChannelMask()
{
    return mConfig.channelMask;
}

Return<void> StreamOut::getSupportedChannelMasks(AudioFormat format, getSupportedChannelMasks_cb _hidl_cb)
{
    if (mConfig.format == format) {
        _hidl_cb(Result::OK, {mConfig.channelMask});
    } else {
        _hidl_cb(Result::OK, {});
    }
    return Void();
}

Return<Result> StreamOut::setChannelMask(hidl_bitfield<AudioChannelMask> /*mask*/)
{
    return Result::NOT_SUPPORTED;
}

Return<AudioFormat> StreamOut::getFormat()
{
    return mConfig.format;
}

Return<void> StreamOut::getSupportedFormats(getSupportedFormats_cb _hidl_cb)
{
    _hidl_cb({mConfig.format});
    return Void();
}

Return<Result> StreamOut::setFormat(AudioFormat /*format*/)
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamOut::getAudioProperties(getAudioProperties_cb _hidl_cb)
{
    _hidl_cb(mConfig.sampleRateHz, mConfig.channelMask, mConfig.format);
    return Void();
}

Return<Result> StreamOut::addEffect(uint64_t effectId)
{
    ALOGD("StreamOut::%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::removeEffect(uint64_t effectId)
{
    ALOGD("StreamOut::%s", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::standby()
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

    if (mWriteThread) {
        LOG_ALWAYS_FATAL_IF(!mWriteThread->standby());
    }

    return Result::OK;
}

Return<void> StreamOut::getDevices(getDevices_cb _hidl_cb)
{
    ALOGV("%s:", __FUNC__);
    _hidl_cb(Result::OK, {mDeviceAddr});
    return Void();
}

Return<Result> StreamOut::setDevices(const hidl_vec<DeviceAddress>& /*devices*/)
{
    ALOGV("%s:", __FUNC__);
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::setHwAvSync(uint32_t /*hwAvSync*/)
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamOut::getParameters(const hidl_vec<ParameterValue>& context,
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

Return<Result> StreamOut::setParameters(const hidl_vec<ParameterValue>& context,
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

Return<Result> StreamOut::start()
{
    ALOGV("%s:", __FUNC__);
    return Result::OK;
}

Return<Result> StreamOut::stop()
{
    ALOGV("%s:", __FUNC__);
    return Result::OK;
}

Return<void> StreamOut::createMmapBuffer(int32_t /*minSizeFrames*/, createMmapBuffer_cb _hidl_cb)
{
    ALOGV("%s:", __FUNC__);
    _hidl_cb(Result::NOT_SUPPORTED, {});
    return Void();
}

Return<void> StreamOut::getMmapPosition(getMmapPosition_cb _hidl_cb)
{
    ALOGV("%s:", __FUNC__);
    _hidl_cb(Result::NOT_SUPPORTED, {});
    return Void();
}

Return<Result> StreamOut::closeEx(const bool fromDctor)
{
    if (mDevice) {
        mWriteThread.reset();
        mDevice->removeStream(this);
        return Result::OK;
    }
    else if (fromDctor)
    {
        return Result::OK;
    }
    return Result::INVALID_STATE;
}

Return<Result> StreamOut::close()
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);
    return closeEx(false);
}

// Methods from audio::V4_0::IStreamOut follow.
Return<uint32_t> StreamOut::getLatency()
{
    return this->getFrameCount() * 1000 / this->getSampleRate();
}

Return<Result> StreamOut::setVolume(float left, float right)
{
    ALOGV("%s: left=%.2f right=%.2f", __FUNC__, left, right);
    return Result::OK;
}

Return<void> StreamOut::updateSourceMetadata(const SourceMetadata& sourceMetadata)
{
    ALOGV("%s: dev:0x%x", __FUNC__, mDeviceAddr.device);

    for (int i = 0; i < (int)sourceMetadata.tracks.size(); i++)
    {
        const PlaybackTrackMetadata &track = sourceMetadata.tracks[i];
        ALOGD("%s: track[%d]: usage=%d, contentType=%d, gain=%.2f", __FUNC__, i,
            track.usage, track.contentType, track.gain);
    }

    return Void();
}

Return<void> StreamOut::prepareForWriting(uint32_t frameSize, uint32_t framesCount, prepareForWriting_cb _hidl_cb)
{
    if (mWriteThread) {  // INVALID_STATE if the method was already called.
        _hidl_cb(Result::INVALID_STATE, {}, {}, {}, {});
        return Void();
    }

    // Check frameSize and framesCount
    if (frameSize == 0 || framesCount == 0) {
        ALOGE("%s: zero frameSize (%u) or framesCount (%u)", __FUNC__, frameSize, framesCount);
        _hidl_cb(Result::INVALID_ARGUMENTS, {}, {}, {}, {});
        return Void();
    }

    if (frameSize > (INT_MAX / framesCount)) {
        ALOGE("%s: buffer too big: %u*%u bytes > MAX_BUFFER_SIZE (%u)", __FUNC__, frameSize, framesCount, INT_MAX);
        _hidl_cb(Result::INVALID_ARGUMENTS, {}, {}, {}, {});
        return Void();
    }

    auto t = std::make_unique<StreamThreadWrite>(this, frameSize * framesCount);
    if (!t->init()) {
        ALOGW("%s: failed to start writer thread", __FUNC__);
        _hidl_cb(Result::INVALID_ARGUMENTS, {}, {}, {}, {});
        return Void();
    }

    _hidl_cb(Result::OK,
        *t->mCommandMQ.getDesc(),
        *t->mDataMQ.getDesc(),
        *t->mStatusMQ.getDesc(),
        {.pid = getpid(), .tid = t->getTid().get()});

    mWriteThread = std::move(t);
    return Void();
}

Return<void> StreamOut::getRenderPosition(getRenderPosition_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, 0);
    return Void();
}

Return<void> StreamOut::getNextWriteTimestamp(getNextWriteTimestamp_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, 0);
    return Void();
}

Return<Result> StreamOut::setCallback(const sp<IStreamOutCallback>& /*callback*/)
{
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::clearCallback()
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamOut::supportsPauseAndResume(supportsPauseAndResume_cb _hidl_cb)
{
    _hidl_cb(false, false);
    return Void();
}

Return<Result> StreamOut::pause()
{
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::resume()
{
    return Result::NOT_SUPPORTED;
}

Return<bool> StreamOut::supportsDrain()
{
    return false;
}

Return<Result> StreamOut::drain(AudioDrain /*type*/)
{
    return Result::NOT_SUPPORTED;
}

Return<Result> StreamOut::flush()
{
    return Result::NOT_SUPPORTED;
}

Return<void> StreamOut::getPresentationPosition(getPresentationPosition_cb _hidl_cb)
{
    _hidl_cb(Result::NOT_SUPPORTED, {}, {});
    return Void();
}

Return<Result> StreamOut::selectPresentation(int32_t /*presentationId*/, int32_t /*programId*/)
{
    return Result::NOT_SUPPORTED;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android
