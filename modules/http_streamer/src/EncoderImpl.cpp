#include "EncoderImpl.h"
#include "FFmpeg.h"

#include <iostream>

namespace video_streamer
{

namespace
{

void avCodecContextFree(AVCodecContext* ptr)
{
    avcodec_free_context(&ptr);
}

AVPixelFormat getPixFmt(const AVCodecContext* encodecContext, const AVCodecContext* codecContext, const AVPixelFormat* pixFmts)
{
	AVPixelFormat pixFmt = pixFmts ? pixFmts[0] : AV_PIX_FMT_NONE;
	if(codecContext->codec_id == encodecContext->codec_id)
	{
		pixFmt = codecContext->pix_fmt;
	}

	if (pixFmt == AV_PIX_FMT_NONE)
	{
		throw std::runtime_error("createFrameEncoder encodecContext unknown pixel format");
	}

	return pixFmt;
}

AVUniquePtr<AVCodecContext> makeEncoderContext(const ::std::string& encoderName, AVCodecContext* codecContext)
{
	auto codecName = avcodec_get_name(codecContext->codec_id);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext codec name: %s\n", codecName);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder name: %s\n", encoderName.c_str());

    auto encoder = avcodec_find_encoder_by_name(encoderName.c_str());
    auto encodecContextPtr = makeAVUniquePtr<AVCodecContext>(std::bind(&avcodec_alloc_context3, encoder), &avCodecContextFree);
	encodecContextPtr->codec_id = AV_CODEC_ID_MJPEG;
	encodecContextPtr->codec_type = AVMEDIA_TYPE_VIDEO;
	encodecContextPtr->sample_aspect_ratio  = codecContext->sample_aspect_ratio ;
	encodecContextPtr->bit_rate = 400000;
	encodecContextPtr->width = codecContext->width;
	encodecContextPtr->height = codecContext->height;
	encodecContextPtr->time_base = av_inv_q(codecContext->framerate);
	encodecContextPtr->pix_fmt = getPixFmt(encodecContextPtr.get(), codecContext, encoder->pix_fmts);

	auto pixFmtName = av_get_pix_fmt_name(encodecContextPtr->pix_fmt);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder pix fmt: %s\n", pixFmtName);

    if(avcodec_open2(encodecContextPtr.get(), encoder, nullptr) < 0)
	{
		throw std::runtime_error("Failed to open AVMEDIA_TYPE_VIDEO encoder");
	}

    return encodecContextPtr;
}

} // anonymous

EncoderImpl::EncoderImpl(AVUniquePtr<AVCodecContext> codecContext)
	: m_encodecContext(std::move(codecContext))
{

}

EncoderImpl::EncoderImpl(const ::std::string& encoderName, gsl::not_null<AVCodecContext*> codecContext)
    : m_encodecContext(makeEncoderContext(encoderName, codecContext))
{
}

AVUniquePtr<AVPacket> EncoderImpl::encode(AVUniquePtr<AVFrame> frame)
{
	if (avcodec_send_frame(m_encodecContext.get(), frame.get()) < 0)
	{
		av_log(nullptr, AV_LOG_ERROR, "Error while sending a frame to the encoder\n");
		return nullptr;
	}

	auto packet = makeAVPacket();
	auto result = avcodec_receive_packet(m_encodecContext.get(), packet.get());
	if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
    {
        av_log(nullptr, AV_LOG_ERROR, "EOF or EAGAIN while receiving a packet from the encoder\n");
        return nullptr;
    }
    else if (result < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "Error while receiving a packet from the encoder\n");
        return nullptr;
    }

	return packet;
}

AVCodecContext* EncoderImpl::getAVCodecContext() const
{
    return m_encodecContext.get();
}

} // video_streamer