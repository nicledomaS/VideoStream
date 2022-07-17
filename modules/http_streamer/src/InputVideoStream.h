#pragma once

#include "InputStream.h"

#include <string>
#include <map>

namespace video_streamer
{

struct InputConfig;
class Format;
class Decoder;
class Encoder;

class InputVideoStream : public InputStream
{
public:
	InputVideoStream(const InputConfig& inputConfig) noexcept;
    ~InputVideoStream() override;

	void run(const std::atomic_bool& stopper) override;

	void onNotify(const NotifyFunction& func) noexcept override
	{
		m_notify = func;
	}

    std::shared_ptr<Encoder> GetFrameEncoder() override;


private:
	InputVideoStream() = delete;
	InputVideoStream(const InputVideoStream&) = delete;
	InputVideoStream& operator=(const InputVideoStream&) = delete;
	InputVideoStream(InputVideoStream&& rhs) = delete;
	InputVideoStream& operator=(InputVideoStream&& rhs) = delete;

    std::unique_ptr<Format> m_format;
    std::unique_ptr<Decoder> m_decoder;
	NotifyFunction m_notify;
	std::shared_ptr<Encoder> m_frameEncoder = nullptr;
};

} // video_streamer