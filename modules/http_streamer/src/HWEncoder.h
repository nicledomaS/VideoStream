#pragma once

#include "Encoder.h"

#include <string>

struct AVCodecContext;

namespace video_streamer
{

class HWEncoder : public Encoder
{
public:
    HWEncoder(const ::std::string& encoderName, gsl::not_null<AVCodecContext*> codecContext);

    AVUniquePtr<AVPacket> encode(AVUniquePtr<AVFrame> swFrame) override;
    AVCodecContext* getAVCodecContext() const override;

private:
    std::unique_ptr<Encoder> m_encoder;
};

} // video_streamer