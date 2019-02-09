/*
 * PublicKeyUser.cpp
 *
 *  Created on: Jan 27, 2019
 *      Author: Daniel
 */

#include "PublicKeyUser.hpp"

std::unordered_map<std::string, std::unique_ptr<PublicKeyUser>> PublicKeyUser::uwindows;

extern "C" void public_key_user_quit(GtkWidget* button, gpointer data)
{
	PublicKeyUser* screen = static_cast<PublicKeyUser*>(data);
	screen->onclickQuit();
}

extern "C" void onclick_public_key_user_edit(GtkWidget* button, gpointer data)
{
	PublicKeyUser* screen = static_cast<PublicKeyUser*>(data);
	screen->onclickEdit();
}

extern "C" void onclick_public_key_user_remove(GtkWidget* button, gpointer data)
{
	PublicKeyUser* screen = static_cast<PublicKeyUser*>(data);
	screen->onclickRemove();
}

PublicKeyUser::PublicKeyUser(const std::string& puser) :
user(puser),
settings(Settings::getInstance()),
r(R::getInstance())
{
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/public_keyu.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "window_public_keyu"));
	const std::string title = settings->getNickname(puser) + r->getString(R::StringID::PUBLIC_KEYU_TITLE);
	gtk_window_set_title(window, title.c_str());
	g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK(public_key_user_quit), this);
	dump = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "public_keyu_dump"));
	dumpBuffer = gtk_text_buffer_new(NULL);
	std::string dumpDisplay = SodiumUtils::SODIUM_PUBLIC_HEADER() + settings->getPublicKeyString(puser);
	if(dumpDisplay == SodiumUtils::SODIUM_PUBLIC_HEADER())
	{
		dumpDisplay = r->getString(R::StringID::PUBLIC_KEY_OV_NOKEY) + "\n";
	}
	gtk_text_buffer_set_text(dumpBuffer, dumpDisplay.c_str(), -1);
	gtk_text_view_set_buffer(dump, dumpBuffer);
	edit = GTK_BUTTON(gtk_builder_get_object(builder, "public_keyu_edit"));
	gtk_button_set_label(edit, r->getString(R::StringID::GENERIC_EDIT).c_str());
	g_signal_connect(G_OBJECT(edit),"clicked", G_CALLBACK(onclick_public_key_user_edit), this);
	remove = GTK_BUTTON(gtk_builder_get_object(builder, "public_keyu_remove"));
	gtk_button_set_label(remove, r->getString(R::StringID::GENERIC_REMOVE).c_str());
	g_signal_connect(G_OBJECT(remove),"clicked", G_CALLBACK(onclick_public_key_user_remove), this);


	gtk_window_set_default_size(GTK_WINDOW(window), 400, 75);
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show(GTK_WIDGET(window));
	g_object_unref(builder);
}

PublicKeyUser::~PublicKeyUser()
{
	gtk_widget_destroy(GTK_WIDGET(window));
	g_object_unref(window);
	g_object_unref(dumpBuffer);
}

//static
void PublicKeyUser::renderNew(const std::string& puser)
{
	if(uwindows.count(puser) > 0)
	{//don't open 2 windows for the same contact
		return;
	}

	uwindows[puser] = std::make_unique<PublicKeyUser>(puser);
}

void PublicKeyUser::onclickQuit()
{
	uwindows.erase(user);
}

void PublicKeyUser::onclickEdit()
{
	std::unique_ptr<unsigned char[]> newkey;
	const std::string message = r->getString(R::StringID::PUBLIC_KEYU_EDIT_TITLE);
	const std::string error = r->getString(R::StringID::PUBLIC_KEYU_EDIT_ERROR);
	Utils::setupPublicKey(message, window, Log::TAG::PUBLIC_KEYU, error, newkey);
	if(newkey.get() != NULL)
	{
		settings->modifyPublicKey(user, newkey);
		settings->save();
		PublicKeyOverview* ovScreen = PublicKeyOverview::getInstance();
		if(ovScreen != NULL)
		{
			ovScreen->pushEdit(user);
			ovScreen->asyncResult(Vars::Broadcast::PUBLIC_KEYOV_EDIT);
		}

		const std::string keyDump = Stringify::stringify(newkey.get(), crypto_box_PUBLICKEYBYTES);
		gtk_text_buffer_set_text(dumpBuffer, keyDump.c_str(), -1);
		gtk_text_view_set_buffer(dump, dumpBuffer);
	}
}

void PublicKeyUser::onclickRemove()
{
	settings->removePublicKey(user);
	settings->save();
	PublicKeyOverview* ovScreen = PublicKeyOverview::getInstance();
	if(ovScreen != NULL)
	{
		ovScreen->pushEdit(user);
		ovScreen->asyncResult(Vars::Broadcast::PUBLIC_KEYOV_EDIT);
	}
	const std::string nokey = r->getString(R::StringID::PUBLIC_KEY_OV_NOKEY) + "\n";
	gtk_text_buffer_set_text(dumpBuffer, nokey.c_str(), -1);
	gtk_text_view_set_buffer(dump, dumpBuffer);
}
