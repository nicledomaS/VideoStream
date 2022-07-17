#pragma once

#include "StreamSession.h"

struct AVIOContext;

namespace video_streamer
{

class HttpStreamSession: public StreamSession
{
public:
	explicit HttpStreamSession(AVIOContext* clientContext) noexcept;
	~HttpStreamSession() noexcept;

	HttpStreamSession(HttpStreamSession&& rhs) noexcept;
	HttpStreamSession& operator=(HttpStreamSession&& rhs) noexcept;

	void connect() override;
	void send(gsl::not_null<AVPacket*> packet) override;
	bool isConnected() const noexcept override;
private:
	HttpStreamSession(const HttpStreamSession&) = delete;
	HttpStreamSession& operator=(const HttpStreamSession&) = delete;

	AVIOContext* m_clientContext;
	bool m_isConnect = false;
};

} // video_streamer