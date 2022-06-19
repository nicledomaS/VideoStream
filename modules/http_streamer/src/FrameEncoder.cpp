#include "FrameEncoder.h"
#include "FFmpeg.h"

#include <memory>
#include <iostream>

namespace video_streamer
{

std::shared_ptr<FrameEncoder> createFrameEncoder(const std::string& encoderName, gsl::not_null<const AVCodecContext*> codecContext)
{
	auto encoder = avcodec_find_encoder_by_name(encoderName.c_str());
	AVCodecContext* encodecContext = avcodec_alloc_context3(encoder);
	encodecContext->codec_id = AV_CODEC_ID_MJPEG;
	encodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
	encodecContext->sample_aspect_ratio  = codecContext->sample_aspect_ratio ;
	encodecContext->bit_rate = 400000;
	encodecContext->width = codecContext->width;
	encodecContext->height = codecContext->height;
	encodecContext->time_base = av_inv_q(codecContext->framerate);

	std::cout << "createFrameEncoder coder time_base num: " << codecContext->time_base.num << std::endl;
	std::cout << "createFrameEncoder coder time_base den: " << codecContext->time_base.den << std::endl;

	std::cout << "createFrameEncoder encoder time_base num: " << encodecContext->time_base.num << std::endl;
	std::cout << "createFrameEncoder encoder time_base den: " << encodecContext->time_base.den << std::endl;

	auto codecName = avcodec_get_name(codecContext->codec_id);
	std::cout << "createFrameEncoder codec name: " << codecName << std::endl;
	std::cout << "createFrameEncoder encoder name: " << encoderName << std::endl;

	if (encoder->pix_fmts)
	{
		encodecContext->pix_fmt = encoder->pix_fmts[0];
	}
	else
	{
		encodecContext->pix_fmt = codecContext->pix_fmt;
	}

    if(avcodec_open2(encodecContext, encoder, NULL) < 0)
	{
		std::cout << "createFrameEncoder encodecContext did not open" << std::endl;
	}

    return std::make_shared<FrameEncoder>(encodecContext);
}

FrameEncoder::FrameEncoder(gsl::not_null<AVCodecContext*> codecContext)
    : m_codecContext(codecContext),
    m_packet(av_packet_alloc())
{
}

std::vector<unsigned char> FrameEncoder::encode(gsl::not_null<const AVFrame*> frame)
{
    auto ret = avcodec_send_frame(m_codecContext, frame);
	if (ret < 0) {
		return {};
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(m_codecContext, m_packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return {};
		else if (ret < 0) {
			return {};
		}

		std::vector<unsigned char> jpg(m_packet->data, m_packet->data + m_packet->size);
		av_packet_unref(m_packet);
		return jpg;
	}

	return {};
}

} // video_streamer