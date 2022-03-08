#pragma once

#include "InputStream.h"

#include <string>
#include <atomic>
#include <thread>

struct AVFormatContext;
struct SwsContext;
struct AVFrame;

namespace video_streamer
{

class FrameEncoder;

class FFmpegInput: public InputStream
{
public:
	FFmpegInput(AVFormatContext* formatContext, SwsContext* swsContext, const std::string& id) noexcept;
	~FFmpegInput() noexcept;

	void start() override;
	void stop() override;

	bool isStarted() const override;

	void onNotify(const NotifyFunction& func) noexcept override
	{
		m_notify = func;
	}
	
	std::shared_ptr<FrameEncoder> GetFrameEncoder() override;

private:
	FFmpegInput() = delete;
	FFmpegInput(const FFmpegInput&) = delete;
	FFmpegInput& operator=(const FFmpegInput&) = delete;
	FFmpegInput(FFmpegInput&& rhs) = delete;
	FFmpegInput& operator=(FFmpegInput&& rhs) = delete;

	AVFormatContext* m_formatContext;
	SwsContext* m_swsContext;
	std::string m_id;
	std::function<void(const std::string&, const AVFrame*)> m_notify;
	std::atomic_bool m_isStarted;
	std::shared_ptr<FrameEncoder> m_frameEncoder = nullptr;
	std::thread m_thread;
};

std::unique_ptr<FFmpegInput> CreateFFmpegInput(
	const std::string& inputId, const std::string& url, const std::string& hwName, int id) noexcept;

} // video_streamer