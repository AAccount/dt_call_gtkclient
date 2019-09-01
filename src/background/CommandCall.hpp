/*
 * CommandCall.hpp
 *
 *  Created on: Dec 31, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_COMMANDCALL_HPP_
#define SRC_BACKGROUND_COMMANDCALL_HPP_

#include <string>
#include <iostream>
#include <string.h>
#include <thread>
#include "AsyncReceiver.hpp"
#include "../vars.hpp"
#include "../utils.hpp"
#include "../SodiumSocket.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../R.hpp"

namespace CommandCall
{
	void execute();
};

#endif /* SRC_BACKGROUND_COMMANDCALL_HPP_ */
