#ifndef DBLOG_H
#define DBLOG_H

#include <string>
#include <iostream>
#include <sstream>

class Log
{
public:
	typedef enum {INBOUND, OUTBOUND, ERROR, INFO} TYPE;
	typedef enum
	{
		MAIN,
		SODIUM_SOCKET,
		SETTINGS,
		CMD_LISTENER,
		CMD_ACCEPT,
		CMD_CALL,
		CMD_END,
		HEARTBEAT,
		LOGIN,
		SETTINGS_UI,
		USER_HOME,
		CALL_SCREEN,
		OPUS_CODEC,
		UTILS,
		PUBLIC_KEY_OV,
		PUBLIC_KEYU
	} TAG;

	TYPE getType() const;
	Log(TAG ctag, const std::string& cmessage, TYPE type);
	std::string toString() const;

private:
	const TAG tag;
	const std::string message;
	const TYPE type;
	friend std::ostream& operator<<(std::ostream &strm, const Log&);

	std::string typeString() const;
	std::string tagString() const;
};

#endif //DBLOG_H
