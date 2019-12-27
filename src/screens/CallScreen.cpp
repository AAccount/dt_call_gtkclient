/*
 * CallScreen.cpp
 *
 *  Created on: Dec 29, 2018
 *      Author: Daniel
 */

//https://freedesktop.org/software/pulseaudio/doxygen/simple.html

#include "CallScreen.hpp"

bool CallScreen::onScreen = false;
CallScreen::Mode CallScreen::mode;
CallScreen* CallScreen::instance = NULL;

extern "C" void onclick_call_screen_end()
{
	CallScreen* instance = CallScreen::getInstance();
	if(instance != NULL)
	{
		instance->onclickEnd();
	}
}

extern "C" void onclick_call_screen_mute()
{
	CallScreen* instance = CallScreen::getInstance();
	if(instance != NULL)
	{
		instance->onclickMute();
	}
}

extern "C" void onclick_call_screen_accept()
{
	CallScreen* instance = CallScreen::getInstance();
	if(instance != NULL)
	{
		instance->onclickAccept();
	}
}

CallScreen::CallScreen() :
r(R::getInstance()),
logger(Logger::getInstance())
{
	Settings* settings = Settings::getInstance();

	GtkBuilder* builder = gtk_builder_new_from_resource("/gtkclient/call_screen.glade");
	window = GTK_WINDOW(gtk_builder_get_object(builder, "call_screen_window"));
	gtk_window_set_title(window, r->getString(R::StringID::CALL_SCREEN_TITLE).c_str());
	status = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_status"));
	gtk_label_set_text(status, r->getString(R::StringID::CALL_SCREEN_STATUS_RINGING).c_str());
	time = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_time"));
	gtk_label_set_text(time, "0:00");
	callerID = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_callerid"));
	const std::string nickname = settings->getNickname(Vars::callWith);
	gtk_label_set_text(callerID, nickname.c_str());
	buttonEnd = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_end"));
	gtk_button_set_label(buttonEnd, r->getString(R::StringID::CALL_SCREEN_BUTTON_END).c_str());
	buttonMute = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_mute"));
	gtk_button_set_label(buttonMute, r->getString(R::StringID::CALL_SCREEN_BUTTON_MUTE).c_str());
	gtk_widget_set_sensitive((GtkWidget*)buttonMute, false);
	buttonAccept = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_accept"));
	gtk_button_set_label(buttonAccept, r->getString(R::StringID::CALL_SCREEN_BUTTON_ACCEPT).c_str());
	if(mode == Mode::DIALING) //you've obviously accepted the call considering you're dialing it
	{
		gtk_widget_set_sensitive((GtkWidget*)buttonAccept, false);
	}
	destroyHandleID = g_signal_connect(G_OBJECT(window),"destroy", onclick_call_screen_end, NULL);
	stats = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "call_screen_stats"));

	//setup the timer and stats display
	min = sec = 0;
	timeBuilder = std::stringstream();
	statBuilder = std::stringstream();
	missingLabel = r->getString(R::StringID::CALL_SCREEN_STAT_MISSING);
	txLabel = r->getString(R::StringID::CALL_SCREEN_STAT_TX);
	rxLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RX);
	garbageLabel = r->getString(R::StringID::CALL_SCREEN_STAT_GARBAGE);
	rxSeqLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RXSEQ);
	txSeqLabel = r->getString(R::StringID::CALL_SCREEN_STAT_TXSEQ);
	skippedLabel = r->getString(R::StringID::CALL_SCREEN_STAT_SKIP);
	oorangeLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RANGE);
	statsBuffer = gtk_text_buffer_new(NULL);
	try
	{
		timeCounterThread = std::thread(&CallScreen::timeCounter, this);
	}
	catch(std::system_error& e)
	{
		const std::string error = "timerThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
	}

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_widget_show((GtkWidget*)window);
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	SoundEffects::getInstance()->ring();

	AsyncCentral::getInstance()->registerReceiver(this);
	onScreen = true;
}

CallScreen::~CallScreen()
{
	Vars::ustate = Vars::UserState::NONE;
	SoundEffects::getInstance()->stopRing();
	Voice::getInstance()->stop();
	
	AsyncCentral::getInstance()->broadcast(Vars::Broadcast::USERHOME_UNLOCK);
	AsyncCentral::getInstance()->removeReceiver(this);
	timeCounterThread.join();
	
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
	g_object_unref(statsBuffer);
}

//static
CallScreen* CallScreen::getInstance()
{
	return instance;
}

//static
int CallScreen::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new CallScreen();
	return 0;
}

//static
int CallScreen::remove(void* a)
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
	return 0;
}

