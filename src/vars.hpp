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

#include "SodiumSocket.hpp"
#include "StringRes.hpp"

class Vars
{
public:
	Vars();
	virtual ~Vars();

	static StringRes::Language lang;

	//setup information
	static std::string serverAddress;
	static int commandPort;
	static int mediaPort;
	static std::unique_ptr<unsigned char[]> serverCert;
	static std::string username;
	static std::unique_ptr<unsigned char[]> privateKey;


	//sockets
	static SodiumSocket commandSocket;
	static std::string sessionKey;
};

#endif /* SRC_VARS_HPP_ */
