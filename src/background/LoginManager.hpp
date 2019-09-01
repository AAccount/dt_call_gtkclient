/*
 * Login.hpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_LOGINMANAGER_HPP_
#define SRC_BACKGROUND_LOGINMANAGER_HPP_
#include <string>
#include <vector>
#include <memory>
#include <time.h>
#include <thread>
#include <mutex>
#include "AsyncReceiver.hpp"
#include "../vars.hpp"
#include "../utils.hpp"
#include "../stringify.hpp"
#include "../sodium_utils.hpp"
#include "../R.hpp"
#include "../stringify.hpp"
#include "../settings.hpp"
#include "CmdListener.hpp"
#include "../screens/CallScreen.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "Heartbeat.hpp"

class LoginManager
{
public:
	static LoginManager* getInstance();
	void execute(AsyncReceiver* receiver, bool retry);
	
private:
	static LoginManager* instance;
	static bool heartbeatStarted;
	
	LoginManager();
	virtual ~LoginManager();
	
	const int RETRY_DELAY = 60;
	const int LOGIN_MAX_SEGMENTS = 3;
	
	bool running;
	std::mutex inUse;
	R* r;
	Logger* logger;
	
	bool loginFunction(AsyncReceiver* receiver);
};

#endif /* SRC_BACKGROUND_LOGINASYNC_HPP_ */
