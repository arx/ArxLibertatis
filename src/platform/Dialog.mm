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

#include "platform/Dialog.h"

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

namespace platform {

bool showDialog(DialogType type, const std::string & message, const std::string & title) {
	
	bool result = true;
	NSAlert *alert = [NSAlert new];
	
	[alert setMessageText: [NSString stringWithUTF8String: title.c_str()]];
	[alert setInformativeText: [NSString stringWithUTF8String: message.c_str()]];
	
	switch(type) {
		
		default:
		case DialogInfo:
			[alert setAlertStyle: NSInformationalAlertStyle];
			[alert runModal];
			break;
		
		case DialogWarning:
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert runModal];
			break;
		
		case DialogError:
			[alert setAlertStyle: NSCriticalAlertStyle];
			[alert runModal];
			break;
		
		case DialogWarnYesNo:
			[alert setAlertStyle: NSWarningAlertStyle];
			/* fall-through */
		
		case DialogYesNo:
			[alert addButtonWithTitle: @"Yes"];
			[alert addButtonWithTitle: @"No"];
			switch([alert runModal]) {
				default:
				case NSAlertFirstButtonReturn: result = true; break;
				case NSAlertSecondButtonReturn: result = false; break;
			}
			break;
		
		case DialogOkCancel:
			[alert addButtonWithTitle: @"OK"];
			[alert addButtonWithTitle: @"Cancel"];
			switch([alert runModal]) {
			default:
				case NSAlertFirstButtonReturn: result = true; break;
				case NSAlertSecondButtonReturn: result = false; break;
			}
			break;
	}
	
	[alert release];
	
	return result;
}

} // namespace platform
