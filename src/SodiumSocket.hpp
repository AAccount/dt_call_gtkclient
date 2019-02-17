/*
 * SSLSocket.hpp
 *
 *  Created on: Dec 12, 2017
 *      Author: Daniel
 */

#ifndef SRC_SODIUMSOCKET_HPP_
#define SRC_SODIUMSOCKET_HPP_

#include <string>
#include <iostream>
#include <memory>
#include <exception>

#include <sodium.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "R.hpp"
#include "sodium_utils.hpp"
#include "stringify.hpp"
#include "utils.hpp"
#include "Log.hpp"
#include "Logger.hpp"

class SodiumSocket
{
public:
	SodiumSocket();
	SodiumSocket(const std::string& address, int port, unsigned char serverPublic[]);
	virtual ~SodiumSocket();

	int readBinary(std::unique_ptr<unsigned char[]>& output);
	std::string readString();
	void writeBinary(unsigned char uchars[], int amount);
	void writeString(const std::string& message);
	void stop();
	void getTcpKeyCopy(std::unique_ptr<unsigned char[]>& output) const;

private:
	const static int READ_BUFFER_SIZE = 4096;

	int socketFD;
	unsigned char serverPublic[crypto_box_PUBLICKEYBYTES];
	unsigned char tcpKey[crypto_secretbox_KEYBYTES];
	bool useable;

	const R* r;
	Logger* logger;
};

#endif /* SRC_SODIUMSOCKET_HPP_ */
