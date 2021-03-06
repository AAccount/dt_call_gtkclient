/*
 * CommandAccept.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "OperatorCommand.hpp"

void OperatorCommand::execute(OperatorCommand which)
{
	Logger* logger = Logger::getInstance();
	R* r = R::getInstance();
	
	try
	{
		std::thread asyncThread([logger, which, r] {
			std::string command;
			switch(which)
			{
				case OperatorCommand::ACCEPT:
					command = "|accept|";
					break;
				case OperatorCommand::CALL:
					command = "|call|";
					break;
				case OperatorCommand::END:
					if(Vars::callWith == "")
					{
						return;
					}
					command = "|end|";
					break;
				default:
					logger->insertLog(Log(Log::TAG::OPERATOR_COMMAND, r->getString(R::StringID::OPERATOR_COMMAND_BAD) + std::to_string(which), Log::TYPE::ERROR).toString());
					return;
			}
				
			const std::string respBase = std::to_string(Utils::now()) + command + Vars::callWith + "|";
			const std::string resp = respBase + Vars::sessionKey;
			logger->insertLog(Log(Log::TAG::OPERATOR_COMMAND, respBase+"...", Log::TYPE::OUTBOUND).toString());
			try
			{
				Vars::commandSocket.get()->writeString(resp);
			}
			catch(std::string& e)
			{
				logger->insertLog(Log(Log::TAG::OPERATOR_COMMAND, e, Log::TYPE::ERROR).toString());
			}
			
			switch(which)
			{
				case OperatorCommand::ACCEPT:
					AsyncCentral::getInstance()->broadcast(Vars::Broadcast::USERHOME_LOCK);
					break;
				case OperatorCommand::END:
					Vars::callWith = "";
					break;
				case OperatorCommand::CALL:
				default:
					//call requires no special "after sending" procedures
					break;
			}
		});
		asyncThread.detach();
	}
	catch(std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::OPERATOR_COMMAND, error, Log::TYPE::ERROR).toString());
	}

}

