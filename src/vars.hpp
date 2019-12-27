/*
 * vars.hpp
 *
 *  Created on: Dec 24, 2017
 *      Author: Daniel
 */

#ifndef SRC_VARS_HPP_
#define SRC_VARS_HPP_

#include <gtk/gtk.h>
#include <string>
#include <memory>
#include <atomic>

#include <sys/socket.h>

#include "R.hpp"
#include "SodiumSocket.hpp"

class Vars
{
public:

	const static int MAX_UDP = 1400;
	typedef enum {NONE, INIT, INCALL} UserState;
	typedef enum {CALL_TRY, CALL_START, CALL_END, LOGIN_NOTOK, LOGIN_OK, USERHOME_UNLOCK, USERHOME_LOCK, USERHOME_CONTACTEDITED, PUBLIC_KEYOV_EDIT, MIC_MUTE, MIC_UNMUTE} Broadcast;

	Vars();
	virtual ~Vars();

	//setup information
	static struct sockaddr_in mediaPortAddrIn;
	static std::string serverAddress;
	static int commandPort;
	static int mediaPort;
	static std::unique_ptr<unsigned char[]> serverCert;
	static std::string username;
	static std::unique_ptr<unsigned char[]> privateKey;

	static UserState ustate;

	//sockets
	static std::unique_ptr<SodiumSocket> commandSocket;
	static int mediaSocket;
	static std::string sessionKey;
	static std::string callWith;
	static std::unique_ptr<unsigned char[]> voiceKey;
	
	//for the services that r/w sockets: heartbeat, cmdlistener, don't do it when the program is exiting.
	static std::atomic<bool> isExiting;
};

#endif /* SRC_VARS_HPP_ */
