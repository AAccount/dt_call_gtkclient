/*
 * CommandAccept.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_COMMANDACCEPT_HPP_
#define SRC_BACKGROUND_COMMANDACCEPT_HPP_

#include <string>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include "AsyncReceiver.hpp"
#include "../vars.hpp"
#include "../utils.hpp"
#include "../SodiumSocket.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../R.hpp"

namespace CommandAccept
{
	void execute();
};

#endif /* SRC_BACKGROUND_COMMANDACCEPT_HPP_ */
