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

#include "arxedit/model/ResourcesTreeModel.h"

#include <QStringList>

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent) {
	m_parentItem = parent;
	m_itemData = data;
}

TreeItem::~TreeItem() {
	qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem *item) {
	m_childItems.append(item);
}

TreeItem *TreeItem::child(int row) {
	return m_childItems.value(row);
}

int TreeItem::childCount() const {
	return m_childItems.count();
}

int TreeItem::columnCount() const {
	return m_itemData.count();
}

QVariant TreeItem::data(int column) const {
	return m_itemData.value(column);
}

TreeItem *TreeItem::parentItem() {
	return m_parentItem;
}

int TreeItem::row() const {
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
	
	return 0;
}

// ============================================================================
// ============================================================================
// ============================================================================

#include <QStringList>

ResourcesTreeModel::ResourcesTreeModel(PakReader * resources, QObject *parent)
	: QAbstractItemModel(parent)
{
	m_resources = resources;
	
	QList<QVariant> rootData;
	rootData << "Title" << "Summary";
	rootItem = new TreeItem(rootData);
	
	res::path p = res::path();
	
	setupModelData(p, m_resources, rootItem);
	
	//setupModelData(resources.split(QString("\n")), rootItem);
}

ResourcesTreeModel::~ResourcesTreeModel() {
	delete rootItem;
}

int ResourcesTreeModel::columnCount(const QModelIndex &parent) const {
	if(parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant ResourcesTreeModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();
	
	if (role != Qt::DisplayRole)
		return QVariant();
	
	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	
	return item->data(index.column());
}

Qt::ItemFlags ResourcesTreeModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return 0;
	
	return QAbstractItemModel::flags(index);
}

QVariant ResourcesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);
	
	return QVariant();
}

QModelIndex ResourcesTreeModel::index(int row, int column, const QModelIndex &parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex();
	
	TreeItem *parentItem;
	
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());
	
	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex ResourcesTreeModel::parent(const QModelIndex &index) const {
	if (!index.isValid())
		return QModelIndex();
	
	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parentItem();
	
	if (parentItem == rootItem)
		return QModelIndex();
	
	return createIndex(parentItem->row(), 0, parentItem);
}

int ResourcesTreeModel::rowCount(const QModelIndex &parent) const {
	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;
	
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());
	
	return parentItem->childCount();
}

void ResourcesTreeModel::setupModelData(res::path & dirPath, PakDirectory * dir, TreeItem *parent) {

	
	for(PakDirectory::files_iterator i = dir->files_begin(); i != dir->files_end(); ++i) {
		res::path filename = dirPath / i->first;
		PakFile * file = i->second;
		
		QList<QVariant> columnData;
		columnData << QString::fromStdString(i->first);
		
		parent->appendChild(new TreeItem(columnData, parent));
		
	}
	
	for(PakDirectory::dirs_iterator i = dir->dirs_begin(); i != dir->dirs_end(); ++i) {
		res::path dirname = dirPath / i->first;
		
		QList<QVariant> columnData;
		columnData << QString::fromStdString(i->first);
		
		TreeItem * itm = new TreeItem(columnData, parent);
		parent->appendChild(itm);
		
		setupModelData(dirname, &i->second, itm);
	}
}
