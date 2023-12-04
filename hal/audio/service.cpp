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

#include <system/audio.h>
#include <hidl/HidlTransportSupport.h>
#include <android-base/logging.h>
#include <android/hardware/audio/5.0/IDevicesFactory.h>

#include "StreamThread.h"
#include "StreamIn.h"
#include "StreamOut.h"
#include "PrimaryDevice.h"

using namespace android;
using namespace android::hardware;
using namespace android::hardware::audio;
using namespace android::hardware::audio::V5_0;

struct DevicesFactory : public IDevicesFactory
{
    // Methods from ::android::hardware::audio::V5_0::IDevicesFactory follow.
    Return<void> openDevice(const hidl_string& device, openDevice_cb _hidl_cb) override
    {
        if (device == AUDIO_HARDWARE_MODULE_ID_PRIMARY) {
            _hidl_cb(Result::OK, new implementation::PrimaryDevice());
        } else {
            ALOGE("openDevice(%s) is unsupported", device.c_str());
            _hidl_cb(Result::NOT_SUPPORTED, nullptr);
        }
        return Void();
    }

    Return<void> openPrimaryDevice(openPrimaryDevice_cb _hidl_cb) override
    {
        ALOGE("openPrimaryDevice() is not implemented");
        _hidl_cb(Result::NOT_SUPPORTED, nullptr);
        return Void();
    }
};

int main()
{
    sp<IDevicesFactory> hal = new DevicesFactory();

    configureRpcThreadpool(16, true /*callerWillJoin*/);

    const auto status = hal->registerAsService();
    CHECK_EQ(status, android::OK);

    joinRpcThreadpool();
}
