#include "HttpStreamServer.h"
#include "HttpStreamSession.h"
#include "StreamController.h"
#include "FFmpeg.h"

#include <utility>
#include <cassert>
#include <vector>
#include <iostream>
#include <cstdio>

namespace video_streamer
{

	std::unique_ptr<HttpStreamServer> createHttpStreamServer(const std::string& url, std::shared_ptr<StreamController> streamController)
	{
		AVDictionary* options = nullptr;
		if (av_dict_set(&options, "listen", "2", 0) < 0)
		{
			return nullptr;
		}

		AVIOContext* serverContext = nullptr;
		if (avio_open2(&serverContext, url.c_str(), AVIO_FLAG_WRITE, NULL, &options) < 0)
		{
			std::cout << "avio_open2 bad" << std::endl;
			av_dict_free(&options);
			return nullptr;
		}
		return std::make_unique<HttpStreamServer>(serverContext, streamController);
	}

	HttpStreamServer::HttpStreamServer(AVIOContext* serverContext, std::shared_ptr<StreamController> streamController) noexcept
		: m_serverContext(serverContext),
		m_streamController(std::move(streamController)),
		m_isStarted(false)
	{
	}

	HttpStreamServer::~HttpStreamServer() noexcept
	{
		avio_close(m_serverContext);
	}

	void HttpStreamServer::run()
	{
		assert(("m_clientContext has already run", !m_isStarted));

		if(m_isStarted)
		{
			return;
		}
		
		m_isStarted = true;

		while (m_isStarted)
		{
			AVIOContext* clientContext = nullptr;
			if (avio_accept(m_serverContext, &clientContext) < 0)
			{
				break;
			}

			std::cout << "New connection " << std::endl;

			int ret = 0;
			std::string resource;
			while ((ret = avio_handshake(clientContext)) > 0) {
				uint8_t* str = nullptr;
				size_t len = 0;
				av_opt_get(clientContext, "resource", AV_OPT_SEARCH_CHILDREN, &str);
				len = strlen(reinterpret_cast<const char*>(str));
				if (str && len)
				{
					resource = std::string(reinterpret_cast<const char*>(str));
					av_freep(&str);
					break;
				}	
				av_freep(&str);
			}

			auto name = resource.size() > 1 && resource[0] == '/' ? resource.substr(1, resource.size() - 1) : "";

			if (!name.empty() && m_streamController->hasInputStream(name))
			{
				createHttpStreamSession(name, clientContext);
			}
			else
			{
				if (av_opt_set_int(clientContext, "reply_code", AVERROR_HTTP_NOT_FOUND, AV_OPT_SEARCH_CHILDREN) == 0) {
					while ((ret = avio_handshake(clientContext)) > 0);
				}

				avio_flush(clientContext);
				avio_close(clientContext);
			}
		}

		m_isStarted = false;
	}

	void HttpStreamServer::createHttpStreamSession(const std::string& name, AVIOContext* clientContext)
	{
		assert(("clientContext is not init", clientContext != nullptr));

		auto session = std::make_unique<HttpStreamSession>(clientContext);
		session->connect();
		if (session->isConnected())
		{
			m_streamController->subscribe(name, std::move(session));
		}
		else
		{
			// log warn
			std::cout << "Session was not connected" << std::endl;
		}
	}

} // video_streamer