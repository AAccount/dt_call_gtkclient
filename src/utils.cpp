/*
 * Utils.cpp
 *
 *  Created on: Aug 14, 2016
 *      Author: Daniel
 */

#include "utils.hpp"

#include <time.h>
#include <stdint.h>

void Utils::show_popup(const std::string& message, GtkWindow* parent)
{
	GtkWidget* popup = gtk_message_dialog_new(parent,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			message.c_str(),
			NULL);
//	g_signal_connect_swapped(popup, "response", G_CALLBACK(gtk_widget_destroy), popup);
	gtk_dialog_run(GTK_DIALOG(popup));
	gtk_widget_destroy(popup);
}

std::string Utils::file_chooser(const std::string& message, GtkWindow* parent)
{
	std::string path = "";
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget* dialog = gtk_file_chooser_dialog_new(message.c_str(), parent, action,
			"Cancel", GTK_RESPONSE_CANCEL,
			"Open", GTK_RESPONSE_ACCEPT,
			NULL);
	const gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	if(result == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
		char* filename = gtk_file_chooser_get_filename(chooser);
		path = std::string(filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
	return path;
}

//never changed since it was written, just copied and pasted
std::string Utils::dumpSmallFile(const std::string& path)
{
	std::ifstream fileStream(path);
	if(!fileStream.good())
	{
		return "";
	}
	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	fileStream.close();
	return stringStream.str();
}

//use a vector to prevent reading out of bounds
std::vector<std::string> Utils::parse(unsigned char command[])
{
//timestamp|login1|username
//timestamp|login2|username|challenge_decrypted

//session key is always the last one for easy censoring in the logs
//timestamp|call|otheruser|sessionkey
//timestamp|lookup|otheruser|sessionkey
//timestamp|reject|otheruser|sessionkey
//timestamp|accept|otheruser|sessionkey
//timestamp|end|otheruser|sessionkey
//timestamp|passthrough|otheruser|(aes key encrypted)|sessionkey
//timestamp|ready|otheruser|sessionkey

	char* token;
	char* save;
	int i = 0;
	std::vector<std::string> result;
	token = strtok_r((char*)command, "|", &save);
	while(token != NULL && i < 5) //TODO: this 5 came from COMMAND_MAX_SEGMENTS
	{
		result.push_back(std::string(token));
		token = strtok_r(NULL, "|", &save);
		i++;
	}
	return result;
}

bool Utils::validTS(time_t ts)
{
	const time_t now = time(NULL);
	const time_t bigger = std::max(ts, now);
	const time_t smaller = std::min(ts, now);
	const time_t difference = bigger - smaller;
	if(difference > 60*5)
	{
		return false;
	}
	return true;
}

time_t Utils::now()
{
	const time_t nowvar = time(NULL);
	return nowvar;
}
