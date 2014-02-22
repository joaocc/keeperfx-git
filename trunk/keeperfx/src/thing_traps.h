/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.h
 *     Header file for thing_traps.c.
 * @par Purpose:
 *     Traps support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_TRAPS_H
#define DK_THING_TRAPS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define TRAP_TYPES_COUNT        7
/******************************************************************************/
#pragma pack(1)

enum ThingTrapModels {
    TngTrp_None = 0,
    TngTrp_Unknown01,
    TngTrp_Unknown02,
    TngTrp_Unknown03,
    TngTrp_Unknown04,
    TngTrp_Unknown05,
    TngTrp_Unknown06,
    TngTrp_Unknown07,
    TngTrp_Unknown08,
    TngTrp_Unknown09,
    TngTrp_Unknown10,
};

struct Thing;

struct TrapStats {  // sizeof=54
unsigned long field_0;
unsigned long field_4;
unsigned long field_8;
unsigned char field_C;
unsigned long field_D;
unsigned char field_11;
  unsigned char field_12;
unsigned char field_13;
short field_14;
short field_16;
unsigned char trigger_type;
unsigned char activation_type;
unsigned char field_1A;
  unsigned char field_1B;
short field_1C;
unsigned char field_1E;
unsigned char field_1F;
unsigned char field_20[8];
unsigned char field_28[8];
short field_30;
short field_32;
short field_34;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern unsigned char _DK_trap_to_object[8];
DLLIMPORT struct TrapStats _DK_trap_stats[7]; //not sure - maybe it's 8?
#define trap_stats _DK_trap_stats
/******************************************************************************/
TbBool slab_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y);

TbBool destroy_trap(struct Thing *thing);
struct Thing *create_trap(struct Coord3d *pos, ThingModel trpkind, PlayerNumber plyr_idx);
struct Thing *get_trap_for_position(long pos_x, long pos_y);
struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool trap_is_active(const struct Thing *thing);
TbBool trap_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool thing_is_deployed_trap(const struct Thing *thing);
TbBool rearm_trap(struct Thing *traptng);
TngUpdateRet update_trap(struct Thing *thing);
void init_traps(void);

long remove_traps_around_subtile(long stl_x, long stl_y, long *sell_value);

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2);
TbBool tag_cursor_blocks_place_trap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
