/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 31, 2018
 *      Author: Daniel
 */

#include "CommandCall.hpp"

namespace
{
	Logger* logger = Logger::getInstance("");
	R* r = R::getInstance();
	void* callCommandThread(void* pointer)
	{
		try
		{
			const std::string resp = std::to_string(Utils::now()) + "|call|" + Vars::callWith + "|" + Vars::sessionKey;
			logger->insertLog(Log(Log::TAG::CMD_CALL, resp, Log::TYPE::OUTBOUND).toString());
			Vars::commandSocket.writeString(resp);
		}
		catch(std::string& e)
		{
			logger->insertLog(Log(Log::TAG::CMD_CALL, e, Log::TYPE::ERROR).toString());
		}
		return 0;
	}
}

void CommandCall::execute()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, callCommandThread, NULL) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CMD_CALL, error, Log::TYPE::ERROR).toString());
	}
}

