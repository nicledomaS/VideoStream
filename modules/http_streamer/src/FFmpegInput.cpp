#include "FFmpegInput.h"
#include "FFmpeg.h"
#include "FrameEncoder.h"

#include <iostream>
#include <cassert>
#include <memory>

namespace video_streamer
{

namespace
{

	AVFormatContext* createFormatContext(const std::string& url)
	{
		if (url.empty())
		{
			return nullptr;
		}

		auto* fContext = avformat_alloc_context();
		if (avformat_open_input(&fContext, url.c_str(), nullptr, nullptr) != 0) {
			avformat_free_context(fContext);
			return nullptr;
		}

		if (avformat_find_stream_info(fContext, nullptr) < 0) {
			avformat_free_context(fContext);
			return nullptr;
		}

		return fContext;
	}

	AVFormatContext* createCamFormatContext(const std::string& url)
	{
		if (url.empty())
		{
			return nullptr;
		}

		std::cout << "createCamFormatContext start" << std::endl;

		AVDictionary* dictionary = nullptr;
		av_dict_set(&dictionary, "video_size", "1280x720", 0);
		av_dict_set(&dictionary, "framerate", "10", 0);

		auto* fContext = avformat_alloc_context();
		fContext->video_codec_id = AV_CODEC_ID_MJPEG;
		std::cout << "createCamFormatContext av_find_input_format" << std::endl;
		auto* iformat = av_find_input_format("v4l2");
		if(iformat)
		{
			std::cout << "iformat found" << std::endl;
		}
		// ("video=" + url).c_str() windows!!!

		std::cout << "createCamFormatContext avformat_open_input" << std::endl;
		if (avformat_open_input(&fContext, url.c_str(), iformat, &dictionary) != 0) {
			avformat_free_context(fContext);
			return nullptr;
		}

		std::cout << "createCamFormatContext avformat_find_stream_info" << std::endl;
		if (avformat_find_stream_info(fContext, nullptr) < 0) {
			avformat_free_context(fContext);
			return nullptr;
		}

		return fContext;
	}

	AVCodecContext* getCodecContext(AVFormatContext* fContext, AVCodec** dec, int& streamId)
	{
		streamId = av_find_best_stream(fContext, AVMEDIA_TYPE_VIDEO, -1, -1, dec, 0);
		if (streamId < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
			return nullptr;
		}

		return fContext->streams[streamId]->codec;
	}

	// static AVPixelFormat FindPixFmt(const AVHWDeviceType type)
	// {
	// 	enum AVPixelFormat fmt;
	// 	switch (type) {
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_CUDA;
	// 		break;
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_VAAPI:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_VAAPI;
	// 		break;
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_DXVA2:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_NV12;
	// 		break;
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_NV12;
	// 		break;
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_VDPAU:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_VDPAU;
	// 		break;
	// 	case AVHWDeviceType::AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_VIDEOTOOLBOX;
	// 		break;
	// 	default:
	// 		fmt = AVPixelFormat::AV_PIX_FMT_NONE;
	// 		break;
	// 	}
	// 	return fmt;
	// }

	bool isHWPixFmt(const AVPixelFormat pixFmt)
	{
		switch (pixFmt) {
			case AVPixelFormat::AV_PIX_FMT_VAAPI:
			case AVPixelFormat::AV_PIX_FMT_DXVA2_VLD:
			case AVPixelFormat::AV_PIX_FMT_D3D11:
			case AVPixelFormat::AV_PIX_FMT_VDPAU:
			case AVPixelFormat::AV_PIX_FMT_VIDEOTOOLBOX:
				return true;
			default:
				return false;
		}
	}

	AVBufferRef* createHwContext(AVHWDeviceType deviceType, int id)
	{
		if (deviceType == AVHWDeviceType::AV_HWDEVICE_TYPE_NONE)
		{
			return nullptr;
		}
		char device[128] = "";

		snprintf(device, sizeof(device), "%d", id);
		AVBufferRef* hw_device_ctx = nullptr;
		av_hwdevice_ctx_create(&hw_device_ctx, deviceType, device, nullptr, 0);
		return hw_device_ctx;
	}

	bool codecOpen(AVCodecContext* codecContext, AVCodec* decoder, AVHWDeviceType deviceType, int id)
	{
		if (auto* hw_device_ctx = createHwContext(deviceType, id))
		{
			codecContext->hw_device_ctx = av_buffer_ref(hw_device_ctx);
		}

		codecContext->thread_count = 1;

		av_opt_set_int(codecContext, "refcounted_frames", 1, 0);
		if ((avcodec_open2(codecContext, decoder, nullptr)) < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Cannot open video decoder\n");
			return false;
		}

		return true;
	}

	void avPacketFree(AVPacket* ptr)
	{
		av_packet_unref(ptr);
		av_packet_free(&ptr);
	}

	using AVPacketUnique = std::unique_ptr<AVPacket, decltype(&avPacketFree)>;

