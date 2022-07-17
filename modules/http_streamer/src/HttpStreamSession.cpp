#include "HttpStreamSession.h"
#include "FFmpeg.h"

#include <utility>
#include <cassert>
#include <string>
#include <vector>

namespace video_streamer
{

	namespace
	{

		constexpr auto header = "Cache-Control: no-cache\r\nPragma: no-cache\r\nConnection: keep-alive\r\n";
		constexpr auto contentType = "multipart/x-mixed-replace; boundary=myboundary";

	} // anonymous

	HttpStreamSession::HttpStreamSession(AVIOContext* clientContext) noexcept
		: m_clientContext(clientContext)
	{
	}

	HttpStreamSession::~HttpStreamSession() noexcept
	{
		if (m_clientContext)
		{
			avio_flush(m_clientContext);
			avio_close(m_clientContext);
		}
	}

	HttpStreamSession::HttpStreamSession(HttpStreamSession&& rhs) noexcept
		: m_clientContext(std::exchange(rhs.m_clientContext, nullptr)),
		m_isConnect(std::exchange(rhs.m_isConnect, false))
	{
	}

	HttpStreamSession& HttpStreamSession::operator=(HttpStreamSession&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_clientContext = std::exchange(rhs.m_clientContext, nullptr);
			m_isConnect = std::exchange(rhs.m_isConnect, false);
		}

		return *this;
	}

	void HttpStreamSession::send(gsl::not_null<AVPacket*> packet)
	{
		// assert(("m_clientContext is not init", m_clientContext != nullptr));

		std::string header =
			"--myboundary\r\nContent-Type:image/jpeg\r\nContent-Length: " + std::to_string(packet->size) + "\r\n\r\n";

		std::vector<unsigned char> buff;
		buff.reserve(header.size());
		buff.insert(buff.end(), header.begin(), header.end());
		buff.insert(buff.end(), packet->data, packet->data + packet->size);

		avio_write(m_clientContext, buff.data(), static_cast<int>(buff.size()));
		avio_flush(m_clientContext);
	}

	void HttpStreamSession::connect()
	{
		// assert(("m_clientContext has already connected", !m_isConnect));

		if (m_isConnect)
		{
			return;
		}

		av_opt_set_int(m_clientContext, "reply_code", 200, AV_OPT_SEARCH_CHILDREN);

		while (avio_handshake(m_clientContext) > 0);

		m_isConnect = true;
	}

	bool HttpStreamSession::isConnected() const noexcept
	{
		return m_clientContext ? m_isConnect && m_clientContext->error == 0 : false;
	}

} // video_streamer