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

class UserHome : public virtual AsyncReceiver
{
public:
	static void render();
	static void remove();
	static UserHome* instance;

	void asyncResult(int result);
	void onclickDial();

private:
	UserHome();
	virtual ~UserHome();
	static bool onScreen;
	GtkWidget* window;
	GtkEntry* entry;
	GtkBox* contactList;
	gulong destroySignalID;
};

#endif /* SRC_SCREENS_USERHOME_HPP_ */
