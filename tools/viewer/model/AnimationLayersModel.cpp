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

#include "AnimationLayersModel.h"

AnimationLayersModel::AnimationLayersModel(QObject * parent)
	: QAbstractListModel(parent)
{
}

int AnimationLayersModel::rowCount(const QModelIndex & parent) const {
	Q_UNUSED(parent);
	return MAX_ANIM_LAYERS;
}

QVariant AnimationLayersModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	int row = index.row();
	
	if(row >= int(MAX_ANIM_LAYERS))
		return QVariant();
	
	if(role == Qt::DisplayRole) {
		const AnimLayer & layer = m_currentAnimation[row];
		ANIM_HANDLE * anim = layer.cur_anim;
		if(anim) {
			return QString::fromStdString(anim->path.filename());
		} else {
			return "Empty";
		}
	} else {
		return QVariant();
	}
}

QVariant AnimationLayersModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if(role != Qt::DisplayRole)
		return QVariant();
	
	if(orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}

Qt::ItemFlags AnimationLayersModel::flags(const QModelIndex & index) const {
	if(!index.isValid())
		return Qt::ItemIsEnabled;
	
	return QAbstractItemModel::flags(index);
}

void AnimationLayersModel::setAnimation(const QModelIndex & index, QString path) {
	
	if(!index.isValid()) {
		return;
	}
	
	int row = index.row();
	
	if(!path.isEmpty()) {
		ANIM_HANDLE * anim = EERIE_ANIMMANAGER_Load(path.toStdString());
		
		AnimLayer & layer = m_currentAnimation[row];
		ANIM_Set(layer, anim);
		layer.flags |= EA_LOOP;
	} else {
		AnimLayer & layer = m_currentAnimation[row];
		layer.cur_anim = NULL;
	}
	
	
	QModelIndex idx = QAbstractListModel::index(row);
	
	emit dataChanged(idx, idx);
}
