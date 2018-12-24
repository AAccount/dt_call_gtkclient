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
#include "../StringRes.hpp"

namespace LoginAsync
{
	typedef enum {LOGIN_OK, LOGIN_NOTOK} LoginResult;
	void execute(AsyncReceiver* receiver);
};

#endif /* SRC_BACKGROUND_LOGINASYNC_HPP_ */
