/*
 * CallScreen.cpp
 *
 *  Created on: Dec 29, 2018
 *      Author: Daniel
 */

#include "CallScreen.hpp"

bool CallScreen::onScreen = false;
CallScreen::Mode CallScreen::mode;
CallScreen* CallScreen::instance = NULL;

extern "C" void call_screen_quit()
{
	Utils::quit(Vars::privateKey.get(), Vars::voiceKey.get());
}

CallScreen::CallScreen() :
r(R::getInstance()),
logger(Logger::getInstance(""))
{
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/call_screen.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "call_screen_window"));
	gtk_window_set_title(window, r->getString(R::StringID::CALL_SCREEN_TITLE).c_str());
	status = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_status"));
	gtk_label_set_text(status, r->getString(R::StringID::CALL_SCREEN_STATUS_RINGING).c_str());
	time = GTK_LABEL(gtk_builder_get_object(builder, "call_screen_time"));
	gtk_label_set_text(time, "0:00");
	buttonEnd = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_end"));
	gtk_button_set_label(buttonEnd, r->getString(R::StringID::USER_HOME_BUTTON_CONTACT).c_str());
	buttonMute = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_mute"));
	gtk_button_set_label(buttonMute, r->getString(R::StringID::USER_HOME_BUTTON_CONTACT).c_str());
	buttonAccept = GTK_BUTTON(gtk_builder_get_object(builder, "call_screen_accept"));
	gtk_button_set_label(buttonAccept, r->getString(R::StringID::USER_HOME_BUTTON_CONTACT).c_str());
	if(mode == Mode::DIALING) //you've obviously accepted the call considering you're dialing it
	{
		gtk_widget_set_sensitive((GtkWidget*)buttonAccept, false);
	}
	stats = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "call_screen_stats"));
}

CallScreen::~CallScreen()
{
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
void CallScreen::render()
{

}

//static
void CallScreen::remove()
{

}

void CallScreen::asyncResult(int result)
{

}
