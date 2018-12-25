/*
 * prefs.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: Daniel
 */

#include "settings.hpp"

Settings* Settings::instance = NULL;

Settings* Settings::getInstance()
{
	if(instance == NULL)
	{
		instance = new Settings();
	}
	return instance;
}

Settings::Settings() //mostly copied from server's user utils
: settingsNames(std::unordered_map<SettingName, std::string>()),
  locationValues(std::unordered_map<FileLocation, std::string>()),
  settingsTable(std::unordered_map<std::string, std::string>()),
  contacts(std::unordered_map<std::string, std::string>()),
  publicKeys(std::unordered_map<std::string, std::string>())
{
	strings = StringRes::getInstance();

	settingsNames[SettingName::SETTING_ADDR] = "server_address";
	settingsNames[SettingName::SETTING_COMMAND_PORT] = "command_port";
	settingsNames[SettingName::SETTING_MEDIA_PORT] = "media_port";
	settingsNames[SettingName::SETTING_CERT] = "server_publickey";
	settingsNames[SettingName::SETTING_UNAME] = "username";
	settingsNames[SettingName::SETTING_PRIVATE_KEY] = "user_privatekey";

	const std::string home(getenv("HOME"));
	const std::string prefix = home+"/.DTCallClient/";
	locationValues[FileLocation::LOCATION_SETTINGS_FILE] = prefix+"settings";
	locationValues[FileLocation::LOCATION_CONTACTS_MAPPING] = prefix+"contacts";
	locationValues[FileLocation::LOCATION_PUBLIC_KEYS] = prefix+"public_keys_stringified";
	locationValues[FileLocation::LOCATION_SERVER_CERT] = prefix+"server_cert.na";
	locationValues[FileLocation::LOCATION_PRIVATE_KEY] = prefix+"my_private.na";

	parseSettingsSave(locationValues[FileLocation::LOCATION_SETTINGS_FILE], settingsTable);
	parseSettingsSave(locationValues[FileLocation::LOCATION_CONTACTS_MAPPING], contacts);
	parseSettingsSave(locationValues[FileLocation::LOCATION_PUBLIC_KEYS], publicKeys);
}

Settings::~Settings()
{
	// TODO Auto-generated destructor stub. Nothing to destruct.
}

int Settings::getInt(SettingName setting, int defaultValue) const
{
	if(settingsNames.count(setting) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_INTMIA) << std::to_string(setting) << "\n";
		return defaultValue;
	}

	std::string rawValue;
	const std::string settingName = settingsNames.at(setting);
	if(settingsTable.count(settingName) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_INTMIA) << settingName << "\n";
		return defaultValue;
	}

	try
	{
		rawValue = settingsTable.at(settingName);
		return std::stoi(rawValue);
	}
	catch(std::exception& e) //catch misconfigured settings
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_BADINT) << settingName << " "  << rawValue << "\n";
		std::cerr << e.what();
		return defaultValue;
	}
}

std::string Settings::getString(SettingName setting, std::string defaultValue) const
{
	if(settingsNames.count(setting) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_STRINGMIA) << std::to_string(setting) << "\n";
		return defaultValue;
	}

	const std::string settingName = settingsNames.at(setting);
	if(settingsTable.count(settingName) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_STRINGMIA) << settingName << "\n";
		return defaultValue;
	}

	std::string value = settingsTable.at(settingName);
	return value;
}

void Settings::setInt(SettingName setting, int newValue)
{
	if(settingsNames.count(setting) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_INTMIA) << std::to_string(setting) << "\n";
		return;
	}
	const std::string settingName = settingsNames[setting];
	settingsTable[settingName] = std::to_string(newValue);
}

void Settings::setString(SettingName setting, std::string newValue)
{
	if(settingsNames.count(setting) == 0)
	{
		std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_STRINGMIA) << std::to_string(setting) << "\n";
		return;
	}
	const std::string settingName = settingsNames[setting];
	settingsTable[settingName] = newValue;
}

void Settings::modifyContact(const std::string& name, const std::string& nickName)
{
	if(nickName == "")
	{
		contacts[name] = name;
	}
	else
	{
		contacts[name] = nickName;
	}
}

void Settings::removeContact(const std::string& name)
{
	if(contacts.count(name) > 0)
	{
		contacts.erase(name);
	}
}

std::string Settings::getNickname(const std::string& name) const
{
	if(contacts.count(name) > 0)
	{
		return contacts.at(name);
	}
	else
	{
		return name;
	}
}

void Settings::modifyPublicKey(const std::string& name, std::unique_ptr<unsigned char[]>& publicKey)
{
	publicKeys[name] = Stringify::stringify(publicKey.get(), crypto_box_PUBLICKEYBYTES);
}

void Settings::removePublicKey(const std::string& name)
{
	publicKeys.erase(name);
}

void Settings::getPublicKey(const std::string& name, std::unique_ptr<unsigned char[]>& output) const
{
	if(publicKeys.count(name) > 0)
	{
		output = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
		const std::string publicKeyString = publicKeys.at(name);
		Stringify::destringify(publicKeyString, output.get());
	}
	else
	{
		output = std::unique_ptr<unsigned char[]>();
	}
}

