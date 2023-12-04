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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_STREAMOUT_H
#define ANDROID_HARDWARE_AUDIO_V5_0_STREAMOUT_H

#include <android/hardware/audio/5.0/IStreamOut.h>

#include "StreamPort.h"

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

class PrimaryDevice;
class StreamThread;

using namespace ::android;
using namespace ::android::hardware;
using namespace ::android::hardware::audio;
using namespace ::android::hardware::audio::V5_0;
using namespace ::android::hardware::audio::common::V5_0;

class StreamOut : public IStreamOut
{
public:
    StreamOut(sp<PrimaryDevice> device, int32_t ioHandle,
              const DeviceAddress& deviceAddr, const AudioConfig& config,
              hidl_bitfield<AudioOutputFlag> flags, const SourceMetadata& sourceMetadata)
        : mDevice(std::move(device)),
          mIoHandle(ioHandle),
          mDeviceAddr(deviceAddr),
          mConfig(config),
          mFlags(flags),
          mMetadata(sourceMetadata)
    {
        mSink = StreamPortSinkBase::create(mDeviceAddr, mConfig, mFlags, mWrittenFrames);
        LOG_ALWAYS_FATAL_IF(!mSink);
        mMixControl = StreamPortControlBase::create(mDeviceAddr);
    }

    ~StreamOut(void) {
        this->closeEx(true);
        mSink.reset();
        mMixControl.reset();
    }

    // Methods from ::android::hardware::audio::V5_0::IStream follow.
    Return<uint64_t> getFrameSize() override;
    Return<uint64_t> getFrameCount() override;
    Return<uint64_t> getBufferSize() override;
    Return<uint32_t> getSampleRate() override;
    Return<void> getSupportedSampleRates(AudioFormat format, getSupportedSampleRates_cb _hidl_cb) override;
    Return<Result> setSampleRate(uint32_t sampleRateHz) override;
    Return<hidl_bitfield<AudioChannelMask>> getChannelMask() override;
    Return<void> getSupportedChannelMasks(AudioFormat format, getSupportedChannelMasks_cb _hidl_cb) override;
    Return<Result> setChannelMask(hidl_bitfield<AudioChannelMask> mask) override;
    Return<AudioFormat> getFormat() override;
    Return<void> getSupportedFormats(getSupportedFormats_cb _hidl_cb) override;
    Return<Result> setFormat(AudioFormat format) override;
    Return<void> getAudioProperties(getAudioProperties_cb _hidl_cb) override;
    Return<Result> addEffect(uint64_t effectId) override;
    Return<Result> removeEffect(uint64_t effectId) override;
    Return<Result> standby() override;
    Return<void> getDevices(getDevices_cb _hidl_cb) override;
    Return<Result> setDevices(const hidl_vec<DeviceAddress>& devices) override;
    Return<Result> setHwAvSync(uint32_t hwAvSync) override;
    Return<void> getParameters(const hidl_vec<ParameterValue>& context, const hidl_vec<hidl_string>& keys, getParameters_cb _hidl_cb) override;
    Return<Result> setParameters(const hidl_vec<ParameterValue>& context, const hidl_vec<ParameterValue>& parameters) override;
    Return<Result> start() override;
    Return<Result> stop() override;
    Return<void> createMmapBuffer(int32_t minSizeFrames, createMmapBuffer_cb _hidl_cb) override;
    Return<void> getMmapPosition(getMmapPosition_cb _hidl_cb) override;
    Return<Result> close() override;
    Return<Result> closeEx(const bool fromDctor);

    // Methods from ::android::hardware::audio::V4_0::IStreamOut follow.
    Return<uint32_t> getLatency() override;
    Return<Result> setVolume(float left, float right) override;
    Return<void> updateSourceMetadata(const SourceMetadata& sourceMetadata) override;
    Return<void> prepareForWriting(uint32_t frameSize, uint32_t framesCount, prepareForWriting_cb _hidl_cb) override;
    Return<void> getRenderPosition(getRenderPosition_cb _hidl_cb) override;
    Return<void> getNextWriteTimestamp(getNextWriteTimestamp_cb _hidl_cb) override;
    Return<Result> setCallback(const sp<IStreamOutCallback>& callback) override;
    Return<Result> clearCallback() override;
    Return<void> supportsPauseAndResume(supportsPauseAndResume_cb _hidl_cb) override;
    Return<Result> pause() override;
    Return<Result> resume() override;
    Return<bool> supportsDrain() override;
    Return<Result> drain(AudioDrain type) override;
    Return<Result> flush() override;
    Return<void> getPresentationPosition(getPresentationPosition_cb _hidl_cb) override;
    Return<Result> selectPresentation(int32_t presentationId, int32_t programId) override;

    /* */
    Return<Result> getPresentationPositionImpl(uint64_t* frames, TimeSpec* timeStamp);
    uint64_t &getFrameCounter() { return mWrittenFrames; }

    /* */
    const DeviceAddress &getDeviceAddress() const { return mDeviceAddr; }
    const AudioConfig &getAudioConfig() const { return mConfig; }
    const hidl_bitfield<AudioOutputFlag> &getAudioOutputFlags() const { return mFlags; }

    /* */
    const std::unique_ptr<StreamPortSinkBase>& getSink(void) { return mSink; };
    const std::unique_ptr<StreamPortControlBase>& getMixer(void) { return mMixControl; }

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    const sp<PrimaryDevice>                 mDevice;
    const int32_t                           mIoHandle;
    const DeviceAddress                     mDeviceAddr;
    const AudioConfig                       mConfig;
    const hidl_bitfield<AudioOutputFlag>    mFlags;
    const SourceMetadata&                   mMetadata;

    std::unique_ptr<StreamPortSinkBase>     mSink;
    std::unique_ptr<StreamPortControlBase>  mMixControl;
    std::unique_ptr<StreamThread>           mWriteThread;
    size_t                                  mWrittenFrames = 0;
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_STREAMOUT_H
