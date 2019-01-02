/*
 * CmdListener.cpp
 *
 *  Created on: Dec 27, 2018
 *      Author: Daniel
 */

#include "CmdListener.hpp"

namespace
{
	const int COMMAND_MAX_SEGMENTS = 5;
	const int UDP_RETRIES = 10;
	bool isCallInitiator = false;
	bool haveVoiceKey = false;
	bool preparationsComplete = false;

	R* r;
	Logger* logger;

	void sendReady();
	void giveUp();
	void* serviceThread(void* context)
	{
		r = R::getInstance();
		logger = Logger::getInstance("");

		logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_START), Log::TYPE::INFO).toString());
		bool inputValid = true;
		while(inputValid)
		{
			//responses from the server command connection will always be in text format
			//timestamp|available|other_person
			//timestamp|incoming|trying_to_call
			//timestamp|start|other_person
			//timestamp|end|other_person
			//timestamp|prepare|public key|other_person
			//timestamp|direct|(encrypted aes key)|other_person
			//timestamp|invalid

			try
			{
				const std::string fromServer = Vars::commandSocket.readString();
				const std::string fromServerCopy = fromServer;
				logger->insertLog(Log(Log::TAG::CMD_LISTENER, fromServerCopy, Log::TYPE::INBOUND).toString());
				const std::vector<std::string> respContents = Utils::parse((unsigned char*)fromServer.c_str());
				if(respContents.size() > COMMAND_MAX_SEGMENTS)
				{
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_TOOMANY_SEGMENTS) + std::to_string(respContents.size()), Log::TYPE::ERROR).toString());
					continue;
				}

				if(!Utils::validTS(respContents.at(0)))
				{
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_BADTS), Log::TYPE::ERROR).toString());
					continue;
				}

				const std::string command = respContents.at(1);
				const std::string involved = respContents.at(respContents.size()-1);

				if(command == "incoming")
				{
					Vars::ustate = Vars::UserState::INIT;
					isCallInitiator = false;
					haveVoiceKey = false;
					preparationsComplete = false;
					Vars::callWith = involved;

					CallScreen::mode = CallScreen::Mode::RECEIVING;
					CallScreen::render();
					continue;
				}

				if(!(involved == Vars::callWith))
				{
					const std::string error = r->getString(R::StringID::CMDLISTENER_WRONG_OTHER) + Vars::callWith+"/"+involved;
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, error, Log::TYPE::ERROR).toString());
					continue;
				}

				if(command == "available")
				{
					Vars::ustate = Vars::UserState::INIT;
					isCallInitiator = true;
					haveVoiceKey = false;
					preparationsComplete = false;
					UserHome::instance->asyncResult(Vars::Broadcast::CALL_TRY);
				}
				else if(command == "prepare")
				{
					const std::string receivedKeyDump = respContents.at(2);
					const std::string receivedKeyCore = receivedKeyDump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES*3);
					std::unique_ptr<unsigned char[]> receivedKey = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
					Stringify::destringify(receivedKeyCore, receivedKey.get());
					std::unique_ptr<unsigned char[]> expectedKey;
					Settings::getInstance()->getPublicKey(Vars::callWith, expectedKey);
					if(expectedKey.get() != NULL)
					{
						if(!memcmp(receivedKey.get(), expectedKey.get(), crypto_box_PUBLICKEYBYTES))
						{
							const std::string expecting = Stringify::stringify(expectedKey.get(), crypto_box_PUBLICKEYBYTES);
							const std::string got = Stringify::stringify(receivedKey.get(), crypto_box_PUBLICKEYBYTES);
							const std::string error = r->getString(R::StringID::CMDLISTENER_PUBLICKEY_MISMATCH) + Vars::callWith + ") " + expecting + "/" + got;
							logger->insertLog(Log(Log::TAG::CMD_LISTENER, error, Log::TYPE::ERROR).toString());
							giveUp();
							continue;
						}
					}
					else
					{
						Settings::getInstance()->modifyPublicKey(Vars::callWith, receivedKey);
						Settings::getInstance()->save();
						expectedKey = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
						memcpy(expectedKey.get(), receivedKey.get(), crypto_box_PUBLICKEYBYTES);
					}

					if(isCallInitiator)
					{
						Vars::voiceKey = std::make_unique<unsigned char[]>(crypto_secretbox_KEYBYTES);
						randombytes_buf(Vars::voiceKey.get(), crypto_secretbox_KEYBYTES);
						std::unique_ptr<unsigned char[]> yourPublic;
						Settings::getInstance()->getPublicKey(Vars::callWith, yourPublic);
						std::unique_ptr<unsigned char[]> output;
						int outputLength;
						SodiumUtils::sodiumEncrypt(true, Vars::voiceKey.get(), crypto_secretbox_KEYBYTES, Vars::privateKey.get(), yourPublic.get(), output, outputLength);
						const std::string outputStringified = Stringify::stringify(output.get(), outputLength);

						const std::string passthrough = std::to_string(Utils::now()) + "|passthrough|" + Vars::callWith + "|" + outputStringified + "|" + Vars::sessionKey;
						Vars::commandSocket.writeString(passthrough);
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, passthrough, Log::TYPE::OUTBOUND).toString());
					}

					const bool registeredUDP = CmdListener::registerUDP();
					if(registeredUDP)
					{
						preparationsComplete = true;
						sendReady();
					}
					else
					{
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::ERR_UDP_REGISTRATION_FAILED), Log::TYPE::ERROR).toString());
						giveUp();
					}
				}
				else if(command == "direct")
				{
					const std::string setupString = respContents.at(2);
					const int setupDesgringifiedLength = setupString.length() / 3;
					unsigned char setup[setupDesgringifiedLength] = {};
					std::unique_ptr<unsigned char[]> callWithKey;
					Settings::getInstance()->getPublicKey(Vars::callWith, callWithKey);
					std::unique_ptr<unsigned char[]> voiceKeyDecrypted;
					int voiceKeyDecLength = 0;
					SodiumUtils::sodiumDecrypt(false, setup, setupDesgringifiedLength, Vars::privateKey.get(), callWithKey.get(), Vars::voiceKey, voiceKeyDecLength);

					if(voiceKeyDecLength != 0)
					{
						haveVoiceKey = true;
						sendReady();
					}
					else
					{
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_PASSTHROUGH_FAIL), Log::TYPE::ERROR).toString());
						giveUp();
						continue;
					}
				}
				else if(command == "start")
				{
					Vars::ustate = Vars::UserState::INCALL;
					CallScreen::instance->asyncResult(Vars::Broadcast::CALL_START);
				}

				else if(command == "end")
				{
					Vars::UserState oldState = Vars::ustate;
					Vars::ustate = Vars::UserState::NONE;
					Vars::callWith = "";

					if(oldState == Vars::UserState::NONE)
					{//won't be in the phone call screen yet. tell user home the call can't be made
						UserHome::instance->asyncResult(Vars::Broadcast::CALL_END);
					}
					else //INIT or INCALL
					{
						UserHome::instance->asyncResult(Vars::Broadcast::UNLOCK_USERHOME);
						CallScreen::instance->asyncResult(Vars::Broadcast::CALL_END);
					}
				}

			}
			catch(std::string& e)
			{
				inputValid = false;
				logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_IOERROR), Log::TYPE::ERROR).toString());
				Vars::commandSocket.stop();
				LoginAsync::execute(UserHome::instance, true);
			}
			catch(std::out_of_range& e)
			{
				logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_OORANGE), Log::TYPE::ERROR).toString());
			}
		}
		return NULL;
	}

	void sendReady()
	{
		if(haveVoiceKey && preparationsComplete)
		{
			const std::string ready = std::to_string(Utils::now()) + "|ready|" + Vars::callWith + "|" + Vars::sessionKey;
			try
			{
				Vars::commandSocket.writeString(ready);
				logger->insertLog(Log(Log::TAG::CMD_LISTENER, ready, Log::TYPE::OUTBOUND).toString());
			}
			catch(std::string& e)
			{
				giveUp();
				throw e;
			}
		}
	}

	void giveUp()
	{
		CommandEnd::execute();
		CallScreen::instance->asyncResult(Vars::Broadcast::CALL_END);
	}
}

