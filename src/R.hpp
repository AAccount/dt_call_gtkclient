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
		VERSION,
		COMMIT,

		GENERIC_EDIT,
		GENERIC_REMOVE,
		GENERIC_SAVE,

		ERR_NOT_ONSCREEN,
		ERR_SOCKET,
		ERR_THREAD_CREATE,
		ERR_UDP_REGISTRATION_FAILED,

		CONNECTFD_SOCKET_SYSCALL,
		CONNECTFD_INET_PTON,
		CONNECTFD_CONNECT_SYSCALL,

		SODIUM_DEFAULT_CONSTRUCTOR,
		SODIUM_SEND_TEMP_PUBLIC,
		SODIUM_READ_TCP_SYMM,
		SODIUM_DECRYPT_TCP_SYMM,
		SODIUM_READ,
		SODIUM_WRITE,
		SODIUM_NOTREADABLE,
		SODIUM_NOTWRITEABLE,

		SETTINGS_UI_TITLE_INITIAL_SETUP,
		SETTINGS_UI_TITLE_EDIT,
		SETTINGS_UI_PLACEHOLDER_ADDR,
		SETTINGS_UI_PLACEHOLDER_COMMAND,
		SETTINGS_UI_PLACEHOLDER_MEDIA,
		SETTINGS_UI_PLACEHOLDER_NAME,
		SETTINGS_UI_LOGIN,
		SETTINGS_UI_CHOOSE_SERVER_CERT,
		SETTINGS_UI_BAD_SERVER_CERT,
		SETTINGS_UI_SERVER_CERT_OK,
		SETTINGS_UI_CHOOSE_PRIVATE_KEY,
		SETTINGS_UI_PRIVATE_KEY_BAD,
		SETTINGS_UI_PRIVATE_KEY_OK,
		SETTINGS_UI_INCOMPLETE,
		SETTINGS_UI_LOGIN_FAIL,

		PUBLIC_KEY_OV_TITLE,
		PUBLIC_KEY_OV_NOKEY,
		PUBLIC_KEY_OV_MIA,

		PUBLIC_KEYU_TITLE,
		PUBLIC_KEYU_EDIT_TITLE,
		PUBLIC_KEYU_EDIT_ERROR,

		MAIN_SKIP_TOHOME,

		USER_HOME_ONLINE,
		USER_HOME_OFFLINE,
		USER_HOME_PLACEHOLDER_ENTRY,
		USER_HOME_BUTTON_DIAL,
		USER_HOME_BUTTON_CONTACT,
		USER_HOME_LABEL_CONTACTS,
		USER_HOME_CANT_DIAL,
		USER_HOME_CONTACTS_NOENTRY,
		USER_HOME_CONTACTS_EDIT_NOENTRY,
		USER_HOME_CONTACTS_REMOVE_NOENTRY,
		USER_HOME_CONTACTS_NOT_REGISTERED,

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
		CALL_SCREEN_STAT_MB,
		CALL_SCREEN_STAT_KB,
		CALL_SCREEN_STAT_B,
		CALL_SCREEN_LAST_UDP_FOREVER,
		CALL_SCREEN_POPUP_MUTE_WARNING,
		CALL_SCREEN_MEDIA_ENC_START,
		CALL_SCREEN_MEDIA_ENC_DESC,
		CALL_SCREEN_MEDIA_ENC_PA_ERR,
		CALL_SCREEN_MEDIA_ENC_OPUS_ERR,
		CALL_SCREEN_MEDIA_ENC_SODIUM_ERR,
		CALL_SCREEN_MEDIA_ENC_STOP,
		CALL_SCREEN_MEDIA_ENC_LATENCY,
		CALL_SCREEN_MEDIA_ENC_NETWORK_ERR,
		CALL_SCREEN_MEDIA_DEC_START,
		CALL_SCREEN_MEDIA_DEC_DESC,
		CALL_SCREEN_MEDIA_DEC_PA_ERR,
		CALL_SCREEN_MEDIA_DEC_OPUS_ERR,
		CALL_SCREEN_MEDIA_DEC_STOP,
		CALL_SCREEN_MEDIA_DEC_SODIUM_ERR,
		CALL_SCREEN_MEDIA_DEC_NETWORK_ERR,
		CALL_SCREEN_MEDIA_DEC_LATENCY,
		CALL_SCREEN_MEDIA_RINGTONE_DESC,

		EDIT_CONTACT_ENTRY_PLACEHOLDER,

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
		CMDLISTENER_UDP_TIMEOUT_REGISTER,
		CMDLISTENER_UDP_TIMEOUT_REMOVE_TIMEOUT,

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
