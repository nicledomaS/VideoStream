#include "HWDecoder.h"
#include "HWUtils.h"
#include "DecoderImpl.h"
#include "FFmpeg.h"

#include <iostream>

namespace video_streamer
{

namespace
{

AVUniquePtr<AVCodecContext> makeDecoderContext(AVFormatContext* formatContext, const std::string& hwName, const std::string& id)
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

    if (auto* hwDeviceCtx = createHwContext(hwName, id))
    {
        codecContext->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
        av_log(nullptr, AV_LOG_INFO, "makeDecoderContext hw device: %s, id: %s found\n", hwName.c_str(), id.c_str());
    }
    else
    {
        av_log(nullptr, AV_LOG_ERROR, "makeDecoderContext hw device: %s, id: %s not found\n", hwName.c_str(), id.c_str());
    }

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

HWDecoder::HWDecoder(gsl::not_null<AVFormatContext*> formatContext, const std::string& hwName, const std::string& id)
    : m_decoder(std::make_unique<DecoderImpl>(makeDecoderContext(formatContext, hwName, id)))
{
}

AVUniquePtr<AVFrame> HWDecoder::decode(AVUniquePtr<AVPacket> packet)
{
    auto hwFrame = m_decoder->decode(std::move(packet));
    if(!hwFrame)
    {
        return nullptr;
    }

    if(isHWPixFmt(AVPixelFormat(hwFrame->format)))
    {
        auto swFrame = makeAVFrame();
        if (av_hwframe_transfer_data(swFrame.get(), hwFrame.get(), 0) > 0)
        {
            av_log(nullptr, AV_LOG_ERROR, "HWDecoder::decode: Error transferring the data to system memory\n");
            return nullptr;
        }

        return swFrame;
    }
    else
    {
        auto pixFmtName = av_get_pix_fmt_name(AVPixelFormat(hwFrame->format));
	    av_log(nullptr, AV_LOG_ERROR, "HWDecoder::decode: pix fmt: %s\n", pixFmtName);
        av_log(nullptr, AV_LOG_ERROR, "HWDecoder::decode: pix_fmt unsupport\n");
    }

    return nullptr;
}

AVCodecContext* HWDecoder::getAVCodecContext() const
{
    return m_decoder->getAVCodecContext();
}

} // video_streamer