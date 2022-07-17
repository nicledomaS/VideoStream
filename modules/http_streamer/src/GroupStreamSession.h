#pragma once

#include "AVMemory.h"

#include <gsl/gsl>

#include <vector>
#include <memory>
#include <mutex>

namespace video_streamer
{

class StreamSession;
class Encoder;

class GroupStreamSession
{
public:
    explicit GroupStreamSession(std::shared_ptr<Encoder> frameEncoder);

    void addStream(std::unique_ptr<StreamSession> streamSession);
    void pushFrame(AVUniquePtr<AVFrame> frame);

private:
    std::shared_ptr<Encoder> m_frameEncoder;
    std::vector<std::unique_ptr<StreamSession>> m_streamSession;
    mutable std::mutex m_mtx;
};

} // video_streamer