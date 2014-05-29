/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_PLATFORM_DIALOG_H
#define ARX_PLATFORM_DIALOG_H

#include <string>

namespace platform {

enum DialogType {
	DialogInfo,
	DialogWarning,
	DialogError,
	DialogYesNo,
	DialogOkCancel
};

void showInfoDialog(const std::string & message, const std::string & title = "Information");
void showWarningDialog(const std::string & message, const std::string & title = "Warning");
void showErrorDialog(const std::string & message, const std::string & title = "Error");

bool askYesNo(const std::string & question, const std::string & title = "Yes/No?");
bool askOkCancel(const std::string & question, const std::string & title = "Ok/Cancel?");

} // namespace platform

#endif // ARX_PLATFORM_DIALOG_H
