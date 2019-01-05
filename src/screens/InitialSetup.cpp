/*
 * initial_setup.cpp
 *
 *  Created on: Dec 28, 2017
 *      Author: Daniel
 */

#include "InitialSetup.hpp"

InitialSetup* InitialSetup::instance = NULL;
bool InitialSetup::onScreen = false;

extern "C" void initial_setup_quit()
{
	Vars::commandSocket.stop();
	Utils::quit(Vars::privateKey.get(), Vars::voiceKey.get());
}

InitialSetup::InitialSetup()
{
	GtkBuilder* builder;
	r = R::getInstance();
	logger = Logger::getInstance("");

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/initial_setup.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "window_initial_setup"));
	gtk_window_set_title(window, r->getString(R::StringID::INITIAL_SETUP_TITLE).c_str());
	addr = GTK_ENTRY(gtk_builder_get_object(builder, "id_initial_setup_addr"));
	gtk_entry_set_placeholder_text(addr, r->getString(R::StringID::INITIAL_SETUP_PLACEHOLDER_ADDR).c_str());
	commandPort = GTK_ENTRY(gtk_builder_get_object(builder, "id_initial_setup_command"));
	gtk_entry_set_placeholder_text(commandPort, r->getString(R::StringID::INITIAL_SETUP_PLACEHOLDER_COMMAND).c_str());
	mediaPort = GTK_ENTRY(gtk_builder_get_object(builder, "id_initial_setup_media"));
	gtk_entry_set_placeholder_text(mediaPort, r->getString(R::StringID::INITIAL_SETUP_PLACEHOLDER_MEDIA).c_str());
	serverCert = GTK_BUTTON(gtk_builder_get_object(builder, "id_initial_setup_certificate"));
	gtk_button_set_label(serverCert, r->getString(R::StringID::INITIAL_SETUP_CHOOSE_SERVER_CERT).c_str());
	username = GTK_ENTRY(gtk_builder_get_object(builder, "id_initial_setup_name"));
	gtk_entry_set_placeholder_text(username, r->getString(R::StringID::INITIAL_SETUP_PLACEHOLDER_NAME).c_str());
	privateKey = GTK_BUTTON(gtk_builder_get_object(builder, "id_initial_setup_private_key"));
	gtk_button_set_label(privateKey, r->getString(R::StringID::INITIAL_SETUP_CHOOSE_PRIVATE_KEY).c_str());
	login = GTK_BUTTON(gtk_builder_get_object(builder, "id_initial_setup_next"));
	gtk_button_set_label(login, r->getString(R::StringID::INITIAL_SETUP_LOGIN).c_str());
	destroySignalID = g_signal_connect(G_OBJECT(window),"destroy", initial_setup_quit, NULL);

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
		const std::string certOk = r->getString(R::StringID::INITIAL_SETUP_SERVER_CERT_OK);
		gtk_button_set_label(serverCert, certOk.c_str());
		gotServerCert = true;
	}
	if(Vars::privateKey.get() != NULL)
	{
		const std::string privateOk = r->getString(R::StringID::INITIAL_SETUP_PRIVATE_KEY_OK);
		gtk_button_set_label(privateKey, privateOk.c_str());
		gotPrivateKey = true;
	}
}

InitialSetup::~InitialSetup()
{
	//this window is purposely going away, unhook it from the main quit function
	g_signal_handler_disconnect(window, destroySignalID);

	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
int InitialSetup::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new InitialSetup();
	return 0;
}

//static
int InitialSetup::remove(void* a)
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
void InitialSetup::onclick_initial_setup_certificate()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::INITIAL_SETUP, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	const std::string message = r->getString(R::StringID::INITIAL_SETUP_CHOOSE_SERVER_CERT);
	const std::string certPath = Utils::fileChooser(message, window);

	if(certPath == "")
	{
		return;
	}

	const std::string certDump = Utils::dumpSmallFile(certPath);
	const bool ok = SodiumUtils::checkSodiumPublic(certDump);
	if(!ok)
	{
		const std::string error = r->getString(R::StringID::INITIAL_SETUP_BAD_SERVER_CERT);
		logger->insertLog(Log(Log::TAG::INITIAL_SETUP, error, Log::TYPE::ERROR).toString());
		Utils::showPopup(error, window);
		return;
	}
	const std::string certStringified = certDump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES*3);
	Vars::serverCert = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
	Stringify::destringify(certStringified, Vars::serverCert.get());
	const std::string certOk = r->getString(R::StringID::INITIAL_SETUP_SERVER_CERT_OK);
	logger->insertLog(Log(Log::TAG::INITIAL_SETUP, certOk, Log::TYPE::INFO).toString());
	gtk_button_set_label(serverCert, certOk.c_str());
	gotServerCert = true;
}

