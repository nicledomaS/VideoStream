#include "HWEncoder.h"
#include "EncoderImpl.h"
#include "HWUtils.h"
#include "FFmpeg.h"

#include <iostream>

namespace video_streamer
{

namespace
{

AVUniquePtr<AVCodecContext> makeEncoderContext(const ::std::string& encoderName, AVCodecContext* codecContext, const std::string& hwName, const std::string& id)
{
	auto codecName = avcodec_get_name(codecContext->codec_id);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext codec name: %s\n", codecName);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder name: %s\n", encoderName.c_str());

    auto encodec = avcodec_find_encoder_by_name(encoderName.c_str());
	if(!encodec)
	{
		av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder not found\n");
	}
    auto encodecContextPtr = makeAVCodecContext(encodec);
	encodecContextPtr->codec_id = encodec->id;
	encodecContextPtr->codec_type = encodec->type;
	encodecContextPtr->sample_aspect_ratio  = codecContext->sample_aspect_ratio;
	encodecContextPtr->bit_rate = 400000;
	encodecContextPtr->width = codecContext->width;
	encodecContextPtr->height = codecContext->height;
	encodecContextPtr->time_base = av_inv_q(codecContext->framerate);
	encodecContextPtr->pix_fmt = FindHWPixFmt(av_hwdevice_find_type_by_name(hwName.c_str()));

	auto pixFmtName = av_get_pix_fmt_name(encodecContextPtr->pix_fmt);
	av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder pix fmt: %s\n", pixFmtName);

	if (auto* hwDeviceCtx = createHwContext(hwName, id))
    {
        codecContext->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
		setHWFrameCtx(encodecContextPtr.get(), hwDeviceCtx);
        av_log(nullptr, AV_LOG_INFO, "makeDecoderContext hw device: %s, id: %s found\n", hwName.c_str(), id.c_str());
    }
    else
    {
        av_log(nullptr, AV_LOG_ERROR, "makeDecoderContext hw device: %s, id: %s not found\n", hwName.c_str(), id.c_str());
    }

    if(avcodec_open2(encodecContextPtr.get(), encodec, nullptr) < 0)
	{
		throw std::runtime_error("Failed to open AVMEDIA_TYPE_VIDEO encoder");
	}

    return encodecContextPtr;
}

} // anonymous

HWEncoder::HWEncoder(const ::std::string& encoderName, gsl::not_null<AVCodecContext*> codecContext)
	: m_encoder(std::make_unique<EncoderImpl>(makeEncoderContext(encoderName, codecContext, "dxva2", "0")))
{
}

AVUniquePtr<AVPacket> HWEncoder::encode(AVUniquePtr<AVFrame> swFrame)
{
	auto hwFrame = makeAVFrame();
	auto codecContext = getAVCodecContext();
	if (av_hwframe_get_buffer(codecContext->hw_frames_ctx, hwFrame.get(), 0) < 0) {
		// fprintf(stderr, "Error code: %s.\n", av_err2str(err));
		return nullptr;
	}
	if (!hwFrame->hw_frames_ctx) {
		return nullptr;
	}
	if (av_hwframe_transfer_data(hwFrame.get(), swFrame.get(), 0) > 0)
	{
		av_log(nullptr, AV_LOG_INFO, "HWDecoder::decode: Error transferring the data to system memory\n");
		return nullptr;
	}

	return m_encoder->encode(std::move(hwFrame));
}

AVCodecContext* HWEncoder::getAVCodecContext() const
{
    return m_encoder->getAVCodecContext();
}

} // video_streamer