void CallScreen::asyncResult(int result, const std::string& info)
{
	if(result == Vars::Broadcast::CALL_START)
	{
		min = sec = 0;
		changeToCallMode();
	}
	else if(result == Vars::Broadcast::CALL_END)
	{
		Utils::runOnUiThread(&CallScreen::remove);
	}
	else if(result == Vars::Broadcast::MIC_MUTE)
	{
		const std::string text = r->getString(R::StringID::CALL_SCREEN_BUTTON_MUTE);
		gtk_button_set_label(buttonMute, text.c_str());
	}
	else if(result == Vars::Broadcast::MIC_UNMUTE)
	{
		const std:: string text = r->getString(R::StringID::CALL_SCREEN_BUTTON_MUTE_UNMUTE);
		gtk_button_set_label(buttonMute, text.c_str());
	}
}

void CallScreen::onclickEnd()
{	
	OperatorCommand::execute(OperatorCommand::OperatorCommand::END);
	g_signal_handler_disconnect(G_OBJECT(window), destroyHandleID); //onclick call end is actually ran again after the window is gone unless you tell it not to
	Utils::runOnUiThread(&CallScreen::remove);
}

void CallScreen::onclickMute()
{
	Voice::getInstance()->toggleMic();
	const std::string warning = r->getString(R::StringID::CALL_SCREEN_POPUP_MUTE_WARNING);
	Utils::showPopup(warning, window);
}

void CallScreen::onclickAccept()
{
	OperatorCommand::execute(OperatorCommand::OperatorCommand::ACCEPT);
}

void CallScreen::timeCounter()
{//this function is run on the UI thread: started form the constructor (which was called by the ui thread).
	const int A_SECOND = 1;
	while(Vars::ustate != Vars::UserState::NONE)
	{
		updateTime();
		if((Vars::ustate == Vars::UserState::INIT) && (sec == INIT_TIMEOUT))
		{
			OperatorCommand::execute(OperatorCommand::OperatorCommand::END);
			onclickEnd();
		}

		updateStats();
		if(onScreen)
		{
			Utils::runOnUiThread(&CallScreen::updateUi, this);
		}
		sleep(A_SECOND);
	}
}

void CallScreen::updateTime()
{
	if(sec == 59)
	{
		sec = 0;
		min++;
	}
	else
	{
		sec++;
	}

	timeBuilder.str(std::string());
	if(sec < 10)
	{
		timeBuilder << min << ":0" << sec;
	}
	else
	{
		timeBuilder  << min << ":" << sec;
	}
	runningTime = timeBuilder.str();
}

void CallScreen::updateStats()
{
	statBuilder.str(std::string());
	statBuilder.precision(3); //match the android version
	std::string rxUnits, txUnits;
	
	Voice* voice = Voice::getInstance();
	const int missing = voice->getTxSeq() - voice->getRxSeq();
	statBuilder << missingLabel << ": " << (missing > 0 ? missing : 0) << " " << garbageLabel << ": " << voice->getGarbage() << "\n"
			<< rxLabel << ": " << formatInternetMetric(voice->getRxtotal(), rxUnits) << rxUnits << " " << txLabel << ": " << formatInternetMetric(voice->getTxtotal(), txUnits) << txUnits <<"\n"
			<< rxSeqLabel << ": " << voice->getRxSeq() << " " << txSeqLabel << ": " << voice->getTxSeq() << "\n"
			<< skippedLabel << ": " << voice->getSkipped() << " " << oorangeLabel << ":  " << voice->getOorange();
	currentStats = statBuilder.str();
}

int CallScreen::updateUi(void* context)
{
	CallScreen* screen = static_cast<CallScreen*>(context);
	gtk_label_set_text(screen->time, screen->runningTime.c_str());
	gtk_text_buffer_set_text(screen->statsBuffer, screen->currentStats.c_str(), -1);
	gtk_text_view_set_buffer(screen->stats, screen->statsBuffer);
	return 0;
}

double CallScreen::formatInternetMetric(int metric, std::string& units)
{
	const double MEGA = 1000000.0;
	const double KILO = 1000.0;
	double dmetric = (double)metric;
	if(metric > MEGA)
	{
		units = r->getString(R::StringID::CALL_SCREEN_STAT_MB);
		return dmetric / MEGA;
	}
	else if (metric > KILO)
	{
		units = r->getString(R::StringID::CALL_SCREEN_STAT_KB);
		return dmetric / KILO;
	}
	else
	{
		units = r->getString(R::StringID::CALL_SCREEN_STAT_B);
		return dmetric;
	}
}

void CallScreen::changeToCallMode()
{
	gtk_label_set_text(status, r->getString(R::StringID::CALL_SCREEN_STATUS_INCALL).c_str());
	gtk_widget_set_sensitive((GtkWidget*)buttonAccept, false);
	gtk_widget_set_sensitive((GtkWidget*)buttonMute, true);
	min = sec = 0;

	SoundEffects::getInstance()->stopRing();
	Voice::getInstance()->start();
}