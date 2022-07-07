#pragma once

#include "Decoder.h"

#include <gsl/gsl>

namespace video_streamer
{

class HWDecoder : public Decoder
{
public:
    HWDecoder(gsl::not_null<AVFormatContext*> formatContext, const std::string& hwName, const std::string& id);

    AVUniquePtr<AVFrame> decode(AVUniquePtr<AVPacket> packet) override;

    AVCodecContext* getAVCodecContext() const override;
private:
    std::unique_ptr<Decoder> m_decoder;
};

} // video_streamer