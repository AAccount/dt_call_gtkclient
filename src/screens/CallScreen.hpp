/*
 * CallScreen.hpp
 *
 *  Created on: Dec 29, 2018
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_CALLSCREEN_HPP_
#define SRC_SCREENS_CALLSCREEN_HPP_

#include <string>

#include <pthread.h>
#include <gtk/gtk.h>
#include <pulse/simple.h>

#include "../codec/Opus.hpp"
#include "../background/AsyncReceiver.hpp"
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
	static void render();
	static void remove();
	static CallScreen* instance;

	void asyncResult(int result);
	void onclickDial();

private:
	CallScreen();
	virtual ~CallScreen();
	static bool onScreen;

	R* r;
	Logger* logger;

	GtkWindow* window;
	GtkLabel* status;
	GtkLabel* time;
	GtkTextView* stats;
	GtkButton* buttonEnd;
	GtkButton* buttonMute;
	GtkButton* buttonAccept;
};

#endif /* SRC_SCREENS_CALLSCREEN_HPP_ */
