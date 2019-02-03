/*
 * PublicKeyUser.hpp
 *
 *  Created on: Jan 27, 2019
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_PUBLICKEYUSER_HPP_
#define SRC_SCREENS_PUBLICKEYUSER_HPP_

#include <string>
#include <memory>
#include <gtk/gtk.h>
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../settings.hpp"
#include "../BlockingQ.hpp"
#include "../vars.hpp"
#include "../stringify.hpp"
#include "../stringify.hpp"
#include "PublicKeyOverview.hpp"

class PublicKeyUser
{
public:
	static void renderNew(const std::string& puser);

	void onclickEdit();
	void onclickRemove();
	void onclickQuit();

	explicit PublicKeyUser(const std::string& puser);
	virtual ~PublicKeyUser();
private:

	static std::unordered_map<std::string, std::unique_ptr<PublicKeyUser>> uwindows;

	const std::string user;

	GtkWindow* window;
	GtkTextView* dump;
	GtkButton* edit;
	GtkButton* remove;
	Settings* settings;
	R* r;
	GtkTextBuffer* dumpBuffer;
};

#endif /* SRC_SCREENS_PUBLICKEYUSER_HPP_ */
