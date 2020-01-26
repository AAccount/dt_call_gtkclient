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
#include <memory>
#include <string>
#include <atomic>
#include <sstream>

#include <pulse/simple.h>
#include <limits.h>
#include <math.h>

#include "../codec/Opus.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../vars.hpp"
#include "../background/AsyncCentral.hpp"
#include "../background/CmdListener.hpp"
#include "SodiumUDP.hpp"

class Voice
{
public:
	static Voice* getInstance();
	bool connect();
	void start();
	void stop();
	void toggleMic();
	void setVoiceKey(std::unique_ptr<unsigned char[]>& key);	
	std::string stats();
	
private:
	Voice();
	Voice(const Voice& orig) = delete;
	virtual ~Voice();
	static Voice* instance;
	
	Logger* logger;
	R* r;
	
	SodiumUDP udp;
	bool mute;
	
	bool stopRequested;
	void stopOnError();
	std::mutex stopMutex;
	
	std::thread encodeThread;
	void mediaEncode();
	bool encodeThreadAlive;
	std::atomic<double> encodedb;
	std::string encodedbLabel;
	
	std::thread decodeThread;
	void mediaDecode();
	bool decodeThreadAlive;
	std::atomic<double> decodedb;
	std::string decodedbLabel;
	
	double db(std::unique_ptr<short[]>& sound, int size);
	std::stringstream statBuilder;
};

#endif /* VOICE_HPP */

