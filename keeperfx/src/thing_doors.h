/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.h
 *     Header file for thing_doors.c.
 * @par Purpose:
 *     XXXX functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_DOORS_H
#define DK_THING_DOORS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define DOOR_TYPES_COUNT        5
/******************************************************************************/
#pragma pack(1)

enum DoorStates {
    DorSt_Unused = 0,
    DorSt_Open,
    DorSt_Closed,
    DorSt_Opening,
    DorSt_Closing,
    DorSt_Unknown05,
};

struct Thing;

struct DoorStats { // sizeof = 8
    unsigned short field_0;
    long health;
    unsigned short field_6;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern struct DoorStats _DK_door_stats[5][2];
#define door_stats _DK_door_stats
DLLIMPORT extern unsigned char _DK_door_to_object[DOOR_TYPES_COUNT];
/******************************************************************************/
TbBool subtile_has_door_thing_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

struct Thing *create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4);
TbBool thing_is_deployed_door(const struct Thing *thing);
void lock_door(struct Thing *thing);
void unlock_door(struct Thing *thing);
long destroy_door(struct Thing *thing);
TngUpdateRet process_door(struct Thing *thing);

char find_door_angle(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);

TbBool find_door_of_type(ThingModel model, PlayerNumber owner);
TbBool find_trap_of_type(ThingModel model, PlayerNumber owner);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
