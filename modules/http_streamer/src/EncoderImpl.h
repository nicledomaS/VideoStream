#pragma once

#include "Encoder.h"

#include <string>


namespace video_streamer
{

class EncoderImpl : public Encoder
{
public:
    explicit EncoderImpl(AVUniquePtr<AVCodecContext> codecContext);
    EncoderImpl(const ::std::string& encoderName, gsl::not_null<AVCodecContext*> codecContext);

    AVUniquePtr<AVPacket> encode(AVUniquePtr<AVFrame> frame) override;
    AVCodecContext* getAVCodecContext() const override;

private:
    AVUniquePtr<AVCodecContext> m_encodecContext;
};

} // video_streamer