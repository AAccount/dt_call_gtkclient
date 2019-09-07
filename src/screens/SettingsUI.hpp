/*
 * initial_server.hpp
 *
 *  Created on: Dec 28, 2017
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_SETTINGSUI_HPP_
#define SRC_SCREENS_SETTINGSUI_HPP_

#include <gtk/gtk.h>
#include <iostream>
#include <memory>
#include <sodium.h>
#include <algorithm>
#include <cctype>
#include "../vars.hpp"
#include "../utils.hpp"
#include "../sodium_utils.hpp"
#include "../stringify.hpp"
#include "../R.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/LoginManager.hpp"
#include "../settings.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "UserHome.hpp"
#include "../background/AsyncCentral.hpp"

class SettingsUI : public virtual AsyncReceiver
{
public:
	static bool initialSetup;

	static int render(void* a);
	static int remove(void* a);
	static SettingsUI* getInstance();

	void setupCertificate();
	void nextFunction();
	void setupPrivateKey();
	void asyncResult(int result) override;

private:
	SettingsUI();
	virtual ~SettingsUI();

	static SettingsUI* instance;

	static bool onScreen;
	GtkWindow* window;
	GtkEntry* addr;
	GtkEntry* commandPort;
	GtkEntry* mediaPort;
	GtkButton* serverCert;
	GtkEntry* username;
	GtkButton* privateKey;
	GtkButton* next;

	void login();
	void saveSettings();
	void removeWhiteSpace(std::string& s);

	R* r;
	Logger* logger;
	bool gotServerCert = false;
	bool gotPrivateKey = false;
	gulong destroySignalID;
};

#endif /* SRC_SCREENS_SETTINGSUI_HPP_ */
