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
#include <gtk/gtk.h>
#include <time.h>
#include <sodium.h>
#include "vars.hpp"

namespace Utils
{
	//used for parsing the configuration file: remove whitespace preceding/trailing and comments
	void show_popup(const std::string& message, GtkWindow* parent);
	std::string file_chooser(const std::string& message, GtkWindow* parent);
	std::string dumpSmallFile(const std::string& path);
	std::vector<std::string> parse(unsigned char command[]);
	bool validTS(time_t ts);
	time_t now();
	void quit();
};

#endif /* UTILS_HPP_ */
