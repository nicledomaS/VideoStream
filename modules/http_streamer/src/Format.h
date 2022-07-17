#pragma once

#include "AVMemory.h"

namespace video_streamer
{

class Format
{
public:
    virtual ~Format() = default;

    virtual AVUniquePtr<AVPacket> getPacket() = 0;
    virtual AVFormatContext* getAVFormatContext() const = 0;
};

} // video_streamer