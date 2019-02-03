/*
 * EditContact.cpp
 *
 *  Created on: Jan 12, 2019
 *      Author: Daniel
 */

#include "EditContact.hpp"

BlockingQ<std::string> EditContact::editedContacts;
std::unordered_map<std::string, std::unique_ptr<EditContact>> EditContact::editWindows;

extern "C" void edit_contact_quit(GtkWidget* button, gpointer data)
{
	EditContact* screen = (EditContact*)data;
	screen->onclickQuit();
}

extern "C" void onclick_edit_contact_save(GtkWidget* button, gpointer data)
{
	EditContact* screen = (EditContact*)data;
	screen->onclickSave();
}

EditContact::EditContact(const std::string& toEdit) :
settings(Settings::getInstance()),
r(R::getInstance()),
contactInEdit(toEdit)
{
	const std::string placeholder = r->getString(R::StringID::EDIT_CONTACT_ENTRY_PLACEHOLDER) + contactInEdit;

	GtkBuilder* builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "glade/edit_contact.glade", NULL);
	window = GTK_WINDOW(gtk_builder_get_object(builder, "edit_contact_window"));
	gtk_window_set_title(window, placeholder.c_str());
	g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK(edit_contact_quit), this);
	entry = GTK_ENTRY(gtk_builder_get_object(builder, "edit_contact_entry"));
	gtk_entry_set_placeholder_text(entry, placeholder.c_str());
	ok = GTK_BUTTON(gtk_builder_get_object(builder, "edit_contact_ok"));
	gtk_button_set_label(ok, r->getString(R::StringID::GENERIC_SAVE).c_str());
	g_signal_connect(G_OBJECT(ok),"clicked", G_CALLBACK(onclick_edit_contact_save), this);

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 75);
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show((GtkWidget*)window);
	g_object_unref(builder);

}

EditContact::~EditContact()
{
	gtk_widget_destroy((GtkWidget*)window);
	g_object_unref(window);
}

//static
void EditContact::renderNew(const std::string& toEdit)
{
	if(editWindows.count(toEdit) > 0)
	{//don't open 2 windows for the same contact
		return;
	}

	editWindows[toEdit] = std::make_unique<EditContact>(toEdit);
}

void EditContact::onclickQuit()
{
	editWindows.erase(contactInEdit);
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

	editedContacts.push(contactInEdit);
	UserHome* home = UserHome::getInstance();
	home->asyncResult(Vars::Broadcast::USERHOME_CONTACTEDITED);

	onclickQuit();
}
