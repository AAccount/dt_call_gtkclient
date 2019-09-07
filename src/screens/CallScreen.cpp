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
std::unique_ptr<short[]> CallScreen::ringtone;

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
logger(Logger::getInstance()),
ringtoneDone(false),
encodeThreadAlive(false),
decodeThreadAlive(false),
ringThreadAlive(false)
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
	try
	{
		timeCounterThread = std::thread(&CallScreen::timeCounter, this);
	}
	catch(std::system_error& e)
	{
		const std::string error = "timerThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
	}

	//other setup
	muted = muteStatusNew = false;
	reconnectionAttempted = false;
	memset(&lastReceivedTimestamp, 0, sizeof(struct timeval));

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_widget_show((GtkWidget*)window);
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	ring();

	AsyncCentral::getInstance()->registerReceiver(this);
	onScreen = true;
}

CallScreen::~CallScreen()
{
	AsyncCentral::getInstance()->removeReceiver(this)
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

void CallScreen::asyncResult(int result)
{
	if(result == Vars::Broadcast::CALL_START)
	{
		min = sec = 0;
		changeToCallMode();
	}
	else if (result == Vars::Broadcast::CALL_END)
	{
		onclickEnd();
	}
}

void CallScreen::onclickEnd()
{
	stopRing();
	Vars::ustate = Vars::UserState::NONE;

	shutdown(Vars::mediaSocket, 2);
	close(Vars::mediaSocket);
	Vars::mediaSocket = -1;

	unsigned char* voiceKey = Vars::voiceKey.get();
	if(voiceKey != NULL)
	{
		randombytes_buf(voiceKey, crypto_box_SECRETKEYBYTES);
	}

	if(encodeThreadAlive)
	{
		encodeThread.join();
		encodeThreadAlive = false;
	}
	if(decodeThreadAlive)
	{
		decodeThread.join();
		decodeThreadAlive = false;
	}
	
	CommandEnd::execute();
	UserHome::getInstance()->asyncResult(Vars::Broadcast::USERHOME_UNLOCK);
	g_signal_handler_disconnect(G_OBJECT(window), destroyHandleID); //onclick call end is actually ran again after the window is gone unless you tell it not to
	Utils::runOnUiThread(&CallScreen::remove);
}

void CallScreen::onclickMute()
{
	muted = !muted;
	muteStatusNew = true;
	const std::string warning = r->getString(R::StringID::CALL_SCREEN_POPUP_MUTE_WARNING);
	Utils::showPopup(warning, window);
}

void CallScreen::onclickAccept()
{
	CommandAccept::execute();
}

void CallScreen::timeCounter()
{//this function is run on the UI thread: started form the constructor (which was called by the ui thread).
	const int A_SECOND = 1;
	while(Vars::ustate != Vars::UserState::NONE)
	{
		updateTime();
		if((Vars::ustate == Vars::UserState::INIT) && (sec == INIT_TIMEOUT))
		{
			CommandEnd::execute();
			onclickEnd();
		}

		if(Vars::ustate == Vars::UserState::INCALL)
		{
			std::unique_lock<std::mutex> receivedTimestampLock(receivedTimestampMutex);
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
	const int missing = txSeq - rxSeq;
	statBuilder << missingLabel << ": " << (missing > 0 ? missing : 0) << " " << garbageLabel << ": " << garbage << "\n"
			<< rxLabel << ": " << formatInternetMetric(rxtotal, rxUnits) << rxUnits << " " << txLabel << ": " << formatInternetMetric(txtotal, txUnits) << txUnits <<"\n"
			<< rxSeqLabel << ": " << rxSeq << " " << txSeqLabel << ": " << txSeq << "\n"
			<< skippedLabel << ": " << skipped << " " << oorangeLabel << ":  " << oorange;
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

	stopRing();

	Opus::init();
	try
	{
		encodeThread = std::thread(&CallScreen::mediaEncode, this);
		encodeThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "mediaEncodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
		onclickEnd();
	}
	
	try
	{
		decodeThread = std::thread(&CallScreen::mediaDecode, this);
		decodeThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "mediaDecodeThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
		onclickEnd();
	}
}

void CallScreen::mediaEncode()
{
	//assuming pulse audio get microphone always works (unlike android audio record)
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_START), Log::TYPE::INFO).toString());

	//setup pulse audio as the voice recorder
	pa_simple* wavRecorder = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_DESC);
	wavRecorder = pa_simple_new(NULL, self.c_str(), PA_STREAM_RECORD, NULL, description.c_str(), &ss, NULL, NULL, NULL);
	int latencyErr;
	pa_usec_t latency = pa_simple_get_latency(wavRecorder, &latencyErr);
	const std::string info = r->getString(R::StringID::CALL_SCREEN_MEDIA_ENC_LATENCY) + std::to_string(latency);
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, info, Log::TYPE::INFO).toString());

	const int wavFrames = Opus::WAVFRAMESIZE;
	std::unique_ptr<unsigned char[]> packetBufferArray = std::make_unique<unsigned char[]>(Vars::MAX_UDP);
	unsigned char* packetBuffer = packetBufferArray.get();
	std::unique_ptr<short[]> wavBufferArray = std::make_unique<short[]>(wavFrames);
	short* wavBuffer = wavBufferArray.get();
	std::unique_ptr<unsigned char[]> encodedBufferArray = std::make_unique<unsigned char[]>(wavFrames);
	unsigned char* encodedBuffer = encodedBufferArray.get();

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

		const int sent = sendto(Vars::mediaSocket, packetEncrypted.get(), packetEncryptedLength, 0, (struct sockaddr*)&Vars::mediaPortAddrIn, sizeof(struct sockaddr_in));
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
}



