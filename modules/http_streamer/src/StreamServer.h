#pragma once

struct AVIOContext;

namespace video_streamer
{

	class StreamServer
	{
	public:
		virtual ~StreamServer() = default;
		virtual void run() = 0;
	};

} // video_streamer