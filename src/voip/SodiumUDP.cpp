/* 
 * File:   SodiumUDP.cpp
 * Author: Daniel
 * 
 * Created on January 20, 2020, 8:15 PM
 */

#include "SodiumUDP.hpp"

SodiumUDP::SodiumUDP(const std::string& server, int port) :
logger(Logger::getInstance()),
r(R::getInstance()),
//stats		
garbage(0), rxtotal(0), txtotal(0), rxSeq(0), txSeq(0), skipped(0), oorange(0),
missingLabel(r->getString(R::StringID::SODIUM_UDP_STAT_MISSING)),
txLabel(r->getString(R::StringID::SODIUM_UDP_STAT_TX)),
rxLabel(r->getString(R::StringID::SODIUM_UDP_STAT_RX)),
garbageLabel(r->getString(R::StringID::SODIUM_UDP_STAT_GARBAGE)),
rxSeqLabel(r->getString(R::StringID::SODIUM_UDP_STAT_RXSEQ)),
txSeqLabel(r->getString(R::StringID::SODIUM_UDP_STAT_TXSEQ)),
skippedLabel(r->getString(R::StringID::SODIUM_UDP_STAT_SKIP)),
oorangeLabel(r->getString(R::StringID::SODIUM_UDP_STAT_RANGE)),
//reconnection info
lastReceivedTimestamp(0),
reconnectionAttempted(false),
reconnectTries(0),
receiveMonitorAlive(false),

stopRequested(false),
//actual internals
useable(false),
mediaSocket(-1),
address(server),
port(port),
//threads
txalive(false),
txpool(Vars::MAX_UDP),
txplaintext(std::make_unique<unsigned char[]>(Vars::MAX_UDP)),
rxalive(false),
rxpool(Vars::MAX_UDP),
rxplaintext(std::make_unique<unsigned char[]>(Vars::MAX_UDP))
{
	memset(&mediaPortAddrIn, 0, sizeof(struct sockaddr_in));
	
	struct timeval now;
	memset(&now, 0, sizeof(struct timeval));
	gettimeofday(&now, NULL);
	const long creationUsec = (long)(now.tv_usec);
	creationTime = std::to_string(creationUsec);
}

void SodiumUDP::setVoiceSymmetricKey(std::unique_ptr<unsigned char[]> key)
{
	voiceKey = std::move(key);
}

SodiumUDP::~SodiumUDP()
{
	randombytes_buf(voiceKey.get(), crypto_box_SECRETKEYBYTES);
	randombytes_buf(txplaintext.get(), Vars::MAX_UDP);
	randombytes_buf(rxplaintext.get(), Vars::MAX_UDP);
}

bool SodiumUDP::connect()
{
	return registerUDP();
}

void SodiumUDP::start()
{
	try
	{
		receiveMonitorThread = std::thread(&SodiumUDP::receiveMonitor, this);
		receiveMonitorAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = creationTime + " receiveMonitorThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
	
	try
	{
		txthread = std::thread(&SodiumUDP::tx, this);
		txalive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = creationTime + "txthread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
	
	try
	{
		rxthread = std::thread(&SodiumUDP::rx, this);
		rxalive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = creationTime + "rxthread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		stopOnError();
		return;
	}
}

void SodiumUDP::closeSocket()
{
	const std::string message = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_CLOSE);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, message, Log::TYPE::INFO).toString());
	useable = false;
	
	if(mediaSocket > 0)
	{
		shutdown(mediaSocket, 2);
		close(mediaSocket);
		mediaSocket = -1;
	}
	
	if(receiveMonitorAlive)
	{
		receiveMonitorThread.join();
		receiveMonitorAlive = false;
	}
	
	if(txalive)
	{
		txq.interrupt();
		txthread.join();
		txalive = false;
	}
	
	if(rxalive)
	{
		rxq.interrupt();
		rxthread.join();
		rxalive = false;
	}
}

