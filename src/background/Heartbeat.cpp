/*
 * Heartbeat.cpp
 *
 *  Created on: Dec 28, 2018
 *      Author: Daniel
 */

#include "Heartbeat.hpp"

namespace
{
	void* heartbeatThread(void* pointer)
	{
		const int TIMEOUT = 60*5;
		const std::string ping = "D";
		while(true)
		{
			try
			{
				Vars::commandSocket.writeString(ping);
				sleep(TIMEOUT);
			}
			catch(std::string& e)
			{
				std::cerr << "couldn't write heartbeat: " << e << "\n";
				Vars::commandSocket.stop();
				LoginAsync::execute(UserHome::instance, true);
				break;
			}
		}
		return 0;
	}
}

void Heartbeat::startService()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, heartbeatThread, NULL) != 0)
	{
		std::string error = "cannot create the login async thread (" + std::to_string(errno) + ") " + std::string(strerror(errno));
		std::cout << error << "\n";
	}
}