	AVPacketUnique makeAVPacket()
	{
		AVPacketUnique packet(nullptr, &avPacketFree);
		packet.reset(av_packet_alloc());

		return packet;
	}

} // anonymous

std::unique_ptr<FFmpegInput> CreateFFmpegInput(const std::string& inputId, const std::string& url, const std::string& hwName, int id) noexcept
{
auto fContext = createCamFormatContext(url);
	if (!fContext)
	{
		return nullptr;
	}

	std::cout << "Format context created" << std::endl;
	int streamId = 0;
	AVCodec* decoder = nullptr;
	auto codecContext = getCodecContext(fContext, &decoder, streamId);

	AVHWDeviceType deviceType = av_hwdevice_find_type_by_name(hwName.c_str());

	if (!codecContext || !decoder || !codecOpen(codecContext, decoder, deviceType, id))
	{
		avformat_free_context(fContext);
		return nullptr;
	}
	// auto pixFmt = deviceType != AVHWDeviceType::AV_HWDEVICE_TYPE_NONE ? FindPixFmt(deviceType) : codecContext->pix_fmt;
	// SwsContext* swsContext = sws_getContext(
	// 	codecContext->width,
	// 	codecContext->height,
	// 	pixFmt,
	// 	codecContext->width,
	// 	codecContext->height,
	// 	AV_PIX_FMT_BGR24,
	// 	SWS_BICUBIC, NULL, NULL, NULL);
	// std::cout << "FFmpegInput will be created" << std::endl;
	return std::make_unique<FFmpegInput>(fContext, codecContext, nullptr, inputId);
}

FFmpegInput::FFmpegInput(AVFormatContext* formatContext, AVCodecContext* codecContext, SwsContext* swsContext, const std::string& id) noexcept
	: m_formatContext(formatContext),
	m_codecContext(codecContext),
	m_swsContext(swsContext),
	m_id(id),
	m_isStarted(false)
{
}

FFmpegInput::~FFmpegInput() noexcept
{
	m_isStarted = false;
	if(m_thread.joinable())
	{
		m_thread.join();
	}

	avformat_free_context(m_formatContext);
	m_formatContext = nullptr;

	sws_freeContext(m_swsContext);
	m_swsContext = nullptr;
}

void FFmpegInput::start()
{
	// auto test = ("m_thread is started", !m_thread.joinable());
	// assert(test);

	std::cout << "Start input stream" << std::endl;
	

	m_thread = std::thread([this](){
		int streamId = 0;

		av_read_play(m_formatContext);

		auto* picture = av_frame_alloc();
		auto* sw_frame = av_frame_alloc();
		auto* picture_rgb = av_frame_alloc();
		 
		auto size2 = av_image_get_buffer_size(AVPixelFormat::AV_PIX_FMT_BGR24, m_codecContext->width, m_codecContext->height, 1);
		auto* picture_buffer_2 = (uint8_t*)(av_malloc(size2));

		av_image_fill_arrays(picture_rgb->data, picture_rgb->linesize, picture_buffer_2, AVPixelFormat::AV_PIX_FMT_BGR24, m_codecContext->width, m_codecContext->height, 1);

		AVFrame* tmp_frame = nullptr;


		m_isStarted = true;

		auto packet = makeAVPacket();

		while (av_read_frame(m_formatContext, packet.get()) >= 0 && m_isStarted)
		{
			if (packet->stream_index == streamId)
			{
				auto ret = avcodec_send_packet(m_codecContext, packet.get());
				if (ret < 0) {
					av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
					break;
				}
				while (ret >= 0)
				{
					ret = avcodec_receive_frame(m_codecContext, picture);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
						break;
					}
					else if (ret < 0) {
						av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
						break;
					}

					if (ret >= 0) {
						static int i = 0;

						if (isHWPixFmt(AVPixelFormat(picture->format)))
						{
							if ((av_hwframe_transfer_data(sw_frame, picture, 0)) < 0) {
								fprintf(stderr, "Error transferring the data to system memory\n");
								continue;
							}
							tmp_frame = sw_frame;
						}
						else
							tmp_frame = picture;

						
						// if (sws_scale(m_swsContext, tmp_frame->data, tmp_frame->linesize, 0,
						// 	cContext->height, picture_rgb->data, picture_rgb->linesize))
						// {

						// }

						if (m_notify)
						{
							m_notify(m_id, tmp_frame);
						}


						av_frame_unref(picture);
					}
				}
			}

			av_packet_unref(packet.get());
		}

		m_isStarted = false;

	});
}

void FFmpegInput::stop()
{
	m_isStarted = false;
	if(m_thread.joinable())
	{
		m_thread.join();
	}
}

bool FFmpegInput::isStarted() const
{
	return m_isStarted;
}

std::shared_ptr<FrameEncoder> FFmpegInput::GetFrameEncoder()
{
	if(!m_frameEncoder)
	{
		m_frameEncoder = createFrameEncoder("mjpeg", m_codecContext);
		if(!m_frameEncoder)
		{
			throw 1;
		}
	}
	
	return m_frameEncoder;
}

} // video_streamer