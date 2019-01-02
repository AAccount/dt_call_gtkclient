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

bool Utils::validTS(const std::string& ts)
{
	time_t tsraw = 0;
	try
	{
		tsraw = std::stoull(ts);
	}
	catch(std::invalid_argument& e)
	{
		return false;
	}

	const time_t now = time(NULL);
	const time_t bigger = std::max(tsraw, now);
	const time_t smaller = std::min(tsraw, now);
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

void Utils::quit(unsigned char privateKey[], unsigned char voiceKey[])
{
	if(privateKey != NULL)
	{
		randombytes_buf(privateKey, crypto_box_SECRETKEYBYTES);
	}

	if(voiceKey != NULL)
	{
		randombytes_buf(voiceKey, crypto_secretbox_KEYBYTES);
	}
	exit(0);
}

bool Utils::connectFD(int& fd, int type, const std::string& caddr, int cport, struct sockaddr_in* serv_addr)
{
	fd = socket(type, SOCK_STREAM, 0);
	if(fd < 0)
	{
//		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_SOCKET_SYSCALL);
		return false;
	}

	memset(serv_addr, '0', sizeof(struct sockaddr_in));
	serv_addr->sin_family = type;
	serv_addr->sin_port = htons(cport);
	const int result = inet_pton(type, caddr.c_str(), &serv_addr->sin_addr);
	if(result < 0)
	{
//		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_INET_PTON);
		return false;
	}

	const int connectOk = connect(fd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in));
	if(connectOk < 0)
	{
//		throw strings->getString(StringRes::Language::EN, StringRes::StringID::ERR_SODIUM_CONNECT_SYSCALL);
		return false;
	}
	return true;
}
