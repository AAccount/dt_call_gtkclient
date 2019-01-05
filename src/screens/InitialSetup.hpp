/*
 * initial_server.hpp
 *
 *  Created on: Dec 28, 2017
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_INITIALSETUP_HPP_
#define SRC_SCREENS_INITIALSETUP_HPP_

#include <gtk/gtk.h>
#include <iostream>
#include <memory>
#include <sodium.h>
#include "../vars.hpp"
#include "../utils.hpp"
#include "../sodium_utils.hpp"
#include "../stringify.hpp"
#include "../R.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/LoginAsync.hpp"
#include "../settings.hpp"
#include "../Logger.hpp"
#include "../Log.hpp"
#include "UserHome.hpp"

class InitialSetup : public virtual AsyncReceiver
{
public:
	static int render(void* a);
	static int remove(void* a);
	static InitialSetup* getInstance();

	void setupCertificate();
	void setupLogin();
	void setupPrivateKey();
	void asyncResult(int result) override;

private:
	InitialSetup();
	virtual ~InitialSetup();

	static InitialSetup* instance;

	static bool onScreen;
	GtkWindow* window;
	GtkEntry* addr;
	GtkEntry* commandPort;
	GtkEntry* mediaPort;
	GtkButton* serverCert;
	GtkEntry* username;
	GtkButton* privateKey;
	GtkButton* login;

	R* r;
	Logger* logger;
	bool gotServerCert = false;
	bool gotPrivateKey = false;
	gulong destroySignalID;
};

#endif /* SRC_SCREENS_INITIALSETUP_HPP_ */
