/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
	DialogWarnYesNo,
	DialogOkCancel
};

/*!
 * \brief Show an informative message box
 *
 * \param message text of the message to display.
 * \param title   title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is:
 *
 *  * Lines staring with " * " are turned into bulleted lists.
 *
 *  * URLs starting with "http://" or "https://" are turned into links.
 *
 *  * Text surrounded by quotes is made bold.
 *
 *  * Lines starting with "-> " are made italic.
 *
 * This function does not retun until the user closes the dialog.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function retuns immediately.
 */
void showInfoDialog(const std::string & message, const std::string & title = "Information");

/*!
 * \brief Show a warning message box
 *
 * \param message text of the message to display.
 * \param title   title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is. See \ref showInfoDialog for the markup syntax.
 *
 * This function does not retun until the user closes the dialog.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function retuns immediately.
 */
void showWarningDialog(const std::string & message, const std::string & title = "Warning");


/*!
 * \brief Show an error message box
 *
 * \param message text of the message to display.
 * \param title   title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is. See \ref showInfoDialog for the markup syntax.
 *
 * This function does not retun until the user closes the dialog.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function retuns immediately.
 */
void showErrorDialog(const std::string & message, const std::string & title = "Error");

/*!
 * \brief Show a dialog asking a Yes/No question
 *
 * \param question text of the question to display.
 * \param title    title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is. See \ref showInfoDialog for the markup syntax.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function immediately returns \c true.
 *
 * \return \c true if the user clicked "Yes", or \c false if the user clicked "No" or
 *         closed the dialog.
 */
bool askYesNo(const std::string & question, const std::string & title = "Yes/No?");

/*!
 * \brief Show an error dialog asking a Yes/No question
 *
 * \param question text of the question to display.
 * \param title    title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is. See \ref showInfoDialog for the markup syntax.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function immediately returns \c true.
 *
 * \return \c true if the user clicked "Yes", or \c false if the user clicked "No" or
 *         closed the dialog.
 */
bool askYesNoWarning(const std::string & question, const std::string & title = "Warning");

/*!
 * \brief Show a dialog asking a Ok/Cancel question
 *
 * \param question text of the question to display.
 * \param title    title for the dialog window.
 *
 * A simple markup syntax is supported on some systems. On other systems, the message is
 * displayed as-is. See \ref showInfoDialog for the markup syntax.
 *
 * \note this may not be supported on all platforms. If dialogs are not supported, the
 *       message is printed to stdout and the function immediately returns \c true.
 *
 * \return \c true if the user clicked "Ok", or \c false if the user clicked "Cancel" or
 *         closed the dialog.
 */
bool askOkCancel(const std::string & question, const std::string & title = "Ok/Cancel?");

} // namespace platform

#endif // ARX_PLATFORM_DIALOG_H
