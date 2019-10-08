/*
 * CommandAccept.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_OPERATORCOMMAND_HPP_
#define SRC_BACKGROUND_OPERATORCOMMAND_HPP_

#include <string>
#include <iostream>
#include <thread>

#include <string.h>

#include "AsyncCentral.hpp"
#include "../vars.hpp"
#include "../utils.hpp"
#include "../SodiumSocket.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../R.hpp"

namespace OperatorCommand
{
	typedef enum {ACCEPT, CALL, END} OperatorCommand;
	void execute(OperatorCommand which);
};

#endif /* SRC_BACKGROUND_OPERATORCOMMAND_HPP_ */
