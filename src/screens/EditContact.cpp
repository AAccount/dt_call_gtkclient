/*
 * EditContact.cpp
 *
 *  Created on: Jan 12, 2019
 *      Author: Daniel
 */

#include "EditContact.hpp"

bool EditContact::onScreen = false;
EditContact* EditContact::instance = NULL;
std::string EditContact::contactInEdit = "";

EditContact::EditContact() :
settings(Settings::getInstance()),
r(R::getInstance())
{
	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/edit_contact.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "edit_contact_window"));
	entry = GTK_ENTRY(gtk_builder_get_object(builder, "edit_contact_entry"));
	const std::string placeholder = r->getString(R::StringID::EDIT_CONTACT_ENTRY_PLACEHOLDER) + contactInEdit;
	gtk_entry_set_placeholder_text(entry, placeholder.c_str());
	ok = GTK_BUTTON(gtk_builder_get_object(builder, "edit_contact_ok"));
	gtk_button_set_label(ok, r->getString(R::StringID::EDIT_CONTACT_SAVE).c_str());

	gtk_window_set_default_size(GTK_WINDOW(window), 300, 75);
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show((GtkWidget*)window);
	g_object_unref(builder);

	onScreen = true;
}

EditContact::~EditContact()
{
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
EditContact* EditContact::getInstance()
{
	return instance;
}

//static
int EditContact::render(void* a)
{
	onScreen = true;
	if(instance != NULL)
	{//only 1 version of the screen will be active
		delete instance;
	}
	instance = new EditContact();

	return 0;
}

//static
int EditContact::remove(void* a)
{
	onScreen = false;
	if(instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
	return 0;
}

void EditContact::onclickSave()
{
	const std::string nickname = std::string(gtk_entry_get_text(entry));
	if(nickname.empty())
	{
		return;
	}
	settings->modifyContact(contactInEdit, nickname);
	settings->save();

	UserHome* home = UserHome::getInstance();
	home->asyncResult(Vars::Broadcast::USERHOME_CONTACTEDITED);
}


extern "C" void onclick_edit_contact_save()
{
	EditContact* instance = EditContact::getInstance();
	if(instance != NULL)
	{
		instance->onclickSave();
	}

}
