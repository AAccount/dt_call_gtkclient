/*
 * CallScreen.hpp
 *
 *  Created on: Dec 29, 2018
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_CALLSCREEN_HPP_
#define SRC_SCREENS_CALLSCREEN_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <memory>

#include <sodium.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <pulse/simple.h>
#include <sys/time.h>

#include "../codec/Opus.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/CommandEnd.hpp"
#include "../background/CommandAccept.hpp"
#include "../background/CmdListener.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../vars.hpp"
#include "../utils.hpp"

class CallScreen: public virtual AsyncReceiver
{
public:
	typedef enum {DIALING, RECEIVING} Mode;
	static Mode mode;
	static int render(void* a);
	static int remove(void* a);
	static CallScreen* instance;

	void asyncResult(int result) override;
	void onclickEnd();
	void onclickMute();
	void onclickAccept();

	constexpr static double RINGTONE_SAMPLERATE = 8000.0;
	constexpr static double TONE_TIME = 1.5;
	constexpr static double SILENCE_TIME = 1.0;
	constexpr static double RINGTONE_DIVISION = 10.0;
	static std::unique_ptr<short[]> ringtone;

private:
	CallScreen();
	virtual ~CallScreen();
	static bool onScreen;

	const static int HEADERS = 52;
	int min, sec;
	pthread_mutex_t rxTSLock;
	void* timeCounter(void);
	static void* timeCounterHelp(void* context);
	struct timeval lastReceivedTimestamp;
	void updateTime();
	void updateStats();
	std::stringstream timeBuilder;
	std::stringstream statBuilder;
	std::string missingLabel, txLabel, rxLabel, garbageLabel, rxSeqLabel, txSeqLabel, skippedLabel, oorangeLabel;
	int garbage, rxtotal, txtotal, rxSeq, txSeq, skipped, oorange;
	double formatInternetMetric(int metric, std::string& units);
	GtkTextBuffer* statsBuffer;

	const static int INIT_TIMEOUT = 30;
	pa_simple* ringtonePlayer = NULL;
	pthread_mutex_t ringtoneLock;
	bool ringtoneDone;
	void ring();
	static void* ringThread(void* context);
	void stopRing();

	void changeToCallMode();
	bool muted, muteStatusNew;
	pthread_mutex_t deadUDPLock;
	bool reconnectionAttempted;
	void* mediaEncode(void);
	static void* mediaEncodeHelp(void* context);
	void* mediaDecode(void);
	static void* mediaDecodeHelp(void* context);
	void reconnectUDP();
	const static int OORANGE_LIMIT = 100;


	R* r;
	Logger* logger;

	GtkWindow* window;
	GtkLabel* status;
	GtkLabel* time;
	GtkTextView* stats;
	GtkLabel* callerID;
	GtkButton* buttonEnd;
	GtkButton* buttonMute;
	GtkButton* buttonAccept;
};

#endif /* SRC_SCREENS_CALLSCREEN_HPP_ */
