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
#include <thread>
#include <mutex>
#include <atomic>

#include <gtk/gtk.h>
#include <sys/time.h>

#include "../background/AsyncReceiver.hpp"
#include "../background/OperatorCommand.hpp"
#include "../background/CmdListener.hpp"
#include "../background/AsyncCentral.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../vars.hpp"
#include "../utils.hpp"
#include "../settings.hpp"
#include "../voip/SoundEffects.hpp"
#include "../voip/Voice.hpp"

class CallScreen: public virtual AsyncReceiver
{
public:
	typedef enum {DIALING, RECEIVING} Mode;
	static Mode mode;

	static int render(void* a);
	static int remove(void* a);
	static CallScreen* getInstance();
	
	void asyncResult(int result, const std::string& info) override;
	void onclickEnd();
	void onclickMute();
	void onclickAccept();

private:
	CallScreen();
	virtual ~CallScreen();
	static bool onScreen;
	static CallScreen* instance;

	int min, sec;
	std::thread timeCounterThread;
	void timeCounter();
	void updateTime();
	void updateStats();
	std::stringstream timeBuilder;
	std::stringstream statBuilder;
	std::string missingLabel, txLabel, rxLabel, garbageLabel, rxSeqLabel, txSeqLabel, skippedLabel, oorangeLabel;
	
	double formatInternetMetric(int metric, std::string& units);
	GtkTextBuffer* statsBuffer;
	std::string currentStats, runningTime;
	static int updateUi(void* context);

	const static int INIT_TIMEOUT = 20;
	
	void changeToCallMode();

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
