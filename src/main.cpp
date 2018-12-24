/*
 * main.cpp
 *
 *  Created on: Dec 11, 2017
 *      Author: Daniel
 */
//https://prognotes.net/2015/07/gtk-3-glade-c-programming-template/
//https://prognotes.net/2015/06/gtk-3-c-program-using-glade-3/
//https://prognotes.net/2016/03/gtk-3-c-code-hello-world-tutorial-using-glade-3/
#include "main.hpp"

int main(int argc, char *argv[])
{
//	if(argc == 3)
//	{
//		const std::string languageFlag(argv[1]);
//		const std::string language(argv[2]);
//
//		if(languageFlag == "-l" || languageFlag == "-language")
//		{
//			if(language == "other")
//			{
//				Vars::lang = StringRes::Language::other_
//			}
//		}
//	}

	gtk_init(&argc, &argv);
	InitialSetup::render();
	gtk_main();

	return 0;
}
