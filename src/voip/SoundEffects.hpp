/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SoundEffects.hpp
 * Author: Daniel
 *
 * Created on December 25, 2019, 9:19 PM
 */

#ifndef SOUNDEFFECTS_HPP
#define SOUNDEFFECTS_HPP

#include <thread>
#include <mutex>
#include <string>
#include <memory>

#include <pulse/simple.h>

#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../vars.hpp"

class SoundEffects
{
public:
	static SoundEffects* getInstance();
	void ring();
	void stopRing();
	
private:
	SoundEffects();
	SoundEffects(const SoundEffects& orig) = delete;
	virtual ~SoundEffects();
	static SoundEffects* instance;
	
	constexpr static double RINGTONE_SAMPLERATE = 8000.0;
	constexpr static double TONE_TIME = 1.5;
	constexpr static double SILENCE_TIME = 1.0;
	constexpr static double RINGTONE_DIVISION = 10.0;
	
	std::unique_ptr<short[]> ringtone;
	
	pa_simple* ringtonePlayer = NULL;
	std::atomic<bool> ringtoneDone;
	std::thread ringThread;
	bool ringThreadAlive;
	
	Logger* logger;
	R* r;
};

#endif /* SOUNDEFFECTS_HPP */