void SodiumUDP::tx()
{
	const std::string start = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_TX_START);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, start, Log::TYPE::INFO).toString());

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		std::unique_ptr<unsigned char[]> txbuffer = NULL;
		try
		{
			txbuffer = txq.pop();
		}
		catch(std::runtime_error& exception)
		{
			continue;
		}
		
		const int packetLength = txqLengths.pop();
		const int sent = sendto(mediaSocket, txbuffer.get(), packetLength, 0, (struct sockaddr*)&mediaPortAddrIn, sizeof(struct sockaddr_in));
		txpool.returnBuffer(txbuffer);
		if(sent < 0)
		{
			const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_TX_ERR);
			logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
			const bool reconnected = reconnectUDP();
			if(!reconnected)
			{
				stopOnError();
				break;
			}
		}
		else
		{
			txtotal = txtotal + packetLength + HEADERS;
		}
	}
	
	const std::string stop = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_TX_STOP);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, stop, Log::TYPE::INFO).toString());
}

void SodiumUDP::rx()
{
	const std::string start = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_RX_START);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, start, Log::TYPE::INFO).toString());
	
	while(Vars::ustate == Vars::UserState::INCALL)
	{
		std::unique_ptr<unsigned char[]> packetBuffer = rxpool.getBuffer();
		memset(packetBuffer.get(), 0, rxpool.getBufferSize());

		struct sockaddr_in sender;
		socklen_t senderLength = sizeof(struct sockaddr_in);
		const int receivedLength = recvfrom(mediaSocket, packetBuffer.get(), Vars::MAX_UDP, 0, (struct sockaddr*)&sender, &senderLength);
		if(receivedLength < 0)
		{
			const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_RX_ERROR);
			logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
			const bool reconnected = reconnectUDP(); 
			if(!reconnected)
			{
				stopOnError();
				rxpool.returnBuffer(packetBuffer);
				break;
			}
			continue;
		}
		else
		{
			struct timeval now;
			memset(&now, 0, sizeof(struct timeval));
			gettimeofday(&now, NULL);
			lastReceivedTimestamp = (long)(now.tv_usec);
			rxtotal = rxtotal + receivedLength + HEADERS;
			
			rxq.push(packetBuffer);
			rxqLengths.push(receivedLength);
		}
	}
	
	const std::string stop = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_RX_STOP);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, stop, Log::TYPE::INFO).toString());
}

void SodiumUDP::write(std::unique_ptr<unsigned char[]>& outData, int size)
{
	if(!useable)
	{
		const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_WRITE_CANT);
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		return;
	}
	
	memset(txplaintext.get(), 0, Vars::MAX_UDP);
	SodiumUtils::disassembleInt(txSeq, txplaintext.get());
	txSeq++;
	memcpy(txplaintext.get()+sizeof(uint32_t), outData.get(), size);

	std::unique_ptr<unsigned char[]> packetEncrypted = txpool.getBuffer();
	int packetEncryptedLength = 0;
	SodiumUtils::sodiumEncrypt(false, txplaintext.get(), sizeof(uint32_t)+size, voiceKey.get(), NULL, packetEncrypted, packetEncryptedLength);
	if(packetEncryptedLength < 1)
	{
		const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_ENCRYPT_ERR);
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		txpool.returnBuffer(packetEncrypted);
		return;
	}
	txq.push(packetEncrypted);
	txqLengths.push(packetEncryptedLength);
}

int SodiumUDP::read(std::unique_ptr<unsigned char[]>& inData, int inSize)
{
	if(!useable)
	{
		const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_READ_CANT);
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		return 0;
	}
	
	std::unique_ptr<unsigned char[]> packetBuffer = NULL;
	try
	{
		packetBuffer = rxq.pop();
	}
	catch(std::runtime_error& error)
	{
		return 0;
	}
	int receivedLength = rxqLengths.pop();
	
	memset(rxplaintext.get(), 0, Vars::MAX_UDP);
	int packetDecryptedLength = 0;
	SodiumUtils::sodiumDecrypt(false, packetBuffer.get(), receivedLength, voiceKey.get(), NULL, rxplaintext, packetDecryptedLength);
	rxpool.returnBuffer(packetBuffer);
	if(packetDecryptedLength < sizeof (uint32_t)) //should have received at least the sequence number
	{
		const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_DECRYPT_ERR);
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
		garbage++;
		return 0;
	}

	const int sequence = SodiumUtils::reassembleInt(rxplaintext.get());
	if (sequence <= rxSeq)
	{
		skipped++;
		return 0;
	}

	if (std::abs(sequence - rxSeq) > OORANGE_LIMIT)
	{
		oorange++;
		return 0;
	}
	else
	{
		rxSeq = sequence;
	}

	const int opusLength = packetDecryptedLength - sizeof (uint32_t);
	memset(inData.get(), 0, inSize);
	memcpy(inData.get(), rxplaintext.get() + sizeof(uint32_t), opusLength);
	return opusLength;
}

