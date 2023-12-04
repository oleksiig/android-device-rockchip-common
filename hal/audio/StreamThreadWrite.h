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

#ifndef ANDROID_HARDWARE_AUDIO_V5_0_STREAM_WRITE_THREAD_H
#define ANDROID_HARDWARE_AUDIO_V5_0_STREAM_WRITE_THREAD_H

#include <hidl/MQDescriptor.h>
#include <fmq/MessageQueue.h>
#include <utils/ThreadDefs.h>
#include <cutils/sched_policy.h>
#include <sys/resource.h>

namespace android {
namespace hardware {
namespace audio {
namespace implementation {

using namespace ::android;
using namespace ::android::hardware;
using namespace ::android::hardware::audio;
using namespace ::android::hardware::audio::V5_0;
using namespace ::android::hardware::audio::common::V5_0;

class StreamThreadWrite : public StreamThread
{
    typedef MessageQueue<IStreamOut::WriteCommand, kSynchronizedReadWrite> CommandMQ;
    typedef MessageQueue<IStreamOut::WriteStatus, kSynchronizedReadWrite> StatusMQ;
    typedef MessageQueue<uint8_t, kSynchronizedReadWrite> DataMQ;

public:
    // StreamThread's lifespan never exceeds StreamOut's lifespan.
    StreamThreadWrite(StreamOut *stream, const size_t mqBufferSize)
        : StreamThread(),
          mSink(stream->getSink()),
          mBuffer(nullptr),
          mStream(stream),
          mCommandMQ(1),
          mStatusMQ(1),
          mDataMQ(mqBufferSize, true /* EventFlag */) { }

    virtual ~StreamThreadWrite(void)
    {
        if (mThread.joinable()) {
            requestExit();
            mThread.join();
        }
    }

    EventFlag *getEventFlag() override
    {
        return mEfGroup.get();
    }

    bool init(void)
    {
        if (!mCommandMQ.isValid()) {
            ALOGE("StreamThreadWrite::%s: mCommandMQ is invalid", __func__);
            return false;
        }
        if (!mDataMQ.isValid()) {
            ALOGE("StreamThreadWrite::%s: mDataMQ is invalid", __func__);
            return false;
        }
        if (!mStatusMQ.isValid()) {
            ALOGE("StreamThreadWrite::%s: mStatusMQ is invalid", __func__);
            return false;
        }

        status_t status;

        EventFlag* rawEfGroup = nullptr;
        status = EventFlag::createEventFlag(mDataMQ.getEventFlagWord(), &rawEfGroup);
        if (status != OK || !rawEfGroup) {
            ALOGE("StreamThreadWrite::%s rawEfGroup is invalid", __func__);
            return false;
        } else {
            mEfGroup.reset(rawEfGroup);
        }

        mBuffer.reset(new (std::nothrow) uint8_t[mDataMQ.getQuantumCount()]);
        if (mBuffer == nullptr) {
            ALOGE("StreamThreadWrite::%s allocate buffer failed", __func__);
            return false;
        }

        mThread = std::thread(&StreamThreadWrite::threadLoop, this);
        return StreamThread::isRunning();
    }

private:
    friend StreamOut;

    const std::unique_ptr<StreamPortSinkBase>&  mSink;
    std::unique_ptr<uint8_t[]>                  mBuffer;
    std::unique_ptr<EventFlag, forEventFlag>    mEfGroup;

    StreamOut *const        mStream;
    CommandMQ               mCommandMQ;
    StatusMQ                mStatusMQ;
    DataMQ                  mDataMQ;
    IStreamOut::WriteStatus mStatus;

    void threadLoop(void)
    {
        setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_AUDIO);
        set_sched_policy(0, SP_FOREGROUND);

        mTid.set_value(pthread_self());

        while(true)
        {
            uint32_t efState = 0;
            mEfGroup->wait(MessageQueueFlagBits::NOT_EMPTY | STANDBY_REQUEST | EXIT_REQUEST, &efState);

            if (efState & EXIT_REQUEST) {
                return;
            }

            if (!(efState & (MessageQueueFlagBits::NOT_EMPTY | 0))) {
                continue;  // Nothing to do, continue loop
            }

            if (!mCommandMQ.read(&mStatus.replyTo)) {
                continue;  // Nothing to do, continue loop
            }

            switch (mStatus.replyTo)
            {
                case IStreamOut::WriteCommand::WRITE:
                    doWrite();
                    break;
                case IStreamOut::WriteCommand::GET_PRESENTATION_POSITION:
                    doGetPresentationPosition();
                    break;
                case IStreamOut::WriteCommand::GET_LATENCY:
                    doGetLatency();
                    break;
                default:
                    ALOGE("StreamThreadWrite::Unknown write thread command code %d", mStatus.replyTo);
                    mStatus.retval = Result::NOT_SUPPORTED;
                    break;
            }

            if (!mStatusMQ.write(&mStatus)) {
                ALOGE("StreamThreadWrite::status message queue write failed");
            }

            mEfGroup->wake(static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL));
        }
	}

    void processCommand()
    {

    }

    void doGetLatency()
    {
		mStatus.retval = Result::OK;
		mStatus.reply.latencyMs = mStream->getLatency();
	}

    void doGetPresentationPosition(void)
    {
		mStatus.retval =
			mSink->getPresentationPosition(mStatus.reply.presentationPosition.frames,
                                           mStatus.reply.presentationPosition.timeStamp);
	}

    void doWrite(void)
    {
		const size_t availToRead = mDataMQ.availableToRead();
		mStatus.retval = Result::OK;
		mStatus.reply.written = 0;

		if (mDataMQ.read(&mBuffer[0], availToRead)) {
			ssize_t writeResult = mSink->write(&mBuffer[0], availToRead);
			if (writeResult >= 0) {
				mStatus.reply.written = writeResult;
			} else {
				mStatus.retval = Result::INVALID_STATE;
			}
		}
	}
};

}  // namespace implementation
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V5_0_STREAM_WRITE_THREAD_H
