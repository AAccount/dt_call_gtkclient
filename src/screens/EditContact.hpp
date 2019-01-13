/*
 * EditContact.hpp
 *
 *  Created on: Jan 12, 2019
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_EDITCONTACT_HPP_
#define SRC_SCREENS_EDITCONTACT_HPP_

#include <string>
#include <gtk/gtk.h>
#include "UserHome.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../settings.hpp"

class EditContact
{
public:
	static int render(void* a);
	static int remove(void* a);
	static EditContact* getInstance();
	static std::string contactInEdit;

	void onclickSave();

private:

	static EditContact* instance;
	EditContact();
	virtual ~EditContact();
	static bool onScreen;

	GtkWindow* window;
	GtkEntry* entry;
	GtkButton* ok;
	Settings* settings;
	R* r;
};

#endif /* SRC_SCREENS_EDITCONTACT_HPP_ */
