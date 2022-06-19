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

	namespace
	{

		constexpr auto Listen = "listen";
		constexpr auto Resource = "resource";
		constexpr auto TlsVerify = "tls_verify";
		constexpr auto CertFile = "cert_file";
		constexpr auto KeyFile = "key_file";

		constexpr auto ReconectAtEof = "reconnect_at_eof";
		constexpr auto ReconectStreamed = "reconnect_streamed";
		constexpr auto MultipleReuest = "multiple_requests";
		constexpr auto ChankedPost = "chunked_post";

		constexpr auto Header = "headers";
		constexpr auto ContentType = "content_type";

		constexpr auto HeaderValue = "Cache-Control: no-cache\r\nPragma: no-cache\r\nConnection: keep-alive\r\n";
		constexpr auto ContentTypeValue = "multipart/x-mixed-replace; boundary=myboundary";

		constexpr auto EnableValue = "1";

		constexpr auto ListenValue = "1";
		constexpr auto TlsVerifyValue = "0";

		AVDictionary* getHttpOptions()
		{
			AVDictionary* options = nullptr;
			av_dict_set(&options, Listen, ListenValue, 0);

			av_dict_set(&options, ReconectAtEof, EnableValue, 0);
			av_dict_set(&options, ReconectStreamed, EnableValue, 0);
			av_dict_set(&options, MultipleReuest, EnableValue, 0);
			av_dict_set(&options, ChankedPost, EnableValue, 0);

			av_dict_set(&options, Header, HeaderValue, 0);
			av_dict_set(&options, ContentType, ContentTypeValue, 0);

			return options;
		}

		AVDictionary* getHttpsOptions(const std::string& cert, const std::string& key)
		{
			AVDictionary* options = getHttpOptions();
			av_dict_set(&options, TlsVerify, TlsVerifyValue, 0);
			av_dict_set(&options, CertFile, cert.c_str(), 0);
			av_dict_set(&options, KeyFile, key.c_str(), 0);

			return options;
		}

		std::string getResource(AVIOContext* clientContext)
		{
			uint8_t* res = nullptr;
			av_opt_get(clientContext, Resource, AV_OPT_SEARCH_CHILDREN, &res);
			
			auto resource = std::string(reinterpret_cast<const char*>(res));
			av_freep(&res);

			return resource;
		}

	} // anonymous

	HttpStreamServer::HttpStreamServer(
		const std::string& url, std::shared_ptr<StreamController> streamController) noexcept
		: m_url(url),
		m_optionsBuilder([](){ return getHttpOptions(); }),
		m_streamController(std::move(streamController)),
		m_isStarted(false)
	{
	}

	HttpStreamServer::HttpStreamServer(
		const std::string& url,
		const std::string& cert,
		const std::string& key,
		std::shared_ptr<StreamController> streamController) noexcept
		: m_url(url),
		m_cert(cert),
		m_key(key),
		m_optionsBuilder([this](){ return getHttpsOptions(m_cert, m_key); }),
		m_streamController(std::move(streamController)),
		m_isStarted(false)
	{
	}

	void HttpStreamServer::run()
	{
		// assert(("m_clientContext has already run", !m_isStarted));

		if(m_isStarted)
		{
			return;
		}
		
		m_isStarted = true;

		while (m_isStarted)
		{
			AVIOContext* clientContext = accept();
			if(clientContext)
			{
				std::cout << "New connection " << std::endl;

				int ret = 0;
				std::string resource = getResource(clientContext);

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
			else
			{
				std::cout << "Filed connect" << std::endl;
			}
		}

		m_isStarted = false;
	}

	void HttpStreamServer::createHttpStreamSession(const std::string& name, AVIOContext* clientContext)
	{
		// assert(("clientContext is not init", clientContext != nullptr));

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

	AVIOContext* HttpStreamServer::accept()
	{
		auto options = m_optionsBuilder();
		AVIOContext* clientContext = nullptr;
		if (avio_open2(&clientContext, m_url.c_str(), AVIO_FLAG_WRITE, NULL, &options) < 0)
		{
			std::cout << "avio_open2 bad" << std::endl;
			av_dict_free(&options);
			return nullptr;
		}
		return clientContext;
	}

} // video_streamer