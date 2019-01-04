/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "CommandAccept.hpp"

namespace
{
	Logger* logger = Logger::getInstance("");
	R* r = R::getInstance();

	void* callAcceptThread(void* pointer)
	{
		try
		{
			const std::string resp = std::to_string(Utils::now()) + "|accept|" + Vars::callWith + "|" + Vars::sessionKey;
			logger->insertLog(Log(Log::TAG::CMD_ACCEPT, resp, Log::TYPE::OUTBOUND).toString());
			Vars::commandSocket.writeString(resp);
			UserHome::instance->asyncResult(Vars::Broadcast::LOCK_USERHOME);
		}
		catch(std::string& e)
		{
			logger->insertLog(Log(Log::TAG::CMD_ACCEPT, e, Log::TYPE::ERROR).toString());
		}
		return 0;
	}
}

void CommandAccept::execute()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, callAcceptThread, NULL) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CMD_ACCEPT, error, Log::TYPE::ERROR).toString());
	}
}

