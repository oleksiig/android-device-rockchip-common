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

#define LOG_TAG "HealthHAL"
#include <log/log.h>

#include <android-base/file.h>
#include <android-base/strings.h>

#include <hidl/HidlTransportSupport.h>

#include "Health.h"

namespace android {
namespace hardware {
namespace health {
namespace V2_0 {
namespace implementation {

// Methods from ::android::hardware::health::V2_0::IHealth follow.
Return<health::V2_0::Result> Health::registerCallback(const sp<health::V2_0::IHealthInfoCallback>& callback)
{
    if (callback == nullptr) {
        return Result::SUCCESS;
    }

    {
        std::lock_guard<decltype(mCallbacksLock)> lock(mCallbacksLock);
        mCallbacks.push_back(callback);
        // unlock
    }

    auto linkRet = callback->linkToDeath(this, 0u /* cookie */);
    if (!linkRet.withDefault(false)) {
        ALOGW( "%s: Cannot link to death: %s", __func__,
            (linkRet.isOk() ? "linkToDeath returns false" : linkRet.description().c_str()));
        // ignore the error
    }

    return Result::SUCCESS;
}

Return<health::V2_0::Result> Health::unregisterCallback(const sp<health::V2_0::IHealthInfoCallback>& callback)
{
    return unregisterCallbackInternal(callback) ? Result::SUCCESS : Result::NOT_FOUND;
}

bool Health::unregisterCallbackInternal(const sp<IBase>& callback)
{
    if (callback == nullptr)
        return false;

    bool removed = false;
    std::lock_guard<decltype(mCallbacksLock)> _lock(mCallbacksLock);
    for (auto it = mCallbacks.begin(); it != mCallbacks.end();) {
        if (interfacesEqual(*it, callback)) {
            it = mCallbacks.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    (void)callback->unlinkToDeath(this).isOk();  // ignore errors
    return removed;
}

void Health::serviceDied(uint64_t /* cookie */, const wp<IBase>& who)
{
    (void)unregisterCallbackInternal(who.promote());
}

Return<health::V2_0::Result> Health::update()
{
    V2_0::HealthInfo healthInfo = {};

    healthInfo.legacy.chargerAcOnline = true;
    healthInfo.legacy.batteryPresent = false;
    healthInfo.legacy.batteryLevel = 0;
    healthInfo.legacy.batteryStatus = V1_0::BatteryStatus::UNKNOWN;
    healthInfo.legacy.batteryHealth = V1_0::BatteryHealth::UNKNOWN;

    for (auto it = mCallbacks.begin(); it != mCallbacks.end();) {
        (*it++)->healthInfoChanged(healthInfo);
    }

    return Result::SUCCESS;
}

Return<void> Health::getChargeCounter(getChargeCounter_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, 0);
    return Void();
}

Return<void> Health::getCurrentNow(getCurrentNow_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, 0);
    return Void();
}

Return<void> Health::getCurrentAverage(getCurrentAverage_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, 0);
    return Void();
}

Return<void> Health::getCapacity(getCapacity_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, 0);
    return Void();
}

Return<void> Health::getEnergyCounter(getEnergyCounter_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, 0);
    return Void();
}

Return<void> Health::getChargeStatus(getChargeStatus_cb _hidl_cb)
{
    ALOGD("%s", __func__);
    _hidl_cb(Result::SUCCESS, V1_0::BatteryStatus::UNKNOWN);
    return Void();
}

Return<void> Health::getStorageInfo(getStorageInfo_cb _hidl_cb)
{
    hidl_vec<struct StorageInfo> info;

    ALOGD("%s", __func__);
    _hidl_cb(Result::NOT_SUPPORTED, info);
    return Void();
}

Return<void> Health::getDiskStats(getDiskStats_cb _hidl_cb)
{
    std::vector<struct DiskStats> stats;
    get_disk_stats(stats);

    hidl_vec<struct DiskStats> stats_vec(stats);
    if (!stats.size()) {
        _hidl_cb(Result::NOT_SUPPORTED, stats_vec);
    } else {
        _hidl_cb(Result::SUCCESS, stats_vec);
    }
    return Void();
}

Return<void> Health::getHealthInfo(getHealthInfo_cb _hidl_cb)
{
    V2_0::HealthInfo healthInfo = {};

    ALOGD("%s", __func__);
    _hidl_cb(Result::NOT_SUPPORTED, healthInfo);
    return Void();
}

void Health::get_disk_stats(std::vector<DiskStats>& vec_stats)
{
    const size_t kDiskStatsSize = 11;
    const std::string kDiskStatsPath = "/sys/block/mmcblk1/stat";
    struct DiskStats stats = {};

    stats.attr.isInternal = true;
    stats.attr.isBootDevice = true;
    stats.attr.name = std::string("uSD");

    std::string buffer;
    if (!::android::base::ReadFileToString(kDiskStatsPath, &buffer)) {
        ALOGE("ReadFileToString(%s) failed", kDiskStatsPath.c_str());
        return;
    }

    // Regular diskstats entries
    std::stringstream ss(buffer);
    for (uint i = 0; i < kDiskStatsSize; ++i) {
        ss >> *(reinterpret_cast<uint64_t*>(&stats) + i);
    }

    vec_stats.resize(1);
    vec_stats[0] = stats;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace V2_0
}  // namespace health
}  // namespace hardware
}  // namespace android
