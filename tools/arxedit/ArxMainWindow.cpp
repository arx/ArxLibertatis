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

#include "arxedit/ArxMainWindow.h"

#include <QFileDialog>

#include "arxedit/CameraManipulator.h"
#include "arxedit/ViewportGrid.h"

#include "core/Config.h"

ArxMainWindow::ArxMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags) {

	ui.setupUi(this);
	    
    QObject::connect(&mAutoUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
    
    //On the test system, a value of one here gives a high frame rate and still allows
    //event processing to take place. A value of 0 doubles the frame rate but the mouse
    //becomes jumpy. This property is configerable via setAutoUpdateInterval().
    mAutoUpdateTimer.setInterval(1);
	mAutoUpdateTimer.start();

	mRenderWidget = new EventHandlingRenderWidget();
	mRenderWidget->setSizePolicy(QSizePolicy::Expanding , QSizePolicy::Expanding );
	setCentralWidget(mRenderWidget);

	initOgre();
}

ArxMainWindow::~ArxMainWindow() {

}

Ogre::RenderWindow* ArxMainWindow::getRenderWindow() const {
	return mRenderWidget->getOgreRenderWindow();
}

EventHandlingRenderWidget* ArxMainWindow::getRenderWidget() const {
	return mRenderWidget;
}

void ArxMainWindow::initOgre() {
	try {
		mRoot = new Ogre::Root("plugins.cfg");
		mRenderSystem = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");

		Ogre::Root::getSingletonPtr()->setRenderSystem(mRenderSystem);
		Ogre::Root::getSingletonPtr()->initialise(false);

		Ogre::NameValuePairList ogreWindowParams;
		mRenderWidget->initialise(&ogreWindowParams);
    } catch(Ogre::Exception& e) {
        QString error
            (
            "Failed to create the Ogre::Root object. This is a fatal error and the "
            "application will now exit. There are a few known reasons why this can occur:\n\n"
            "    1) Ensure your plugins.cfg has the correct path to the plugins.\n"
            "    2) In plugins.cfg, use Unix style directory separators. I.e '/' rather than '\\'.\n"
            "    3) If your plugins.cfg is trying to load the Direct3D plugin, make sure you have DirectX installed on your machine.\n\n"
            "The message returned by Ogre was:\n\n"
            );
        error += QString::fromStdString(e.getFullDescription().c_str());

        qCritical("%s", error.toAscii().constData());
        showErrorMessageBox(error);

        //Not much else we can do here...
        std::exit(1);
    }
}

void ArxMainWindow::initScene() {

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(config.paths.data.string().c_str(), "FileSystem"); //place for textures

	mSceneManager = Ogre::Root::getSingletonPtr()->createSceneManager(Ogre::ST_GENERIC, "default");
	Ogre::RenderWindow* pRenderWindow = mRenderWidget->getOgreRenderWindow();
	Ogre::Camera* pCamera = mSceneManager->createCamera("main");
	
	Ogre::Viewport* pViewport = pRenderWindow->addViewport(pCamera);
	CameraManipulator* cameraMan = new CameraManipulator(pCamera);
	mRenderWidget->setEventHandler(cameraMan);
	Ogre::Root::getSingletonPtr()->addFrameListener(cameraMan);

	// Match arx settings (more or less)
	const float PLAYER_SPEED = 200;
	cameraMan->setTopSpeed(PLAYER_SPEED);
	pCamera->setFOVy(Ogre::Degree(70.0f));

	Ogre::ViewportGrid* grid = new Ogre::ViewportGrid(mSceneManager, pViewport);
	grid->setRenderMiniAxes();
	grid->setRenderScale();
	grid->setPerspectiveSize(30000);
	grid->enable();
}

void ArxMainWindow::onFileOpenLevel() {
	/*loadArxLevel(pSceneManager);

	// TODO - Use spawn info from level file
	pCamera->setPosition(Ogre::Vector3(0,10,500));
    pCamera->lookAt(Ogre::Vector3(0,0,0));*/

	fs::path levelDir = config.paths.data / "graph" / "levels";
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Level"), QString::fromStdString(levelDir.string()) , tr("Level File (*.dlf)"));

	if(!fileName.isEmpty())
	{
		mLevel = new Arx::Level(mSceneManager);
		mLevel->load(fileName.toAscii().constData());
	}
}
void ArxMainWindow::onFileNewLevel() {
}

void ArxMainWindow::update() {
    mRenderWidget->update();
}

void ArxMainWindow::shutdown() {
    mAutoUpdateTimer.stop();
}

void ArxMainWindow::showErrorMessageBox(const QString& text) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Error");
    msgBox.setIconPixmap(QPixmap(":/images/icons/dialog-error.svg"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setText(text);
    msgBox.exec();
}

void ArxMainWindow::loadScene() {
	


}