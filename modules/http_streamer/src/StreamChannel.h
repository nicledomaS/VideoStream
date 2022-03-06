#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

struct AVFrame;

namespace video_streamer
{

	class GroupStreamSession;
	class StreamSession;
	class FrameEncoder;

	class StreamChannel
	{
	public:
		void createGroup(const std::string& name, std::shared_ptr<FrameEncoder> frameEncoder);
		void removeGroup(const std::string& name);

		void addSession(const std::string& name, std::unique_ptr<StreamSession> session);
		void notify(const std::string& name, const AVFrame* frame);

	private:
		std::shared_ptr<GroupStreamSession> findGroup(const std::string& name);

		std::unordered_map<std::string, std::shared_ptr<GroupStreamSession>> m_sessions;
		mutable std::mutex m_mtx;
	};

} // video_streamer