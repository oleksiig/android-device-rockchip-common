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

#pragma once

extern "C" {
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <crypto_scrypt.h>
}

#include <memory>
#include <unordered_map>

#include <gatekeeper/gatekeeper.h>

namespace android {
namespace hardware {
namespace gatekeeper {
namespace V1_0 {
namespace implementation {

typedef
struct fast_hash_s
{
    uint64_t salt;
    uint8_t digest[SHA256_DIGEST_LENGTH];
} fast_hash_t;

typedef std::unordered_map<uint32_t, ::gatekeeper::failure_record_t> FailureRecordMap;
typedef std::unordered_map<uint64_t, fast_hash_t> FastHashMap;

class GatekeeperDevice : public ::gatekeeper::GateKeeper {
public:
    GatekeeperDevice();
    ~GatekeeperDevice() = default;


    virtual bool GetAuthTokenKey(const uint8_t **auth_token_key, uint32_t *length) const;
    virtual void GetPasswordKey(const uint8_t **password_key, uint32_t *length);
    virtual void ComputePasswordSignature(uint8_t* signature, uint32_t signature_length,
                                          const uint8_t*, uint32_t, const uint8_t* password,
                                          uint32_t password_length, ::gatekeeper::salt_t salt) const;
    virtual void GetRandom(void *random, uint32_t requested_size) const;
    virtual void ComputeSignature(uint8_t* signature, uint32_t signature_length, const uint8_t*,
                                  uint32_t, const uint8_t*, const uint32_t) const;
    virtual uint64_t GetMillisecondsSinceBoot() const;
    virtual bool GetFailureRecord(uint32_t uid, ::gatekeeper::secure_id_t user_id,
                                  ::gatekeeper::failure_record_t *record, bool secure);
    virtual bool ClearFailureRecord(uint32_t uid, ::gatekeeper::secure_id_t user_id, bool secure);
    virtual bool WriteFailureRecord(uint32_t uid, ::gatekeeper::failure_record_t *record, bool secure);
    virtual bool IsHardwareBacked() const;

private:
    std::unique_ptr<uint8_t[]> key_;
    FailureRecordMap failure_map_;
    FastHashMap fast_hash_map_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace gatekeeper
}  // namespace hardware
}  // namespace android
