#pragma once

#include <gsl/gsl>

struct AVPacket;

namespace video_streamer
{

	class StreamSession
	{
	public:
		virtual ~StreamSession() = default;
		virtual void connect() = 0;
		virtual void send(gsl::not_null<AVPacket*> packet) = 0;
		virtual bool isConnected() const noexcept = 0;
	};

} // video_streamer