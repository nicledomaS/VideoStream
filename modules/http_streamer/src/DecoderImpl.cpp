#include "DecoderImpl.h"
#include "FFmpeg.h"

#include <iostream>

namespace video_streamer
{

namespace
{

AVUniquePtr<AVCodecContext> makeDecoderContext(AVFormatContext* formatContext)
{
    auto streamId = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    auto* stream = formatContext->streams[streamId];
    
    auto* codecpar = stream->codecpar;
    auto* decoder = avcodec_find_decoder(codecpar->codec_id);
    
    auto codecContext = makeAVCodecContext(decoder);
    auto codecContextPtr = codecContext.get();
    auto result = avcodec_parameters_to_context(codecContextPtr, codecpar);
    if (result)
    {
        throw std::runtime_error("Failed parameters to context AVMEDIA_TYPE_VIDEO codec");
    }

    codecContext->framerate = av_guess_frame_rate(formatContext, stream, nullptr);
    codecContext->thread_count = 1;

    av_opt_set_int(codecContextPtr, "refcounted_frames", 1, 0);

    auto codecName = avcodec_get_name(codecContext->codec_id);
    av_log(nullptr, AV_LOG_INFO, "makeDecoderContext codec name: %s\n", codecName);
    auto pixFmtName = av_get_pix_fmt_name(codecContext->pix_fmt);
	av_log(nullptr, AV_LOG_INFO, "makeDecoderContext encoder pix fmt: %s\n", pixFmtName);

    if (avcodec_open2(codecContextPtr, decoder, nullptr) < 0)
    {
        throw std::runtime_error("Failed to open AVMEDIA_TYPE_VIDEO codec");
    }

    return codecContext;
}

} // anonymous

DecoderImpl::DecoderImpl(gsl::not_null<AVFormatContext*> formatContext)
    : m_codecContext(makeDecoderContext(formatContext))
{
}

DecoderImpl::DecoderImpl(AVUniquePtr<AVCodecContext> codecContext)
    : m_codecContext(std::move(codecContext))
{
}

AVUniquePtr<AVFrame> DecoderImpl::decode(AVUniquePtr<AVPacket> packet)
{
    if (avcodec_send_packet(m_codecContext.get(), packet.get()) < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
        return nullptr;
    }

    auto frame = makeAVFrame();
    auto result = avcodec_receive_frame(m_codecContext.get(), frame.get());
    if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
    {
        av_log(nullptr, AV_LOG_ERROR, "EOF or EAGAIN while receiving a frame from the decoder\n");
        return nullptr;
    }
    else if (result < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
        return nullptr;
    }

    return frame;
}

AVCodecContext* DecoderImpl::getAVCodecContext() const
{
    return m_codecContext.get();
}

} // video_streamer