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

#include "GatekeeperDevice.h"
#include "Gatekeeper.h"

namespace android {
namespace hardware {
namespace gatekeeper {
namespace V1_0 {
namespace implementation {

inline ::gatekeeper::SizedBuffer hidl_vec2sized_buffer(const hidl_vec<uint8_t>& vec)
{
    if (vec.size() == 0 || vec.size() > std::numeric_limits<uint32_t>::max()) return {};
    auto dummy = new uint8_t[vec.size()];
    std::copy(vec.begin(), vec.end(), dummy);
    return {dummy, static_cast<uint32_t>(vec.size())};
}

// Methods from ::android::hardware::gatekeeper::V1_0::IGatekeeper follow.
Return<void> Gatekeeper::enroll(uint32_t uid __attribute__((unused)),
        const hidl_vec<uint8_t>& currentPasswordHandle,
        const hidl_vec<uint8_t>& currentPassword,
        const hidl_vec<uint8_t>& desiredPassword,
        enroll_cb _hidl_cb)
{
    if (desiredPassword.size() == 0) {
        _hidl_cb({GatekeeperStatusCode::ERROR_GENERAL_FAILURE, 0, {}});
        return Void();
    }

    ::gatekeeper::EnrollRequest request(uid, hidl_vec2sized_buffer(currentPasswordHandle),
                          hidl_vec2sized_buffer(desiredPassword),
                          hidl_vec2sized_buffer(currentPassword));
    ::gatekeeper::EnrollResponse response;
    impl_->Enroll(request, &response);

    if (response.error == ::gatekeeper::ERROR_RETRY) {
        _hidl_cb({GatekeeperStatusCode::ERROR_RETRY_TIMEOUT, response.retry_timeout, {}});
    } else if (response.error != ::gatekeeper::ERROR_NONE) {
        _hidl_cb({GatekeeperStatusCode::ERROR_GENERAL_FAILURE, 0, {}});
    } else {
        hidl_vec<uint8_t> new_handle(response.enrolled_password_handle.Data<uint8_t>(),
                                     response.enrolled_password_handle.Data<uint8_t>() +
                                             response.enrolled_password_handle.size());
        _hidl_cb({GatekeeperStatusCode::STATUS_OK, response.retry_timeout, new_handle});
    }

    return Void();
}

Return<void> Gatekeeper::verify(uint32_t uid __attribute__((unused)),
                                uint64_t challenge __attribute__((unused)),
                                const hidl_vec<uint8_t>& enrolledPasswordHandle,
                                const hidl_vec<uint8_t>& providedPassword,
                                verify_cb _hidl_cb)
{
    if (enrolledPasswordHandle.size() == 0) {
        _hidl_cb({GatekeeperStatusCode::ERROR_GENERAL_FAILURE, 0, {}});
        return Void();
    }

    ::gatekeeper::VerifyRequest request(uid, challenge, hidl_vec2sized_buffer(enrolledPasswordHandle),
                          hidl_vec2sized_buffer(providedPassword));
    ::gatekeeper::VerifyResponse response;
    impl_->Verify(request, &response);

    if (response.error == ::gatekeeper::ERROR_RETRY) {
        _hidl_cb({GatekeeperStatusCode::ERROR_RETRY_TIMEOUT, response.retry_timeout, {}});
    } else if (response.error != ::gatekeeper::ERROR_NONE) {
        _hidl_cb({GatekeeperStatusCode::ERROR_GENERAL_FAILURE, 0, {}});
    } else {
        hidl_vec<uint8_t> auth_token(
                response.auth_token.Data<uint8_t>(),
                response.auth_token.Data<uint8_t>() + response.auth_token.size());

        _hidl_cb({response.request_reenroll ? GatekeeperStatusCode::STATUS_REENROLL
                                            : GatekeeperStatusCode::STATUS_OK,
                  response.retry_timeout, auth_token});
    }
    return Void();
}

Return<void> Gatekeeper::deleteUser(uint32_t uid __attribute__((unused)), deleteUser_cb _hidl_cb)
{
    _hidl_cb({GatekeeperStatusCode::ERROR_NOT_IMPLEMENTED, 0, {}});
    return Void();
}

Return<void> Gatekeeper::deleteAllUsers(deleteAllUsers_cb _hidl_cb)
{
    _hidl_cb({GatekeeperStatusCode::ERROR_NOT_IMPLEMENTED, 0, {}});
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace gatekeeper
}  // namespace hardware
}  // namespace android
