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

#include <QApplication>

#include <string>
#include <cstdlib>

#include "crashreporter/ui/ErrorReportDialog.h"
#include "crashreporter/ErrorReport.h"

#include "core/Version.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"

int main(int argc, char * argv[]) {
	
	Q_INIT_RESOURCE(CrashReporter);
	
	#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	#endif
	
	QApplication app(argc, argv);
	
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_PLATFORM != ARX_PLATFORM_MACOS
	QIcon icon = QIcon::fromTheme(arx_icon_name.c_str(), QIcon::fromTheme("dialog-error"));
	app.setWindowIcon(icon);
	#endif
	
	Logger::initialize();
	
	LogWarning << arx_name + " Crash Reporter starting!";
	
	QString sharedMemoryName;
	const QStringList args = app.arguments();
	QStringList::const_iterator itArgs;
	for (itArgs = args.constBegin(); itArgs != args.constEnd(); ++itArgs) {
		if((*itArgs).startsWith("--crashinfo=")) {
			QString crashInfo = (*itArgs);
			crashInfo.remove("--crashinfo=");
			sharedMemoryName = crashInfo;
		}
	}
	
	if(sharedMemoryName.isEmpty()) {
		LogError << "Missing --crashinfo parameter!";
		return EXIT_FAILURE;
	}
	
	ErrorReport errorReport(sharedMemoryName);
	
	ErrorReportDialog errorReportDlg(errorReport);
	errorReportDlg.show();
	
	return app.exec();
}
