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

#ifndef ARX_TOOLS_VIEWER_MODEL_ANIMATIONLAYERSMODEL_H
#define ARX_TOOLS_VIEWER_MODEL_ANIMATIONLAYERSMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QStringList>

#include "animation/Animation.h"
#include "game/Entity.h"

class AnimationLayersModel : public QAbstractListModel {
	Q_OBJECT
	
public:
	AnimationLayersModel(QObject *parent = 0);
	
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	Qt::ItemFlags flags(const QModelIndex &index) const;
	
	void setAnimation(const QModelIndex &index, QString path);
	
	AnimLayer m_currentAnimation[MAX_ANIM_LAYERS];
	
private:
	
};

#endif // ARX_TOOLS_VIEWER_MODEL_ANIMATIONLAYERSMODEL_H
