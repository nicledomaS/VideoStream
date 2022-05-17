#pragma once

#include "StreamServer.h"

#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>

struct AVIOContext;
struct AVDictionary;

namespace video_streamer
{

class StreamController;

class HttpStreamServer: public StreamServer
{
public:
	HttpStreamServer(
		const std::string& url,	std::shared_ptr<StreamController> streamController) noexcept;
		
	HttpStreamServer(
		const std::string& url,
		const std::string& cert,
		const std::string& key,
		std::shared_ptr<StreamController> streamController) noexcept;

	void run() override;

private:
	HttpStreamServer(const HttpStreamServer&) = delete;
	HttpStreamServer& operator=(const HttpStreamServer&) = delete;
	HttpStreamServer(HttpStreamServer&&) = delete;
	HttpStreamServer& operator=(HttpStreamServer&&) = delete;

	AVIOContext* accept();

	void createHttpStreamSession(const std::string& name, AVIOContext* clientContext);

	std::string m_url;
	std::string m_cert;
	std::string m_key;
	std::function<AVDictionary*()> m_optionsBuilder;
	std::shared_ptr<StreamController> m_streamController;
	std::atomic_bool m_isStarted;
};

std::unique_ptr<HttpStreamServer> createHttpStreamServer(
	const std::string& url, std::shared_ptr<StreamController> streamController);

} // video_streamer