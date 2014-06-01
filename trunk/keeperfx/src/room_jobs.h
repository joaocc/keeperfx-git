/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_jobs.h
 *     Header file for room_jobs.c.
 * @par Purpose:
 *     List of things in room maintain functions.
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
#ifndef DK_ROOM_JOBS_H
#define DK_ROOM_JOBS_H

#include "globals.h"
#include "bflib_basics.h"

#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;
struct Dungeon;

#pragma pack()
/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing);
TbBool creature_is_working_in_room(const struct Thing *creatng, const struct Room *room);

TbBool add_creature_to_work_room(struct Thing *crtng, struct Room *room);
TbBool add_creature_to_torture_room(struct Thing *creatng, const struct Room *room);
TbBool remove_creature_from_specific_room(struct Thing *creatng, struct Room *room);
TbBool remove_creature_from_work_room(struct Thing *thing);
TbBool remove_creature_from_torture_room(struct Thing *creatng);

short send_creature_to_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref);

struct Thing *find_object_in_room_for_creature_matching_bool_filter(struct Thing *creatng, const struct Room *room, Thing_Bool_Filter matcher_cb);

TbBool worker_needed_in_dungeons_room_kind(const struct Dungeon *dungeon, RoomKind rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
