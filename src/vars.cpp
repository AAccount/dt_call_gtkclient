/*
 * vars.cpp
 *
 *  Created on: Dec 24, 2017
 *      Author: Daniel
 */

#include "vars.hpp"

struct sockaddr_in Vars::mediaPortAddrIn;
std::string Vars::serverAddress = "";
int Vars::commandPort = 0;
int Vars::mediaPort = 0;
std::unique_ptr<unsigned char[]> Vars::serverCert;
std::string Vars::username = "";
std::unique_ptr<unsigned char[]> Vars::privateKey;

Vars::UserState Vars::ustate = NONE;

std::unique_ptr<SodiumSocket> Vars::commandSocket;
int Vars::mediaSocket = -1;
std::string Vars::sessionKey = "";
std::string Vars::callWith = "";
std::unique_ptr<unsigned char[]> Vars::voiceKey;

std::atomic<bool> Vars::isExiting(false);
bool Vars::callEndIntentForCallScreen = false;
Vars::Vars()
{
}

Vars::~Vars()
{
}

