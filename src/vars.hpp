/*
 * vars.hpp
 *
 *  Created on: Dec 24, 2017
 *      Author: Daniel
 */

#ifndef SRC_VARS_HPP_
#define SRC_VARS_HPP_

#include <string>
#include <memory>

#include "R.hpp"
#include "SodiumSocket.hpp"

class Vars
{
public:

	typedef enum {NONE, INIT, INCALL} UserState;
	typedef enum {CALL_TRY, CALL_START, CALL_END, LOGIN_NOTOK, LOGIN_OK, UNLOCK_USERHOME} Broadcast;

	Vars();
	virtual ~Vars();

	//setup information
	static std::string serverAddress;
	static int commandPort;
	static int mediaPort;
	static std::unique_ptr<unsigned char[]> serverCert;
	static std::string username;
	static std::unique_ptr<unsigned char[]> privateKey;

	static UserState ustate;

	//sockets
	static SodiumSocket commandSocket;
	static int mediaSocket;
	static std::string sessionKey;
	static std::string callWith;
	static std::unique_ptr<unsigned char[]> voiceKey;
};

#endif /* SRC_VARS_HPP_ */
