/*
 * Utils.hpp
 *
 *  Created on: Aug 14, 2016
 *      Author: Daniel
 *
 *  Common stuff used by multiple files
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_
#include <stdint.h>
#include <stdbool.h>
#include <random>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <gtk/gtk.h>
#include <time.h>
#include <sodium.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Logger.hpp"
#include "Log.hpp"
#include "R.hpp"
#include "sodium_utils.hpp"
#include "stringify.hpp"

namespace Utils
{
	//used for parsing the configuration file: remove whitespace preceding/trailing and comments
	void showPopup(const std::string& message, GtkWindow* parent);
	std::string fileChooser(const std::string& message, GtkWindow* parent);
	std::string dumpSmallFile(const std::string& path);
	std::vector<std::string> parse(unsigned char command[]);
	bool validTS(const std::string& ts);
	time_t now();
	void quit(unsigned char privateKey[]);
	bool connectFD(int& fd, int type, const std::string& caddr, int cport, struct sockaddr_in* serv_addr);
	void runOnUiThread(GSourceFunc func);
	void runOnUiThread(GSourceFunc func, gpointer data);
	void setupPublicKey(const std::string& title, GtkWindow* window, Log::TAG tag, const std::string& error, std::unique_ptr<unsigned char[]>& output);
};

#endif /* UTILS_HPP_ */
