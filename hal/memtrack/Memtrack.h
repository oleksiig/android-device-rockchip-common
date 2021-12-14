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

#ifndef ANDROID_HARDWARE_MEMTRACK_V1_0_MEMTRACK_H
#define ANDROID_HARDWARE_MEMTRACK_V1_0_MEMTRACK_H

#include <android/hardware/memtrack/1.0/IMemtrack.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace memtrack {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using namespace ::android::hardware;

struct Memtrack : public IMemtrack {
    // Methods from ::android::hardware::memtrack::V1_0::IMemtrack follow.
    Return<void> getMemory(int32_t pid, memtrack::V1_0::MemtrackType type, getMemory_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace memtrack
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEMTRACK_V1_0_MEMTRACK_H
