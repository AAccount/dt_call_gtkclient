/*
 * SSLSocket.cpp
 *
 *  Created on: Dec 12, 2017
 *      Author: Daniel
 */

#include "SodiumSocket.hpp"

SodiumSocket::SodiumSocket() :
socketFD(0),
strings(StringRes::getInstance())
{
	memset(serverPublic, 0, crypto_box_PUBLICKEYBYTES);
	memset(tcpKey, 0, crypto_secretbox_KEYBYTES);
}

SodiumSocket::SodiumSocket(const std::string& caddr, int cport, unsigned char cserverPublic[])
:strings(StringRes::getInstance())
{
	memset(serverPublic, 0, crypto_box_PUBLICKEYBYTES);
	memcpy(serverPublic, cserverPublic, crypto_box_PUBLICKEYBYTES);

	memset(tcpKey, 0, crypto_secretbox_KEYBYTES);
	unsigned char tempPublic[crypto_box_PUBLICKEYBYTES] {};
	unsigned char tempPrivate[crypto_box_SECRETKEYBYTES] {};
	crypto_box_keypair(tempPublic, tempPrivate);

	try
	{
		connectFD(caddr, cport);

		const int writePublicErr = write(socketFD, tempPublic, crypto_box_PUBLICKEYBYTES);
		if(writePublicErr == -1)
		{
			throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_SEND_TEMP_PUBLIC);
		}

		unsigned char symmetricEncrypted[ENCRYPTION_BUFFER_SIZE] = {};
		const int readAmount = read(socketFD, symmetricEncrypted, ENCRYPTION_BUFFER_SIZE);
		if(readAmount < 0)
		{
			throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_READ_TCP_SYMM);
		}

		std::unique_ptr<unsigned char[]> symmetricDecrypted;
		int length = 0;
		SodiumUtils::sodiumDecrypt(true, symmetricEncrypted, readAmount, tempPrivate, serverPublic, symmetricDecrypted, length);
		if(length == 0)
		{
			throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_DECRYPT_TCP_SYMM);
		}
		randombytes_buf(tempPrivate, crypto_box_SECRETKEYBYTES);
		memcpy(tcpKey, symmetricDecrypted.get(), crypto_secretbox_KEYBYTES);
		randombytes_buf(symmetricDecrypted.get(), crypto_box_SECRETKEYBYTES);
	}
	catch(std::string& e)
	{
		throw e;
	}
}

SodiumSocket::~SodiumSocket()
{
	randombytes_buf(tcpKey, crypto_box_SECRETKEYBYTES);
}

void SodiumSocket::connectFD(const std::string& caddr, int cport)
{
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socketFD < 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_SOCKET_SYSCALL);
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(cport);
	const int result = inet_pton(AF_INET, caddr.c_str(), &serv_addr.sin_addr);
	if(result < 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_INET_PTON);
	}

	const int connectOk = connect(socketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(connectOk < 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_CONNECT_SYSCALL);
	}
}

void SodiumSocket::getTcpKeyCopy(std::unique_ptr<unsigned char[]>& output) const
{
	memcpy(output.get(), tcpKey, crypto_secretbox_KEYBYTES);
}

void SodiumSocket::stop() const
{
	shutdown(socketFD, 2);
	close(socketFD);
}

std::string SodiumSocket::readString()
{
	std::unique_ptr<unsigned char[]> binary;
	int binaryLength = readBinary(binary);
	std::string result((char*)binary.get(), binaryLength);
	return result;
}

int SodiumSocket::readBinary(std::unique_ptr<unsigned char[]>& output)
{
	unsigned char encrypted[ENCRYPTION_BUFFER_SIZE] = {};
	const int amountRead = read(socketFD, encrypted, ENCRYPTION_BUFFER_SIZE);
	if(amountRead < 1)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_READ);
	}

	int decryptionLength = 0;
	SodiumUtils::sodiumDecrypt(false, encrypted, amountRead, tcpKey, NULL, output, decryptionLength);
	if(decryptionLength == 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_READ);
	}
	return decryptionLength;
}

void SodiumSocket::writeString(const std::string& message)
{
	writeBinary((unsigned char*)message.c_str(), message.length());
}

void SodiumSocket::writeBinary(unsigned char uchars[], int amount)
{
	std::unique_ptr<unsigned char[]> encrypted;
	int encryptionLength = 0;
	SodiumUtils::sodiumEncrypt(false, uchars, amount, tcpKey, NULL, encrypted, encryptionLength);
	if(encryptionLength == 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_WRITE);
	}

	const int writeAmount = write(socketFD, encrypted.get(), encryptionLength);
	if(writeAmount == 0)
	{
		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_WRITE);
	}
}
