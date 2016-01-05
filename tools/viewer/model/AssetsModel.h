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

#ifndef ARX_TOOLS_VIEWER_MODEL_ASSETSMODEL_H
#define ARX_TOOLS_VIEWER_MODEL_ASSETSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QVariant>

#include "io/resource/PakReader.h"

class AssetItem {
public:
	static AssetItem * createRoot();
	static AssetItem * createGroup(AssetItem * parent, QString name);
	static AssetItem * createFile(AssetItem * parent, res::path path);
	
	~AssetItem();
	
	AssetItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column, int role) const;
	int row() const;
	AssetItem *parentItem();
	
private:
	AssetItem(AssetItem * parent = 0);
	AssetItem(res::path & path, AssetItem *parent = 0);
	
	AssetItem * m_parentItem;
	QList<AssetItem *> m_childItems;
	QString m_name;
	res::path m_path;
};

class AssetsModel : public QAbstractItemModel {
	Q_OBJECT
	
public:
	enum MyRules {
		ArxPathRole = Qt::UserRole
	};
	
	explicit AssetsModel(PakReader & data, QObject *parent = 0);
	~AssetsModel();
	
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	
private:
	void setupModelData(res::path path, PakDirectory & dir, AssetItem *parent);
	
	AssetItem *m_rootItem;
};

#endif // ARX_TOOLS_VIEWER_MODEL_ASSETSMODEL_H
