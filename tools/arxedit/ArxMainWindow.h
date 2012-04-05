/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TOOLS_ARXEDIT_ARXMAINWINDOW_H
#define ARX_TOOLS_ARXEDIT_ARXMAINWINDOW_H

// Qt
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtCore/QTimer>

#include "ui_ArxMainWindow.h"

#include "arxedit/RenderWidget.h"
#include "arxedit/ArxLevel.h"

class ArxMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ArxMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~ArxMainWindow();

	Ogre::RenderWindow* getRenderWindow() const;
	EventHandlingRenderWidget* getRenderWidget() const;

	void initScene();

public slots:
    void shutdown();
    void update();	
	
	void onFileOpenLevel();
	void onFileNewLevel();

    void showErrorMessageBox(const QString& text);

private:
	void initOgre();

	void loadScene();
	void loadLights();
	

private:
    Ui::ArxMainWindowClass ui;

    QTimer mAutoUpdateTimer;

    Ogre::RenderSystem* mRenderSystem;
    Ogre::Root* mRoot;
	Ogre::SceneManager* mSceneManager;

    EventHandlingRenderWidget* mRenderWidget;

	Arx::Level* mLevel;
};

#endif // ARX_TOOLS_ARXEDIT_ARXMAINWINDOW_H
