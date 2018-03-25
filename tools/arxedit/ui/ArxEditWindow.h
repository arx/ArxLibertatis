/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TOOLS_ARXEDIT_UI_ARXMAINWINDOW_H
#define ARX_TOOLS_ARXEDIT_UI_ARXMAINWINDOW_H

#include <QMainWindow>

class AssetsModel;
class ArxViewer;

namespace Ui {
class ArxMainWindowClass;
}

class ArxMainWindow : public QMainWindow {
	Q_OBJECT
	
public:
	ArxMainWindow(QWidget *parent = 0);
	~ArxMainWindow();
	
private slots:
	void onValidateAll();
	void on_assetsTree_doubleClicked(const QModelIndex &index);
	
private:
	void loadScene();
	void loadLights();
	
private:
	Ui::ArxMainWindowClass * ui;
	AssetsModel * m_assetsModel = nullptr;
	ArxViewer * m_modelViewer = nullptr;
};

#endif // ARX_TOOLS_ARXEDIT_UI_ARXMAINWINDOW_H
