#pragma once

#include "StreamServer.h"

#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

struct AVIOContext;

namespace video_streamer
{

class StreamController;

class HttpStreamServer: public StreamServer
{
public:
	HttpStreamServer(AVIOContext* serverContext, std::shared_ptr<StreamController> streamController) noexcept;
	~HttpStreamServer() noexcept;

	void run() override;

private:
	HttpStreamServer(const HttpStreamServer&) = delete;
	HttpStreamServer& operator=(const HttpStreamServer&) = delete;
	HttpStreamServer(HttpStreamServer&&) = delete;
	HttpStreamServer& operator=(HttpStreamServer&&) = delete;

	void createHttpStreamSession(const std::string& name, AVIOContext* clientContext);

	AVIOContext* m_serverContext;
	std::shared_ptr<StreamController> m_streamController;
	std::atomic_bool m_isStarted;
};

std::unique_ptr<HttpStreamServer> createHttpStreamServer(
	const std::string& url, std::shared_ptr<StreamController> streamController);

} // video_streamer