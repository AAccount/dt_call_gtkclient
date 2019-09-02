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
#include <thread>
#include <mutex>
#include <atomic>

#include <sodium.h>
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
#include "../settings.hpp"

class CallScreen: public virtual AsyncReceiver
{
public:
	typedef enum {DIALING, RECEIVING} Mode;
	static Mode mode;

	static int render(void* a);
	static int remove(void* a);
	static CallScreen* getInstance();
	
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
	static CallScreen* instance;

	const static int HEADERS = 52;
	int min, sec;
	std::mutex receivedTimestampMutex;
	std::thread timeCounterThread;
	void timeCounter();
	struct timeval lastReceivedTimestamp;
	void updateTime();
	void updateStats();
	std::stringstream timeBuilder;
	std::stringstream statBuilder;
	std::string missingLabel, txLabel, rxLabel, garbageLabel, rxSeqLabel, txSeqLabel, skippedLabel, oorangeLabel;
	int garbage, rxtotal, txtotal, rxSeq, txSeq, skipped, oorange;
	double formatInternetMetric(int metric, std::string& units);
	GtkTextBuffer* statsBuffer;
	std::string currentStats, runningTime;
	static int updateUi(void* context);

	const static int INIT_TIMEOUT = 30;
	pa_simple* ringtonePlayer = NULL;
	std::atomic<bool> ringtoneDone;
	void ring();
	std::thread ringThread;
	bool ringThreadAlive;
	void stopRing();

	void changeToCallMode();
	bool muted, muteStatusNew;
	std::mutex deadUDPMutex;
	bool reconnectionAttempted;
	std::thread encodeThread;
	void mediaEncode();
	bool encodeThreadAlive;
	std::thread decodeThread;
	void mediaDecode();
	bool decodeThreadAlive;
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
	gulong destroyHandleID;
};

#endif /* SRC_SCREENS_CALLSCREEN_HPP_ */
