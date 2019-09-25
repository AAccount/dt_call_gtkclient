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
	UserHome* screen = UserHome::getInstance();
	screen->onclickQuit();
}

extern "C" void user_home_contact_button(GtkWidget* button, gpointer data)
{
	UserHome* screen = UserHome::getInstance();
	screen->onclickContact((GtkButton*)button);
}

extern "C" void user_home_contact_edit(GtkWidget* button, gpointer data)
{
	UserHome* screen = UserHome::getInstance();
	screen->onclickContactEdit((GtkButton*)button);}

extern "C" void user_home_contact_remove(GtkWidget* button, gpointer data)
{
	UserHome* screen = UserHome::getInstance();
	screen->onclickContactRemove((GtkButton*)button);
}

UserHome::UserHome() :
settings(Settings::getInstance()),
r(R::getInstance()),
logger(Logger::getInstance()),
contactToContainer(std::unordered_map<std::string, GtkBox*>()),
buttonToContact(std::unordered_map<GtkButton*, std::string>()),
contactToButton(std::unordered_map<std::string, GtkButton*>()),
editButtonToContact(std::unordered_map<GtkButton*, std::string>()),
contactToEditButton(std::unordered_map<std::string, GtkButton*>()),
removeButtonToContact(std::unordered_map<GtkButton*, std::string>()),
contactToRemoveButton(std::unordered_map<std::string, GtkButton*>())
{
	GtkBuilder* builder = gtk_builder_new_from_resource("/gtkclient/user_home2.glade");
	window = GTK_WINDOW(gtk_builder_get_object(builder, "user_home_window"));
	gtk_window_set_title(window, r->getString(R::StringID::SELF).c_str());
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
	destroyHandleID = g_signal_connect(G_OBJECT(window),"destroy", user_home_quit, NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show((GtkWidget*)window);
	g_object_unref(builder);

	if(Vars::sessionKey.empty())
	{
		gtk_label_set_text(connectionStatus, r->getString(R::StringID::USER_HOME_OFFLINE).c_str());
		LoginManager::getInstance()->execute(this);
	}
	else
	{
		gtk_label_set_text(connectionStatus, r->getString(R::StringID::USER_HOME_ONLINE).c_str());
	}

	const std::vector<std::string> contacts = settings->getAllContacts();
	for(std::string contact : contacts)
	{
		renderContact(contact);
	}

	onScreen = true;
	AsyncCentral::getInstance()->registerReceiver(this);
	Utils::runOnUiThread(&SettingsUI::remove);
}

UserHome::~UserHome()
{
	AsyncCentral::getInstance()->removeReceiver(this);
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
UserHome* UserHome::getInstance()
{
	return instance;
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
	UserHome* screen = static_cast<UserHome*>(context);
	gtk_widget_set_sensitive((GtkWidget*)(screen->dial), false);
	return 0;
}

int UserHome::unlockDial(void* context)
{
	UserHome* screen = static_cast<UserHome*>(context);
	gtk_widget_set_sensitive((GtkWidget*)(screen->dial), true);
	return 0;
}

int UserHome::statusOnline(void* context)
{
	UserHome* screen = static_cast<UserHome*>(context);
	const std::string text = screen->r->getString(R::StringID::USER_HOME_ONLINE);
	gtk_label_set_text(screen->connectionStatus, text.c_str());
	return 0;
}

int UserHome::statusOffline(void* context)
{
	UserHome* screen = static_cast<UserHome*>(context);
	const std::string text = screen->r->getString(R::StringID::USER_HOME_OFFLINE);
	gtk_label_set_text(screen->connectionStatus, text.c_str());
	return 0;
}

int UserHome::changeContactButton(void* context)
{
	UserHome* screen = static_cast<UserHome*>(context);
	const std::string contact = EditContact::editedContacts.pop();
	const std::string newNickname = screen->settings->getNickname(contact);
	if(screen->contactToButton.count(contact) < 1)
	{
		const std::string error = screen->r->getString(R::StringID::USER_HOME_CONTACTS_NOT_REGISTERED) + contact;
		screen->logger->insertLog(Log(Log::TAG::USER_HOME, error, Log::TYPE::ERROR).toString());
		return 0;
	}
	GtkButton* contactButton = screen->contactToButton[contact];
	gtk_button_set_label(contactButton, newNickname.c_str());
	return 0;
}

void UserHome::asyncResult(int result, const std::string& info)
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
		CallScreen::mode = CallScreen::Mode::DIALING;
		Utils::runOnUiThread(&CallScreen::render);
	}
	else if(result == Vars::Broadcast::USERHOME_UNLOCK)
	{
		Utils::runOnUiThread(&UserHome::unlockDial, this);
	}
	else if(result == Vars::Broadcast::USERHOME_LOCK)
	{
		Utils::runOnUiThread(&UserHome::lockDial, this);
	}
	else if(result == Vars::Broadcast::USERHOME_CONTACTEDITED)
	{
		Utils::runOnUiThread(&UserHome::changeContactButton, this);
	}
}

void UserHome::onclickQuit()
{
	g_signal_handler_disconnect(G_OBJECT(window), destroyHandleID); //onclick call end is actually ran again after the window is gone unless you tell it not to
	Vars::isExiting = true;
	Vars::commandSocket.get()->stop();
	Utils::quit(Vars::privateKey.get(), Vars::voiceKey.get());
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
	const std::string who = std::string(gtk_entry_get_text(entry));
	if(who.empty() || settings->contactExists(who))
	{
		return;
	}

	settings->modifyContact(who, who);
	settings->save();
	renderContact(who);
}

