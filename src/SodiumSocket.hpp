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
#include "sodium_utils.hpp"
#include "StringRes.hpp"
#include "stringify.hpp"

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
	void stop() const;
	void getTcpKeyCopy(std::unique_ptr<unsigned char[]>& output) const;

private:
	void connectFD(const std::string& address, int port);

	const static int DECRYPTION_BUFFER_SIZE = 2048;
	const static int ENCRYPTION_BUFFER_SIZE = DECRYPTION_BUFFER_SIZE*2;

	int socketFD;
	unsigned char serverPublic[crypto_box_PUBLICKEYBYTES];
	unsigned char tcpKey[crypto_secretbox_KEYBYTES];

	const StringRes* strings;
};

#endif /* SRC_SODIUMSOCKET_HPP_ */
