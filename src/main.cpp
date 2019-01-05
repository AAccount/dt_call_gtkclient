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
	Logger* logger = Logger::getInstance("/tmp/"); //logs don't need to be kept forever like the server
	LoginAsync::init();

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

	//generate 1/10th of a second of the ~440hz tone used as the ringtone
	const double TONE_FREQ = 440;
	const double TOTAL_SAMPLES = CallScreen::RINGTONE_SAMPLERATE/CallScreen::RINGTONE_DIVISION;
	CallScreen::ringtone = std::make_unique<short[]>(TOTAL_SAMPLES);

	const double AMP = SHRT_MAX-5;
	const double FACTOR = (2.0*M_PI*TONE_FREQ) / CallScreen::RINGTONE_SAMPLERATE;
	short* ringtoneArray = CallScreen::ringtone.get();
	for(double i=0.0; i<TOTAL_SAMPLES; i=i+1.0)
	{
		ringtoneArray[(int)i] = AMP*sin(FACTOR*i);
	}

	R::activeLang = R::Language::EN;

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
		logger->insertLog(Log(Log::TAG::MAIN, R::getInstance()->getString(R::StringID::MAIN_SKIP_TOHOME), Log::TYPE::INFO).toString());
		UserHome::render(NULL);
	}
	else
	{
		InitialSetup::render(NULL); //this IS the ui thread run the render function directly
	}
	gtk_main();

	return 0;
}
