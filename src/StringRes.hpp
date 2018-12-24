/*
 * StringRes.hpp
 *
 *  Created on: Dec 16, 2018
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_STRINGRES_HPP_
#define SRC_SCREENS_STRINGRES_HPP_

#include <unordered_map>
#include <string>

class StringRes
{
public:
	typedef enum {EN} Language;
	typedef enum
	{
		ERR_SODIUM_SEND_TEMP_PUBLIC,
		ERR_SODIUM_READ_TCP_SYMM,
		ERR_SODIUM_DECRYPT_TCP_SYMM,
		ERR_SODIUM_SOCKET_SYSCALL,
		ERR_SODIUM_INET_PTON,
		ERR_SODIUM_CONNECT_SYSCALL,
		ERR_NOT_ONSCREEN,
		ERR_SODIUM_READ,
		ERR_SODIUM_WRITE,
		ERR_PREF_BADCONFIG,
		ERR_PREF_INTMIA,
		ERR_PREF_BADINT,
		ERR_PREF_STRINGMIA,

		INITIAL_SETUP_CHOOSE_SERVER_CERT,
		INITIAL_SETUP_BAD_SERVER_CERT,
		INITIAL_SETUP_SERVER_CERT_OK,
		INITIAL_SETUP_CHOOSE_PRIVATE_KEY,
		INITIAL_SETUP_PRIVATE_KEY_BAD,
		INITIAL_SETUP_PRIVATE_KEY_OK,
		INITIAL_SETUP_INCOMPLETE,

		LOGINASYNC_LOGIN1_FORMAT,
		LOGINASYNC_LOGIN1_TS,
		LOGINASYNC_CALLENGE_FAILED,
		LOGINASYNC_LOGIN2_FORMAT,
		LOGINASYNC_LOGIN2_TS
	} StringID;

	static StringRes* getInstance();
	const std::string getString(Language lang, StringID id) const;

private:
	StringRes();
	virtual ~StringRes();

	static StringRes* instance;
	std::unordered_map<StringID, std::string> en;
};

#endif /* SRC_SCREENS_STRINGRES_HPP_ */
