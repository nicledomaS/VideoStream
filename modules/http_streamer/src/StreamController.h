#pragma once

#include <thread>
#include <memory>
#include <string>
#include <unordered_map>

namespace video_streamer
{

class InputStream;
class StreamChannel;
class StreamSession;
class Worker;
    
class StreamController
{
public:
    explicit StreamController(const std::shared_ptr<StreamChannel>& streamChannel);
    ~StreamController();

    void addInputStream(const std::string& name, std::unique_ptr<InputStream> inputStream);
    void removeInputStream(const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<Worker>> m_inputStreams;
    std::shared_ptr<StreamChannel> m_streamChannel;
};

} // video_streamer
