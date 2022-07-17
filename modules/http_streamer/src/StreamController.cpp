#include "StreamController.h"
#include "StreamSession.h"
#include "InputStream.h"
#include "StreamChannel.h"
#include "GroupStreamSession.h"
#include "Worker.h"

#include <iostream>

namespace video_streamer
{

StreamController::StreamController(const std::shared_ptr<StreamChannel>& streamChannel)
    : m_streamChannel(streamChannel)
{
}

StreamController::~StreamController()
{
}

void StreamController::addInputStream(const std::string& name, std::unique_ptr<InputStream> inputStream)
{
    auto groupStream = m_streamChannel->createGroup(name, inputStream->GetFrameEncoder());
    if (groupStream)
    {
        inputStream->onNotify([groupStream](AVUniquePtr<AVFrame> frame)
        {
            groupStream->pushFrame(std::move(frame));
        });
        m_inputStreams.insert({ name, std::make_unique<Worker>(std::move(inputStream)) });
    }
    else
    {
        // log warn
        std::cout << "createGroup: StreamGroup was not create" << std::endl;
    }
}

void StreamController::removeInputStream(const std::string& name)
{
    m_inputStreams.erase(name);
    m_streamChannel->removeGroup(name);
}
    
} // video_streamer