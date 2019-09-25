/*
 * AsyncReceiver.hpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_ASYNCRECEIVER_HPP_
#define SRC_BACKGROUND_ASYNCRECEIVER_HPP_

#include <string>

class AsyncReceiver
{
public:
	virtual ~AsyncReceiver(){};
	virtual void asyncResult(int result, const std::string& info) = 0;
};

#endif /* SRC_BACKGROUND_ASYNCRECEIVER_HPP_ */
