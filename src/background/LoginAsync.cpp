/*
 * Login.cpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#include "LoginAsync.hpp"

namespace
{
	const int RETRY_DELAY = 60;
	const int LOGIN_MAX_SEGMENTS = 3;
	bool retry = false;
	R* r;
	Logger* logger;

	static pthread_mutex_t inUse;
	static bool running = false;
	static bool heartbeatStarted = false;

	bool loginFunction(AsyncReceiver* receiver)
	{
		r = R::getInstance();
		logger = Logger::getInstance("");
		try
		{
			Vars::commandSocket = SodiumSocket(Vars::serverAddress, Vars::commandPort, Vars::serverCert.get());
			const std::string login = std::to_string(Utils::now()) + "|login1|" + Vars::username;
			Vars::commandSocket.writeString(login);

			const std::string loginChallenge = Vars::commandSocket.readString();
			const std::vector<std::string> loginChallengeContents = Utils::parse((unsigned char*) loginChallenge.c_str());
			if (loginChallengeContents.size() != LOGIN_MAX_SEGMENTS)
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_FORMAT), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}
			if (loginChallengeContents[1] != "login1resp")
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_FORMAT), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}

			if (!Utils::validTS(loginChallengeContents[0]))
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN1_TS), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}

			const int challengeLength = loginChallengeContents[2].length() / 3;
			std::unique_ptr<unsigned char[]> challengeUCharsArray = std::make_unique<unsigned char[]>(challengeLength);
			unsigned char* challengeUChars = challengeUCharsArray.get();
			Stringify::destringify(loginChallengeContents[2], challengeUChars);

			int decryptedLength = 0;
			std::unique_ptr<unsigned char[]> decrypted;
			SodiumUtils::sodiumDecrypt(true, challengeUChars, challengeLength, Vars::privateKey.get(), Vars::serverCert.get(), decrypted, decryptedLength);
			if (decryptedLength == 0)
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_CALLENGE_FAILED), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}
			const std::string keepudp = (Vars::ustate == Vars::UserState::INCALL) ? "|keepudp" : "";
			const std::string challengeDec((char*) decrypted.get(), decryptedLength);
			const std::string loginChallengeResponse = std::to_string(Utils::now()) + "|login2|" + Vars::username + "|" + challengeDec + keepudp;
			Vars::commandSocket.writeString(loginChallengeResponse);

			const std::string answerResponse = Vars::commandSocket.readString();
			const std::vector<std::string> answerResponseContents = Utils::parse((unsigned char*) answerResponse.c_str());
			if (answerResponseContents.size() != LOGIN_MAX_SEGMENTS)
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_FORMAT), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}
			if (answerResponseContents[1] != "login2resp")
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_FORMAT), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}

			if (!Utils::validTS(answerResponseContents[0]))
			{
				logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_LOGIN2_TS), Log::TYPE::ERROR).toString());
				receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
				return false;
			}

			Vars::sessionKey = answerResponseContents[2];
			receiver->asyncResult(Vars::Broadcast::LOGIN_OK);
			CmdListener::startService();
			if(!heartbeatStarted)
			{
				heartbeatStarted = true;
				Heartbeat::startService();
			}
			return true;
		}
		catch(std::exception& e)
		{
			logger->insertLog(Log(Log::TAG::LOGIN, std::string(e.what()), Log::TYPE::ERROR).toString());
			receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}
		catch(std::string& e)
		{
			logger->insertLog(Log(Log::TAG::LOGIN, e, Log::TYPE::ERROR).toString());
			receiver->asyncResult(Vars::Broadcast::LOGIN_NOTOK);
			return false;
		}
		return false;
	}

	void* loginThread(void* context)
	{
		AsyncReceiver* receiver = static_cast<AsyncReceiver*>(context);
		bool ok = loginFunction(receiver);
		while(retry && !ok)
		{
			sleep(RETRY_DELAY);
			ok = loginFunction(receiver);
		}
		pthread_mutex_lock(&inUse);
			running = false;
		pthread_mutex_unlock(&inUse);
		return 0;
	}
}

void LoginAsync::init()
{
	pthread_mutex_init(&inUse, NULL);
}

void LoginAsync::execute(AsyncReceiver* receiver, bool pretry)
{
	bool willDo = false;
	pthread_mutex_lock(&inUse);
		if(!running)
		{
			running = true;
			willDo = true;
		}
	pthread_mutex_unlock(&inUse);
	if(!willDo)
	{
		logger->insertLog(Log(Log::TAG::LOGIN, r->getString(R::StringID::LOGINASYNC_REJECT_REQUEST), Log::TYPE::ERROR).toString());
		return;
	}

	retry = pretry;
	pthread_t thread;
	if(pthread_create(&thread, NULL, loginThread, receiver) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::LOGIN, error, Log::TYPE::ERROR).toString());
	}
}
