/*
 * vars.cpp
 *
 *  Created on: Dec 24, 2017
 *      Author: Daniel
 */

#include "vars.hpp"

std::string Vars::serverAddress = "";
int Vars::commandPort = 0;
int Vars::mediaPort = 0;
std::unique_ptr<unsigned char[]> Vars::serverCert = std::unique_ptr<unsigned char[]>();
std::string Vars::username = "";
std::unique_ptr<unsigned char[]> Vars::privateKey = std::unique_ptr<unsigned char[]>();

Vars::UserState Vars::ustate = NONE;

SodiumSocket Vars::commandSocket;
int Vars::mediaSocket = -1;
std::string Vars::sessionKey = "";
std::string Vars::callWith = "";
std::unique_ptr<unsigned char[]> Vars::voiceKey = std::unique_ptr<unsigned char[]>();

Vars::Vars()
{
}

Vars::~Vars()
{
}

