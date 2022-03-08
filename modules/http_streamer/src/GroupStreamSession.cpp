#include "GroupStreamSession.h"
#include "StreamSession.h"
#include "FrameEncoder.h"

namespace video_streamer
{

GroupStreamSession::GroupStreamSession(std::shared_ptr<FrameEncoder> frameEncoder)
    : m_frameEncoder(std::move(frameEncoder))
{
}

void GroupStreamSession::addStream(std::unique_ptr<StreamSession> streamSession)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_streamSession.push_back(std::move(streamSession));
}

void GroupStreamSession::pushFrame(gsl::not_null<const AVFrame*> frame)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if(m_streamSession.empty())
    {
        return;
    }

    auto img = m_frameEncoder->encode(frame);

    for (auto it = m_streamSession.begin(); it != m_streamSession.end();)
    {
        if ((*it)->isConnected())
        {
            (*it)->send(std::move(img));
            ++it;
        }
        else
        {
            it = m_streamSession.erase(it);
        }
    }
}

} // video_streamer