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

#ifndef ARX_TOOLS_VIEWER_MODEL_LIGHTSMODEL_H
#define ARX_TOOLS_VIEWER_MODEL_LIGHTSMODEL_H

#include <QAbstractTableModel>
#include <QObject>

#include <vector>

#include "scene/Light.h"

class StageLighting;

class LightsModel : public QAbstractTableModel {
	Q_OBJECT
	
public:
	LightsModel(QObject *parent = 0);
	
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	bool setData(const QModelIndex & index, const QVariant & value, int role);
	
	Qt::ItemFlags flags(const QModelIndex &index) const;
	
	void updateBackingData();
	
	struct Entry {
		bool enabled;
		bool visualize;
		EERIE_LIGHT light;
	};
	
	std::vector<Entry> lights;
	
private:
	void addEntry(const EERIE_LIGHT & light);
	void init();
};

#endif // ARX_TOOLS_VIEWER_MODEL_LIGHTSMODEL_H
