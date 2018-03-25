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

#include "arxedit/ui/ArxEditWindow.h"

#include <QFileDialog>

#include "boost/foreach.hpp"

#include "core/Config.h"

#include "core/Application.h"
#include "core/Config.h"

#include "io/fs/SystemPaths.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"
#include "io/log/FileLogger.h"
#include "io/resource/PakReader.h"

#include "arxedit/model/AssetsModel.h"
#include "arxedit/model/ResourcesTreeModel.h"
#include "arxedit/ui/ArxEditWindow.h"
#include "arxedit/ui/ArxViewer.h"

ArxMainWindow::ArxMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	g_resources = new PakReader;
	
	static const char * default_paks[][2] = {
		{ "data.pak", NULL },
		{ "loc.pak", "loc_default.pak" },
		{ "data2.pak", NULL },
		{ "sfx.pak", NULL },
		{ "speech.pak", "speech_default.pak" },
	};
	
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(g_resources->addArchive(fs::findDataFile(default_paks[i][0]))) {
			continue;
		}
		if(default_paks[i][1] && g_resources->addArchive(fs::findDataFile(default_paks[i][1]))) {
			continue;
		}
		std::ostringstream oss;
		oss << "Missing required data file: \"" << default_paks[i][0] << "\"";
		if(default_paks[i][1]) {
			oss << " (or \"" << default_paks[i][1] << "\")";
		}
		LogError << oss.str();
	}
	
	// Load optional patch files
	for(const fs::path & base : fs::getDataDirs()) {
		g_resources->addFiles(base / "editor", "editor");
		g_resources->addFiles(base / "game", "game");
		g_resources->addFiles(base / "graph", "graph");
		g_resources->addFiles(base / "localisation", "localisation");
		g_resources->addFiles(base / "misc", "misc");
		g_resources->addFiles(base / "sfx", "sfx");
		g_resources->addFiles(base / "speech", "speech");
	}
	
	m_assetsModel = new AssetsModel(*g_resources, this);
	ui.assetsTree->setModel(m_assetsModel);
	
	
	ResourcesTreeModel * model = new ResourcesTreeModel(g_resources, this);
	
	ui.treeView->setModel(model);
}

ArxMainWindow::~ArxMainWindow() {

}

void ArxMainWindow::onValidateAll()
{
	
}

void ArxMainWindow::showErrorMessageBox(const QString& text) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Error");
    msgBox.setIconPixmap(QPixmap(":/images/icons/dialog-error.svg"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setText(text);
    msgBox.exec();
}

void ArxMainWindow::on_assetsTree_doubleClicked(const QModelIndex &index) {
	if(!m_modelViewer) {
		m_modelViewer = new ArxViewer(m_assetsModel, this);
	}
	
	m_modelViewer->show();
	
	QString path = m_assetsModel->data(index, AssetsModel::ArxPathRole).toString();
	
	m_modelViewer->loadObject(path);
}
