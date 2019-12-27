/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Voice.hpp
 * Author: Daniel
 *
 * Created on December 25, 2019, 9:18 PM
 */

#ifndef VOICE_HPP
#define VOICE_HPP

#include <thread>
#include <mutex>
#include <memory>
#include <string>

#include <pulse/simple.h>

#include "../codec/Opus.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../vars.hpp"
#include "../background/AsyncCentral.hpp"
#include "../background/CmdListener.hpp"

class Voice
{
public:
	static Voice* getInstance();
	void start();
	void stop();
	void toggleMic();
	
	int getGarbage() const;
	int getRxtotal() const;
	int getTxtotal() const;
	int getRxSeq() const;
	int getTxSeq() const;
	int getSkipped() const;
	int getOorange() const;
private:
	const static int HEADERS = 52;

	Voice();
	Voice(const Voice& orig) = delete;
	virtual ~Voice();
	static Voice* instance;
	
	int garbage, rxtotal, txtotal, rxSeq, txSeq, skipped, oorange;
	bool mute;
	
	std::thread receiveMonitorThread;
	void receiveMonitor();
	bool receiveMonitorAlive;
	std::mutex receivedTimestampMutex;
	struct timeval lastReceivedTimestamp;
	std::mutex deadUDPMutex;
	bool reconnectionAttempted;
	bool reconnectUDP();
	int reconnectTries;

	bool stopRequested;
	void stopOnError();
	std::mutex stopMutex;
	
	std::thread encodeThread;
	void mediaEncode();
	bool encodeThreadAlive;
	std::thread decodeThread;
	void mediaDecode();
	bool decodeThreadAlive;
	const static int OORANGE_LIMIT = 100;
	
	Logger* logger;
	R* r;
};

#endif /* VOICE_HPP */

