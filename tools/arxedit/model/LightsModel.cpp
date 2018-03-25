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

#include "arxedit/model/LightsModel.h"

#include "scene/Light.h"


LightsModel::LightsModel(QObject * parent)
: QAbstractTableModel(parent)
{
	init();
}

int LightsModel::rowCount(const QModelIndex & parent) const {
	Q_UNUSED(parent);
	return lights.size();
}

int LightsModel::columnCount(const QModelIndex & parent) const {
	Q_UNUSED(parent);
	return 2;
}

QVariant LightsModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	int row = index.row();
	int column = index.column();
	
	if(row >= int(lights.size()))
		return QVariant();
	
	if(role == Qt::DisplayRole) {
		return QVariant();
		//return lights.at(row).enabled;
	} else if(role == Qt::CheckStateRole) {
		const Entry & light = lights[row];
		switch(column) {
			case 0: return light.enabled ? Qt::Checked : Qt::Unchecked;
			case 1: return light.visualize ? Qt::Checked : Qt::Unchecked;
			default: QVariant();
		}
	}
	
	return QVariant();
}

QVariant LightsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if(role != Qt::DisplayRole)
		return QVariant();
	
	if(orientation == Qt::Horizontal) {
		switch(section) {
			case 0: return "E";
			case 1: return "V";
			default: return QVariant();
		}
	} else {
		return QString("%1").arg(section);
	}
}

bool LightsModel::setData(const QModelIndex & index, const QVariant & value, int role) {
	
	int row = index.row();
	int column = index.column();
	
	if(role == Qt::CheckStateRole) {
		bool v = (value == Qt::Checked);
		Entry & light = lights[row];
		switch (column) {
			case 0: light.enabled = v;   break;
			case 1: light.visualize = v; break;
		}
	}
	
	updateBackingData();
	
	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags LightsModel::flags(const QModelIndex & index) const {
	if(!index.isValid())
		return Qt::ItemIsEnabled;
	
	Qt::ItemFlags baseFlags = QAbstractItemModel::flags(index);
	
	return baseFlags | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}


void LightsModel::addEntry(const EERIE_LIGHT & light) {
	Entry e;
	e.enabled = true;
	e.visualize = false;
	e.light = light;
	lights.push_back(e);
}

void LightsModel::init() {

	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(500.f, 0.f, 0.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(-500.f, 0.f, 0.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(0.f, 500.f, 0.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(0.f, -500.f, 0.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(0.f, 0.f, 500.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	{
		EERIE_LIGHT light = EERIE_LIGHT();
		light.exist = 1;
		light.pos = Vec3f(0.f, 0.f, -500.f);
		light.rgb = Color3f::gray(1.f);
		light.intensity = 10.f;
		light.fallstart = 1000;
		light.fallend = light.fallstart;
		addEntry(light);
	}
	
	updateBackingData();
}

void LightsModel::updateBackingData() {
	// Register the lights in the global datastructures
	int lightsCount = 0;
	for(size_t i = 0; i < lights.size(); i++) {
		if(lights[i].enabled) {
			RecalcLight(&lights[i].light);
			g_culledDynamicLights[lightsCount] = &lights[i].light;
			lightsCount ++;
		}
	}
	g_culledDynamicLightsCount = lightsCount;
}
