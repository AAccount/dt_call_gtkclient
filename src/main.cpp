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
//				Vars::lang = StringRes::Language::other_
//			}
//		}
//	}

	Settings* settings = Settings::getInstance();
	Vars::serverAddress = settings->getString(Settings::SETTING_ADDR, "");
	Vars::commandPort = settings->getInt(Settings::SETTING_COMMAND_PORT, 0);
	Vars::mediaPort = settings->getInt(Settings::SETTING_MEDIA_PORT, 0);
	Vars::username = settings->getString(Settings::SETTING_UNAME, "");
	const bool gotServerCert = settings->getSodiumFile(Settings::SETTING_CERT);
	const bool gotPrivateKey = settings->getSodiumFile(Settings::SETTING_PRIVATE_KEY);

	if(!Vars::serverAddress.empty() && Vars::commandPort != 0 && Vars::mediaPort != 0 && !Vars::username.empty()
			&& gotServerCert && gotPrivateKey)
	{
		std::cerr << "would've been able to skip to home\n";
	}
	else
	{
		std::cerr << "not able to skip to home\n";
	}
	InitialSetup::render();
	gtk_main();

	return 0;
}