void UserHome::renderContact(const std::string& name)
{
	GtkBox* container = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_box_set_homogeneous(container, false);
	contactToContainer[name] = container;

	GtkButton* contact = GTK_BUTTON(gtk_button_new_with_label(settings->getNickname(name).c_str()));
	g_signal_connect(G_OBJECT(contact),"clicked", G_CALLBACK(user_home_contact_button), NULL);
	gtk_box_pack_start(container, (GtkWidget*)contact, true, true, 0);
	buttonToContact[contact] = name;
	contactToButton[name] = contact;

	GtkButton* edit = GTK_BUTTON(gtk_button_new_with_label(r->getString(R::StringID::GENERIC_EDIT).c_str()));
	g_signal_connect(G_OBJECT(edit),"clicked", G_CALLBACK(user_home_contact_edit), NULL);
	gtk_box_pack_start(container, (GtkWidget*)edit, false, true, 0);
	editButtonToContact[edit] = name;
	contactToEditButton[name] = edit;

	GtkButton* remove = GTK_BUTTON(gtk_button_new_with_label(r->getString(R::StringID::GENERIC_REMOVE).c_str()));
	g_signal_connect(G_OBJECT(remove),"clicked", G_CALLBACK(user_home_contact_remove), NULL);
	gtk_box_pack_start(container, (GtkWidget*)remove, false, true, 0);
	removeButtonToContact[remove] = name;
	contactToRemoveButton[name] = remove;

	gtk_widget_show(GTK_WIDGET(contact));
	gtk_widget_show(GTK_WIDGET(edit));
	gtk_widget_show(GTK_WIDGET(remove));
	gtk_widget_show(GTK_WIDGET(container));
	gtk_container_add((GtkContainer*)contactList, (GtkWidget*)container);
}

void UserHome::onclickContact(GtkButton* button)
{
	if(buttonToContact.count(button) < 1)
	{
		const std::string error = r->getString(R::StringID::USER_HOME_CONTACTS_NOENTRY);
		logger->insertLog(Log(Log::TAG::USER_HOME, error, Log::TYPE::ERROR).toString());
		return;
	}
	const std::string actualContact = buttonToContact[button];
	gtk_entry_set_text(entry, actualContact.c_str());
}

void UserHome::onclickContactEdit(GtkButton* button)
{
	if(editButtonToContact.count(button) < 1)
	{
		const std::string error = r->getString(R::StringID::USER_HOME_CONTACTS_EDIT_NOENTRY);
		logger->insertLog(Log(Log::TAG::USER_HOME, error, Log::TYPE::ERROR).toString());
		return;
	}

	const std::string actualContact = editButtonToContact[button];
	EditContact::renderNew(actualContact);
}

void UserHome::onclickContactRemove(GtkButton* button)
{
	if(removeButtonToContact.count(button) < 1)
	{
		const std::string error = r->getString(R::StringID::USER_HOME_CONTACTS_REMOVE_NOENTRY);
		logger->insertLog(Log(Log::TAG::USER_HOME, error, Log::TYPE::ERROR).toString());
		return;
	}
	const std::string actualContact = removeButtonToContact[button];
	settings->removeContact(actualContact);
	settings->save();

	try
	{
		GtkBox* container = contactToContainer.at(actualContact);
		gtk_widget_destroy((GtkWidget*)container);
		contactToContainer.erase(actualContact);

		GtkButton* contactButton = contactToButton.at(actualContact);
		buttonToContact.erase(contactButton);
		contactToButton.erase(actualContact);
		GtkButton* editButton = contactToEditButton.at(actualContact);
		editButtonToContact.erase(editButton);
		contactToEditButton.erase(actualContact);
		removeButtonToContact.erase(button);
		contactToRemoveButton.erase(actualContact);
	}
	catch(std::out_of_range& e)
	{
		const std::string error = r->getString(R::StringID::USER_HOME_CONTACTS_REMOVE_NOENTRY) + e.what();
		logger->insertLog(Log(Log::TAG::USER_HOME, error, Log::TYPE::ERROR).toString());
	}

}

void UserHome::onclickAbout()
{
	GtkWidget* about = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), r->getString(R::StringID::SELF).c_str());
	const std::string vstring = r->getString(R::StringID::VERSION) + ":" + r->getString(R::StringID::COMMIT);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), vstring.c_str());
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

extern "C" void onclick_user_home_dial()
{
	UserHome* instance = UserHome::getInstance();
	if(instance != NULL)
	{
		instance->onclickDial();
	}
}

extern "C" void onclick_user_home_add_contact()
{
	UserHome* instance = UserHome::getInstance();
	if(instance != NULL)
	{
		instance->onclickNewContact();
	}
}

extern "C" void onclick_menu_settings()
{
	UserHome* instance = UserHome::getInstance();
	if(instance != NULL)
	{
		SettingsUI::initialSetup = false;
		Utils::runOnUiThread(&SettingsUI::render);
	}
}

extern "C" void onclick_menu_publickeys()
{
	UserHome* instance = UserHome::getInstance();

	if(instance != NULL)
	{
		Utils::runOnUiThread(&PublicKeyOverview::render);
	}
}

extern "C" void onclick_menu_quit()
{
	UserHome* instance = UserHome::getInstance();
	if(instance != NULL)
	{
		user_home_quit();
	}
}

extern "C" void onclick_menu_about()
{
	UserHome* instance = UserHome::getInstance();
	if(instance != NULL)
	{
		instance->onclickAbout();
	}
}
