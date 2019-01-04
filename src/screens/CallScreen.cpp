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

extern "C" void call_screen_quit()
{
	unsigned char* voiceKey = Vars::voiceKey.get();
	if(voiceKey != NULL)
	{
		randombytes_buf(voiceKey, crypto_box_SECRETKEYBYTES);
	}
}

CallScreen::CallScreen() :
r(R::getInstance()),
logger(Logger::getInstance(""))
{
	//typical ui setup
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/call_screen.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "call_screen_window"));
	gtk_window_set_title(window, r->getString(R::StringID::CALL_SCREEN_TITLE).c_str());
	status = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_status"));
	gtk_label_set_text(status, r->getString(R::StringID::CALL_SCREEN_STATUS_RINGING).c_str());
	time = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_time"));
	gtk_label_set_text(time, "0:00");
	callerID = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_callerid"));
	gtk_label_set_text(callerID, Vars::callWith.c_str()); //TODO: use the nick name
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
	g_signal_connect(G_OBJECT(window),"destroy", call_screen_quit, NULL);
	stats = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "call_screen_stats"));

	//setup the timer and stats display
	min = sec = 0;
	pthread_mutex_init(&rxTSLock, NULL);
	timeBuilder = std::stringstream();
	statBuilder = std::stringstream();
	garbage = rxtotal = txtotal = rxSeq = txSeq = skipped = oorange = 0;
	missingLabel = r->getString(R::StringID::CALL_SCREEN_STAT_MISSING);
	txLabel = r->getString(R::StringID::CALL_SCREEN_STAT_TX);
	rxLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RX);
	garbageLabel = r->getString(R::StringID::CALL_SCREEN_STAT_GARBAGE);
	rxSeqLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RXSEQ);
	txSeqLabel = r->getString(R::StringID::CALL_SCREEN_STAT_TXSEQ);
	skippedLabel = r->getString(R::StringID::CALL_SCREEN_STAT_SKIP);
	oorangeLabel = r->getString(R::StringID::CALL_SCREEN_STAT_RANGE);
	statsBuffer = gtk_text_buffer_new(NULL);
	pthread_t timerThread;
	if(pthread_create(&timerThread, NULL, &CallScreen::timeCounterHelp, this) != 0)
	{
		const std::string error = "timerThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
	}

	//other setup
	muted = muteStatusNew = false;
	pthread_mutex_init(&deadUDPLock, NULL);
	reconnectionAttempted = false;

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_widget_show((GtkWidget*)window);
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	onScreen = true;
}

CallScreen::~CallScreen()
{
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
	g_object_unref(statsBuffer);
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

void CallScreen::asyncResult(int result)
{
	if(result == Vars::Broadcast::CALL_START)
	{
		min = sec = 0;
		changeToCallMode();
	}
	else if (result == Vars::Broadcast::CALL_END)
	{
		call_screen_quit();
		Utils::runOnUiThread(&CallScreen::remove);
	}
}

void CallScreen::onclickEnd()
{
	Vars::ustate = Vars::UserState::NONE;
	CommandEnd::execute();
	asyncResult(Vars::Broadcast::CALL_END);
	UserHome::instance->asyncResult(Vars::Broadcast::UNLOCK_USERHOME);
}

void CallScreen::onclickMute()
{
	muted = !muted;
	muteStatusNew = true;
	const std::string warning = r->getString(R::StringID::CALL_SCREEN_POPUP_MUTE_WARNING);
	Utils::show_popup(warning, window);
}

void CallScreen::onclickAccept()
{
	CommandAccept::execute();
}

//static
void* CallScreen::timeCounterHelp(void* context)
{
	return((CallScreen*)context)->timeCounter();
}

void* CallScreen::timeCounter(void)
{//this function is run on the UI thread: started form the constructor (which was called by the ui thread).
	const int A_SECOND = 1;
	const int INIT_TIMEOUT = 30;
	while(Vars::ustate != Vars::UserState::NONE)
	{
		updateTime();
		if((Vars::ustate == Vars::UserState::INIT) && (sec == INIT_TIMEOUT))
		{
			CommandEnd::execute();
			onclickEnd();
		}

		pthread_mutex_lock(&rxTSLock);
		{
			const int ASECOND_AS_US = 1000000;
			struct timeval now;
			memset(&now, 0, sizeof(struct timeval));
			gettimeofday(&now, NULL);
			const int btw = now.tv_usec - lastReceivedTimestamp.tv_usec;
			if(btw > ASECOND_AS_US && Vars::mediaSocket != -1)
			{
				logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_LAST_UDP_FOREVER), Log::TYPE::ERROR).toString());
				shutdown(Vars::mediaSocket, 2);
				close(Vars::mediaSocket);
				Vars::mediaSocket = -1;
			}
		}
		pthread_mutex_unlock(&rxTSLock);

		updateStats();
		sleep(A_SECOND);
	}
	return 0;
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
	const std::string runningTime = timeBuilder.str();
	gtk_label_set_text(time, runningTime.c_str());

}

