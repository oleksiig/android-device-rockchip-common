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

#define LOG_TAG "MemtrackHAL"
#include <log/log.h>

#include "Memtrack.h"

namespace android {
namespace hardware {
namespace memtrack {
namespace V1_0 {
namespace implementation {

using namespace ::android::hardware;

// Methods from ::android::hardware::memtrack::V1_0::IMemtrack follow.
Return<void> Memtrack::getMemory(int32_t pid, memtrack::V1_0::MemtrackType type, getMemory_cb _hidl_cb)
{
    hidl_vec<MemtrackRecord> records;

    switch (type) {
        case MemtrackType::OTHER:
            ALOGV("getMemory(OTHER): for pid=%d", pid);
            break;
        case MemtrackType::GL:
            ALOGV("getMemory(GL): for pid=%d", pid);
            break;
        case MemtrackType::GRAPHICS:
            ALOGV("getMemory(GRAPHICS): for pid=%d", pid);
            break;
        case MemtrackType::MULTIMEDIA:
            ALOGV("getMemory(MULTIMEDIA): for pid=%d", pid);
            break;
        case MemtrackType::CAMERA:
            ALOGV("getMemory(CAMERA): for pid=%d", pid);
            break;
    };

    _hidl_cb(MemtrackStatus::SUCCESS, records);
    return Void();
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace V1_0
}  // namespace memtrack
}  // namespace hardware
}  // namespace android
