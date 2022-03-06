#pragma once

#include <vector>

namespace video_streamer
{

	class StreamSession
	{
	public:
		virtual ~StreamSession() = default;
		virtual void connect() = 0;
		virtual void send(const std::vector<unsigned char>& img) = 0;
		virtual bool isConnected() const noexcept = 0;
	};

} // video_streamer