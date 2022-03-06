#include "HttpStreamSession.h"
#include "FFmpeg.h"

#include <utility>
#include <cassert>
#include <string>

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

	void HttpStreamSession::send(const std::vector<unsigned char>& img)
	{
		assert(("m_clientContext is not init", m_clientContext != nullptr));

		std::string header =
			"--myboundary\r\nContent-Type:image/jpeg\r\nContent-Length: " + std::to_string(img.size()) + "\r\n\r\n";

		std::vector<unsigned char> buff;
		buff.reserve(header.size());
		buff.insert(buff.end(), header.begin(), header.end());
		buff.insert(buff.end(), img.begin(), img.end());

		avio_write(m_clientContext, buff.data(), buff.size());
		avio_flush(m_clientContext);
	}

	void HttpStreamSession::connect()
	{
		assert(("m_clientContext has already connected", !m_isConnect));

		if (m_isConnect)
		{
			return;
		}
		
		av_opt_set(m_clientContext, "headers", header, AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_clientContext, "reconnect_at_eof", "1", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_clientContext, "reconnect_streamed", "1", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_clientContext, "multiple_requests", "1", AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_clientContext, "content_type", contentType, AV_OPT_SEARCH_CHILDREN);
		av_opt_set(m_clientContext, "chunked_post", "1", AV_OPT_SEARCH_CHILDREN);

		av_opt_set_int(m_clientContext, "reply_code", 200, AV_OPT_SEARCH_CHILDREN);

		while (avio_handshake(m_clientContext) > 0);

		m_isConnect = true;
	}

	bool HttpStreamSession::isConnected() const noexcept
	{
		return m_clientContext ? m_isConnect && m_clientContext->error == 0 : false;
	}

} // video_streamer