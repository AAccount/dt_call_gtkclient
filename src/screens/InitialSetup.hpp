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
#include "../StringRes.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/LoginAsync.hpp"

class InitialSetup : public virtual AsyncReceiver
{
public:
	static void render();
	static void remove();

	//required public to make the extern C calls work. don't touch
	static InitialSetup* instance;
	void onclick_initial_setup_certificate();
	void onclick_initial_setup_login();
	void onclick_initial_setup_private_key();
	void asyncResult(int result);

private:
	InitialSetup();
	virtual ~InitialSetup();

	static bool onScreen;
	GtkWidget* window;
	GtkEntry* addr;
	GtkEntry* commandPort;
	GtkEntry* mediaPort;
	GtkButton* serverCert;
	GtkEntry* username;
	GtkButton* privateKey;
	GtkButton* login;

	StringRes* strings;
	bool gotServerCert = false;
	bool gotPrivateKey = false;
};

#endif /* SRC_SCREENS_INITIALSETUP_HPP_ */