void InitialSetup::onclick_initial_setup_private_key()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::INITIAL_SETUP, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	const std::string message = r->getString(R::StringID::INITIAL_SETUP_CHOOSE_PRIVATE_KEY);
	const std::string privateKeyPath = Utils::fileChooser(message, window);
	if(privateKeyPath == "")
	{
		return;
	}

	const std::string privateKeyDump = Utils::dumpSmallFile(privateKeyPath);
	const bool ok = SodiumUtils::checkSodiumPrivate(privateKeyDump);
	if(!ok)
	{
		const std::string error = r->getString(R::StringID::INITIAL_SETUP_PRIVATE_KEY_BAD);
		logger->insertLog(Log(Log::TAG::INITIAL_SETUP, error, Log::TYPE::ERROR).toString());
		Utils::showPopup(error, window);
		return;
	}

	const std::string privateKeyStringified = privateKeyDump.substr(SodiumUtils::SODIUM_PRIVATE_HEADER().length(), crypto_box_SECRETKEYBYTES*3);
	Vars::privateKey = std::make_unique<unsigned char[]>(crypto_box_SECRETKEYBYTES);
	Stringify::destringify(privateKeyStringified, Vars::privateKey.get());
	const std::string privateOk = r->getString(R::StringID::INITIAL_SETUP_PRIVATE_KEY_OK);
	logger->insertLog(Log(Log::TAG::INITIAL_SETUP, privateOk, Log::TYPE::INFO).toString());
	gtk_button_set_label(privateKey, privateOk.c_str());
	gotPrivateKey = true;
}

void InitialSetup::onclick_initial_setup_login()
{
	if(!onScreen)
	{
		logger->insertLog(Log(Log::TAG::INITIAL_SETUP, r->getString(R::StringID::ERR_NOT_ONSCREEN), Log::TYPE::ERROR).toString());
		return;
	}

	Vars::serverAddress = std::string(gtk_entry_get_text(addr));
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
	}

	Vars::username = std::string(gtk_entry_get_text(username));
	const std::string trial = "address: " + Vars::serverAddress + " command: " + std::to_string(Vars::commandPort) + " media: " + std::to_string(Vars::mediaPort) + " Username: " + Vars::username;
	logger->insertLog(Log(Log::TAG::USER_HOME, trial, Log::TYPE::INFO).toString());

	if(!Vars::serverAddress.empty() && Vars::commandPort != 0 && Vars::mediaPort != 0 && gotServerCert
			&& !Vars::username.empty() && gotPrivateKey)
	{
		LoginAsync::execute(this, false);
	}
	else
	{
		const std::string error = r->getString(R::StringID::INITIAL_SETUP_INCOMPLETE);
		Utils::showPopup(error, window);
	}
}

void InitialSetup::asyncResult(int result)
{
	if(result == Vars::Broadcast::LOGIN_OK)
	{
		Settings* settings = Settings::getInstance();
		settings->setString(Settings::SettingName::SETTING_ADDR, Vars::serverAddress);
		settings->setInt(Settings::SettingName::SETTING_COMMAND_PORT, Vars::commandPort);
		settings->setInt(Settings::SettingName::SETTING_MEDIA_PORT, Vars::mediaPort);
		settings->setString(Settings::SettingName::SETTING_UNAME, Vars::username);
		settings->save();

		Utils::runOnUiThread(&UserHome::render);
	}
	else if(result == Vars::Broadcast::LOGIN_NOTOK)
	{
		const std::string error = r->getString(R::StringID::INITIAL_SETUP_LOGIN_FAIL);
		Utils::showPopup(error, window);
	}
}

extern "C" void onclick_initial_setup_certificate()
{
	if(InitialSetup::instance != NULL)
	{
		InitialSetup::instance->onclick_initial_setup_certificate();
	}
}

extern "C" void onclick_initial_setup_private_key()
{
	if(InitialSetup::instance != NULL)
	{
		InitialSetup::instance->onclick_initial_setup_private_key();
	}
}

extern "C" void onclick_initial_setup_login()
{
	if(InitialSetup::instance != NULL)
	{
		InitialSetup::instance->onclick_initial_setup_login();
	}
}
