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
#ifndef ARX_TOOLS_ARXEDIT_MODEL_RESOURCESTREEMODEL_H
#define ARX_TOOLS_ARXEDIT_MODEL_RESOURCESTREEMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QVariant>

#include "io/resource/PakReader.h"

class TreeItem {
public:
	explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem = 0);
	~TreeItem();
	
	void appendChild(TreeItem *child);
	
	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeItem *parentItem();
private:
	QList<TreeItem*> m_childItems;
	QList<QVariant> m_itemData;
	TreeItem *m_parentItem;
};

class ResourcesTreeModel : public QAbstractItemModel {
	Q_OBJECT
	
public:
	explicit ResourcesTreeModel(PakReader * resources, QObject *parent = 0);
	~ResourcesTreeModel();
	
	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	
private:
	PakReader * m_resources;
	
	void setupModelData(res::path & dirPath, PakDirectory * dir, TreeItem *parent);
	
	TreeItem *rootItem;
};

#endif // ARX_TOOLS_ARXEDIT_MODEL_RESOURCESTREEMODEL_H
