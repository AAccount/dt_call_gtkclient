/*
 * settings_ui.cpp
 *
 *  Created on: Dec 28, 2017
 *      Author: Daniel
 */

#include "SettingsUI.hpp"

SettingsUI* SettingsUI::instance = NULL;
bool SettingsUI::onScreen = false;
bool SettingsUI::initialSetup;

extern "C" void settings_ui_quit()
{
	if(SettingsUI::initialSetup)
	{
		Vars::commandSocket.stop();
		Utils::quit(Vars::privateKey.get(), Vars::voiceKey.get());
	}
}

SettingsUI::SettingsUI() :
r(R::getInstance()),
logger(Logger::getInstance(""))
{
	GtkBuilder* builder = gtk_builder_new_from_resource("/gtkclient/settings_ui.glade");
	window = GTK_WINDOW(gtk_builder_get_object(builder, "window_settings_ui"));
	addr = GTK_ENTRY(gtk_builder_get_object(builder, "id_settings_ui_addr"));
	gtk_entry_set_placeholder_text(addr, r->getString(R::StringID::SETTINGS_UI_PLACEHOLDER_ADDR).c_str());
	commandPort = GTK_ENTRY(gtk_builder_get_object(builder, "id_settings_ui_command"));
	gtk_entry_set_placeholder_text(commandPort, r->getString(R::StringID::SETTINGS_UI_PLACEHOLDER_COMMAND).c_str());
	mediaPort = GTK_ENTRY(gtk_builder_get_object(builder, "id_settings_ui_media"));
	gtk_entry_set_placeholder_text(mediaPort, r->getString(R::StringID::SETTINGS_UI_PLACEHOLDER_MEDIA).c_str());
	serverCert = GTK_BUTTON(gtk_builder_get_object(builder, "id_settings_ui_certificate"));
	gtk_button_set_label(serverCert, r->getString(R::StringID::SETTINGS_UI_CHOOSE_SERVER_CERT).c_str());
	username = GTK_ENTRY(gtk_builder_get_object(builder, "id_settings_ui_name"));
	gtk_entry_set_placeholder_text(username, r->getString(R::StringID::SETTINGS_UI_PLACEHOLDER_NAME).c_str());
	privateKey = GTK_BUTTON(gtk_builder_get_object(builder, "id_settings_ui_private_key"));
	gtk_button_set_label(privateKey, r->getString(R::StringID::SETTINGS_UI_CHOOSE_PRIVATE_KEY).c_str());
	next = GTK_BUTTON(gtk_builder_get_object(builder, "id_settings_ui_next"));
	if(initialSetup)
	{
		gtk_window_set_title(window, r->getString(R::StringID::SETTINGS_UI_TITLE_INITIAL_SETUP).c_str());
		gtk_button_set_label(next, r->getString(R::StringID::SETTINGS_UI_LOGIN).c_str());
	}
	else
	{
		gtk_window_set_title(window, r->getString(R::StringID::SETTINGS_UI_TITLE_EDIT).c_str());
		gtk_button_set_label(next, r->getString(R::StringID::GENERIC_SAVE).c_str());
	}
	destroySignalID = g_signal_connect(G_OBJECT(window),"destroy", settings_ui_quit, NULL);

	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);
	gtk_widget_show((GtkWidget*)window);

	if(!Vars::serverAddress.empty())
	{
		gtk_entry_set_text(addr, Vars::serverAddress.c_str());
	}
	if(Vars::commandPort != 0)
	{
		gtk_entry_set_text(commandPort, std::to_string(Vars::commandPort).c_str());
	}
	if(Vars::mediaPort != 0)
	{
		gtk_entry_set_text(mediaPort, std::to_string(Vars::mediaPort).c_str());
	}
	if(!Vars::username.empty())
	{
		gtk_entry_set_text(username, Vars::username.c_str());
	}
	if(Vars::serverCert.get() != NULL)
	{
		const std::string certOk = r->getString(R::StringID::SETTINGS_UI_SERVER_CERT_OK);
		gtk_button_set_label(serverCert, certOk.c_str());
		gotServerCert = true;
	}
	if(Vars::privateKey.get() != NULL)
	{
		const std::string privateOk = r->getString(R::StringID::SETTINGS_UI_PRIVATE_KEY_OK);
		gtk_button_set_label(privateKey, privateOk.c_str());
		gotPrivateKey = true;
	}
}

SettingsUI::~SettingsUI()
{
	//this window is purposely going away, unhook it from the main quit function
	g_signal_handler_disconnect(window, destroySignalID);

	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
SettingsUI* SettingsUI::getInstance()
{
	return instance;
}

//static
int SettingsUI::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new SettingsUI();
	return 0;
}

//static
int SettingsUI::remove(void* a)
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
	return 0;
}

//static
void SettingsUI::setupCertificate()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::SETTINGS_UI, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	const std::string message = r->getString(R::StringID::SETTINGS_UI_CHOOSE_SERVER_CERT);
	const std::string error = r->getString(R::StringID::SETTINGS_UI_BAD_SERVER_CERT);
	Utils::setupPublicKey(message, window, Log::TAG::SETTINGS_UI, error, Vars::serverCert);

	const std::string certOk = r->getString(R::StringID::SETTINGS_UI_SERVER_CERT_OK);
	logger->insertLog(Log(Log::TAG::SETTINGS_UI, certOk, Log::TYPE::INFO).toString());
	gtk_button_set_label(serverCert, certOk.c_str());
	gotServerCert = true;
}

