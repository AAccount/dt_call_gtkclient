/*
 * UserHome.hpp
 *
 *  Created on: Dec 25, 2018
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_USERHOME_HPP_
#define SRC_SCREENS_USERHOME_HPP_
#include <string>
#include <iostream>
#include <gtk/gtk.h>
#include "../background/AsyncReceiver.hpp"
#include "InitialSetup.hpp"
#include "CallScreen.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../background/CommandCall.hpp"

class UserHome : public virtual AsyncReceiver
{
public:
	static int render(void* a);
	static int remove(void* a);
	static UserHome* getInstance();

	void asyncResult(int result) override;
	void onclickDial();
	void onclickNewContact();


private:
	static UserHome* instance;

	UserHome();
	virtual ~UserHome();
	static bool onScreen;

	static int lockDial(void* context);
	static int unlockDial(void* context);
	static int statusOnline(void* context);
	static int statusOffline(void* context);

	Logger* logger;
	R* r;
	GtkWindow* window;
	GtkLabel* connectionStatus;
	GtkEntry* entry;
	GtkButton* dial;
	GtkLabel* contactsLabel;
	GtkButton* addContact;
	GtkBox* contactList;
	gulong destroySignalID;
};

#endif /* SRC_SCREENS_USERHOME_HPP_ */
