/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file spdigger_stack.h
 *     Header file for spdigger_stack.c.
 * @par Purpose:
 *     Special diggers task stack support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 04 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_SPDIGGER_STACK_H
#define DK_SPDIGGER_STACK_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum SpecialDiggerTask {
    DigTsk_None = 0,
    DigTsk_ImproveDungeon,
    DigTsk_ConvertDungeon,
    DigTsk_ReinforceWall,
    DigTsk_PickUpUnconscious,
    DigTsk_PickUpCorpse,
    DigTsk_PicksUpSpellBook,
    DigTsk_PicksUpCrateToArm,
    DigTsk_PicksUpCrateForWorkshop,
    DigTsk_DigOrMine,
    DigTsk_PicksUpGoldPile, // 10
};

enum SpecialLastJobKinds {
    SDLstJob_None = 0,
    SDLstJob_DigOrMine,
    SDLstJob_ConvImprDungeon,
    SDLstJob_ReinforceWall3,
    SDLstJob_UseTraining4,
    SDLstJob_Unkn5,
    SDLstJob_Unkn6,
    SDLstJob_Unkn7,
    SDLstJob_Unkn8,
    SDLstJob_ReinforceWall9,
};

enum SpecialDiggerDigTaskKinds {
    SDDigTask_None = 0,
    SDDigTask_DigEarth,
    SDDigTask_MineGold,
    SDDigTask_Unknown3,
};

/******************************************************************************/
#pragma pack(1)

struct Dungeon;
struct Thing;

struct SlabCoord {

    unsigned char x;
    unsigned char y;
};

struct ExtraSquares {
    unsigned long index;
    unsigned char flgmask;
};

#pragma pack()
/******************************************************************************/
TbBool add_to_imp_stack_using_pos(long stl_num, long task_type, struct Dungeon *dungeon);
long find_in_imp_stack_using_pos(long stl_num, long task_type, const struct Dungeon *dungeon);
long find_in_imp_stack_starting_at(long task_type, long start_pos, const struct Dungeon *dungeon);
long add_undug_to_imp_stack(struct Dungeon *dungeon, long num);
void add_pretty_and_convert_to_imp_stack(struct Dungeon *dungeon);
long add_unclaimed_gold_to_imp_stack(struct Dungeon *dungeon);
void setup_imp_stack(struct Dungeon *dungeon);
long add_unclaimed_unconscious_bodies_to_imp_stack(struct Dungeon *dungeon, long a2);
TbBool add_unclaimed_dead_bodies_to_imp_stack(struct Dungeon *dungeon, long max_tasks);
long add_unclaimed_spells_to_imp_stack(struct Dungeon *dungeon, long a2);
TbBool add_object_for_trap_to_imp_stack(struct Dungeon *dungeon, struct Thing *thing);
TbBool add_empty_traps_to_imp_stack(struct Dungeon *dungeon, long num);
TbBool add_unclaimed_traps_to_imp_stack(struct Dungeon *dungeon);

TbBool imp_will_soon_be_arming_trap(struct Thing *traptng);
long imp_will_soon_be_working_at_excluding(struct Thing *thing, long a2, long a3);
TbBool imp_will_soon_be_getting_object(PlayerNumber plyr_idx, const struct Thing *objtng);

void add_reinforce_to_imp_stack(struct Dungeon *dungeon);
TbBool imp_stack_update(struct Thing *thing);
long check_out_imp_stack(struct Thing *thing);
long check_out_imp_last_did(struct Thing *thing);
long check_place_to_convert_excluding(struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y);
long check_place_to_pretty_excluding(struct Thing *thing, long a2, long a3);
long check_out_imp_has_money_for_treasure_room(struct Thing *thing);
long check_out_available_imp_tasks(struct Thing *thing);
long check_out_imp_tokes(struct Thing *thing);
long check_place_to_dig_and_get_position(struct Thing *thing, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y);
long check_place_to_reinforce(struct Thing *thing, long a2, long a3);
long check_out_uncrowded_reinforce_position(struct Thing *thing, SubtlCodedCoords stl_num, long *retslb_x, long *retslb_y);
void force_any_creature_dragging_owned_thing_to_drop_it(struct Thing *dragtng);
void force_any_creature_dragging_thing_to_drop_it(struct Thing *dragtng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
