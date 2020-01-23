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

#include <pulse/simple.h>

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
	void setVoiceKey(std::unique_ptr<unsigned char[]> key);	
	std::string stats();
	
private:

	Voice();
	Voice(const Voice& orig) = delete;
	virtual ~Voice();
	static Voice* instance;

	SodiumUDP udp;
	bool mute;
	
	bool stopRequested;
	void stopOnError();
	std::mutex stopMutex;
	
	std::thread encodeThread;
	void mediaEncode();
	bool encodeThreadAlive;
	std::thread decodeThread;
	void mediaDecode();
	bool decodeThreadAlive;
	
	Logger* logger;
	R* r;
};

#endif /* VOICE_HPP */

