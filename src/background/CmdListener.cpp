/*
 * CmdListener.cpp
 *
 *  Created on: Dec 27, 2018
 *      Author: Daniel
 */

#include "CmdListener.hpp"

CmdListener* CmdListener::instance;

CmdListener::CmdListener() :
isCallInitiator(false),
haveVoiceKey(false),
preparationsComplete(false),
r(R::getInstance()),
logger(Logger::getInstance()),
asyncCentral(AsyncCentral::getInstance())
{
	
}

CmdListener::~CmdListener()
{
	
}

//static
void CmdListener::startService()
{
	if(instance != NULL)
	{
		delete instance;
	}
	instance = new CmdListener();
	instance->startInternal();	
}

void CmdListener::startInternal()
{
	try
	{
		std::thread serviceThread([this] {

			logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_START), Log::TYPE::INFO).toString());
			bool inputValid = true;
			while (inputValid)
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
					const std::string fromServer = Vars::commandSocket.get()->readString();
					const std::vector<std::string> respContents = Utils::parse((unsigned char*) fromServer.c_str());
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, censorIncomingCmd(respContents), Log::TYPE::INBOUND).toString());
					if (respContents.size() > COMMAND_MAX_SEGMENTS)
					{
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_TOOMANY_SEGMENTS) + std::to_string(respContents.size()), Log::TYPE::ERROR).toString());
						continue;
					}

					if (!Utils::validTS(respContents.at(0)))
					{
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_BADTS), Log::TYPE::ERROR).toString());
						continue;
					}

					const std::string command = respContents.at(1);
					const std::string involved = respContents.at(respContents.size() - 1);

					if (command == "incoming")
					{
						Vars::ustate = Vars::UserState::INIT;
						isCallInitiator = false;
						haveVoiceKey = false;
						preparationsComplete = false;
						Vars::callWith = involved;

						CallScreen::mode = CallScreen::Mode::RECEIVING;
						Utils::runOnUiThread(&CallScreen::render);
						continue;
					}

					if (!(involved == Vars::callWith))
					{
						const std::string error = r->getString(R::StringID::CMDLISTENER_WRONG_OTHER) + Vars::callWith + "/" + involved;
						logger->insertLog(Log(Log::TAG::CMD_LISTENER, error, Log::TYPE::ERROR).toString());
						continue;
					}

					if (command == "available")
					{
						Vars::ustate = Vars::UserState::INIT;
						isCallInitiator = true;
						preparationsComplete = false;
						asyncCentral->broadcast(Vars::Broadcast::CALL_TRY);
						asyncCentral->broadcast(Vars::Broadcast::USERHOME_LOCK);
					}
					else if (command == "prepare")
					{
						const std::string receivedKeyDump = respContents.at(2);
						const std::string receivedKeyCore = receivedKeyDump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES * 3);
						std::unique_ptr<unsigned char[] > receivedKey = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
						Stringify::destringify(receivedKeyCore, receivedKey.get());
						std::unique_ptr<unsigned char[] > expectedKey;
						Settings::getInstance()->getPublicKey(Vars::callWith, expectedKey);
						if (expectedKey.get() != NULL)
						{
							const int compare = memcmp(receivedKey.get(), expectedKey.get(), crypto_box_PUBLICKEYBYTES);
							if (compare != 0)
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

						if (isCallInitiator)
						{
							std::unique_ptr<unsigned char[]> voiceKey = std::make_unique<unsigned char[]>(crypto_secretbox_KEYBYTES);
							std::unique_ptr<unsigned char[] > yourPublic;
							Settings::getInstance()->getPublicKey(Vars::callWith, yourPublic);
							std::unique_ptr<unsigned char[] > output = std::make_unique<unsigned char[]>(2048);
							int outputLength;
							SodiumUtils::sodiumEncrypt(true, voiceKey.get(), crypto_secretbox_KEYBYTES, Vars::privateKey.get(), yourPublic.get(), output, outputLength);
							const std::string outputStringified = Stringify::stringify(output.get(), outputLength);

							const time_t now = Utils::now();
							const std::string passthroughBase = std::to_string(now) + "|passthrough|" + Vars::callWith;
							const std::string passthrough = passthroughBase + "|" + outputStringified + "|" + Vars::sessionKey;
							Vars::commandSocket.get()->writeString(passthrough);
							logger->insertLog(Log(Log::TAG::CMD_LISTENER, passthroughBase+"|...|...", Log::TYPE::OUTBOUND).toString());
							haveVoiceKey = true;
							Voice::getInstance()->setVoiceKey(voiceKey);
						}

						const bool registeredUDP = Voice::getInstance()->connect();
						if (registeredUDP)
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
					else if (command == "direct")
					{
						const std::string setupString = respContents.at(2);
						const int setupDesgringifiedLength = setupString.length() / 3;
						std::unique_ptr<unsigned char[] > setupArray = std::make_unique<unsigned char[]>(setupDesgringifiedLength);
						unsigned char* setup = setupArray.get();
						Stringify::destringify(setupString, setup);
						std::unique_ptr<unsigned char[] > callWithKey;
						Settings::getInstance()->getPublicKey(Vars::callWith, callWithKey);
						std::unique_ptr<unsigned char[] > voiceKeyDecrypted = std::make_unique<unsigned char[]>(crypto_secretbox_KEYBYTES);
						int voiceKeyDecLength = 0;
						SodiumUtils::sodiumDecrypt(true, setup, setupDesgringifiedLength, Vars::privateKey.get(), callWithKey.get(), voiceKeyDecrypted, voiceKeyDecLength);

						if (voiceKeyDecLength == crypto_secretbox_KEYBYTES)
						{
							haveVoiceKey = true;
							Voice::getInstance()->setVoiceKey(voiceKeyDecrypted);
							sendReady();
						}
						else
						{
							logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_PASSTHROUGH_FAIL), Log::TYPE::ERROR).toString());
							giveUp();
							continue;
						}
					}
					else if (command == "start")
					{
						Vars::ustate = Vars::UserState::INCALL;
						asyncCentral->broadcast(Vars::Broadcast::CALL_START);
					}

					else if (command == "end")
					{
						Vars::ustate = Vars::UserState::NONE;
						Vars::callWith = "";
						asyncCentral->broadcast(Vars::Broadcast::USERHOME_UNLOCK);
						asyncCentral->broadcast(Vars::Broadcast::CALL_END);
					}

				}
				catch (std::string& e)
				{
					if(Vars::isExiting)
					{
						return;
					}
					inputValid = false;
					Vars::commandSocket.get()->stop();
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_IOERROR), Log::TYPE::ERROR).toString());
					LoginManager::getInstance()->execute(true);
				}
				catch (std::out_of_range& e)
				{
					logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_OORANGE), Log::TYPE::ERROR).toString());
				}
			}
		});
		serviceThread.detach();
	}
	catch(std::system_error& e)
	{
		const std::string error = r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CMD_LISTENER, error, Log::TYPE::ERROR).toString());
	}
}

