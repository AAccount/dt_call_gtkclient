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
#include <vector>
#include <unordered_map>
#include <gtk/gtk.h>
#include "CallScreen.hpp"
#include "EditContact.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/CommandCall.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../settings.hpp"
#include "SettingsUI.hpp"

class UserHome : public virtual AsyncReceiver
{
public:
	static int render(void* a);
	static int remove(void* a);
	static UserHome* getInstance();

	void asyncResult(int result) override;
	void onclickDial();
	void onclickNewContact();
	void onclickContact(GtkButton* button);
	void onclickContactEdit(GtkButton* button);
	void onclickContactRemove(GtkButton* button);

private:
	static UserHome* instance;

	UserHome();
	virtual ~UserHome();
	static bool onScreen;

	static int lockDial(void* context);
	static int unlockDial(void* context);
	static int statusOnline(void* context);
	static int statusOffline(void* context);
	static int changeContactButton(void* context);

	void renderContact(const std::string& name);
	std::unordered_map<std::string, GtkBox*> contactToContainer;
	std::unordered_map<GtkButton*, std::string> buttonToContact;
	std::unordered_map<std::string, GtkButton*> contactToButton;
	std::unordered_map<GtkButton*, std::string> editButtonToContact;
	std::unordered_map<std::string, GtkButton*> contactToEditButton;
	std::unordered_map<GtkButton*, std::string> removeButtonToContact;
	std::unordered_map<std::string, GtkButton*> contactToRemoveButton;

	Logger* logger;
	R* r;
	Settings* settings;
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
