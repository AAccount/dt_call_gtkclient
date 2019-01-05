/*
 * UserHome.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: Daniel
 */

#include "UserHome.hpp"

bool UserHome::onScreen = false;
UserHome* UserHome::instance = NULL;

extern "C" void user_home_quit()
{
	Vars::commandSocket.stop();
	Utils::quit(Vars::privateKey.get(), Vars::voiceKey.get());
}

UserHome::UserHome()
{
	r = R::getInstance();
	logger = Logger::getInstance("");

	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/user_home2.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "user_home_window"));
	gtk_window_set_title(window, r->getString(R::StringID::USER_HOME_TITLE).c_str());
	connectionStatus = GTK_LABEL(gtk_builder_get_object(builder, "user_home_connection"));
	entry = GTK_ENTRY(gtk_builder_get_object(builder, "user_home_entry"));
	gtk_entry_set_placeholder_text(entry, r->getString(R::StringID::USER_HOME_PLACEHOLDER_ENTRY).c_str());
	dial = GTK_BUTTON(gtk_builder_get_object(builder, "user_home_dial"));
	gtk_button_set_label(dial, r->getString(R::StringID::USER_HOME_BUTTON_DIAL).c_str());
	addContact = GTK_BUTTON(gtk_builder_get_object(builder, "user_home_add_contact"));
	gtk_button_set_label(addContact, r->getString(R::StringID::USER_HOME_BUTTON_CONTACT).c_str());
	contactList = GTK_BOX(gtk_builder_get_object(builder, "user_home_contact_list"));
	contactsLabel = GTK_LABEL(gtk_builder_get_object(builder, "user_home_label_contacts"));
	gtk_label_set_text(contactsLabel, r->getString(R::StringID::USER_HOME_LABEL_CONTACTS).c_str());
	destroySignalID = g_signal_connect(G_OBJECT(window),"destroy", user_home_quit, NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show((GtkWidget*)window);
	g_object_unref(builder);

	if(Vars::sessionKey.empty())
	{
		gtk_label_set_text(connectionStatus, r->getString(R::StringID::USER_HOME_OFFLINE).c_str());
		LoginAsync::execute(this, true);
	}
	else
	{
		gtk_label_set_text(connectionStatus, r->getString(R::StringID::USER_HOME_ONLINE).c_str());
	}

	onScreen = true;
	Utils::runOnUiThread(&InitialSetup::remove);
}

UserHome::~UserHome()
{
	//this window is purposely going away, unhook it from the main quit function
	g_signal_handler_disconnect(window, destroySignalID);

	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
int UserHome::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new UserHome();

	return 0;
}

//static
int UserHome::remove(void* a)
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
	return 0;
}

int UserHome::lockDial(void* context)
{
	UserHome* screen = (UserHome*)context;
	gtk_widget_set_sensitive((GtkWidget*)(screen->dial), false);
	return 0;
}

int UserHome::unlockDial(void* context)
{
	UserHome* screen = (UserHome*)context;
	gtk_widget_set_sensitive((GtkWidget*)(screen->dial), true);
	return 0;
}

int UserHome::statusOnline(void* context)
{
	UserHome* screen = (UserHome*)context;
	const std::string text = screen->r->getString(R::StringID::USER_HOME_ONLINE);
	gtk_label_set_text(screen->connectionStatus, text.c_str());
	return 0;
}

int UserHome::statusOffline(void* context)
{
	UserHome* screen = (UserHome*)context;
	const std::string text = screen->r->getString(R::StringID::USER_HOME_OFFLINE);
	gtk_label_set_text(screen->connectionStatus, text.c_str());
	return 0;
}
void UserHome::asyncResult(int result)
{
	if(result == Vars::Broadcast::LOGIN_OK)
	{
		Utils::runOnUiThread(&UserHome::statusOnline, this);
	}
	else if(result == Vars::Broadcast::LOGIN_NOTOK)
	{
		Utils::runOnUiThread(&UserHome::statusOffline, this);
	}
	else if(result == Vars::Broadcast::CALL_END)
	{
		Utils::showPopup(r->getString(R::StringID::USER_HOME_CANT_DIAL), window);
	}
	else if(result == Vars::Broadcast::CALL_TRY)
	{
		CallScreen::mode == CallScreen::Mode::DIALING;
		Utils::runOnUiThread(&CallScreen::render);
	}
	else if(result == Vars::Broadcast::UNLOCK_USERHOME)
	{
		Utils::runOnUiThread(&UserHome::unlockDial, this);
	}
	else if(result == Vars::Broadcast::LOCK_USERHOME)
	{
		Utils::runOnUiThread(&UserHome::lockDial, this);
	}
}

void UserHome::onclickDial()
{
	const std::string who = std::string(gtk_entry_get_text(entry));
	if(who.empty())
	{
		return;
	}

	Vars::callWith = who;
	CommandCall::execute();
}

void UserHome::onclickNewContact()
{
	std::cout << "clicked new contact\n";
}

extern "C" void onclick_user_home_dial()
{
	if(UserHome::instance != NULL)
	{
		UserHome::instance->onclickDial();
	}
}

extern "C" void onclick_user_home_add_contact()
{
	if(UserHome::instance != NULL)
	{
		UserHome::instance->onclickNewContact();
		std::cerr << "clicked add contact\n";
	}
}

extern "C" void onclick_menu_settings()
{
	if(UserHome::instance != NULL)
	{
		std::cerr << "open the menu window\n";
	}
}

extern "C" void onclick_menu_publickeys()
{
	if(UserHome::instance != NULL)
	{
		std::cerr << "clicked menu public keys\n";
	}
}

extern "C" void onclick_menu_quit()
{
	if(UserHome::instance != NULL)
	{
		std::cerr << "clicked menu quit\n";
	}
}

extern "C" void onclick_menu_about()
{
	if(UserHome::instance != NULL)
	{
		std::cerr << "clicked menu about\n";
	}
}
