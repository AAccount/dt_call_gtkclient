/*
 * Heartbeat.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "Heartbeat.hpp"

namespace
{
	Logger* logger = Logger::getInstance("");
	R* r = R::getInstance();
	void* heartbeatThread(void* pointer)
	{
		const int TIMEOUT = 60*5;
		const std::string ping = "D";
		while(true)
		{
			try
			{
				Vars::commandSocket.writeString(ping);
				logger->insertLog(Log(Log::TAG::HEARTBEAT, ping, Log::TYPE::OUTBOUND).toString());
				sleep(TIMEOUT);
			}
			catch(std::string& e)
			{
				const std::string error = r->getString(R::StringID::HEARTBEAT_FAIL) + e;
				logger->insertLog(Log(Log::TAG::HEARTBEAT, error, Log::TYPE::ERROR).toString());
				Vars::commandSocket.stop();
				LoginAsync::execute(UserHome::instance, true);
				break;
			}
		}
		return 0;
	}
}

void Heartbeat::startService()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, heartbeatThread, NULL) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::HEARTBEAT, error, Log::TYPE::ERROR).toString());
	}
}

