#pragma once

#include "AVMemory.h"

#include <gsl/gsl>

namespace video_streamer
{

class Encoder
{
public:
    virtual ~Encoder() = default;

    virtual AVUniquePtr<AVPacket> encode(AVUniquePtr<AVFrame> frame) = 0;
    virtual AVCodecContext* getAVCodecContext() const = 0;
};

} // video_streamer