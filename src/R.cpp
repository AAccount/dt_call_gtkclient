/*
 * StringRes.cpp
 *
 *  Created on: Dec 16, 2018
 *      Author: Daniel
 */

#include "R.hpp"

R* R::instance = NULL;
R::Language R::activeLang;

R* R::getInstance()
{
	if(instance == NULL)
	{
		instance = new R();
	}
	return instance;
}

const std::string R::getString(StringID id) const
{
	const static std::string notFound = std::to_string(id) + " doesn't exist for language " + std::to_string(activeLang);
	if(activeLang == Language::EN)
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

const std::string R::getColor(ColorID id) const
{
	if(colors.count(id) > 0)
	{
		return colors.at(id);
	}
	return "";
}

R::R()
: en(std::unordered_map<StringID, std::string>()),
  colors(std::unordered_map<ColorID, std::string>())

{
	en[StringID::SELF] = "DT Call GTKClient";
	en[StringID::ERR_NOT_ONSCREEN] = "not on screen";
	en[StringID::ERR_SOCKET] = "Couldn't establish socket";
	en[StringID::ERR_THREAD_CREATE] = "Couldn't create thread: ";
	en[StringID::ERR_UDP_REGISTRATION_FAILED] = "Couldn't register udp voice port";

	en[StringID::SODIUM_DEFAULT_CONSTRUCTOR] = "Should not be using the default sodium socket constructor";
	en[StringID::SODIUM_SEND_TEMP_PUBLIC] = "Cannot send temp public key";
	en[StringID::SODIUM_READ_TCP_SYMM] = "Cannot read tcp symmetric key";
	en[StringID::SODIUM_DECRYPT_TCP_SYMM] = "Couldn't decrypt tcp symmetric key";
	en[StringID::SODIUM_SOCKET_SYSCALL] = "Cannot create underlying tcp socket for sodium socket";
	en[StringID::SODIUM_INET_PTON] = "Cannot create struct sockaddr_in for underlying tcp socket for sodium socket";
	en[StringID::SODIUM_CONNECT_SYSCALL] = "Sodium socket can't connectFD";
	en[StringID::SODIUM_READ] = "sodium socket read using symmetric decryption failed";
	en[StringID::SODIUM_WRITE] = "sodium socket write using symmetric encryption failed";
	en[StringID::SODIUM_NOTREADABLE] = "sodium socket isn't useable for reading";
	en[StringID::SODIUM_NOTWRITEABLE] = "sodium socket isn't useable for writing";

	en[StringID::INITIAL_SETUP_TITLE] = "Initial Setup";
	en[StringID::INITIAL_SETUP_PLACEHOLDER_ADDR] = "Server Address or Name";
	en[StringID::INITIAL_SETUP_PLACEHOLDER_COMMAND] = "Command Port Number";
	en[StringID::INITIAL_SETUP_PLACEHOLDER_MEDIA] = "Media Port Number";
	en[StringID::INITIAL_SETUP_PLACEHOLDER_NAME] = "Username";
	en[StringID::INITIAL_SETUP_LOGIN] = "Login";
	en[StringID::INITIAL_SETUP_CHOOSE_SERVER_CERT] = "Choose Server Certificate";
	en[StringID::INITIAL_SETUP_BAD_SERVER_CERT] = "Sodium certificate invalid";
	en[StringID::INITIAL_SETUP_SERVER_CERT_OK] = "Server Sodium OK";
	en[StringID::INITIAL_SETUP_CHOOSE_PRIVATE_KEY] = "Choose Your Private Key";
	en[StringID::INITIAL_SETUP_PRIVATE_KEY_BAD] = "Private key invalid";
	en[StringID::INITIAL_SETUP_PRIVATE_KEY_OK] = "Private Sodium OK";
	en[StringID::INITIAL_SETUP_INCOMPLETE] = "All the required information hasn't been filled in yet";
	en[StringID::INITIAL_SETUP_LOGIN_FAIL] = "Login failed";

	en[StringID::MAIN_SKIP_TOHOME] = "Login information present, skipping to home";

	en[StringID::USER_HOME_ONLINE] = "Signed In";
	en[StringID::USER_HOME_OFFLINE] = "Offline";
	en[StringID::USER_HOME_TITLE] = "DT Call GTK3 Client";
	en[StringID::USER_HOME_PLACEHOLDER_ENTRY] = "Choose someone to call";
	en[StringID::USER_HOME_BUTTON_DIAL] = "Dial";
	en[StringID::USER_HOME_BUTTON_CONTACT] = "Add Contact";
	en[StringID::USER_HOME_LABEL_CONTACTS] = "Contacts";
	en[StringID::USER_HOME_CANT_DIAL] = "User is not available to call";

	en[StringID::CALL_SCREEN_TITLE] = "DT Call";
	en[StringID::CALL_SCREEN_STATUS_RINGING] = "Ringing";
	en[StringID::CALL_SCREEN_STATUS_INCALL] = "In Call";
	en[StringID::CALL_SCREEN_BUTTON_END] = "End";
	en[StringID::CALL_SCREEN_BUTTON_MUTE] = "Mute";
	en[StringID::CALL_SCREEN_BUTTON_MUTE_UNMUTE] = "Unmute";
	en[StringID::CALL_SCREEN_BUTTON_ACCEPT] = "Accept";
	en[StringID::CALL_SCREEN_STAT_MISSING] = "Missing";
	en[StringID::CALL_SCREEN_STAT_TX] = "Out";
	en[StringID::CALL_SCREEN_STAT_RX] = "In";
	en[StringID::CALL_SCREEN_STAT_GARBAGE] = "Garbage";
	en[StringID::CALL_SCREEN_STAT_RXSEQ] = "In#";
	en[StringID::CALL_SCREEN_STAT_TXSEQ] = "Out#";
	en[StringID::CALL_SCREEN_STAT_SKIP] = "Skipped";
	en[StringID::CALL_SCREEN_STAT_RANGE] = "!!Range";
	en[StringID::CALL_SCREEN_STAT_MB] = "mbytes";
	en[StringID::CALL_SCREEN_STAT_KB] = "kbytes";
	en[StringID::CALL_SCREEN_STAT_B] = "bytes";
	en[StringID::CALL_SCREEN_LAST_UDP_FOREVER] = "delay since last received more than 1s";
	en[StringID::CALL_SCREEN_POPUP_MUTE_WARNING] = "DOUBLE CHECK mute label. It can take up to 1 second to change mute/unmute";
	en[StringID::CALL_SCREEN_MEDIA_ENC_START] = "Media encoding thread started";
	en[StringID::CALL_SCREEN_MEDIA_ENC_DESC] = "Voice call record";
	en[StringID::CALL_SCREEN_MEDIA_ENC_PA_ERR] = "Pulse audio read from mic failed with: ";
	en[StringID::CALL_SCREEN_MEDIA_ENC_OPUS_ERR] = "Opus encoder error: ";
	en[StringID::CALL_SCREEN_MEDIA_ENC_STOP] = "Media encoding thread stopped";
	en[StringID::CALL_SCREEN_MEDIA_ENC_SODIUM_ERR] = "voice symmetric encryption failed";
	en[StringID::CALL_SCREEN_MEDIA_ENC_NETWORK_ERR] = "sending voice out the udp socket failed;";
	en[StringID::CALL_SCREEN_MEDIA_DEC_START] = "Media decoding thread started";
	en[StringID::CALL_SCREEN_MEDIA_DEC_DESC] = "Voice call playback";
	en[StringID::CALL_SCREEN_MEDIA_DEC_PA_ERR] = "Pulse audio playback failed with: ";
	en[StringID::CALL_SCREEN_MEDIA_DEC_OPUS_ERR] = "Opus decoder error: ";
	en[StringID::CALL_SCREEN_MEDIA_DEC_STOP] = "Media decoding thread stopped";
	en[StringID::CALL_SCREEN_MEDIA_DEC_SODIUM_ERR] = "voice symmetric decryption failed";
	en[StringID::CALL_SCREEN_MEDIA_DEC_NETWORK_ERR] = "receiving voice on the udp socket failed;";


	en[StringID::LOGINASYNC_LOGIN1_FORMAT] = "login1 improperly formatted";
	en[StringID::LOGINASYNC_LOGIN1_TS] = "login1 bad timestamp";
	en[StringID::LOGINASYNC_CALLENGE_FAILED] = "challenge asymmetric decryption failed";
	en[StringID::LOGINASYNC_LOGIN2_FORMAT] = "login2 improperly formatted";
	en[StringID::LOGINASYNC_LOGIN2_TS] = "login2 bad timestamp";
	en[StringID::LOGINASYNC_REJECT_REQUEST] = "Login thread is already running, rejecting request";

	en[StringID::SETTINGS_AUTOGEN_WARNING] = "####\n#Warning: settings file is automatically generated. Do not edit.\n####\n\n";
	en[StringID::SETTINGS_BADCONFIG] = "preference is misconfigured: ";
	en[StringID::SETTINGS_INTMIA] = "Int preference does not exist: ";
	en[StringID::SETTINGS_BADINT] = "Int preference can't be converted to a string: ";
	en[StringID::SETTINGS_STRINGMIA] = "String preference does not exist: ";

	en[StringID::CMDLISTENER_START] = "Command listener started";
	en[StringID::CMDLISTENER_TOOMANY_SEGMENTS] = "command has too many segments: ";
	en[StringID::CMDLISTENER_BADTS] = "rejecting server response because of a bad timestamp";
	en[StringID::CMDLISTENER_WRONG_OTHER] = "erroneous call with command. expecting/got: ";
	en[StringID::CMDLISTENER_PUBLICKEY_MISMATCH] = "public key mismatch (for) expecting/got: (";
	en[StringID::CMDLISTENER_IOERROR] = "Command listener died";
	en[StringID::CMDLISTENER_OORANGE] = "Out of range exception: ";
	en[StringID::CMDLISTENER_PASSTHROUGH_FAIL] = "Couldn't decrypt passthrough of voice key";
	en[StringID::CMDLISTENER_REGISTERUDP_SEALFAIL] = "failed to create sealed box for udp registration";

	en[StringID::HEARTBEAT_FAIL] = "couldn't write heartbeat: ";

	en[StringID::OPUS_INIT_DECERR] = "decoder create failed: ";
	en[StringID::OPUS_INIT_ENCERR] = "encoder create failed: ";

	colors[ColorID::GREEN] = "<span style=\"background-color: rgba(73,D2,16,1)\">%s</span>";
	colors[ColorID::RED] = "<span style=\"background color: rgba(EF,29,29,1)\">%s</span>";
}

R::~R()
{

}