void CmdListener::startService()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, serviceThread, NULL) != 0)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CMD_LISTENER, error, Log::TYPE::ERROR).toString());
	}
}

bool CmdListener::registerUDP()
{
	struct sockaddr_in serv_addr;
	bool udpConnected = Utils::connectFD(Vars::mediaSocket, SOCK_DGRAM, Vars::serverAddress, Vars::mediaPort, &serv_addr);
	if(!udpConnected)
	{
		return false;
	}

	int retries = UDP_RETRIES;
	while(retries > 0)
	{
		const std::string registration = std::to_string(Utils::now()) + "|" + Vars::sessionKey;
		const int registrationLength = crypto_box_SEALBYTES + registration.length();
		unsigned char sodiumSealedRegistration[registrationLength];
		const int sealed = crypto_box_seal(sodiumSealedRegistration, (unsigned char*)registration.c_str(), registration.length(), Vars::serverCert.get());
		if(!sealed)
		{
			logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_REGISTERUDP_SEALFAIL), Log::TYPE::ERROR).toString());
			retries--;
			continue;
		}

		const int sent = sendto(Vars::mediaSocket, sodiumSealedRegistration, registrationLength, 0, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in));
		if(sent < 0)
		{
			retries--;
			continue;
		}

		//TODO: standardize max_udp
		unsigned char ackBuffer[1500] = {};
		struct sockaddr_in sender;
		socklen_t senderLength = sizeof(struct sockaddr_in);
		const int receivedLength = recvfrom(Vars::mediaSocket, ackBuffer, 1500, 0, (struct sockaddr*)&sender, &senderLength);
		if(receivedLength < 0)
		{
			retries--;
			continue;
		}

		std::unique_ptr<unsigned char[]> decAck;
		std::unique_ptr<unsigned char[]> tcpKey;
		Vars::commandSocket.getTcpKeyCopy(tcpKey);
		int decAckLength = 0;
		SodiumUtils::sodiumDecrypt(false, ackBuffer, receivedLength, tcpKey.get(), NULL, decAck, decAckLength);
		if(decAckLength == 0)
		{
			retries--;
			continue;
		}

		const std::string ackString((char*)decAck.get(), decAckLength);
		const bool ackOK = Utils::validTS(ackString);
		if(ackOK)
		{
			return true;
		}
		retries--;
	}
	return false;
}
