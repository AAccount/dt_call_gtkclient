/*
 * prefs.hpp
 *
 *  Created on: Dec 13, 2017
 *      Author: Daniel
 */

#ifndef SRC_SETTINGS_HPP_
#define SRC_SETTINGS_HPP_
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include <sodium.h>
#include <stdlib.h>

#include "R.hpp"
#include "vars.hpp"
#include "stringify.hpp"
#include "sodium_utils.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "Log.hpp"

class Settings
{
public:
	static Settings* getInstance();

	typedef enum
	{
		SETTING_ADDR,
		SETTING_COMMAND_PORT,
		SETTING_MEDIA_PORT,
		SETTING_CERT,
		SETTING_UNAME,
		SETTING_PRIVATE_KEY
	} SettingName;

	int getInt(SettingName setting, int default_value) const;
	std::string getString(SettingName setting, std::string default_value) const;
	void setInt(SettingName setting, int newValue);
	void setString(SettingName setting, std::string& newValue);

	std::vector<std::string> getAllContacts() const;
	void modifyContact(const std::string& name, const std::string& nickName);
	void removeContact(const std::string& name);
	std::string getNickname(const std::string& name) const;
	bool contactExists(const std::string& name) const;

	std::vector<std::string> getAllRegisteredPublicKeys() const;
	void modifyPublicKey(const std::string& name, std::unique_ptr<unsigned char[]>& publicKey);
	void removePublicKey(const std::string& name);
	void getPublicKey(const std::string& name, std::unique_ptr<unsigned char[]>& output) const;
	std::string getPublicKeyString(const std::string& name) const;

	void save() const;
	void writeSodiumFile(SettingName setting) const;
	bool getSodiumFile(SettingName setting) const;

private:
	typedef enum
	{
		LOCATION_SETTINGS_FILE,
		LOCATION_CONTACTS_MAPPING,
		LOCATION_PUBLIC_KEYS,
		LOCATION_SERVER_CERT,
		LOCATION_PRIVATE_KEY
	} FileLocation;

	static Settings* instance;
	Settings();
	virtual ~Settings();
	void parseSettingsSave(const std::string& location, std::unordered_map<std::string, std::string>& output);
	void writeSettingsSave(const std::string& location, const std::unordered_map<std::string, std::string>& input) const;

	std::unordered_map<std::string, std::string> settingsTable;
	std::unordered_map<std::string, std::string> contacts;
	std::unordered_map<std::string, std::string> publicKeys;

	std::unordered_map<SettingName, std::string> settingsNames;
	std::unordered_map<FileLocation, std::string> locationValues;

	std::string trim(const std::string &input) const;

	R* r;
	Logger* logger;
};

#endif /* SRC_SETTINGS_HPP_ */
