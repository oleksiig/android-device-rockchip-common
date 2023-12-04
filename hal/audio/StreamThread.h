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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_STREAM_THREAD_H
#define ANDROID_HARDWARE_AUDIO_V5_0_STREAM_THREAD_H

#include <thread>
#include <future>
#include <fmq/EventFlag.h>

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

class StreamThreadWrite;

class StreamThread
{
public:
    static constexpr uint32_t STANDBY_REQUEST = 1 << 20;
    static constexpr uint32_t EXIT_REQUEST = 1 << 21;

    virtual ~StreamThread() = default;

    virtual EventFlag *getEventFlag() = 0;

    bool notify(uint32_t mask)
    {
        EventFlag *event = getEventFlag();
        if (event) {
            return NO_ERROR == event->wake(mask);
        }
        return false;
    }

    bool standby(void)
    {
        return notify(STANDBY_REQUEST);
    }

    void requestExit(void)
    {
        notify(EXIT_REQUEST);
    }

    bool isRunning() const
    {
        return mThread.joinable();
    }

    std::future<pthread_t> getTid()
    {
        return mTid.get_future();
    }

private:
    friend StreamThreadWrite;

    std::promise<pthread_t> mTid;
    std::thread             mThread;
};

struct forEventFlag {
    typedef ::android::hardware::EventFlag EventFlag;

    void operator()(EventFlag *x) const {
        LOG_ALWAYS_FATAL_IF(EventFlag::deleteEventFlag(&x) != ::android::OK);
    };
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_STREAM_THREAD_H
