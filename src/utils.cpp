/*
 * Utils.cpp
 *
 *  Created on: Aug 14, 2016
 *      Author: Daniel
 */

#include "utils.hpp"

namespace
{

	std::string popupMessage;
	GtkWindow* popupParent;

	int popupThread(void* a)
	{
		GtkWidget* popup = gtk_message_dialog_new(popupParent,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				popupMessage.c_str(),
				NULL);
		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);
		return 0;
	}
}

void Utils::showPopup(const std::string& message, GtkWindow* parent)
{
	popupMessage = message;
	popupParent = parent;
	runOnUiThread(popupThread);
}

std::string Utils::fileChooser(const std::string& message, GtkWindow* parent)
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
	const static int COMMAND_SEGMENTS_MAX = 5;
	while(token != NULL && i < COMMAND_SEGMENTS_MAX)
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

void Utils::quit(unsigned char privateKey[])
{
	if(privateKey != NULL)
	{
		randombytes_buf(privateKey, crypto_box_SECRETKEYBYTES);
	}
	exit(0);
}

bool Utils::connectFD(int& fd, int type, const std::string& caddr, int cport, struct sockaddr_in* serv_addr)
{
	R* r = R::getInstance();
	Logger* logger = Logger::getInstance();
	std::string typeString = "";
	if(type == SOCK_DGRAM)
	{
		typeString = "SOCK_DGRAM";
	}
	else if (type == SOCK_STREAM)
	{
		typeString = "SOCK_STREAM";
	}
	else
	{
		typeString = "no idea: " + std::to_string(type);
	}

	fd = socket(AF_INET, type, 0);
	if(fd < 0)
	{
		const std::string error = r->getString(R::StringID::CONNECTFD_SOCKET_SYSCALL) + typeString;
		logger->insertLog(Log(Log::TAG::UTILS, error, Log::TYPE::ERROR).toString());
		return false;
	}

	memset(serv_addr, '0', sizeof(struct sockaddr_in));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(cport);
	const int result = inet_pton(AF_INET, caddr.c_str(), &serv_addr->sin_addr);
	if(result < 0)
	{
		const std::string error = r->getString(R::StringID::CONNECTFD_INET_PTON) + typeString;
		logger->insertLog(Log(Log::TAG::UTILS, error, Log::TYPE::ERROR).toString());
		return false;
	}

	const int connectOk = connect(fd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in));
	if(connectOk < 0)
	{
		const std::string error = r->getString(R::StringID::CONNECTFD_CONNECT_SYSCALL) + typeString;
		logger->insertLog(Log(Log::TAG::UTILS, error, Log::TYPE::ERROR).toString());
		return false;
	}
	return true;
}

void Utils::runOnUiThread(GSourceFunc func)
{
	GSource *source = g_idle_source_new();
	g_source_set_callback(source, func, NULL, NULL);
	g_source_attach(source, NULL);
	g_source_unref(source);
}

void Utils::runOnUiThread(GSourceFunc func, gpointer data)
{
	GSource *source = g_idle_source_new();
	g_source_set_callback(source, func, data, NULL);
	g_source_attach(source, NULL);
	g_source_unref(source);
}

void Utils::setupPublicKey(const std::string& title, GtkWindow* window, Log::TAG tag, const std::string& error, std::unique_ptr<unsigned char[]>& output)
{
	const std::string certPath = Utils::fileChooser(title, window);

	if(certPath == "")
	{
		output = std::unique_ptr<unsigned char[]>(); //pointer to nothing
		return;
	}

	const std::string certDump = Utils::dumpSmallFile(certPath);
	const bool ok = SodiumUtils::checkSodiumPublic(certDump);
	if(!ok)
	{
		Logger::getInstance()->insertLog(Log(tag, error, Log::TYPE::ERROR).toString());
		Utils::showPopup(error, window);
		output = std::unique_ptr<unsigned char[]>(); //pointer to nothing
		return;
	}
	const std::string certStringified = certDump.substr(SodiumUtils::SODIUM_PUBLIC_HEADER().length(), crypto_box_PUBLICKEYBYTES*3);
	output = std::make_unique<unsigned char[]>(crypto_box_PUBLICKEYBYTES);
	Stringify::destringify(certStringified, output.get());
}

