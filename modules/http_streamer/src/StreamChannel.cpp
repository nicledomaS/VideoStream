#include "StreamChannel.h"
#include "StreamSession.h"
#include "GroupStreamSession.h"

#include <iostream>

namespace video_streamer
{

std::shared_ptr<GroupStreamSession> StreamChannel::createGroup(const std::string& name, std::shared_ptr<Encoder> frameEncoder)
{
	std::lock_guard lock(m_mtx);
	auto groupStream = std::make_shared<GroupStreamSession>(std::move(frameEncoder));
	m_sessions.insert({ name, groupStream });

	return groupStream;
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

bool StreamChannel::hasGroup(const std::string& name) const
{
	return m_sessions.count(name);
}

std::shared_ptr<GroupStreamSession> StreamChannel::findGroup(const std::string& name)
{
	std::unique_lock lock(m_mtx);
	const auto& it = m_sessions.find(name);
	return it != m_sessions.end() ? it->second : nullptr;
}

} // video_streamer