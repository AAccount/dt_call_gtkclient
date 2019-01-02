/*
 * Heartbeat.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_HEARTBEAT_HPP_
#define SRC_BACKGROUND_HEARTBEAT_HPP_

#include <iostream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../vars.hpp"
#include "LoginAsync.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../R.hpp"

namespace Heartbeat
{
	void startService();
};

#endif /* SRC_BACKGROUND_HEARTBEAT_HPP_ */
