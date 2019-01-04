/*
 * R.hpp
 *
 *  Created on: Dec 16, 2018
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_STRINGRES_HPP_
#define SRC_SCREENS_STRINGRES_HPP_

#include <unordered_map>
#include <string>

class R
{
public:
	typedef enum {EN} Language;
	typedef enum
	{
		SELF,
		ERR_NOT_ONSCREEN,
		ERR_SOCKET,
		ERR_THREAD_CREATE,
		ERR_UDP_REGISTRATION_FAILED,

		SODIUM_DEFAULT_CONSTRUCTOR,
		SODIUM_SEND_TEMP_PUBLIC,
		SODIUM_READ_TCP_SYMM,
		SODIUM_DECRYPT_TCP_SYMM,
		SODIUM_SOCKET_SYSCALL,
		SODIUM_INET_PTON,
		SODIUM_CONNECT_SYSCALL,
		SODIUM_READ,
		SODIUM_WRITE,
		SODIUM_NOTREADABLE,
		SODIUM_NOTWRITEABLE,

		INITIAL_SETUP_TITLE,
		INITIAL_SETUP_PLACEHOLDER_ADDR,
		INITIAL_SETUP_PLACEHOLDER_COMMAND,
		INITIAL_SETUP_PLACEHOLDER_MEDIA,
		INITIAL_SETUP_PLACEHOLDER_NAME,
		INITIAL_SETUP_LOGIN,
		INITIAL_SETUP_CHOOSE_SERVER_CERT,
		INITIAL_SETUP_BAD_SERVER_CERT,
		INITIAL_SETUP_SERVER_CERT_OK,
		INITIAL_SETUP_CHOOSE_PRIVATE_KEY,
		INITIAL_SETUP_PRIVATE_KEY_BAD,
		INITIAL_SETUP_PRIVATE_KEY_OK,
		INITIAL_SETUP_INCOMPLETE,
		INITIAL_SETUP_LOGIN_FAIL,

		MAIN_SKIP_TOHOME,

		USER_HOME_ONLINE,
		USER_HOME_OFFLINE,
		USER_HOME_TITLE,
		USER_HOME_PLACEHOLDER_ENTRY,
		USER_HOME_BUTTON_DIAL,
		USER_HOME_BUTTON_CONTACT,
		USER_HOME_LABEL_CONTACTS,
		USER_HOME_CANT_DIAL,

		CALL_SCREEN_TITLE,
		CALL_SCREEN_STATUS_RINGING,
		CALL_SCREEN_STATUS_INCALL,
		CALL_SCREEN_BUTTON_END,
		CALL_SCREEN_BUTTON_MUTE,
		CALL_SCREEN_BUTTON_MUTE_UNMUTE,
		CALL_SCREEN_BUTTON_ACCEPT,
		CALL_SCREEN_STAT_MISSING,
		CALL_SCREEN_STAT_TX,
		CALL_SCREEN_STAT_RX,
		CALL_SCREEN_STAT_GARBAGE,
		CALL_SCREEN_STAT_RXSEQ,
		CALL_SCREEN_STAT_TXSEQ,
		CALL_SCREEN_STAT_SKIP,
		CALL_SCREEN_STAT_RANGE,
		CALL_SCREEN_LAST_UDP_FOREVER,
		CALL_SCREEN_POPUP_MUTE_WARNING,
		CALL_SCREEN_MEDIA_ENC_START,
		CALL_SCREEN_MEDIA_ENC_DESC,
		CALL_SCREEN_MEDIA_ENC_PA_ERR,
		CALL_SCREEN_MEDIA_ENC_OPUS_ERR,
		CALL_SCREEN_MEDIA_ENC_SODIUM_ERR,
		CALL_SCREEN_MEDIA_ENC_STOP,
		CALL_SCREEN_MEDIA_ENC_NETWORK_ERR,
		CALL_SCREEN_MEDIA_DEC_START,
		CALL_SCREEN_MEDIA_DEC_DESC,
		CALL_SCREEN_MEDIA_DEC_PA_ERR,
		CALL_SCREEN_MEDIA_DEC_OPUS_ERR,
		CALL_SCREEN_MEDIA_DEC_STOP,
		CALL_SCREEN_MEDIA_DEC_SODIUM_ERR,
		CALL_SCREEN_MEDIA_DEC_NETWORK_ERR,

		LOGINASYNC_LOGIN1_FORMAT,
		LOGINASYNC_LOGIN1_TS,
		LOGINASYNC_CALLENGE_FAILED,
		LOGINASYNC_LOGIN2_FORMAT,
		LOGINASYNC_LOGIN2_TS,
		LOGINASYNC_REJECT_REQUEST,

		SETTINGS_AUTOGEN_WARNING,
		SETTINGS_BADCONFIG,
		SETTINGS_INTMIA,
		SETTINGS_BADINT,
		SETTINGS_STRINGMIA,

		CMDLISTENER_START,
		CMDLISTENER_TOOMANY_SEGMENTS,
		CMDLISTENER_BADTS,
		CMDLISTENER_WRONG_OTHER,
		CMDLISTENER_PUBLICKEY_MISMATCH,
		CMDLISTENER_IOERROR,
		CMDLISTENER_OORANGE,
		CMDLISTENER_PASSTHROUGH_FAIL,
		CMDLISTENER_REGISTERUDP_SEALFAIL,

		HEARTBEAT_FAIL,

		OPUS_INIT_DECERR,
		OPUS_INIT_ENCERR
	} StringID;

	typedef enum
	{
		RED,
		GREEN
	} ColorID;

	static R* getInstance();
	const std::string getString(StringID id) const;
	const std::string getColor(ColorID color) const;
	static Language activeLang;

private:
	R();
	virtual ~R();

	static R* instance;
	std::unordered_map<StringID, std::string> en;
	std::unordered_map<ColorID, std::string> colors;
};

#endif /* SRC_SCREENS_STRINGRES_HPP_ */
