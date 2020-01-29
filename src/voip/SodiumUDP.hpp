/* 
 * File:   SodiumUDP.hpp
 * Author: Daniel
 *
 * Created on January 20, 2020, 8:15 PM
 */

#ifndef SODIUMUDP_HPP
#define SODIUMUDP_HPP

#include <thread>
#include <memory>
#include <string>
#include <sstream>
#include <atomic>
#include <mutex>

#include <sodium.h>
#include <sys/time.h>

#include "ByteBufferPool.hpp"
#include "../BlockingQ.hpp"
#include "../R.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "../background/AsyncCentral.hpp"

class SodiumUDP
{
public:
	SodiumUDP(const std::string& server, int port);
	SodiumUDP(const SodiumUDP& orig) = delete; //for now not sure why you'd need to duplicate sodium voice udps
	virtual ~SodiumUDP();
	
	void setVoiceSymmetricKey(std::unique_ptr<unsigned char[]>& key);
	bool connect();
	void start();
	void closeSocket();
	void write(std::unique_ptr<unsigned char[]>& outData, int size);
	int read(std::unique_ptr<unsigned char[]>& inData, int inSize);

	std::string stats();
	
private:
	static int runningID;
	Logger* logger;
	R* r;
	
	const int OORANGE_LIMIT = 100;
	const int HEADERS = 52;

	std::stringstream statBuilder;	
	std::string missingLabel, txLabel, rxLabel, garbageLabel, rxSeqLabel, txSeqLabel, skippedLabel, oorangeLabel;
	std::atomic<int> garbage; 
	std::atomic<int> rxtotal;
	std::atomic<int> txtotal;
	std::atomic<int> rxSeq;
	std::atomic<int> txSeq;
	std::atomic<int> skipped;
	std::atomic<int> oorange;
		
	std::atomic<long> lastReceivedTimestamp;
	std::mutex deadUDPMutex;
	bool reconnectionAttempted;
	bool reconnectUDP();
	int reconnectTries;
	
	void receiveMonitor();
	std::thread receiveMonitorThread;
	bool receiveMonitorAlive;
	
	bool stopRequested;
	void stopOnError();
	std::mutex stopMutex;
	
	void tx();
	bool txalive;
	std::thread txthread;
	ByteBufferPool txpool;
	BlockingQ<std::unique_ptr<unsigned char[]>> txq;
	BlockingQ<int> txqLengths;
	std::unique_ptr<unsigned char[]> txplaintext;
	void rx();
	bool rxalive;
	std::thread rxthread;
	ByteBufferPool rxpool;
	BlockingQ<std::unique_ptr<unsigned char[]>> rxq;
	BlockingQ<int> rxqLengths;
	std::unique_ptr<unsigned char[]> rxplaintext;
	double formatInternetMetric(int metric, std::string& units);
	
	bool registerUDP();
	std::unique_ptr<unsigned char[]> voiceKey;
	int mediaSocket;
	bool useable;
	std::string address;
	struct sockaddr_in mediaPortAddrIn;
	int port;
	
	std::string id;
};

#endif /* SODIUMUDP_HPP */

