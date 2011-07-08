
#ifndef ARX_GAME_INVENTORY_H
#define ARX_GAME_INVENTORY_H

#include "math/MathFwd.h"
#include "scripting/Script.h"

struct INTERACTIVE_OBJ;

struct INVENTORY_SLOT {
	INTERACTIVE_OBJ * io;
	long show;
};

struct INVENTORY_DATA {
	INTERACTIVE_OBJ * io;
	long sizex;
	long sizey;
	INVENTORY_SLOT slot[20][20];
};

const size_t INVENTORY_X = 16;
const size_t INVENTORY_Y = 3;

extern INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];
extern INVENTORY_DATA * SecondaryInventory;
extern INVENTORY_DATA * TSecondaryInventory;
extern INTERACTIVE_OBJ * DRAGINTER;
extern INTERACTIVE_OBJ * ioSteal;
extern long InventoryY;

void PutInFrontOfPlayer(INTERACTIVE_OBJ * io);
bool CanBePutInInventory(INTERACTIVE_OBJ * io);

bool GetItemWorldPosition(INTERACTIVE_OBJ * io, Vec3f * pos);
bool GetItemWorldPositionSound(const INTERACTIVE_OBJ * io, Vec3f * pos);

INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(Vec2s * pos);
void CheckForInventoryReplaceMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * old);

bool InSecondaryInventoryPos(Vec2s * pos);
bool InPlayerInventoryPos(Vec2s * pos);
bool CanBePutInSecondaryInventory(INVENTORY_DATA * id, INTERACTIVE_OBJ * io, long * xx, long * yy);

void CleanInventory();
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand);
bool PutInInventory();
bool TakeFromInventory(Vec2s * pos);
INTERACTIVE_OBJ * GetFromInventory(Vec2s * pos);
bool IsFlyingOverInventory(Vec2s * pos);
void ForcePlayerInventoryObjectLevel(long level);
bool IsInPlayerInventory(INTERACTIVE_OBJ * io);
bool IsInSecondaryInventory(INTERACTIVE_OBJ * io);
bool InInventoryPos(Vec2s * pos);
void ReplaceInAllInventories(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void RemoveFromAllInventories(const INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * ARX_INVENTORY_GetTorchLowestDurability();
void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_OpenClose(INTERACTIVE_OBJ *);
void ARX_INVENTORY_TakeAllFromSecondaryInventory();

void IO_Drop_Item(INTERACTIVE_OBJ * io_src, INTERACTIVE_OBJ * io);

#endif // ARX_GAME_INVENTORY_H
