/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include <QApplication>

#include <string>

#include "ui/errorreportdialog.h"
#include "errorreport.h"

#include "io/log/Logger.h"
#include "io/log/FileLogger.h"

#if defined(_WIN32) || defined(_WIN64)

INT WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) {
	
	Q_INIT_RESOURCE(crashreporter);
	
	QApplication app(__argc, __argv);
	
#else

int main(int argc, char **argv) {
	
	Q_INIT_RESOURCE(crashreporter);
	
	QApplication app(argc, argv);
	
#endif

	Logger::init();
	Logger::add(new logger::File("arxcrashreporter.log", std::ios_base::out | std::ios_base::trunc));

	std::string sharedMemoryName;
	const QStringList args = app.arguments();
	QStringList::const_iterator itArgs;
	for (itArgs = args.constBegin(); itArgs != args.constEnd(); ++itArgs) {
		if((*itArgs).startsWith("-crashinfo=")) {
			QString crashInfo = (*itArgs);
			crashInfo.remove("-crashinfo=");
			sharedMemoryName = crashInfo.toAscii().data();
		}
	}
	
	if(sharedMemoryName.empty()) {
		return -1;
	}

	ErrorReport errorReport(sharedMemoryName);

	ErrorReportDialog errorReportDlg(errorReport);
	errorReportDlg.show();
	
	return app.exec();
}
