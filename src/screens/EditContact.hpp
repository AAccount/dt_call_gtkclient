/*
 * EditContact.hpp
 *
 *  Created on: Jan 12, 2019
 *      Author: Daniel
 */

#ifndef SRC_SCREENS_EDITCONTACT_HPP_
#define SRC_SCREENS_EDITCONTACT_HPP_

#include <string>
#include <memory>
#include <gtk/gtk.h>
#include "UserHome.hpp"
#include "../R.hpp"
#include "../Log.hpp"
#include "../Logger.hpp"
#include "../settings.hpp"
#include "../BlockingQ.hpp"

class EditContact
{
public:

	static BlockingQ<std::string> editedContacts;
	static void renderNew(const std::string& toEdit);

	void onclickSave();
	void onclickQuit();

	//required public for unique_ptr
	EditContact(const std::string& toEdit);
	virtual ~EditContact();

private:
	static std::unordered_map<std::string, std::unique_ptr<EditContact>> editWindows;

	const std::string contactInEdit;
	GtkWindow* window;
	GtkEntry* entry;
	GtkButton* ok;
	Settings* settings;
	R* r;
};

#endif /* SRC_SCREENS_EDITCONTACT_HPP_ */
