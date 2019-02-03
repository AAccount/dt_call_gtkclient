#include "Log.hpp"


Log::Log(TAG ctag, const std::string& cmessage, TYPE ctype) :
	tag(ctag),
	message(cmessage),
	type(ctype)
{
}

std::string Log::toString() const
{
	std::ostringstream ss;
	ss << *this;
	return ss.str();
}

std::ostream& operator<<(std::ostream &strm, const Log& l)
{
	const time_t now = time(0);
	std::string nowStamp = std::string(ctime(&now));
	nowStamp = nowStamp.substr(0, nowStamp.length()-1);
	return strm << nowStamp << " tag=" << l.tagString()
			<< "; message=" << l.message << "; type=" << l.typeString() << ";";
}

Log::TYPE Log::getType() const
{
	return type;
}

std::string Log::typeString() const
{
	switch(type)
	{
	case INBOUND:
		return "inbound";
	case OUTBOUND:
		return "outbound";
	case ERROR:
		return "error";
	case INFO:
		return "info";
	default:
		return "";
	}
}

std::string Log::tagString() const
{
	switch(tag)
	{
	case MAIN:
		return "main";
	case SODIUM_SOCKET:
		return "sodium socket";
	case CMD_LISTENER:
		return "command listener";
	case CMD_ACCEPT:
		return "accept command";
	case CMD_CALL:
		return "call command";
	case CMD_END:
		return "call end command";
	case HEARTBEAT:
		return "heartbeat";
	case LOGIN:
		return "login";
	case SETTINGS_UI:
		return "settings ui";
	case USER_HOME:
		return "user home";
	case CALL_SCREEN:
		return "call screen";
	case OPUS_CODEC:
		return "opus";
	case UTILS:
		return "utils";
	case PUBLIC_KEY_OV:
		return "public key overview";
	case PUBLIC_KEYU:
		return "public key user";
	default:
		return "(unknown tag): " + std::to_string(tag);
	}
}
