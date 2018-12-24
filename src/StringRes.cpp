/*
 * StringRes.cpp
 *
 *  Created on: Dec 16, 2018
 *      Author: Daniel
 */

#include "StringRes.hpp"

StringRes* StringRes::instance = NULL;

StringRes* StringRes::getInstance()
{
	if(instance == NULL)
	{
		instance = new StringRes();
	}
	return instance;
}

const std::string StringRes::getString(Language lang, StringID id) const
{
	const static std::string notFound = std::to_string(id) + " doesn't exist for language " + std::to_string(lang);
	if(lang == Language::EN)
	{
		const bool available = en.count(id) > 0;
		if(available)
		{
			const std::string result = en.at(id);
			return result;
		}
		else
		{
			return notFound;
		}
	}
	return notFound;
}

StringRes::StringRes()
: en(std::unordered_map<StringID, std::string>())
{
	en[StringID::ERR_NOT_ONSCREEN] = "not on screen";

	en[StringID::ERR_SODIUM_SEND_TEMP_PUBLIC] = "Cannot send temp public key";
	en[StringID::ERR_SODIUM_READ_TCP_SYMM] = "Cannot read tcp symmetric key";
	en[StringID::ERR_SODIUM_DECRYPT_TCP_SYMM] = "Couldn't decrypt tcp symmetric key";
	en[StringID::ERR_SODIUM_SOCKET_SYSCALL] = "Cannot create underlying tcp socket for sodium socket";
	en[StringID::ERR_SODIUM_INET_PTON] = "Cannot create struct sockaddr_in for underlying tcp socket for sodium socket";
	en[StringID::ERR_SODIUM_CONNECT_SYSCALL] = "Sodium socket can't connectFD";
	en[StringID::ERR_SODIUM_READ] = "sodium socket read using symmetric decryption failed";
	en[StringID::ERR_SODIUM_WRITE] = "sodium socket write using symmetric encryption failed";
	en[StringID::ERR_PREF_BADCONFIG] = "preference is misconfigured: ";
	en[StringID::ERR_PREF_INTMIA] = "Int preference does not exist: ";
	en[StringID::ERR_PREF_BADINT] = "Int preference can't be converted to a string: ";
	en[StringID::ERR_PREF_STRINGMIA] = "String preference does not exist: ";

	en[StringID::INITIAL_SETUP_CHOOSE_SERVER_CERT] = "Choose Server Certificate";
	en[StringID::INITIAL_SETUP_BAD_SERVER_CERT] = "Sodium certificate invalid";
	en[StringID::INITIAL_SETUP_SERVER_CERT_OK] = "Server Sodium OK";
	en[StringID::INITIAL_SETUP_CHOOSE_PRIVATE_KEY] = "Choose Your Private Key";
	en[StringID::INITIAL_SETUP_PRIVATE_KEY_BAD] = "Private key invalid";
	en[StringID::INITIAL_SETUP_PRIVATE_KEY_OK] = "Private Sodium OK";
	en[StringID::INITIAL_SETUP_INCOMPLETE] = "All the required information hasn't been filled in yet";

	en[StringID::LOGINASYNC_LOGIN1_FORMAT] = "login1 improperly formatted";
	en[StringID::LOGINASYNC_LOGIN1_TS] = "login1 bad timestamp";
	en[StringID::LOGINASYNC_CALLENGE_FAILED] = "challenge asymmetric decryption failed";
	en[StringID::LOGINASYNC_LOGIN2_FORMAT] = "login2 improperly formatted";
	en[StringID::LOGINASYNC_LOGIN2_TS] = "login2 bad timestamp";
}

StringRes::~StringRes()
{

}