void CallScreen::updateStats()
{
	statBuilder.str(std::string());
	statBuilder.precision(3); //match the android version
	std::string rxUnits, txUnits;
	const int missing = txSeq - rxSeq;
	statBuilder << missingLabel << ": " << (missing > 0 ? missing : 0) << " " << garbageLabel << ": " << garbage << "\n"
			<< rxLabel << ": " << formatInternetMetric(rxtotal, rxUnits) << rxUnits << " " << txLabel << ": " << formatInternetMetric(txtotal, txUnits) << txUnits <<"\n"
			<< rxSeqLabel << ": " << rxSeq << " " << txSeqLabel << ": " << txSeq << "\n"
			<< skippedLabel << ": " << skipped << " " << oorangeLabel << ":  " << oorange;
	const std::string currentStats = statBuilder.str();
	gtk_text_buffer_set_text(statsBuffer, currentStats.c_str(), -1);
	gtk_text_view_set_buffer(stats, statsBuffer);
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

	Opus::init();
	pthread_t mediaEncodeThread;
	if(pthread_create(&mediaEncodeThread, NULL, &CallScreen::mediaEncodeHelp, this) != 0)
	{
		const std::string error = "mediaEncodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
		onclickEnd();
	}
	pthread_t mediaDecodeThread;
	if(pthread_create(&mediaDecodeThread, NULL, &CallScreen::mediaDecodeHelp, this) != 0)
	{
		const std::string error = "mediaDecodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::to_string(errno) + ") " + std::string(strerror(errno));
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
		onclickEnd();
	}
}

void* CallScreen::mediaEncode(void)
{
	//assuming pulse audio get microphone always works (unlike android audio record)
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_START), Log::TYPE::INFO).toString());

	//setup pulse audio as the voice recorder
	pa_simple* wavRecorder = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16NE; //TODO: is this really ok or should an endian preference be enforced?
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_DESC);
	wavRecorder = pa_simple_new(NULL, self.c_str(), PA_STREAM_RECORD, NULL, description.c_str(), &ss, NULL, NULL, NULL);

	const int wavFrames = Opus::WAVFRAMESIZE;
	unsigned char packetBuffer[Vars::MAX_UDP] = {};
	short wavBuffer[wavFrames] = {};
	unsigned char encodedBuffer[wavFrames] = {};

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		if(muteStatusNew)
		{
			std::string text = r->getString(R::StringID::CALL_SCREEN_BUTTON_MUTE);
			if(muted)
			{
				text = r->getString(R::StringID::CALL_SCREEN_BUTTON_MUTE_UNMUTE);
			}
			gtk_button_set_label(buttonMute, text.c_str());
			muteStatusNew = false;
		}

		memset(wavBuffer, 0, wavFrames*sizeof(short));
		int paReadError = 0;
		const int paread = pa_simple_read(wavRecorder, wavBuffer, wavFrames*sizeof(short), &paReadError);
		if(paread != 0 )
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_PA_ERR) + std::to_string(paReadError);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			continue;
		}

		//even if muted, still need to record audio in real time
		if(muted)
		{
			memset(wavBuffer, 0, wavFrames*sizeof(short));
		}

		memset(encodedBuffer, 0, wavFrames);
		const int encodeLength = Opus::encode(wavBuffer, wavFrames, encodedBuffer, wavFrames);
		if(encodeLength < 1)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_OPUS_ERR) + std::to_string(encodeLength);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			continue;
		}

		memset(packetBuffer, 0, Vars::MAX_UDP);
		SodiumUtils::disassembleInt(txSeq, packetBuffer);
		txSeq++;
		memcpy(packetBuffer+sizeof(uint32_t), encodedBuffer, encodeLength);

		std::unique_ptr<unsigned char[]> packetEncrypted;
		int packetEncryptedLength = 0;
		SodiumUtils::sodiumEncrypt(false, packetBuffer, sizeof(uint32_t)+encodeLength, Vars::voiceKey.get(), NULL, packetEncrypted, packetEncryptedLength);
		if(packetEncryptedLength < 1)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_SODIUM_ERR);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			continue;
		}

		const int sent = sendto(Vars::mediaSocket, packetEncrypted.get(), packetEncryptedLength, 0, (struct sockaddr*)&Vars::serv_addr, sizeof(struct sockaddr_in));
		if(sent < 0)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_NETWORK_ERR);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			reconnectUDP();
			continue;
		}
		else
		{
			txtotal = txtotal + packetEncryptedLength + HEADERS;
		}
	}
	randombytes_buf(packetBuffer, Vars::MAX_UDP);
	randombytes_buf(wavBuffer, wavFrames*sizeof(short));
	randombytes_buf(encodedBuffer, wavFrames);
	Opus::closeEncoder();
	pa_simple_free(wavRecorder);
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_STOP), Log::TYPE::INFO).toString());
	return 0;
}

