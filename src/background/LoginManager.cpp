/*
 * Login.cpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#include "LoginManager.hpp"

LoginManager* LoginManager::instance;
bool LoginManager::heartbeatStarted = false;

LoginManager* LoginManager::getInstance()
{
	if(instance == NULL)
	{
		instance = new LoginManager();
	}
	return instance;
}

LoginManager::LoginManager() :
inUse(),
running(false),
r(R::getInstance()),
logger(Logger::getInstance()),
asyncCentral(AsyncCentral::getInstance())
{
	
}

LoginManager::~LoginManager()
{
	
}

void LoginManager::execute(bool pretry)
{	
	//Only try to login if it isn't already being done.
	{
		std::unique_lock<std::mutex> useLock(inUse);
		if(!running)
		{
			running = true;
		}
		else
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_REJECT_REQUEST), Log::TYPE::ERROR).toString());
			return;
		}
	}
	
	bool retry = pretry;
	try
	{
		std::thread networkThread([this, retry] {
			bool ok = loginFunction();
			while(retry && !ok)
			{
				sleep(RETRY_DELAY);
				ok = loginFunction();
			}
			{
				std::unique_lock<std::mutex> useLock(inUse);
				running = false;
			}
			
		});
		networkThread.detach();
	}
	catch(std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::LOGIN, error, Log::TYPE::ERROR).toString());
	}
}

bool LoginManager::loginFunction()
{
	try
	{
		Vars::commandSocket = std::make_unique<SodiumSocket>(Vars::serverAddress, Vars::commandPort, Vars::serverCert.get());
		const std::string login = std::to_string(Utils::now()) + "|login1|" + Vars::username;
		Vars::commandSocket.get()->writeString(login);

		const std::string loginChallenge = Vars::commandSocket.get()->readString();
		const std::vector<std::string> loginChallengeContents = Utils::parse((unsigned char*) loginChallenge.c_str());
		if (loginChallengeContents.size() != LOGIN_MAX_SEGMENTS)
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_FORMAT), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}
		if (loginChallengeContents[1] != "login1resp")
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_FORMAT), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}

		if (!Utils::validTS(loginChallengeContents[0]))
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_TS), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}

		const int challengeLength = loginChallengeContents[2].length() / 3;
		std::unique_ptr<unsigned char[] > challengeUCharsArray = std::make_unique<unsigned char[]>(challengeLength);
		unsigned char* challengeUChars = challengeUCharsArray.get();
		Stringify::destringify(loginChallengeContents[2], challengeUChars);

		int decryptedLength = 0;
		std::unique_ptr<unsigned char[] > decrypted = std::make_unique<unsigned char[]>(2048);
		SodiumUtils::sodiumDecrypt(true, challengeUChars, challengeLength, Vars::privateKey.get(), Vars::serverCert.get(), decrypted, decryptedLength);
		if (decryptedLength == 0)
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_CALLENGE_FAILED), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}
		const std::string keepudp = (Vars::ustate == Vars::UserState::INCALL) ? "|keepudp" : "";
		const std::string challengeDec((char*) decrypted.get(), decryptedLength);
		const std::string loginChallengeResponse = std::to_string(Utils::now()) + "|login2|" + Vars::username + "|" + challengeDec + keepudp;
		Vars::commandSocket.get()->writeString(loginChallengeResponse);

		const std::string answerResponse = Vars::commandSocket.get()->readString();
		const std::vector<std::string> answerResponseContents = Utils::parse((unsigned char*) answerResponse.c_str());
		if (answerResponseContents.size() != LOGIN_MAX_SEGMENTS)
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_FORMAT), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}
		if (answerResponseContents[1] != "login2resp")
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_FORMAT), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}

		if (!Utils::validTS(answerResponseContents[0]))
		{
			logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_TS), Log::TYPE::ERROR).toString());
			asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}

		Vars::sessionKey = answerResponseContents[2];
		asyncCentral->broadcast(Vars::Broadcast::LOGIN_OK);
		CmdListener::startService();
		if (!heartbeatStarted)
		{
			heartbeatStarted = true;
			Heartbeat::startService();
		}
		return true;
	}
	catch (std::exception& e)
	{
		logger->insertLog(Log(Log::TAG::LOGIN, std::string(e.what()), Log::TYPE::ERROR).toString());
		asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
		return false;
	}
	catch (std::string& e)
	{
		logger->insertLog(Log(Log::TAG::LOGIN, e, Log::TYPE::ERROR).toString());
		asyncCentral->broadcast(Vars::Broadcast::LOGIN_NOTOK);
		return false;
	}
	return false;
}