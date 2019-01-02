/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "CommandEnd.hpp"

namespace
{
	Logger* logger = Logger::getInstance("");
	R* r = R::getInstance();
	void* callEndThread(void* pointer)
	{
		try
		{
			const std::string resp = std::to_string(Utils::now()) + "|end|" + Vars::callWith + "|" + Vars::sessionKey;
			logger->insertLog(Log(Log::TAG::CMD_END, resp, Log::TYPE::OUTBOUND).toString());
			Vars::commandSocket.writeString(resp);
		}
		catch(std::string& e)
		{
			logger->insertLog(Log(Log::TAG::CMD_END, e, Log::TYPE::ERROR).toString());
		}
		return 0;
	}
}

void CommandEnd::execute()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, callEndThread, NULL) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CMD_END, error, Log::TYPE::ERROR).toString());
	}
}

