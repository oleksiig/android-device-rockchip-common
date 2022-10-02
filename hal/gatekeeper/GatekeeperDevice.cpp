/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include <android-base/memory.h>
#include <gatekeeper/gatekeeper.h>

#include "GatekeeperDevice.h"

namespace android {
namespace hardware {
namespace gatekeeper {
namespace V1_0 {
namespace implementation {

static constexpr uint32_t SIGNATURE_LENGTH_BYTES = 32;

GatekeeperDevice::GatekeeperDevice()
{
    key_.reset(new uint8_t[SIGNATURE_LENGTH_BYTES]);
    memset(key_.get(), 0, SIGNATURE_LENGTH_BYTES);
}

bool GatekeeperDevice::GetAuthTokenKey(const uint8_t **auth_token_key, uint32_t *length) const
{
    ALOGV("%s:", __func__);

    if (nullptr == auth_token_key || nullptr == length)
        return false;

    *auth_token_key = key_.get();
    *length = SIGNATURE_LENGTH_BYTES;
    return true;
}

void GatekeeperDevice::GetPasswordKey(const uint8_t **password_key, uint32_t *length)
{
    ALOGV("%s:", __func__);

    if (nullptr == password_key || nullptr == length)
        return;

    *password_key = key_.get();
    *length = SIGNATURE_LENGTH_BYTES;
}

void GatekeeperDevice::ComputePasswordSignature(uint8_t* signature, uint32_t signature_length,
                                          const uint8_t*, uint32_t, const uint8_t* password,
                                          uint32_t password_length, ::gatekeeper::salt_t salt) const
{
    ALOGV("%s:", __func__);

    if (nullptr == signature)
        return;

    const uint64_t N = 16384;
    const uint32_t r = 8;
    const uint32_t p = 1;

    crypto_scrypt(password, password_length, reinterpret_cast<uint8_t*>(&salt), sizeof(salt), N,
                      r, p, signature, signature_length);
}

void GatekeeperDevice::GetRandom(void *random, uint32_t requested_size) const
{
    ALOGV("%s:", __func__);

    if (nullptr == random)
        return;

    RAND_pseudo_bytes((uint8_t*)random, requested_size);
}

void GatekeeperDevice::ComputeSignature(uint8_t* signature, uint32_t signature_length, const uint8_t*,
                                  uint32_t, const uint8_t*, const uint32_t) const
{
    ALOGV("%s:", __func__);

    if (nullptr == signature)
        return;

    memset(signature, 0, signature_length);
}

uint64_t GatekeeperDevice::GetMillisecondsSinceBoot() const
{
    struct timespec time;
    int res = clock_gettime(CLOCK_BOOTTIME, &time);
    if (res < 0) return 0;
    return (time.tv_sec * 1000) + (time.tv_nsec / 1000 / 1000);
}

bool GatekeeperDevice::GetFailureRecord(uint32_t uid, ::gatekeeper::secure_id_t user_id,
                                  ::gatekeeper::failure_record_t *record, bool secure)
{
    ALOGV("%s: secure=%d", __func__, secure);

    ::gatekeeper::failure_record_t* stored = &failure_map_[uid];

    if (user_id != stored->secure_user_id) {
        stored->secure_user_id = user_id;
        stored->last_checked_timestamp = 0;
        stored->failure_counter = 0;
    }

    memcpy(record, stored, sizeof(*record));

    return true;
}

bool GatekeeperDevice::ClearFailureRecord(uint32_t uid, ::gatekeeper::secure_id_t user_id, bool secure)
{
    ALOGV("%s: secure=%d", __func__, secure);

    ::gatekeeper::failure_record_t* stored = &failure_map_[uid];

    stored->secure_user_id = user_id;
    stored->last_checked_timestamp = 0;
    stored->failure_counter = 0;

    return true;
}

bool GatekeeperDevice::WriteFailureRecord(uint32_t uid, ::gatekeeper::failure_record_t *record, bool secure)
{
    ALOGV("%s: secure=%d", __func__, secure);

    failure_map_[uid] = *record;
    return true;
}

bool GatekeeperDevice::IsHardwareBacked() const
{
    return false;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace gatekeeper
}  // namespace hardware
}  // namespace android
