/* 
 * File:   AsyncCentral.hpp
 * Author: Daniel
 *
 * Created on September 3, 2019, 11:37 PM
 */

#ifndef ASYNCCENTRAL_HPP
#define ASYNCCENTRAL_HPP

#include <unordered_set>
#include <thread>
#include <mutex>

#include "AsyncReceiver.hpp"
#include "../BlockingQ.hpp"
#include "../vars.hpp"

class AsyncCentral
{
public:
	static AsyncCentral* getInstance();
	void registerReceiver(AsyncReceiver* receiver);
	void removeReceiver(AsyncReceiver* receiver);
	void broadcast(int code);
	void broadcast(int code, const std::string& info);
	
private:
	static AsyncCentral* instance;
	
	AsyncCentral();
	AsyncCentral(const AsyncCentral& orig) = delete;
	virtual ~AsyncCentral();
	
	std::unordered_set<AsyncReceiver*> receivers;
	BlockingQ<std::pair<int, std::string>> codes;
	std::mutex receiversMutex;
};

#endif /* ASYNCCENTRAL_HPP */

