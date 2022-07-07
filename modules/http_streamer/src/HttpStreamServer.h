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

class StreamChannel;

class HttpStreamServer: public StreamServer
{
public:
	HttpStreamServer(
		const std::string& url,	const std::shared_ptr<StreamChannel>& streamChannel) noexcept;
		
	HttpStreamServer(
		const std::string& url,
		const std::string& cert,
		const std::string& key,
		const std::shared_ptr<StreamChannel>& streamChannel) noexcept;

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
	std::shared_ptr<StreamChannel> m_streamChannel;
	std::atomic_bool m_isStarted;
};

} // video_streamer