#include "StreamChannel.h"
#include "StreamSession.h"
#include "GroupStreamSession.h"

#include <iostream>

namespace video_streamer
{

void StreamChannel::createGroup(const std::string& name, std::shared_ptr<FrameEncoder> frameEncoder)
{
	std::lock_guard lock(m_mtx);
	m_sessions.insert({ name, std::make_shared<GroupStreamSession>(std::move(frameEncoder)) });
}

void StreamChannel::removeGroup(const std::string& name)
{
	std::lock_guard lock(m_mtx);
	m_sessions.erase(name);
}

void StreamChannel::addSession(const std::string& name, std::unique_ptr<StreamSession> session)
{
	auto groupStream = findGroup(name);
	if (groupStream)
	{
		groupStream->addStream(std::move(session));
	}
	else
	{
		// log warn
		std::cout << "addSession: StreamGroup was not found" << std::endl;
	}
}

void StreamChannel::notify(const std::string& name, const AVFrame* frame)
{
	auto groupStream = findGroup(name);
	if (groupStream)
	{
		groupStream->pushFrame(frame);
	}
	else
	{
		// log warn
		std::cout << "notify: StreamGroup was not found" << std::endl;
	}
}

std::shared_ptr<GroupStreamSession> StreamChannel::findGroup(const std::string& name)
{
	std::unique_lock lock(m_mtx);
	const auto& it = m_sessions.find(name);
	return it != m_sessions.end() ? it->second : nullptr;
}

} // video_streamer