void SettingsUI::setupPrivateKey()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::SETTINGS_UI, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	const std::string message = r->getString(R::StringID::SETTINGS_UI_CHOOSE_PRIVATE_KEY);
	const std::string privateKeyPath = Utils::fileChooser(message, window);
	if(privateKeyPath == "")
	{
		return;
	}

	const std::string privateKeyDump = Utils::dumpSmallFile(privateKeyPath);
	const bool ok = SodiumUtils::checkSodiumPrivate(privateKeyDump);
	if(!ok)
	{
		const std::string error = r->getString(R::StringID::SETTINGS_UI_PRIVATE_KEY_BAD);
		logger->insertLog(Log(Log::TAG::SETTINGS_UI, error, Log::TYPE::ERROR).toString());
		Utils::showPopup(error, window);
		return;
	}

	const std::string privateKeyStringified = privateKeyDump.substr(SodiumUtils::SODIUM_PRIVATE_HEADER().length(), crypto_box_SECRETKEYBYTES*3);
	Vars::privateKey = std::make_unique<unsigned char[]>(crypto_box_SECRETKEYBYTES);
	Stringify::destringify(privateKeyStringified, Vars::privateKey.get());
	const std::string privateOk = r->getString(R::StringID::SETTINGS_UI_PRIVATE_KEY_OK);
	logger->insertLog(Log(Log::TAG::SETTINGS_UI, privateOk, Log::TYPE::INFO).toString());
	gtk_button_set_label(privateKey, privateOk.c_str());
	gotPrivateKey = true;
}

void SettingsUI::nextFunction()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::SETTINGS_UI, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	std::string enteredAddress(gtk_entry_get_text(addr));
	removeWhiteSpace(enteredAddress);
	if(!enteredAddress.empty())
	{
		Vars::serverAddress = enteredAddress;
	}

	try
	{
		const std::string command_string = std::string(gtk_entry_get_text(commandPort));
		Vars::commandPort = std::stoi(command_string);
		const std::string media_string = std::string(gtk_entry_get_text(mediaPort));
		Vars::mediaPort = std::stoi(media_string);
	}
	catch(std::invalid_argument& e)
	{
		std::cerr << e.what() << "\n";
		return;
	}

	std::string enteredUsername(gtk_entry_get_text(username));
	removeWhiteSpace(enteredUsername);
	if(!enteredUsername.empty())
	{
		Vars::username = enteredUsername;
	}

	if(initialSetup)
	{
		login();
	}
	else
	{
		saveSettings();
		remove(NULL);
	}
}

void SettingsUI::removeWhiteSpace(std::string& s)
{
	s.erase(std::remove_if(s.begin(), s.end(),
			[](char c)
			{
				return std::isspace(c);
			}), s.end());
}


void SettingsUI::login()
{
	const std::string trial = "address: " + Vars::serverAddress + " command: " + std::to_string(Vars::commandPort) + " media: " + std::to_string(Vars::mediaPort) + " Username: " + Vars::username;
	logger->insertLog(Log(Log::TAG::USER_HOME, trial, Log::TYPE::INFO).toString());

	if(!Vars::serverAddress.empty() && Vars::commandPort != 0 && Vars::mediaPort != 0 && gotServerCert
			&& !Vars::username.empty() && gotPrivateKey)
	{
		LoginAsync::execute(this, false);
	}
	else
	{
		const std::string error = r->getString(R::StringID::SETTINGS_UI_INCOMPLETE);
		Utils::showPopup(error, window);
	}
}
void SettingsUI::saveSettings()
{
	Settings* settings = Settings::getInstance();
	settings->setString(Settings::SettingName::SETTING_ADDR, Vars::serverAddress);
	settings->setInt(Settings::SettingName::SETTING_COMMAND_PORT, Vars::commandPort);
	settings->setInt(Settings::SettingName::SETTING_MEDIA_PORT, Vars::mediaPort);
	settings->setString(Settings::SettingName::SETTING_UNAME, Vars::username);
	settings->save();
}

void SettingsUI::asyncResult(int result)
{
	if(result == Vars::Broadcast::LOGIN_OK)
	{
		saveSettings();
		Utils::runOnUiThread(&UserHome::render);
	}
	else if(result == Vars::Broadcast::LOGIN_NOTOK)
	{
		const std::string error = r->getString(R::StringID::SETTINGS_UI_LOGIN_FAIL);
		Utils::showPopup(error, window);
	}
}

extern "C" void onclick_settings_ui_certificate()
{
	SettingsUI* instance = SettingsUI::getInstance();
	if(instance != NULL)
	{
		instance->setupCertificate();
	}
}

extern "C" void onclick_settings_ui_private_key()
{
	SettingsUI* instance = SettingsUI::getInstance();
	if(instance != NULL)
	{
		instance->setupPrivateKey();
	}
}

extern "C" void onclick_settings_ui_next()
{
	SettingsUI* instance = SettingsUI::getInstance();
	if(instance != NULL)
	{
		instance->nextFunction();
	}
}
