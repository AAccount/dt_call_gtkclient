/*
 * AsyncReceiver.hpp
 *
 *  Created on: Dec 22, 2018
 *      Author: Daniel
 */

#ifndef SRC_BACKGROUND_ASYNCRECEIVER_HPP_
#define SRC_BACKGROUND_ASYNCRECEIVER_HPP_
class AsyncReceiver
{
public:
	virtual ~AsyncReceiver(){};
	virtual void asyncResult(int result) = 0;
};

#endif /* SRC_BACKGROUND_ASYNCRECEIVER_HPP_ */