void CallScreen::mediaDecode()
{
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_START), Log::TYPE::INFO).toString());

	//setup pulse audio for voice playback
	pa_simple* wavPlayer = NULL;
	pa_sample_spec ss;
	memset(&ss, 0, sizeof(pa_sample_spec));
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = Opus::SAMPLERATE;
	const std::string self = r->getString(R::StringID::SELF);
	const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_DESC);
	wavPlayer = pa_simple_new(NULL, self.c_str(), PA_STREAM_PLAYBACK, NULL, description.c_str(), &ss, NULL, NULL, NULL);
	int latencyErr;
	pa_usec_t latency = pa_simple_get_latency(wavPlayer, &latencyErr);
	const std::string info = r->getString(R::StringID::CALL_SCREEN_MEDIA_DEC_LATENCY) + std::to_string(latency);
	logger->insertLog(Log(Log::TAG::CALL_SCREEN, info, Log::TYPE::INFO).toString());

	const int wavFrames = Opus::WAVFRAMESIZE;
	std::unique_ptr<unsigned char[]> encBufferArray = std::make_unique<unsigned char[]>(wavFrames);
	unsigned char* encBuffer = encBufferArray.get();
	std::unique_ptr<short[]> wavBufferArray = std::make_unique<short[]>(wavFrames);
	short* wavBuffer = wavBufferArray.get();
	std::unique_ptr<unsigned char[]> packetBufferArray = std::make_unique<unsigned char[]>(Vars::MAX_UDP);
	unsigned char* packetBuffer = packetBufferArray.get();

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
		else
		{
			{
				std::unique_lock<std::mutex>(receivedTimestampMutex);
				gettimeofday(&lastReceivedTimestamp, NULL);
			}
			rxtotal = rxtotal + receivedLength + HEADERS;
		}
		std::unique_ptr<unsigned char[]> packetDecrypted;
		int packetDecryptedLength = 0;
		SodiumUtils::sodiumDecrypt(false, packetBuffer, receivedLength, Vars::voiceKey.get(), NULL, packetDecrypted, packetDecryptedLength);
		if(packetDecryptedLength < sizeof(uint32_t)) //should have received at least the sequence number
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
}

void CallScreen::reconnectUDP()
{
	std::unique_lock<std::mutex> deadUDPLock(deadUDPMutex);
	if(Vars::ustate == Vars::UserState::NONE)
	{
		return;
	}

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
			onclickEnd();
		}
	}
}

void CallScreen::ring()
{
	try
	{
		ringThread = std::thread([this] {
			pa_sample_spec ss;
			memset(&ss, 0, sizeof (pa_sample_spec));
			ss.format = PA_SAMPLE_S16LE;
			ss.channels = 1;
			ss.rate = RINGTONE_SAMPLERATE;
			const std::string self = r->getString(R::StringID::SELF);
			const std::string description = r->getString(R::StringID::CALL_SCREEN_MEDIA_RINGTONE_DESC);
			ringtonePlayer = pa_simple_new(NULL, self.c_str(), PA_STREAM_PLAYBACK, NULL, description.c_str(), &ss, NULL, NULL, NULL);

			//write the ringtone 1/10th of a second at a time for 1.5s then 1s of silence.
			//using this weird 1/10th of a second at a time scheme because pa_simple functions all block
			const double TOTAL_SAMPLES = RINGTONE_SAMPLERATE / RINGTONE_DIVISION;
			std::unique_ptr<short[] > silenceArray = std::make_unique<short[]>((int) TOTAL_SAMPLES);
			const int DIVISIONS_RINGTONE = TONE_TIME*RINGTONE_DIVISION;
			const int DIVISIONS_SILENCE = SILENCE_TIME*RINGTONE_DIVISION;
			bool playRingtone = true;
			int divisionsPlayed = 0;
			struct timespec divisionTime;
			memset(&divisionTime, 0, sizeof (struct timespec));
			divisionTime.tv_sec = 0;
			divisionTime.tv_nsec = (int) (1000000000.0 / RINGTONE_DIVISION);
			while(!ringtoneDone)
			{
				short* item = silenceArray.get();;
				if (playRingtone)
				{
					item = ringtone.get();
				}

				int paWriteError = 0;
				pa_simple_write(ringtonePlayer, item, TOTAL_SAMPLES * sizeof(short), &paWriteError);
				nanosleep(&divisionTime, NULL);
				divisionsPlayed++;

				if (playRingtone && (divisionsPlayed == DIVISIONS_RINGTONE))
				{
					divisionsPlayed = 0;
					playRingtone = false;
				}
				else if (!playRingtone && (divisionsPlayed == DIVISIONS_SILENCE))
				{
					divisionsPlayed = 0;
					playRingtone = true;
				}
			}
			pa_simple_free(ringtonePlayer);
		});
		ringThreadAlive = true;
	}
	catch(std::system_error& e)
	{
		const std::string error = "ringThread " + r->getString(R::StringID::ERR_THREAD_CREATE) + std::string(e.what()) + ")";
		logger->insertLog(Log(Log::TAG::CALL_SCREEN, error, Log::TYPE::ERROR).toString());
	}
}

void CallScreen::stopRing()
{
	ringtoneDone = true;
	if(ringThreadAlive) //only call join once
	{
		ringThread.join();
		ringThreadAlive = false;
	}
}
