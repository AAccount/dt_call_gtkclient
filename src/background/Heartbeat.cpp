/*
 * Heartbeat.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "Heartbeat.hpp"

void Heartbeat::startService()
{
	Logger* logger = Logger::getInstance();
	R* r = R::getInstance();
	try
	{
		std::thread asyncThread([logger, r] {
			const int TIMEOUT = 60 * 5;
			const std::string PING = "D";
			while (true)
			{
				try
				{
					if(Vars::isExiting)
					{
						break;
					}
					Vars::commandSocket.get()->writeString(PING);
					logger->insertLog(Log(Log::TAG::HEARTBEAT, PING, Log::TYPE::OUTBOUND).toString());
					sleep(TIMEOUT);
				}
				catch (std::string& e)
				{
					const std::string error = r->getString(R::StringID::HEARTBEAT_FAIL) + e;
					logger->insertLog(Log(Log::TAG::HEARTBEAT, error, Log::TYPE::ERROR).toString());
					Vars::commandSocket.get()->stop();
					LoginManager::getInstance()->execute(UserHome::getInstance(), true);
					break;
				}
			}
		});
		asyncThread.detach();
	}
	catch (std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::HEARTBEAT, error, Log::TYPE::ERROR).toString());
	}
}

