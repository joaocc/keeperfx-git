/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand.h
 *     Header file for power_hand.c.
 * @par Purpose:
 *     power_hand functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_POWERHAND_H
#define DK_POWERHAND_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

#pragma pack()
/******************************************************************************/
void add_creature_to_sacrifice_list(PlayerNumber owner, long model, long explevel);
void place_thing_in_limbo(struct Thing *thing);
unsigned long object_is_pickable_by_hand(struct Thing *thing, long a2);
TbBool thing_is_pickable_by_hand(struct PlayerInfo *player,struct Thing *thing);
void set_power_hand_offset(struct PlayerInfo *player, struct Thing *thing);
struct Thing *process_object_being_picked_up(struct Thing *thing, long a2);
void set_power_hand_graphic(long a1, long a2, long a3);
TbBool power_hand_is_empty(const struct PlayerInfo *player);
TbBool power_hand_is_full(const struct PlayerInfo *player);
struct Thing *get_first_thing_in_power_hand(struct PlayerInfo *player);
TbBool dump_thing_in_power_hand(struct Thing *thing, long plyr_idx);
void draw_power_hand(void);
void clear_things_in_hand(struct PlayerInfo *player);
TbBool magic_use_power_hand(PlayerNumber plyr_idx, unsigned short a2, unsigned short a3, unsigned short tng_idx);
struct Thing *get_nearest_thing_for_slap(PlayerNumber plyr_idx, MapCoord x, MapCoord y);
struct Thing *get_nearest_thing_for_hand_or_slap(PlayerNumber plyr_idx, MapCoord x, MapCoord y);

long can_thing_be_picked_up_by_player(const struct Thing *thing, PlayerNumber plyr_idx);
long can_thing_be_picked_up2_by_player(const struct Thing *thing, PlayerNumber plyr_idx);

TbBool slap_object(struct Thing *thing);
TbBool object_is_slappable(const struct Thing *thing, long plyr_idx);
TbBool thing_slappable(const struct Thing *thing, long plyr_idx);

struct Thing *create_power_hand(PlayerNumber owner);
void delete_power_hand(PlayerNumber owner);
void stop_creatures_around_hand(char a1, unsigned short a2, unsigned short a3);
long place_thing_in_power_hand(struct Thing *thing, long var);
short can_place_thing_here(struct Thing *thing, long x, long y, long dngn_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
