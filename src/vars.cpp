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
std::unique_ptr<unsigned char[]> Vars::serverCert = std::make_unique<unsigned char[]>(0);
std::string Vars::username = "";
std::unique_ptr<unsigned char[]> Vars::privateKey = std::make_unique<unsigned char[]>(0);

SodiumSocket Vars::commandSocket;
std::string Vars::sessionKey = "";

StringRes::Language Vars::lang = StringRes::Language::EN;

Vars::Vars()
{
	// TODO Auto-generated constructor stub
}

Vars::~Vars()
{
	// TODO Auto-generated destructor stub
}

