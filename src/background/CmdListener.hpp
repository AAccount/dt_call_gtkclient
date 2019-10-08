/*
 * CmdListener.h
 *
 *  Created on: Dec 27, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_CMDLISTENER_HPP_
#define SRC_BACKGROUND_CMDLISTENER_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <thread>
#include <string.h>
#include <time.h>
#include <sodium.h>
#include <sys/socket.h>
#include "../vars.hpp"
#include "../utils.hpp"
#include "../sodium_utils.hpp"
#include "../settings.hpp"
#include "../screens/CallScreen.hpp"
#include "LoginManager.hpp"
#include "AsyncCentral.hpp"
#include "OperatorCommand.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "../R.hpp"

class CmdListener
{
public:
	static void startService();
	static bool registerUDP();
	
private:
	static CmdListener* instance;
	CmdListener();
	virtual ~CmdListener();
	
	R* r;
	Logger* logger;
	AsyncCentral* asyncCentral;
	
	const int COMMAND_MAX_SEGMENTS = 5;
	bool isCallInitiator;
	bool haveVoiceKey;
	bool preparationsComplete;
	
	void startInternal();
	void sendReady();
	void giveUp();
	std::string censorIncomingCmd(const std::vector<std::string>& parsed);
};

#endif /* SRC_BACKGROUND_CMDLISTENER_HPP_ */
