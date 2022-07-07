#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace video_streamer
{

	class GroupStreamSession;
	class StreamSession;
	class Encoder;

	class StreamChannel
	{
	public:
		std::shared_ptr<GroupStreamSession> createGroup(const std::string& name, std::shared_ptr<Encoder> frameEncoder);
		void removeGroup(const std::string& name);
		void addSession(const std::string& name, std::unique_ptr<StreamSession> session);
		bool hasGroup(const std::string& name) const;

	private:
		std::shared_ptr<GroupStreamSession> findGroup(const std::string& name);

		std::unordered_map<std::string, std::shared_ptr<GroupStreamSession>> m_sessions;
		mutable std::mutex m_mtx;
	};

} // video_streamer