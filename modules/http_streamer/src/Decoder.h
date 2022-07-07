#pragma once

#include "AVMemory.h"

namespace video_streamer
{

class Decoder
{
public:
    virtual ~Decoder() = default;

    virtual AVUniquePtr<AVFrame> decode(AVUniquePtr<AVPacket> packet) = 0;
    virtual AVCodecContext* getAVCodecContext() const = 0;
};

} // video_streamer