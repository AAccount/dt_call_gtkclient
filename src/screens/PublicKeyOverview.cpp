/*
 * PublicKeyOverview.cpp
 *
 *  Created on: Jan 26, 2019
 *      Author: Daniel
 */

#include "PublicKeyOverview.hpp"

bool PublicKeyOverview::onScreen = false;
PublicKeyOverview* PublicKeyOverview::instance = NULL;

extern "C" void public_key_ov_onclick(GtkWidget* button, gpointer data)
{
	PublicKeyOverview* screen = PublicKeyOverview::getInstance();
	screen->onclick(GTK_BUTTON(button));
}

extern "C" void public_key_ov_quit()
{
	Utils::runOnUiThread(&PublicKeyOverview::remove);
}

PublicKeyOverview::PublicKeyOverview() :
settings(Settings::getInstance()),
r(R::getInstance()),
logger(Logger::getInstance()),
buttonToUser(std::unordered_map<GtkButton*, std::string>()),
userToButton(std::unordered_map<std::string, GtkButton*>()),
edits()
{
	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(window, r->getString(R::StringID::PUBLIC_KEY_OV_TITLE).c_str());
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
	g_signal_connect(G_OBJECT(window),"destroy", public_key_ov_quit, NULL);
	gtk_widget_show(GTK_WIDGET(window));

	userList = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));
	gtk_widget_show(GTK_WIDGET(userList));
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(userList));

	const std::vector<std::string> contacts = settings->getAllContacts();
	users = std::unordered_set<std::string>(contacts.begin(), contacts.end());
	const std::vector<std::string> previouslySeen = settings->getAllRegisteredPublicKeys();
	std::copy(previouslySeen.begin(), previouslySeen.end(), std::inserter(users, users.end()));

	for(const std::string user : users)
	{
		GtkButton* button = GTK_BUTTON(gtk_button_new_with_label(getButtonDisplay(user).c_str()));
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(public_key_ov_onclick), NULL);
		gtk_box_pack_start(userList, GTK_WIDGET(button), true, true, 0);
		gtk_widget_show(GTK_WIDGET(button));
		buttonToUser[button] = user;
		userToButton[user] = button;
	}
	
	AsyncCentral::getInstance()->registerReceiver(this);
}

PublicKeyOverview::~PublicKeyOverview()
{
	AsyncCentral::getInstance()->removeReceiver(this);
	gtk_widget_destroy((GtkWidget*)window);
}

//static
PublicKeyOverview* PublicKeyOverview::getInstance()
{
	return instance;
}

//static
int PublicKeyOverview::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new PublicKeyOverview();

	return 0;
}

//static
int PublicKeyOverview::remove(void* a)
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
	return 0;
}

void PublicKeyOverview::onclick(GtkButton* button)
{
	if(buttonToUser.count(button) == 0)
	{
		const std::string error = r->getString(R::StringID::PUBLIC_KEY_OV_MIA);
		logger->insertLog(Log(Log::TAG::PUBLIC_KEY_OV, error, Log::TYPE::ERROR).toString());
		return;
	}

	const std::string user = buttonToUser[button];
	PublicKeyUser::renderNew(user);
}


void PublicKeyOverview::asyncResult(int result, const std::string& info)
{
	if(result == Vars::Broadcast::PUBLIC_KEYOV_EDIT)
	{
		edits.push(info);
		Utils::runOnUiThread(&PublicKeyOverview::updateButton, this);
	}
}

//static
int PublicKeyOverview::updateButton(void* context)
{
	PublicKeyOverview* screen = static_cast<PublicKeyOverview*>(context);
	const std::string user = screen->edits.pop();
	const std::string display = screen->getButtonDisplay(user);
	if(screen->userToButton.count(user) < 1)
	{
		const std::string error = screen->r->getString(R::StringID::PUBLIC_KEY_OV_MIA) + user;
		screen->logger->insertLog(Log(Log::TAG::PUBLIC_KEY_OV, error, Log::TYPE::ERROR).toString());
		return 0;
	}
	GtkButton* button = screen->userToButton[user];
	gtk_button_set_label(button, display.c_str());
	return 0;
}


std::string PublicKeyOverview::getButtonDisplay(const std::string& user)
{
	std::string display = settings->getNickname(user);
	std::unique_ptr<unsigned char[]> key;
	settings->getPublicKey(user, key);
	if(key.get() == NULL)
	{
		display = display + r->getString(R::StringID::PUBLIC_KEY_OV_NOKEY);
	}
	return display;
}
