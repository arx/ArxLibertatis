
#ifndef ARX_GAME_ITEM_H
#define ARX_GAME_ITEM_H

#include "game/Entity.h"

struct IO_EQUIPITEM_ELEMENT {
	float value;
	short flags;
	short special; // TODO unused?
};

#define IO_EQUIPITEM_ELEMENT_Number 29

struct IO_EQUIPITEM {
	IO_EQUIPITEM_ELEMENT elements[IO_EQUIPITEM_ELEMENT_Number];
};

struct IO_ITEMDATA {
	IO_EQUIPITEM * equipitem; // Equipitem Datas
	long price;
	short maxcount; // max number cumulable
	short count; // current number
	char food_value;
	char stealvalue;
	short playerstacksize;
	short LightValue;
};

#endif // ARX_GAME_ITEM_H
