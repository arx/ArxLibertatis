/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "profiler/ui/ArxProfiler.h"

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

INT WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) {
	
	QApplication app(__argc, __argv);
	
#else

int main(int argc, char **argv) {
	
	#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	#endif
	
	QApplication app(argc, argv);
	
#endif

	ArxProfiler w;
	w.show();

	return app.exec();
}
