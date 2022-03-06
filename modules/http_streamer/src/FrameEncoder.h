#pragma once

#include <gsl/gsl>

#include <vector>
#include <memory>

struct AVCodecContext;
struct AVPacket;
struct AVFrame;

namespace video_streamer
{

class FrameEncoder
{
public:
    explicit FrameEncoder(gsl::not_null<AVCodecContext*> codecContext);

    std::vector<unsigned char> encode(gsl::not_null<const AVFrame*> frame);
private:
    gsl::not_null<AVCodecContext*> m_codecContext;
    gsl::not_null<AVPacket*> m_packet;
};

std::shared_ptr<FrameEncoder> createFrameEncoder(
    const std::string& encoderName, gsl::not_null<const AVCodecContext*> codecContext);

} // video_streamer
