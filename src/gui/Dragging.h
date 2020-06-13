/*
 * Copyright 2013-2020 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_DRAGGING_H
#define ARX_GUI_DRAGGING_H

class Entity;
struct InventoryPos;

enum EntityDragStatus {
	EntityDragStatus_OverHud,
	EntityDragStatus_OnGround,
	EntityDragStatus_Drop,
	EntityDragStatus_Throw,
	EntityDragStatus_Invalid
};

extern EntityDragStatus g_dragStatus;
extern Entity * g_draggedEntity;
extern InventoryPos g_draggedItemPreviousPosition;

void setDraggedEntity(Entity * entity);

void updateDraggedEntity();

#endif // ARX_GUI_DRAGGING_H