void* CallScreen::mediaEncodeHelp(void* context)
{
	return((CallScreen*)context)->mediaEncode();
}

void* CallScreen::mediaDecode(void)
{
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_START), Log::TYPE::INFO).toString());

	//setup pulse audio for voice playback
	pa_simple* wavPlayer = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16NE; //TODO: is this really ok or should an endian preference be enforced?
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_DESC);
	wavPlayer = pa_simple_new(NULL, self.c_str(), PA_STREAM_PLAYBACK, NULL, description.c_str(), &ss, NULL, NULL, NULL);

	const int wavFrames = Opus::WAVFRAMESIZE;
	unsigned char encBuffer[wavFrames] = {};
	short wavBuffer[wavFrames] = {};
	unsigned char packetBuffer[Vars::MAX_UDP] = {};

	while(Vars::ustate == Vars::UserState::INCALL)
	{
		memset(packetBuffer, 0, Vars::MAX_UDP);

		struct sockaddr_in sender;
		socklen_t senderLength = sizeof(struct sockaddr_in);
		const int receivedLength = recvfrom(Vars::mediaSocket, packetBuffer, Vars::MAX_UDP, 0, (struct sockaddr*)&sender, &senderLength);
		if(receivedLength < 0)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_NETWORK_ERR);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			reconnectUDP();
			continue;
		}
		rxtotal = rxtotal + receivedLength + HEADERS;

		std::unique_ptr<unsigned char[]> packetDecrypted;
		int packetDecryptedLength = 0;
		SodiumUtils::sodiumDecrypt(false, packetBuffer, receivedLength, Vars::voiceKey.get(), NULL, packetDecrypted, packetDecryptedLength);
		if(packetDecryptedLength < 1)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_SODIUM_ERR);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			continue;
		}

		const int sequence = SodiumUtils::reassembleInt(packetDecrypted.get());
		if(sequence <= rxSeq)
		{
			skipped++;
			continue;
		}

		if(std::abs(sequence - rxSeq) > OORANGE_LIMIT)
		{
			oorange++;
		}
		else
		{
			rxSeq = sequence;
		}

		const int opusLength = packetDecryptedLength-sizeof(uint32_t);
		memset(encBuffer, 0, wavFrames);
		memcpy(encBuffer, packetDecrypted.get()+sizeof(uint32_t), opusLength);

		memset(wavBuffer, 0, wavFrames*sizeof(short));
		const int frames = Opus::decode(encBuffer, opusLength, wavBuffer, wavFrames);
		if(frames < 1)
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_OPUS_ERR) + std::to_string(frames);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
			randombytes_buf(packetDecrypted.get(), packetDecryptedLength);
			continue;
		}
		int paWriteError = 0;
		const int paWrite = pa_simple_write(wavPlayer, wavBuffer, frames*sizeof(short), &paWriteError);
		if(paWrite != 0 )
		{
			const std::string error = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_PA_ERR) + std::to_string(paWriteError);
			logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
		}
		randombytes_buf(packetDecrypted.get(), packetDecryptedLength);
	}

	randombytes_buf(packetBuffer, Vars::MAX_UDP);
	randombytes_buf(encBuffer, wavFrames);
	randombytes_buf(wavBuffer, wavFrames*sizeof(short));
	pa_simple_free(wavPlayer);
	Opus::closeDecoder();
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_STOP), Log::TYPE::INFO).toString());
	return 0;
}

void* CallScreen::mediaDecodeHelp(void* context)
{
	return((CallScreen*)context)->mediaDecode();
}

void CallScreen::reconnectUDP()
{
pthread_mutex_lock(&deadUDPLock);

	if(reconnectionAttempted)
	{
		reconnectionAttempted = false; //already attempted, reset it for the next connection failure
	}
	else
	{
		const bool reconnected = CmdListener::registerUDP();
		reconnectionAttempted = true;
		if(!reconnected)
		{
			pthread_mutex_unlock(&deadUDPLock);
			onclickEnd();
		}
	}

pthread_mutex_unlock(&deadUDPLock);
}

extern "C" void onclick_call_screen_end()
{
	if(CallScreen::instance != NULL)
	{
		CallScreen::instance->onclickEnd();
	}
}

extern "C" void onclick_call_screen_mute()
{
	if(CallScreen::instance != NULL)
	{
		CallScreen::instance->onclickMute();
	}
}

extern "C" void onclick_call_screen_accept()
{
	if(CallScreen::instance != NULL)
	{
		CallScreen::instance->onclickAccept();
	}
}