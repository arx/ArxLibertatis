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

#include "viewer/model/AssetsModel.h"

#include <QStringList>

#include "io/fs/FilePath.h"
#include "io/log/Logger.h"

AssetItem * AssetItem::createRoot() {
	return new AssetItem();
}

AssetItem * AssetItem::createGroup(AssetItem * parent, QString name) {
	AssetItem * group = new AssetItem(parent);
	group->m_name = name;
	parent->m_childItems.append(group);
	return group;
}

AssetItem *AssetItem::createFile(AssetItem * parent, res::path path) {
	AssetItem * group = new AssetItem(path, parent);
	group->m_name = QString::fromStdString(path.basename());
	parent->m_childItems.append(group);
	return group;
}

AssetItem::AssetItem(AssetItem * parent) {
	m_parentItem = parent;
	m_path = res::path();
}

AssetItem::AssetItem(res::path & path, AssetItem * parent) {
	m_parentItem = parent;
	m_path = path;
}

AssetItem::~AssetItem() {
	qDeleteAll(m_childItems);
}

AssetItem *AssetItem::child(int row) {
	return m_childItems.value(row);
}

int AssetItem::childCount() const {
	return m_childItems.count();
}

int AssetItem::columnCount() const {
	return 1;
}

QVariant AssetItem::data(int column, int role) const {
	
	if(column == 0) {
		if(role == Qt::DisplayRole) {
			return m_name;
		} else if(role == AssetsModel::ArxPathRole) {
			return QString::fromStdString(m_path.string());
		}
	}
	
	return QVariant();
}

AssetItem *AssetItem::parentItem() {
	return m_parentItem;
}

int AssetItem::row() const {
	if(m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<AssetItem*>(this));
	
	return 0;
}


AssetsModel::AssetsModel(PakReader & data, QObject *parent)
	: QAbstractItemModel(parent)
{
	m_rootItem = AssetItem::createRoot();
	
	{
		AssetItem * group = AssetItem::createGroup(m_rootItem, "Objects");
		res::path dirname = res::path("game/graph/obj3d/interactive");
		PakDirectory * dir = data.getDirectory(dirname);
		setupModelData(dirname, *dir, group);
	}
	
	{
		AssetItem * group = AssetItem::createGroup(m_rootItem, "Animations");
		res::path dirname = res::path("graph/obj3d/anims");
		PakDirectory * dir = data.getDirectory(dirname);
		setupModelData(dirname, *dir, group);
	}
}

AssetsModel::~AssetsModel() {
	delete m_rootItem;
}

int AssetsModel::columnCount(const QModelIndex &parent) const {
	
	if(parent.isValid())
		return static_cast<AssetItem*>(parent.internalPointer())->columnCount();
	else
		return 1;
}

QVariant AssetsModel::data(const QModelIndex &index, int role) const {
	
	if(!index.isValid())
		return QVariant();
	
	AssetItem * item = static_cast<AssetItem *>(index.internalPointer());
	
	return item->data(index.column(), role);
}

Qt::ItemFlags AssetsModel::flags(const QModelIndex &index) const {
	
	if(!index.isValid())
		return 0;
	
	return QAbstractItemModel::flags(index);
}

QVariant AssetsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
		return "Name";
	
	return QVariant();
}

QModelIndex AssetsModel::index(int row, int column, const QModelIndex & parent) const {
	
	if(!hasIndex(row, column, parent))
		return QModelIndex();
	
	AssetItem * parentItem;
	
	if(!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<AssetItem *>(parent.internalPointer());
	
	AssetItem * childItem = parentItem->child(row);
	if(childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex AssetsModel::parent(const QModelIndex &index) const {
	if(!index.isValid())
		return QModelIndex();
	
	AssetItem * childItem = static_cast<AssetItem*>(index.internalPointer());
	AssetItem * parentItem = childItem->parentItem();
	
	if(parentItem == m_rootItem)
		return QModelIndex();
	
	return createIndex(parentItem->row(), 0, parentItem);
}

int AssetsModel::rowCount(const QModelIndex &parent) const {
	
	if(parent.column() > 0)
		return 0;
	
	AssetItem *parentItem;
	
	if(!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<AssetItem *>(parent.internalPointer());
	
	return parentItem->childCount();
}

void AssetsModel::setupModelData(res::path path, PakDirectory & dir, AssetItem * parent) {
	
	for(PakDirectory::dirs_iterator i = dir.dirs_begin(); i != dir.dirs_end(); ++i) {
		res::path dirPath = path / i->first;
		AssetItem * dirItem = AssetItem::createGroup(parent, QString(i->first.c_str()));
		setupModelData(dirPath, i->second, dirItem);
	}
	
	for(PakDirectory::files_iterator i = dir.files_begin(); i != dir.files_end(); ++i) {
		res::path filePath = path / i->first;
		AssetItem::createFile(parent, filePath);
	}
}