void Settings::save() const
{
	writeSettingsSave(locationValues.at(FileLocation::LOCATION_SETTINGS_FILE), settingsTable);
	writeSettingsSave(locationValues.at(FileLocation::LOCATION_CONTACTS_MAPPING), contacts);
	writeSettingsSave(locationValues.at(FileLocation::LOCATION_PUBLIC_KEYS), publicKeys);
	writeSodiumFile(SettingName::SETTING_CERT);
	writeSodiumFile(SettingName::SETTING_PRIVATE_KEY);
}

void Settings::writeSodiumFile(SettingName setting) const
{
	std::string header = "";
	std::string stringifiedKey = "";
	std::string output = "";
	switch(setting)
	{
	case SettingName::SETTING_CERT:
	{
		if(Vars::serverCert.get() == NULL) return;
		header = SodiumUtils::SODIUM_PUBLIC_HEADER();
		stringifiedKey = Stringify::stringify(Vars::serverCert.get(), crypto_box_PUBLICKEYBYTES);
		output = locationValues.at(FileLocation::LOCATION_SERVER_CERT);
		break;
	}
	case SettingName::SETTING_PRIVATE_KEY:
	{
		if(Vars::privateKey.get() == NULL) return;
		header = SodiumUtils::SODIUM_PRIVATE_HEADER();
		stringifiedKey = Stringify::stringify(Vars::privateKey.get(), crypto_box_SECRETKEYBYTES);
		output = locationValues.at(FileLocation::LOCATION_PRIVATE_KEY);
		break;
	}
	default:
		return;
	}

	std::ofstream sodiumOut(output);
	sodiumOut << header;
	sodiumOut << stringifiedKey;
	sodiumOut.flush();
	sodiumOut.close();

	if(setting == SettingName::SETTING_PRIVATE_KEY)
	{
		const char* privateKeyStringMemory = &stringifiedKey[0];
		randombytes_buf((void*)privateKeyStringMemory, stringifiedKey.length());
	}
}

bool Settings::getSodiumFile(SettingName setting) const
{
	switch(setting)
	{
	case SettingName::SETTING_CERT:
	{
		const std::string location = locationValues.at(FileLocation::LOCATION_SERVER_CERT);
		const std::string dump = Utils::dumpSmallFile(location);
		if(dump == "")
		{
			Vars::serverCert = std::unique_ptr<unsigned char[]>();
			return false;
		}
		if(!SodiumUtils::checkSodiumPublic(dump)) return false;
		const std::string dumpStringified = dump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES*3);
		Vars::serverCert = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
		Stringify::destringify(dumpStringified, Vars::serverCert.get());
		return true;
		break;
	}
	case SettingName::SETTING_PRIVATE_KEY:
	{
		const std::string location = locationValues.at(FileLocation::LOCATION_PRIVATE_KEY);
		const std::string dump = Utils::dumpSmallFile(location);
		if(dump == "")
		{
			Vars::privateKey = std::unique_ptr<unsigned char[]>();
			return false;
		}
		if(!SodiumUtils::checkSodiumPrivate(dump)) return false;
		const std::string dumpStringified = dump.substr(SodiumUtils::SODIUM_PRIVATE_HEADER().length(), crypto_box_SECRETKEYBYTES*3);
		Vars::privateKey = std::make_unique<unsigned char[]>(crypto_box_SECRETKEYBYTES);
		Stringify::destringify(dumpStringified, Vars::privateKey.get());
		const char* privateKeyStringMemory = &dump[0];
		randombytes_buf((void*)privateKeyStringMemory, dump.length());
		return true;
		break;
	}
	}
	return false;
}

//private functions
//copied and pasted from server's Utils class
std::string Settings::trim (const std::string &input) const
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

void Settings::parseSettingsSave(const std::string& location, std::unordered_map<std::string, std::string>& output)
{
	std::ifstream settingsFile(location);
	std::string line;
	if(!settingsFile.good())
	{
		return;
	}

	while(std::getline(settingsFile, line))
	{
		//skip blank lines and comment lines
		if(line.length() == 0 || line.at(0) == '#')
		{
			continue;
		}

		//read the setting and value
		std::string name, value, publicKeyDump;
		std::stringstream ss(line);
		getline(ss, name, '=');
		getline(ss, value, '=');

		//cleanup the surrounding whitespace and strip the end of line comment
		name = trim(name);
		value = trim(value);

		//need both a setting and its value to continue
		if(name == "" || value == "")
		{
			std::cerr << strings->getString(StringRes::Language::EN, StringRes::StringID::SETTINGS_BADCONFIG);
			continue;
		}

		std::cerr << "READ from settings file: " << name << " has value " << value << "\n";

		//insert into the preferences table. using operator[] because you want it to create a new entry if it doesn't exist
		output[name] = value;

	}
	settingsFile.close();
}

void Settings::writeSettingsSave(const std::string& location, const std::unordered_map<std::string, std::string>& input) const
{
	std::ofstream settingsOut(location);
	settingsOut << strings->getString(Vars::lang, StringRes::StringID::SETTINGS_AUTOGEN_WARNING);
	for(auto& prefs_pair : input)
	{
		settingsOut << prefs_pair.first << " = " << prefs_pair.second << "\n";
	}
	settingsOut.flush();
	settingsOut.close();
}
