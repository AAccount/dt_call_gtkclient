/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 31, 2018
 *      Author: Daniel
 */

#include "CommandCall.hpp"

void CommandCall::execute()
{
	Logger* logger = Logger::getInstance();
	R* r = R::getInstance();
	try
	{
		std::thread asyncThread([logger] {
			try
			{
				const std::string resp = std::to_string(Utils::now()) + "|call|" + Vars::callWith + "|" + Vars::sessionKey;
				logger->insertLog(Log(Log::TAG::CMD_CALL, resp, Log::TYPE::OUTBOUND).toString());
				Vars::commandSocket.get()->writeString(resp);
			}
			catch (std::string& e)
			{
				logger->insertLog(Log(Log::TAG::CMD_CALL, e, Log::TYPE::ERROR).toString());
			}
		});
		asyncThread.detach();
	}
	catch(std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CMD_CALL, error, Log::TYPE::ERROR).toString());
	}
}

