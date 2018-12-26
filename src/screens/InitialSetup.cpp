/*
 * initial_setup.cpp
 *
 *  Created on: Dec 28, 2017
 *      Author: Daniel
 */

#include "InitialSetup.hpp"

InitialSetup* InitialSetup::instance = NULL;
bool InitialSetup::onScreen = false;

InitialSetup::InitialSetup()
{
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/initial_setup.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_initial_setup"));
	addr = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_addr")));
	commandPort = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_command")));
	mediaPort = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_media")));
	serverCert = GTK_BUTTON(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_certificate")));
	username = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_name")));
	privateKey = GTK_BUTTON(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_private_key")));
	login = GTK_BUTTON(GTK_WIDGET(gtk_builder_get_object(builder, "id_initial_setup_next")));
	destroySignalID = g_signal_connect(G_OBJECT(window),"destroy", Utils::quit, NULL);
	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);
	gtk_widget_show(window);

	strings = StringRes::getInstance();

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
		const std::string certOk = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_SERVER_CERT_OK);
		gtk_button_set_label(serverCert, certOk.c_str());
		gotServerCert = true;
	}
	if(Vars::privateKey.get() != NULL)
	{
		const std::string privateOk = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_PRIVATE_KEY_OK);
		gtk_button_set_label(privateKey, privateOk.c_str());
		gotPrivateKey = true;
	}
}

InitialSetup::~InitialSetup()
{
	g_signal_handler_disconnect(window, destroySignalID);
	gtk_widget_destroy(window);
	g_object_unref(window);
}

//static
void InitialSetup::render()
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new InitialSetup();
}

//static
void InitialSetup::remove()
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
	}
}

//static
void InitialSetup::onclick_initial_setup_certificate()
{
	if(!onScreen)
	{
		std::cerr << strings->getString(Vars::lang, StringRes::StringID::ERR_NOT_ONSCREEN) << "\n";
		return;
	}

	const std::string message = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_CHOOSE_SERVER_CERT);
	const std::string certPath = Utils::file_chooser(message, (GtkWindow*)window);

	if(certPath == "")
	{
		return;
	}

	const std::string certDump = Utils::dumpSmallFile(certPath);
	const bool ok = SodiumUtils::checkSodiumPublic(certDump);
	if(!ok)
	{
		const std::string error = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_BAD_SERVER_CERT);
		std::cerr << error << "\n";
		Utils::show_popup(error, (GtkWindow*)window);
		return;
	}
	const std::string certStringified = certDump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES*3);
	Vars::serverCert = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
	Stringify::destringify(certStringified, Vars::serverCert.get());
	const std::string certOk = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_SERVER_CERT_OK);
	std::cerr << certOk <<"\n";
	gtk_button_set_label(serverCert, certOk.c_str());
	gotServerCert = true;
}

void InitialSetup::onclick_initial_setup_private_key()
{
	if(!onScreen)
	{
		std::cerr << strings->getString(Vars::lang, StringRes::StringID::ERR_NOT_ONSCREEN) << "\n";
		return;
	}

	const std::string message = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_CHOOSE_PRIVATE_KEY);
	const std::string privateKeyPath = Utils::file_chooser(message, (GtkWindow*)window);
	if(privateKeyPath == "")
	{
		return;
	}

	const std::string privateKeyDump = Utils::dumpSmallFile(privateKeyPath);
	const bool ok = SodiumUtils::checkSodiumPrivate(privateKeyDump);
	if(!ok)
	{
		const std::string error = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_PRIVATE_KEY_BAD);
		std::cerr << error << "\n";
		Utils::show_popup(error, (GtkWindow*)window);
		return;
	}

	const std::string privateKeyStringified = privateKeyDump.substr(SodiumUtils::SODIUM_PRIVATE_HEADER().length(), crypto_box_SECRETKEYBYTES*3);
	Vars::privateKey = std::make_unique<unsigned char[]>(crypto_box_SECRETKEYBYTES);
	Stringify::destringify(privateKeyStringified, Vars::privateKey.get());
	const std::string privateOk = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_PRIVATE_KEY_OK);
	std::cerr << privateOk << "\n";
	gtk_button_set_label(privateKey, privateOk.c_str());
	gotPrivateKey = true;
}

void InitialSetup::onclick_initial_setup_login()
{
	if(!onScreen)
	{
		std::cerr << strings->getString(Vars::lang, StringRes::StringID::ERR_NOT_ONSCREEN) << "\n";
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
	std::cerr << "address: " << Vars::serverAddress << " command: " << Vars::commandPort << " media: " << Vars::mediaPort << " Username: " << Vars::username << "\n";

	if(!Vars::serverAddress.empty() && Vars::commandPort != 0 && Vars::mediaPort != 0 && gotServerCert
			&& !Vars::username.empty() && gotPrivateKey)
	{
		LoginAsync::execute(this);
	}
	else
	{
		const std::string error = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_INCOMPLETE);
		Utils::show_popup(error, (GtkWindow*)window);
	}
}

void InitialSetup::asyncResult(int result)
{
	if(result == LoginAsync::LoginResult::LOGIN_OK)
	{
		std::cerr << "Login ok";
		Settings* settings = Settings::getInstance();
		settings->setString(Settings::SettingName::SETTING_ADDR, Vars::serverAddress);
		settings->setInt(Settings::SettingName::SETTING_COMMAND_PORT, Vars::commandPort);
		settings->setInt(Settings::SettingName::SETTING_MEDIA_PORT, Vars::mediaPort);
		settings->setString(Settings::SettingName::SETTING_UNAME, Vars::username);
		settings->save();
		UserHome::render();
	}
	else if(result == LoginAsync::LoginResult::LOGIN_NOTOK)
	{
		const std::string error = strings->getString(Vars::lang, StringRes::StringID::INITIAL_SETUP_LOGIN_FAIL);
		Utils::show_popup(error, (GtkWindow*)window);
	}
}

extern "C" void onclick_intial_server_certificate()
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
