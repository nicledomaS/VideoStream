#include "GroupStreamSession.h"
#include "StreamSession.h"
#include "Encoder.h"

namespace video_streamer
{

GroupStreamSession::GroupStreamSession(std::shared_ptr<Encoder> frameEncoder)
    : m_frameEncoder(std::move(frameEncoder))
{
}

void GroupStreamSession::addStream(std::unique_ptr<StreamSession> streamSession)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_streamSession.push_back(std::move(streamSession));
}

void GroupStreamSession::pushFrame(AVUniquePtr<AVFrame> frame)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if(m_streamSession.empty())
    {
        return;
    }

    auto packet = m_frameEncoder->encode(std::move(frame));
    if(!packet)
    {
        return;
    }

    for (auto it = m_streamSession.begin(); it != m_streamSession.end();)
    {
        if ((*it)->isConnected())
        {
            (*it)->send(packet.get());
            ++it;
        }
        else
        {
            it = m_streamSession.erase(it);
        }
    }
}

} // video_streamer