void SodiumUDP::receiveMonitor()
{
	const std::string start = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_RXMON_START);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, start, Log::TYPE::INFO).toString());
	
	const int A_SECOND = 1;
	while(Vars::ustate == Vars::UserState::INCALL)
	{
		if(lastReceivedTimestamp > 0)
		{
			const int ASECOND_AS_US = 1000000;
			struct timeval now;
			memset(&now, 0, sizeof(struct timeval));
			gettimeofday(&now, NULL);
			const int btw = now.tv_usec - lastReceivedTimestamp;
			if(btw > ASECOND_AS_US && mediaSocket != -1)
			{
				const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_LAST_RX_FOREVER);
				logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
				shutdown(mediaSocket, 2);
				close(mediaSocket);
				mediaSocket = -1;
			}
		}
		sleep(A_SECOND);
	}
	
	const std::string stop = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_RXMON_STOP);
	logger->insertLog(Log(Log::TAG::SODIUM_UDP, stop, Log::TYPE::INFO).toString());
}

bool SodiumUDP::reconnectUDP()
{
	std::unique_lock<std::mutex> deadUDPLock(deadUDPMutex);
	if(Vars::ustate == Vars::UserState::NONE)
	{
		std::cout << "no need to try and reconnect\n";
		return false;
	}
	
	const int MAX_UDP_RECONNECTS = 10;
	if(reconnectTries > MAX_UDP_RECONNECTS)
	{
		std::cout <<"over the reconnect limit\n";
		return false;
	}

	if(reconnectionAttempted)
	{
		std::cout << "someone else already tried to reconnect\n";
		reconnectionAttempted = false; //already attempted, reset it for the next connection failure
		return true;
	}
	else
	{
		std::cout <<"trying to reconnect\n";
		rxq.interrupt();
		rxq.clear();
		txq.interrupt();
		txq.clear();
		
		reconnectTries++;
		const bool reconnected = registerUDP();
		reconnectionAttempted = true;
		std::cout <<"did it reconnect? " << std::to_string(reconnected) << "\n";
		return reconnected;
	}
}

void SodiumUDP::stopOnError()
{
	std::unique_lock<std::mutex> stopLock(stopMutex);
	if(!stopRequested)
	{
		stopRequested = true;
		AsyncCentral::getInstance()->broadcast(Vars::Broadcast::CALL_END);
	}
}

std::string SodiumUDP::stats()
{
	statBuilder.str(std::string());
	statBuilder.precision(3); //match the android version
	std::string rxUnits, txUnits;
	
	const int missing = txSeq - rxSeq;
	statBuilder << missingLabel << ": " << (missing > 0 ? missing : 0) << " " << garbageLabel << ": " << garbage << "\n"
			<< rxLabel << ": " << formatInternetMetric(txtotal, rxUnits) << rxUnits << " " << txLabel << ": " << formatInternetMetric(txtotal, txUnits) << txUnits <<"\n"
			<< rxSeqLabel << ": " << txSeq << " " << txSeqLabel << ": " << txSeq << "\n"
			<< skippedLabel << ": " << skipped << " " << oorangeLabel << ":  " << oorange;
	return statBuilder.str();
}

