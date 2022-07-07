#pragma once

#include "Decoder.h"

#include <gsl/gsl>

namespace video_streamer
{

class DecoderImpl : public Decoder
{
public:
    explicit DecoderImpl(gsl::not_null<AVFormatContext*> formatContext);
    explicit DecoderImpl(AVUniquePtr<AVCodecContext> codecContext);

    AVUniquePtr<AVFrame> decode(AVUniquePtr<AVPacket> packet) override;

    AVCodecContext* getAVCodecContext() const override;
private:
    AVUniquePtr<AVCodecContext> m_codecContext;
};

} // video_streamer