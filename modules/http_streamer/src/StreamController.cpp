#include "StreamController.h"
#include "StreamSession.h"
#include "InputStream.h"
#include "StreamChannel.h"

namespace video_streamer
{

StreamController::StreamController()
    : m_streamChannel(std::make_unique<StreamChannel>())
{
}

StreamController::~StreamController()
{
}

void StreamController::addInputStream(const std::string& name, std::unique_ptr<InputStream> inputStream)
{
    if(inputStream)
    {
        m_streamChannel->createGroup(name, inputStream->GetFrameEncoder());
        inputStream->onNotify([this](const std::string& name, const AVFrame* frame)
        {
            m_streamChannel->notify(name, frame);
        });
        inputStream->start();

        m_inputStreams.insert({ name, std::move(inputStream) });
    }
}

void StreamController::removeInputStream(const std::string& name)
{
    m_inputStreams.erase(name);
    m_streamChannel->removeGroup(name);
}
    
void StreamController::subscribe(const std::string& name, std::unique_ptr<StreamSession> streamSession)
{
    m_streamChannel->addSession(name, std::move(streamSession));
}

bool StreamController::hasInputStream(const std::string& name) const
{
    return m_inputStreams.count(name);
}

    
} // video_streamer