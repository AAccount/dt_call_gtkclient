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
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sodium.h>
#include <sys/socket.h>
#include "../vars.hpp"
#include "../utils.hpp"
#include "../screens/UserHome.hpp"
#include "../sodium_utils.hpp"
#include "../settings.hpp"
#include "../screens/CallScreen.hpp"
#include "CommandEnd.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "../R.hpp"

namespace CmdListener
{
	void startService();
	bool registerUDP();
};

#endif /* SRC_BACKGROUND_CMDLISTENER_HPP_ */
