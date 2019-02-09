/*
 * SSLSocket.cpp
 *
 *  Created on: Dec 12, 2017
 *      Author: Daniel
 */

#include "SodiumSocket.hpp"

SodiumSocket::SodiumSocket() :
socketFD(0),
useable(false),
r(R::getInstance()),
logger(Logger::getInstance("/tmp/"))
{
	logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, r->getString(R::StringID::SODIUM_DEFAULT_CONSTRUCTOR), Log::TYPE::ERROR).toString());
	memset(serverPublic, 0, crypto_box_PUBLICKEYBYTES);
	memset(tcpKey, 0, crypto_secretbox_KEYBYTES);
}

SodiumSocket::SodiumSocket(const std::string& caddr, int cport, unsigned char cserverPublic[])
:
useable(false),
r(R::getInstance()),
logger(Logger::getInstance(""))

{
	memset(serverPublic, 0, crypto_box_PUBLICKEYBYTES);
	memcpy(serverPublic, cserverPublic, crypto_box_PUBLICKEYBYTES);

	memset(tcpKey, 0, crypto_secretbox_KEYBYTES);
	unsigned char tempPublic[crypto_box_PUBLICKEYBYTES] {};
	unsigned char tempPrivate[crypto_box_SECRETKEYBYTES] {};
	crypto_box_keypair(tempPublic, tempPrivate);

	try
	{
		struct sockaddr_in serv_addr;
		bool socketok = Utils::connectFD(socketFD, SOCK_STREAM, caddr, cport, &serv_addr);
		if(!socketok)
		{
			const std::string error = r->getString(R::StringID::ERR_SOCKET);
			logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
			throw std::string(error);
		}

		const int writePublicErr = write(socketFD, tempPublic, crypto_box_PUBLICKEYBYTES);
		if(writePublicErr == -1)
		{
			const std::string error = r->getString(R::StringID::SODIUM_SEND_TEMP_PUBLIC);
			logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
			throw std::string(error);
		}

		unsigned char symmetricEncrypted[ENCRYPTION_BUFFER_SIZE] = {};
		const int readAmount = read(socketFD, symmetricEncrypted, ENCRYPTION_BUFFER_SIZE);
		if(readAmount < 0)
		{
			const std::string error = r->getString(R::StringID::SODIUM_READ_TCP_SYMM);
			logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
			throw std::string(error);
		}

		std::unique_ptr<unsigned char[]> symmetricDecrypted;
		int length = 0;
		SodiumUtils::sodiumDecrypt(true, symmetricEncrypted, readAmount, tempPrivate, serverPublic, symmetricDecrypted, length);
		if(length == 0)
		{
			const std::string error = r->getString(R::StringID::SODIUM_DECRYPT_TCP_SYMM);
			logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
			throw std::string(error);
		}
		randombytes_buf(tempPrivate, crypto_box_SECRETKEYBYTES);
		memcpy(tcpKey, symmetricDecrypted.get(), crypto_secretbox_KEYBYTES);
		randombytes_buf(symmetricDecrypted.get(), crypto_box_SECRETKEYBYTES);
	}
	catch(std::string& e)
	{
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, e, Log::TYPE::ERROR).toString());
		throw;
	}
	useable = true;
}

SodiumSocket::~SodiumSocket()
{
	randombytes_buf(tcpKey, crypto_box_SECRETKEYBYTES);
}

void SodiumSocket::getTcpKeyCopy(std::unique_ptr<unsigned char[]>& output) const
{
	output = std::make_unique<unsigned char[]>(crypto_box_SECRETKEYBYTES);
	memcpy(output.get(), tcpKey, crypto_secretbox_KEYBYTES);
}

void SodiumSocket::stop()
{
	shutdown(socketFD, 2);
	close(socketFD);
	useable = false;
}

std::string SodiumSocket::readString()
{
	if(!useable)
	{
		const std::string error = r->getString(R::StringID::SODIUM_NOTREADABLE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		return "";
	}

	std::unique_ptr<unsigned char[]> binary;
	int binaryLength = readBinary(binary);
	std::string result((char*)binary.get(), binaryLength);
	return result;
}

int SodiumSocket::readBinary(std::unique_ptr<unsigned char[]>& output)
{
	if(!useable)
	{
		const std::string error = r->getString(R::StringID::SODIUM_NOTREADABLE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		output = std::unique_ptr<unsigned char[]>();
		return 0;
	}

	unsigned char encrypted[ENCRYPTION_BUFFER_SIZE] = {};
	const int amountRead = read(socketFD, encrypted, ENCRYPTION_BUFFER_SIZE);
	if(amountRead < 1)
	{
		const std::string error = r->getString(R::StringID::SODIUM_READ);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		throw std::string(error);
	}

	int decryptionLength = 0;
	SodiumUtils::sodiumDecrypt(false, encrypted, amountRead, tcpKey, NULL, output, decryptionLength);
	if(decryptionLength == 0)
	{
		const std::string error = r->getString(R::StringID::SODIUM_READ);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		throw std::string(error);
	}
	return decryptionLength;
}

void SodiumSocket::writeString(const std::string& message)
{
	if(!useable)
	{
		const std::string error = r->getString(R::StringID::SODIUM_NOTWRITEABLE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		return;
	}
	writeBinary((unsigned char*)message.c_str(), message.length());
}

void SodiumSocket::writeBinary(unsigned char uchars[], int amount)
{
	if(!useable)
	{
		const std::string error = r->getString(R::StringID::SODIUM_NOTWRITEABLE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		return;
	}

	std::unique_ptr<unsigned char[]> encrypted;
	int encryptionLength = 0;
	SodiumUtils::sodiumEncrypt(false, uchars, amount, tcpKey, NULL, encrypted, encryptionLength);
	if(encryptionLength == 0)
	{
		const std::string error = r->getString(R::StringID::SODIUM_WRITE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		throw std::string(error);
	}

	const int writeAmount = write(socketFD, encrypted.get(), encryptionLength);
	if(writeAmount == 0)
	{
		const std::string error = r->getString(R::StringID::SODIUM_WRITE);
		logger->insertLog(Log(Log::TAG::SODIUM_SOCKET, error, Log::TYPE::ERROR).toString());
		throw std::string(error);
	}
}
