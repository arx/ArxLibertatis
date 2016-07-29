/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "viewer/ui/ArxViewer.h"
#include "ui_ArxViewer.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QtDebug>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QLabel>
#include <QString>

#include <limits>

#include "game/effect/ParticleSystems.h"

#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"

#include "viewer/model/AnimationLayersModel.h"
#include "viewer/model/AssetsModel.h"
#include "viewer/model/LightsModel.h"

#include "viewer/ui/ArxRenderWidget.h"

const char * ParticleParamToStr(ParticleParam pp) {
	switch(pp) {
		case ParticleParam_MagicMissileExplosion:    return "MagicMissileExplosion";
		case ParticleParam_MagicMissileExplosionMar: return "MagicMissileExplosionMar";
		case ParticleParam_Heal: return "Heal";
		case ParticleParam_CreateFood:return "CreateFood";
		case ParticleParam_CurePoison:return "CurePoison";
		case ParticleParam_FireFieldBase:return "FireFieldBase";
		case ParticleParam_FireFieldFlame:return "FireFieldFlame";
		case ParticleParam_Poison1:return "Poison1";
		case ParticleParam_Poison2:return "Poison2";
		case ParticleParam_Poison3:return "Poison3";
	}
}

ArxViewer::ArxViewer(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow(parent, flags)
, ui(new Ui::ArxViewerClass)
{
	ui->setupUi(this);
	
	// Fix missing menu bar in Ubuntu
	ui->menuBar->setNativeMenuBar(false);
	
	setAcceptDrops(true);
	
	m_dataPath = QDir(QCoreApplication::arguments().at(1));
	
	resources = new PakReader;
	resources->addFiles(fs::path(m_dataPath.path().toStdString()));
	
	m_assetsModel = new AssetsModel(*resources, this);
	ui->assetsTree->setModel(m_assetsModel);
	
	m_animationLayersModel = new AnimationLayersModel(this);
	ui->animationLayersList->setModel(m_animationLayersModel);
	
	m_lightsModel = new LightsModel(this);
	ui->lightsTable->setModel(m_lightsModel);
	
	m_renderWidget = new ArxRenderWidget(m_animationLayersModel, m_lightsModel, this);
	ui->verticalLayout->addWidget(m_renderWidget);
	
	
	ui->particlesComboBox->insertItem(0, "NONE", -1);
	for(int i=0;i<ParticleParam_MAX; i++) {
		QString name = QString(ParticleParamToStr((ParticleParam)i));
		ui->particlesComboBox->insertItem(0, name, i);
	}
}

ArxViewer::~ArxViewer() {
	delete ui;
}

void ArxViewer::dragEnterEvent(QDragEnterEvent * event) {
	event->acceptProposedAction();
}

void ArxViewer::dropEvent(QDropEvent * event) {
	
	const QMimeData * mdata = event->mimeData();
	if(mdata->hasUrls()) {
		foreach(QUrl url, mdata->urls()) {
			QString path = url.toLocalFile();
			
			QString relPath = m_dataPath.relativeFilePath(path);
			
			// TODO copy-paste
			if(path.contains(".ftl")) {
				m_renderWidget->openFile2(relPath);
			}
			
			if(path.contains(".tea")) {
				
				QModelIndexList lst = ui->animationLayersList->selectionModel()->selectedIndexes();
				if(!lst.empty()) {
					m_animationLayersModel->setAnimation(lst.first(), relPath);
				}
			}
		}
	}
}

void ArxViewer::on_assetsTree_doubleClicked(const QModelIndex & index) {
	
	QString path = m_assetsModel->data(index, AssetsModel::ArxPathRole).toString();
	
	if(path.contains(".ftl")) {
		m_renderWidget->openFile2(path);
	}
	
	if(path.contains(".tea")) {
		
		QModelIndexList lst = ui->animationLayersList->selectionModel()->selectedIndexes();
		if(!lst.empty()) {
			m_animationLayersModel->setAnimation(lst.first(), path);
		}
	}
}

void ArxViewer::on_pushButton_clicked() {
	
	QModelIndexList lst = ui->animationLayersList->selectionModel()->selectedIndexes();
	if(!lst.empty()) {
		m_animationLayersModel->setAnimation(lst.first(), "");
	}
}

void ArxViewer::on_actionShow_Vertex_Normals_toggled(bool arg1) {
	m_renderWidget->m_showVertexNormals = arg1;
}

void ArxViewer::on_actionShow_Face_Normals_toggled(bool arg1) {
	m_renderWidget->m_showFaceNormals = arg1;
}

void ArxViewer::on_pushButton_2_toggled(bool checked) {
    m_renderWidget->m_play = checked;
}

void ArxViewer::on_horizontalSlider_2_valueChanged(int value) {
	float speed = value / 100.f;
	ui->animationSpeedLabel->setText(QString::number(speed, 'f', 2));
    m_renderWidget->m_playbackSpeed = speed;
}

void ArxViewer::on_debug1CheckBox_toggled(bool checked) {
	extern bool g_debugToggles[10];
	g_debugToggles[0] = checked;
}

void ArxViewer::on_particlesComboBox_currentIndexChanged(int index)
{
	QVariant data = ui->particlesComboBox->itemData(index);
	m_renderWidget->particleSystemLoad(data.toInt());
}
void ArxViewer::on_pushButton_3_clicked()
{
    m_renderWidget->particleSystemReset();
}


