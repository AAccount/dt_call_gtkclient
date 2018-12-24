/*
 * prefs.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: Daniel
 */

#include "prefs.hpp"

Prefs::Prefs(std::string& path) //mostly copied from server's user utils
{
	prefs_path = path;
	stringRes = StringRes::getInstance();

	std::ifstream prefsFile(path);
	std::string line;

	while(std::getline(prefsFile, line))
	{
		//skip blank lines and comment lines
		if(line.length() == 0 || line.at(0) == '#')
		{
			continue;
		}

		//read the preference and value
		std::string name, value, publicKeyDump;
		std::stringstream ss(line);
		getline(ss, name, '=');
		getline(ss, value, '=');

		//cleanup the surrounding whitespace and strip the end of line comment
		name = trim(name);
		value = trim(value);

		//need both a preference and its value to continue
		if(name == "" || value == "")
		{
			std::cerr << stringRes->getString(StringRes::Language::EN, StringRes::StringID::ERR_PREF_BADCONFIG);
			continue;
		}

		std::cout << "READ preference file preference " << name << " has value " << value << "\n";

		//insert into the preferences table. using operator[] because you want it to create a new entry if it doesn't exist
		prefsTable[name] = value;

	}
	prefsFile.close();

}

Prefs::~Prefs()
{
	// TODO Auto-generated destructor stub. Nothing to destruct.
}

int Prefs::getInt(const std::string& prefName, int defaultValue) const
{
	//if the preference doesn't exist return the default
	std::string rawValue;
	if(prefsTable.count(prefName) == 0)
	{
		std::cerr << stringRes->getString(StringRes::Language::EN, StringRes::StringID::ERR_PREF_INTMIA) << prefName << "\n";
		return defaultValue;
	}

	try
	{
		rawValue = prefsTable.at(prefName);
		return std::stoi(rawValue);
	}
	catch(std::exception& e) //catch misconfigured settings
	{
		std::cerr << stringRes->getString(StringRes::Language::EN, StringRes::StringID::ERR_PREF_BADINT) << prefName << " "  << rawValue << "\n";
		std::cerr << e.what();
		return defaultValue;
	}
}

std::string Prefs::getString(const std::string& prefName, std::string defaultValue) const
{
	if(prefsTable.count(prefName) == 0)
	{
		std::cerr << stringRes->getString(StringRes::Language::EN, StringRes::StringID::ERR_PREF_STRINGMIA) << prefName << "\n";
		return defaultValue; //return the default if the preference doesn't exist
	}

	std::string value = prefsTable.at(prefName);
	return value;
}

void Prefs::setInt(const std::string& prefName, int newValue)
{
	prefsTable[prefName] = std::to_string(newValue);
}

void Prefs::setString(const std::string& prefName, std::string newValue)
{
	prefsTable[prefName] = newValue;
}

void Prefs::save() const
{
	std::ofstream prefsOut(prefs_path);
	for(auto& prefs_pair : prefsTable)
	{
		prefsOut << prefs_pair.first << " = " << prefs_pair.second << "\n";
	}
	prefsOut.flush();
	prefsOut.close();
}

//various string constants
const std::string& Prefs::DEFAULT_FILE()
{
	const static std::string value = "~/.dt_call_gtkclient/call_prefs";
	return value;
}

const std::string& Prefs::PREF_ADDR()
{
	const static std::string value = "server_address";
	return value;
}

const std::string& Prefs::PREF_COMMAND_PORT()
{
	const static std::string value = "command_port";
	return value;
}

const std::string& Prefs::PREF_MEDIA_PORT()
{
	const static std::string value = "media_port";
	return value;
}

const std::string& Prefs::PREF_CERT()
{
	const static std::string value = "cert";
	return value;
}

const std::string& Prefs::PREF_UNAME()
{
	const static std::string value = "username";
	return value;
}

const std::string& Prefs::PREF_PRIVATE_KEY()
{
	const static std::string value = "private_key";
	return value;
}

//private functions
//copied and pasted from server's Utils class
std::string Prefs::trim (const std::string &input) const
{//
	//nothing to trim in a blank string
	if(input.length() == 0)
	{
		return input;
	}

	size_t beginning = input.find_first_not_of(" \r\n\t");

	//if there is a comment then start looking BEFORE the comment otherwise find_last_not_of
	//will "OK" the comment characters and fail to trim
	size_t comment = input.find('#');
	size_t ending;
	if(comment != std::string::npos)
	{
		ending = input.find_last_not_of(" #\r\n\t", comment); //strip off the comment
	}
	else
	{
		ending = input.find_last_not_of(" #\r\n\t"); //strip off the comment
	}
	size_t range = ending-beginning+1;
	return input.substr(beginning, range);
}

