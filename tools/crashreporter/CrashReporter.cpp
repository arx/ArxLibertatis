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

#include "crashreporter/ui/ErrorReportDialog.h"
#include "crashreporter/ErrorReport.h"

#include "io/log/Logger.h"
#include "io/log/FileLogger.h"

#if defined(_WIN32) || defined(_WIN64)

INT WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) {
	
	Q_INIT_RESOURCE(CrashReporter);
	
	QApplication app(__argc, __argv);
	
#else

int main(int argc, char **argv) {
	
	Q_INIT_RESOURCE(CrashReporter);
	
	QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
	QApplication app(argc, argv);
	
#endif
	
	Logger::init();
	
	LogWarning << "Arx Crash Reporter starting!";
	
	QString sharedMemoryName;
	const QStringList args = app.arguments();
	QStringList::const_iterator itArgs;
	for (itArgs = args.constBegin(); itArgs != args.constEnd(); ++itArgs) {
		if((*itArgs).startsWith("-crashinfo=")) {
			QString crashInfo = (*itArgs);
			crashInfo.remove("-crashinfo=");
			sharedMemoryName = crashInfo;
		}
	}
	
	if(sharedMemoryName.isEmpty()) {
		return -1;
	}
	
	ErrorReport errorReport(sharedMemoryName);
	
	ErrorReportDialog errorReportDlg(errorReport);
	errorReportDlg.show();
	
	return app.exec();
}
