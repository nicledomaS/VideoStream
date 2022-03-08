#pragma once

#include <functional>
#include <memory>

struct AVFrame;

namespace video_streamer
{

using NotifyFunction = std::function<void(const std::string&, const AVFrame*)>;

class FrameEncoder;

class InputStream
{
public:
    virtual ~InputStream() = default;

    virtual void start() = 0;
	virtual void stop() = 0;

    virtual bool isStarted() const = 0;

    virtual void onNotify(const NotifyFunction& func) = 0;

    virtual std::shared_ptr<FrameEncoder> GetFrameEncoder() = 0;
};

} // video_streamer