/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AsyncCentral.cpp
 * Author: Daniel
 * 
 * Created on September 3, 2019, 11:37 PM
 */

#include "AsyncCentral.hpp"

//static
AsyncCentral* AsyncCentral::instance;

//static
AsyncCentral* AsyncCentral::getInstance()
{
	if(instance == nullptr)
	{
		instance = new AsyncCentral();
	}
	return instance;
}

AsyncCentral::AsyncCentral()
{
	centralThread = std::thread([this] {
		while(!Vars::isExiting)
		{
			std::pair<int, std::string> item = codes.pop();
			std::unique_lock<std::mutex> receiversLock(receiversMutex);
			for(AsyncReceiver* receiver : receivers)
			{
				receiver->asyncResult(item.first, item.second);
			}
		}
	});
	centralThread.detach();
}

AsyncCentral::~AsyncCentral()
{
}

void AsyncCentral::registerReceiver(AsyncReceiver* receiver)
{
	std::unique_lock<std::mutex>receiversLock(receiversMutex);
	receivers.insert(receiver);
}

void AsyncCentral::removeReceiver(AsyncReceiver* receiver)
{
	std::unique_lock<std::mutex>receiversLock(receiversMutex);
	receivers.erase(receiver);
}

void AsyncCentral::broadcast(int code)
{
	codes.push(std::pair<int, std::string>(code, ""));
}

void AsyncCentral::broadcast(int code, const std::string& info)
{
	codes.push(std::pair<int, std::string>(code, info));
}