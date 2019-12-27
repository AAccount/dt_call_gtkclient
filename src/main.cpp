/*
 * main.cpp
 *
 *  Created on: Dec 11, 2017
 *      Author: Daniel
 */
//https://prognotes.net/2015/07/gtk-3-glade-c-programming-template/
//https://prognotes.net/2015/06/gtk-3-c-program-using-glade-3/
//https://prognotes.net/2016/03/gtk-3-c-code-hello-world-tutorial-using-glade-3/
#include "main.hpp"

int main(int argc, char *argv[])
{
	Logger::setLogLocation("/tmp");
	Logger* logger = Logger::getInstance(); //logs don't need to be kept forever like the server

	XInitThreads(); //the magic required to open another window
	gtk_init(&argc, &argv);

//	if(argc == 3)
//	{
//		const std::string languageFlag(argv[1]);
//		const std::string language(argv[2]);
//
//		if(languageFlag == "-l" || languageFlag == "-language")
//		{
//			if(language == "other")
//			{
//				R::activeLang = R::Language::other_
//			}
//		}
//	}

	SoundEffects::getInstance(); //create the ringtone on startup
	R::activeLang = R::Language::EN;

	//try to load all the settings required to login
	Settings* settings = Settings::getInstance();
	Vars::serverAddress = settings->getString(Settings::SETTING_ADDR, "");
	Vars::commandPort = settings->getInt(Settings::SETTING_COMMAND_PORT, 0);
	Vars::mediaPort = settings->getInt(Settings::SETTING_MEDIA_PORT, 0);
	Vars::username = settings->getString(Settings::SETTING_UNAME, "");
	const bool gotServerCert = settings->getSodiumFile(Settings::SETTING_CERT);
	const bool gotPrivateKey = settings->getSodiumFile(Settings::SETTING_PRIVATE_KEY);

	g_resources_register(gresources_get_resource());

	if(!Vars::serverAddress.empty() && Vars::commandPort != 0 && Vars::mediaPort != 0 && !Vars::username.empty() && gotServerCert && gotPrivateKey)
	{
		//if all the login information is there, skip straight to the home screen
		logger->insertLog(Log(Log::TAG::MAIN, R::getInstance()->getString(R::StringID::MAIN_SKIP_TOHOME), Log::TYPE::INFO).toString());
		UserHome::render(NULL);
	}
	else
	{
		//if login information is missing, run the settings screen in initial setup mode
		SettingsUI::initialSetup = true;
		SettingsUI::render(NULL); //this IS the ui thread run the render function directly
	}
	gtk_main();

	return 0;
}
