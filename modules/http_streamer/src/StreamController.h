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
    
class StreamController
{
public:
    StreamController();
    ~StreamController();

    void addInputStream(const std::string& name, std::unique_ptr<InputStream> inputStream);
    void removeInputStream(const std::string& name);

    void subscribe(const std::string& name, std::unique_ptr<StreamSession> streamSession);

    bool hasInputStream(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<InputStream>> m_inputStreams;
    std::unique_ptr<StreamChannel> m_streamChannel;
};

} // video_streamer
