#pragma once

#include "AVMemory.h"

#include <atomic>
#include <functional>
#include <memory>

namespace video_streamer
{

class Encoder;

using NotifyFunction = std::function<void(AVUniquePtr<AVFrame>)>;

class InputStream
{
public:
    virtual ~InputStream() = default;

    virtual void run(const std::atomic_bool& stopper) = 0;
    virtual void onNotify(const NotifyFunction& func) = 0;
    virtual std::shared_ptr<Encoder> GetFrameEncoder() = 0;
};

} // video_streamer