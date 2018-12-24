/*
 * Login.cpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#include "LoginAsync.hpp"

namespace
{
	const int LOGIN_MAX_SEGMENTS = 3;
	void* loginThread(void* context)
	{
		StringRes* strings = StringRes::getInstance();
		AsyncReceiver* receiver = (AsyncReceiver*)context;
		try
		{
			Vars::commandSocket = SodiumSocket(Vars::serverAddress, Vars::commandPort, Vars::serverCert.get());
			const std::string login = std::to_string(Utils::now()) + "|login1|" + Vars::username;
			Vars::commandSocket.writeString(login);

			const std::string loginChallenge = Vars::commandSocket.readString();
			const std::vector<std::string> loginChallengeContents = Utils::parse((unsigned char*) loginChallenge.c_str());
			if (loginChallengeContents.size() != LOGIN_MAX_SEGMENTS)
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN1_FORMAT) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}
			if (loginChallengeContents[1] != "login1resp")
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN1_FORMAT) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}

			const time_t ts1 = std::stoull(loginChallengeContents[0]);
			if (!Utils::validTS(ts1))
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN1_TS) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}

			const int challengeLength = loginChallengeContents[2].length() / 3;
			unsigned char challengeUChars[challengeLength] = {};
			Stringify::destringify(loginChallengeContents[2], challengeUChars);

			int decryptedLength = 0;
			std::unique_ptr<unsigned char[]> decrypted;
			SodiumUtils::sodiumDecrypt(true, challengeUChars, challengeLength, Vars::privateKey.get(), Vars::serverCert.get(), decrypted, decryptedLength);
			if (decryptedLength == 0)
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_CALLENGE_FAILED) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}
			const std::string challengeDec((char*) decrypted.get(), decryptedLength);
			//TODO: keep udp flag
			const std::string loginChallengeResponse = std::to_string(Utils::now()) + "|login2|" + Vars::username + "|" + challengeDec;
			Vars::commandSocket.writeString(loginChallengeResponse);

			const std::string answerResponse = Vars::commandSocket.readString();
			const std::vector<std::string> answerResponseContents = Utils::parse((unsigned char*) answerResponse.c_str());
			if (answerResponseContents.size() != LOGIN_MAX_SEGMENTS)
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN2_FORMAT) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}
			if (answerResponseContents[1] != "login2resp")
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN2_FORMAT) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}

			const time_t ts2 = std::stoull(answerResponseContents[0]);
			if (!Utils::validTS(ts2))
			{
				std::cerr << strings->getString(Vars::lang, StringRes::StringID::LOGINASYNC_LOGIN2_TS) << "\n";
				receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
				return 0;
			}

			Vars::sessionKey = answerResponseContents[2];
			receiver->asyncResult(LoginAsync::LoginResult::LOGIN_OK);
		}
		catch(std::exception& e)
		{
			std::cerr << "exception in login async: " << e.what() << "\n";
			receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
			return 0;
		}
		catch(std::string& e)
		{
			std::cerr << "my exception: " << e << "\n";
			receiver->asyncResult(LoginAsync::LoginResult::LOGIN_NOTOK);
			return 0;
		}
		return 0;
	}
}


void LoginAsync::execute(AsyncReceiver* receiver)
{
	pthread_t callThread;
	if(pthread_create(&callThread, NULL, loginThread, receiver) != 0)
	{
		std::string error = "cannot create the login async thread (" + std::to_string(errno) + ") " + std::string(strerror(errno));
		std::cout << error << "\n";
	}
}

AsyncReceiver::~AsyncReceiver()
{

}
