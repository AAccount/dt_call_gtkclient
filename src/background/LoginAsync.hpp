/*
 * Login.hpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_LOGINASYNC_HPP_
#define SRC_BACKGROUND_LOGINASYNC_HPP_
#include <string>
#include <vector>
#include <memory>
#include <pthread.h>
#include <time.h>
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

namespace LoginAsync
{
	void init();
	void execute(AsyncReceiver* receiver, bool retry);
};

#endif /* SRC_BACKGROUND_LOGINASYNC_HPP_ */