void CmdListener::sendReady()
{
	if (haveVoiceKey && preparationsComplete)
	{
		const std::string readyBase = std::to_string(Utils::now()) + "|ready|" + Vars::callWith + "|";
		const std::string ready = readyBase + Vars::sessionKey;
		try
		{
			Vars::commandSocket.get()->writeString(ready);
			logger->insertLog(Log(Log::TAG::CMD_LISTENER, readyBase+"...", Log::TYPE::OUTBOUND).toString());
		}
		catch (std::string& e)
		{
			giveUp();
			throw;
		}
	}
}

void CmdListener::giveUp()
{
	OperatorCommand::execute(OperatorCommand::OperatorCommand::END);
	asyncCentral->broadcast(Vars::Broadcast::CALL_END);
}

std::string CmdListener::censorIncomingCmd(const std::vector<std::string>& parsed)
{
	if (parsed.size() < 1)
	{
		return "";
	}

	try
	{
		if (parsed.at(1) == "direct")
		{//timestamp|direct|(encrypted aes key)|other_person
			return parsed.at(0) + "|" + parsed.at(1) + "|...|" + parsed.at(3);
		}
		else
		{
			std::string result = "";
			for (int i = 0; i < parsed.size(); i++)
			{
				result = result + parsed.at(i) + "|";
			}
			result = result.substr(0, result.length() - 1);
			return result;
		}
	}
	catch (std::out_of_range& e)
	{
		logger->insertLog(Log(Log::TAG::CMD_LISTENER, r->getString(R::StringID::CMDLISTENER_OORANGE), Log::TYPE::ERROR).toString());
		return "";
	}
}