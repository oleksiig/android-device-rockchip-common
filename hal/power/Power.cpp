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

#define LOG_TAG "PowerHAL"
#include <log/log.h>
#include <stdlib.h>

#include "Power.h"

namespace android {
namespace hardware {
namespace power {
namespace V1_0 {
namespace implementation {

Power::Power(void)
{

}

// Methods from ::android::hardware::power::V1_0::IPower follow.
Return<void> Power::setInteractive(bool interactive)
{
    ALOGD("%s: interactive=%d", __func__, interactive);
    return Void();
}

Return<void> Power::powerHint(power::V1_0::PowerHint hint, int32_t data)
{
    switch(hint) {
        case PowerHint::INTERACTION:
            ALOGD("%s: INTERACTION 0x%08x", __func__, data);
            break;
        case PowerHint::LOW_POWER:
            ALOGD("%s: LOW_POWER 0x%08x", __func__, data);
            break;
        case PowerHint::SUSTAINED_PERFORMANCE:
            ALOGD("%s: SUSTAINED_PERFORMANCE 0x%08x", __func__, data);
            break;
        case PowerHint::LAUNCH:
            ALOGD("%s: LAUNCH 0x%08x", __func__, data);
            break;

        case PowerHint::VSYNC:
        case PowerHint::VR_MODE:
        case PowerHint::VIDEO_ENCODE: /* Obsolete and will never used */
        case PowerHint::VIDEO_DECODE: /* Obsolete and will never used */
            break;
    }
    return Void();
}

Return<void> Power::setFeature(power::V1_0::Feature feature, bool /*activate*/)
{
    if (feature == Feature::POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        ALOGW("Double tap to wake is not supported");
    } else {
        ALOGW("Can't set, feature %d doesn't exist", feature);
    }

    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb)
{
    hidl_vec<PowerStatePlatformSleepState> stats;

    ALOGD("%s", __func__);

    _hidl_cb(stats, Status::SUCCESS);
    return Void();
}

void Power::SysfsWrite(const char *path, const char *val)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        ALOGE("%s: Error opening %s: %s", __func__, path, strerror(errno));
        return;
    }

    int len = write(fd, val, strlen(val));
    if (len < 0) {
        ALOGE("%s: Write error %s: %s", __func__, path, strerror(errno));
    }

    close(fd);
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace V1_0
}  // namespace power
}  // namespace hardware
}  // namespace android
