/*
 * prefs.hpp
 *
 *  Created on: Dec 13, 2017
 *      Author: Daniel
 */

#ifndef SRC_PREFS_HPP_
#define SRC_PREFS_HPP_
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "StringRes.hpp"


class Prefs
{
public:
	Prefs(std::string& path);
	virtual ~Prefs();

	int getInt(const std::string& prefName, int default_value) const;
	std::string getString(const std::string& prefName, std::string default_value) const;
	void setInt(const std::string& prefName, int newValue);
	void setString(const std::string& prefName, std::string newValue);
	void save() const;

	//preference name constants
	static const std::string& DEFAULT_FILE();
	static const std::string& PREF_ADDR();
	static const std::string& PREF_COMMAND_PORT();
	static const std::string& PREF_MEDIA_PORT();
	static const std::string& PREF_CERT();
	static const std::string& PREF_UNAME();
	static const std::string& PREF_PRIVATE_KEY();

private:
	std::string prefs_path;
	std::unordered_map<std::string, std::string> prefsTable;

	std::string trim(const std::string &input) const;

	StringRes* stringRes;
};

#endif /* SRC_PREFS_HPP_ */
