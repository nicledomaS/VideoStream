#pragma once

#include <gsl/gsl>

#include <vector>
#include <memory>
#include <mutex>

struct AVFrame;

namespace video_streamer
{

class StreamSession;
class FrameEncoder;

class GroupStreamSession
{
public:
    explicit GroupStreamSession(std::shared_ptr<FrameEncoder> frameEncoder);

    void addStream(std::unique_ptr<StreamSession> streamSession);
    void pushFrame(gsl::not_null<const AVFrame*> frame);

private:
    std::shared_ptr<FrameEncoder> m_frameEncoder;
    std::vector<std::unique_ptr<StreamSession>> m_streamSession;
    mutable std::mutex m_mtx;
};

} // video_streamer