/*
 * UserHome.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: Daniel
 */

#include "UserHome.hpp"

bool UserHome::onScreen = false;
UserHome* UserHome::instance = NULL;

UserHome::UserHome()
{
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/user_home2.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "user_home_window"));
	entry = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(builder, "user_home_entry")));
	contactList = GTK_BOX(GTK_WIDGET(gtk_builder_get_object(builder, "user_home_contact_list")));
	destroySignalID = g_signal_connect(G_OBJECT(window),"destroy", Utils::quit, NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);
	gtk_widget_show(window);

	onScreen = true;
	InitialSetup::remove();
}

UserHome::~UserHome()
{
	//this window is purposely going away, unhook it from the main quit function
	g_signal_handler_disconnect(window, destroySignalID);

	gtk_widget_destroy(window);
	g_object_unref(window);
}

//static
void UserHome::render()
{
//	InitialSetup::remove();
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new UserHome();
}

//static
void UserHome::remove()
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
}

void UserHome::asyncResult(int result)
{

}

void UserHome::onclickDial()
{
	std::cerr << "begin the whole process \n";
}

extern "C" void onclick_user_home_dial()
{
	if(UserHome::instance != NULL)
	{
		UserHome::instance->onclickDial();
		std::cerr << "clicked dial\n";
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
