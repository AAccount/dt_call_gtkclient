/*
 * PublicKeyOverview.hpp
 *
 *  Created on: Jan 26, 2019
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_PUBLICKEYOVERVIEW_HPP_
#define SRC_SCREENS_PUBLICKEYOVERVIEW_HPP_
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <gtk/gtk.h>
#include "PublicKeyUser.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../settings.hpp"
#include "../BlockingQ.hpp"
#include "../vars.hpp"
#include "../background/AsyncReceiver.hpp"
#include "../background/AsyncCentral.hpp"

class PublicKeyOverview : public virtual AsyncReceiver
{
public:
	static int render(void* a);
	static int remove(void* a);
	static PublicKeyOverview* getInstance();
	void asyncResult(int result, std::string& info) override;
	void onclick(GtkButton* button);

private:
	static PublicKeyOverview* instance;

	PublicKeyOverview();
	virtual ~PublicKeyOverview();
	static bool onScreen;

	std::string getButtonDisplay(const std::string& user);
	static int updateButton(void* context);

	std::unordered_set<std::string> users;
	std::unordered_map<GtkButton*, std::string> buttonToUser;
	std::unordered_map<std::string, GtkButton*> userToButton;

	Logger* logger;
	R* r;
	Settings* settings;

	GtkWindow* window;
	GtkBox* userList;
};

#endif /* SRC_SCREENS_PUBLICKEYOVERVIEW_HPP_ */
