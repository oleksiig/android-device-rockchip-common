/*
 * Copyright (C) 2019 The Android Open Source Project
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


#ifndef ANDROID_HARDWARE_HEALTH_V2_0_HEALTH_H
#define ANDROID_HARDWARE_HEALTH_V2_0_HEALTH_H

#include <android/hardware/health/2.0/IHealth.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace health {
namespace V2_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using namespace android::hardware;

struct Health : public IHealth, hidl_death_recipient
{
    // Methods from ::android::hardware::health::V2_0::IHealth follow.
    Return<health::V2_0::Result> registerCallback(const sp<health::V2_0::IHealthInfoCallback>& callback) override;
    Return<health::V2_0::Result> unregisterCallback(const sp<health::V2_0::IHealthInfoCallback>& callback) override;
    Return<health::V2_0::Result> update() override;
    Return<void> getChargeCounter(getChargeCounter_cb _hidl_cb) override;
    Return<void> getCurrentNow(getCurrentNow_cb _hidl_cb) override;
    Return<void> getCurrentAverage(getCurrentAverage_cb _hidl_cb) override;
    Return<void> getCapacity(getCapacity_cb _hidl_cb) override;
    Return<void> getEnergyCounter(getEnergyCounter_cb _hidl_cb) override;
    Return<void> getChargeStatus(getChargeStatus_cb _hidl_cb) override;
    Return<void> getStorageInfo(getStorageInfo_cb _hidl_cb) override;
    Return<void> getDiskStats(getDiskStats_cb _hidl_cb) override;
    Return<void> getHealthInfo(getHealthInfo_cb _hidl_cb) override;

    void serviceDied(uint64_t cookie, const wp<IBase>& /* who */) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    std::vector<sp<IHealthInfoCallback>> mCallbacks;
    std::recursive_mutex mCallbacksLock;

    void get_disk_stats(std::vector<DiskStats>& vec_stats);
    bool unregisterCallbackInternal(const sp<IBase>& callback);
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace health
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_HEALTH_V2_0_HEALTH_H
