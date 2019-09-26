/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "CommandAccept.hpp"

void CommandAccept::execute()
{
	Logger* logger = Logger::getInstance();
	R* r = R::getInstance();
	
	try
	{
		std::thread asyncThread([logger] {
			try
			{
				const std::string respBase = std::to_string(Utils::now()) + "|accept|" + Vars::callWith + "|";
				const std::string resp = respBase + Vars::sessionKey;
				logger->insertLog(Log(Log::TAG::CMD_ACCEPT, respBase+"...", Log::TYPE::OUTBOUND).toString());
				Vars::commandSocket.get()->writeString(resp);
				AsyncCentral::getInstance()->broadcast(Vars::Broadcast::USERHOME_LOCK);
			}
			catch(std::string& e)
			{
				logger->insertLog(Log(Log::TAG::CMD_ACCEPT, e, Log::TYPE::ERROR).toString());
			}
		});
		asyncThread.detach();
	}
	catch(std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CMD_ACCEPT, error, Log::TYPE::ERROR).toString());
	}

}