double SodiumUDP::formatInternetMetric(int metric, std::string& units)
{
	const double MEGA = 1000000L;
	const double KILO = 1000L;
	double dmetric = (double)metric;
	if(metric > MEGA)
	{
		units = r->getString(R::StringID::SODIUM_UDP_STAT_MB);
		return dmetric / MEGA;
	}
	else if (metric > KILO)
	{
		units = r->getString(R::StringID::SODIUM_UDP_STAT_KB);
		return dmetric / KILO;
	}
	else
	{
		units = r->getString(R::StringID::SODIUM_UDP_STAT_B);
		return dmetric;
	}
}

bool SodiumUDP::registerUDP()
{
	R* r = R::getInstance();
	Logger* logger = Logger::getInstance();
	
	const bool udpConnected = Utils::connectFD(mediaSocket, SOCK_DGRAM, address, port, &mediaPortAddrIn);
	if(!udpConnected)
	{
		return false;
	}

	struct timeval registerTimeout;
	registerTimeout.tv_sec = 0;
	registerTimeout.tv_usec = 100000;

	if(setsockopt(mediaSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&registerTimeout, sizeof(struct timeval)) < 0)
	{
		std::string error= creationTime + " " +  r->getString(R::StringID::SODIUM_UDP_TIMEOUT_REGISTER) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
	}

	const int UDP_RETRIES = 10;
	int retries = UDP_RETRIES;
	while(retries > 0)
	{
		const std::string registration = std::to_string(Utils::now()) + "|" + Vars::sessionKey;
		const int registrationLength = crypto_box_SEALBYTES + registration.length();
		std::unique_ptr<unsigned char[]> sealedRegistrationArray = std::make_unique<unsigned char[]>(registrationLength);
		unsigned char* sodiumSealedRegistration = sealedRegistrationArray.get();
		const int sealed = crypto_box_seal(sodiumSealedRegistration, (unsigned char*)registration.c_str(), registration.length(), Vars::serverCert.get());
		if(sealed != 0)
		{
			const std::string error = creationTime + " " + r->getString(R::StringID::SODIUM_UDP_REGISTERUDP_SEALFAIL);
			logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
			retries--;
			continue;
		}

		const int sent = sendto(mediaSocket, sodiumSealedRegistration, registrationLength, 0, (struct sockaddr*)&mediaPortAddrIn, sizeof(struct sockaddr_in));
		if(sent < 0)
		{
			retries--;
			continue;
		}

		std::unique_ptr<unsigned char[]> ackBufferArray = std::make_unique<unsigned char[]>(Vars::MAX_UDP);
		unsigned char* ackBuffer = ackBufferArray.get();
		struct sockaddr_in sender;
		socklen_t senderLength = sizeof(struct sockaddr_in);
		const int receivedLength = recvfrom(mediaSocket, ackBuffer, Vars::MAX_UDP, 0, (struct sockaddr*)&sender, &senderLength);
		if(receivedLength < 0)
		{
			retries--;
			continue;
		}

		std::unique_ptr<unsigned char[]> decAck = std::make_unique<unsigned char[]>(2048);
		std::unique_ptr<unsigned char[]> tcpKey;
		Vars::commandSocket.get()->getTcpKeyCopy(tcpKey);
		int decAckLength = 0;
		SodiumUtils::sodiumDecrypt(false, ackBuffer, receivedLength, tcpKey.get(), NULL, decAck, decAckLength);
		if(decAckLength == 0)
		{
			retries--;
			continue;
		}

		const std::string ackString((char*)decAck.get(), decAckLength);
		const bool ackOK = Utils::validTS(ackString);
		if(ackOK)
		{
			useable = true;
			struct timeval noTimeout;
			noTimeout.tv_sec = 0;
			noTimeout.tv_usec = 0;

			if(setsockopt(mediaSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&noTimeout, sizeof(struct timeval)) < 0)
			{
				std::string error= creationTime + " " + r->getString(R::StringID::SODIUM_UDP_TIMEOUT_REMOVE_TIMEOUT) + std::to_string(errno) + ") " + std::string(strerror(errno));
				logger->insertLog(Log(Log::TAG::SODIUM_UDP, error, Log::TYPE::ERROR).toString());
			}
			return true;
		}
		retries--;
	}
	return false;
}