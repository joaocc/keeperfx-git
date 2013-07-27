/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_prisn.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_prisn.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_cleanup_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_arrived_at_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_drop_body_in_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_freeze_prisonors(struct Thing *thing);
DLLIMPORT short _DK_creature_in_prison(struct Thing *thing);
DLLIMPORT long _DK_process_prison_function(struct Thing *thing);
DLLIMPORT long _DK_process_prison_food(struct Thing *thing, struct Room *room);
DLLIMPORT long _DK_setup_prison_move(struct Thing *thing, struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool jailbreak_possible(struct Room *room, long plyr_idx)
{
    unsigned long i;
    unsigned long k;
    struct SlabMap *slb;
    if ((room->owner == plyr_idx) || (room->slabs_list <= 0))
        return false;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
            ERRORLOG("Jump to invalid room slab detected");
            break;
        }
        if (slab_by_players_land(plyr_idx, slb_num_decode_x(i), slb_num_decode_y(i)))
            return true;
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > map_tiles_x * map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    return false;
}

short cleanup_prison(struct Thing *thing)
{
  return _DK_cleanup_prison(thing);
}

short creature_arrived_at_prison(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_creature_arrived_at_prison(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_PRISON, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        if (is_my_player_number(room->owner))
            output_message(SMsg_PrisonTooSmall, 0, true);
        cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        set_start_state(thing);
        return 0;
    }
    cctrl->field_82 = game.play_gameturn;
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    internal_set_thing_state(thing, CrSt_CreatureInPrison);
    if (creature_affected_by_spell(thing, SplK_Speed)) {
      terminate_thing_spell_effect(thing, SplK_Speed);
    }
    if (creature_affected_by_spell(thing, SplK_Invisibility)) {
        terminate_thing_spell_effect(thing, SplK_Invisibility);
    }
    if (thing->light_id != 0) {
        light_delete_light(thing->light_id);
        thing->light_id = 0;
    }
    return 1;

}

short creature_drop_body_in_prison(struct Thing *thing)
{
  return _DK_creature_drop_body_in_prison(thing);
}

short creature_freeze_prisonors(struct Thing *thing)
{
  return _DK_creature_freeze_prisonors(thing);
}

long setup_prison_move(struct Thing *thing, struct Room *room)
{
    return _DK_setup_prison_move(thing, room);
}

CrStateRet process_prison_visuals(struct Thing *thing, struct Room *room)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_NULL)
        return Lb_OK;
    if (game.play_gameturn - cctrl->field_82 > 200)
    {
        if (game.play_gameturn - cctrl->field_82 < 250)
        {
            set_creature_instance(thing, CrInst_MOAN, 1, 0, 0);
            if (game.play_gameturn - cctrl->long_9A > 32)
            {
                play_creature_sound(thing, CrSnd_PrisonMoan, 2, 0);
                cctrl->long_9A = game.play_gameturn;
            }
            return Lb_SUCCESS;
        }
        cctrl->field_82 = game.play_gameturn;
    }
    if (setup_prison_move(thing, room))
    {
        thing->continue_state = CrSt_CreatureInPrison;
        return Lb_SUCCESS;
    }
    return Lb_OK;
}

CrStateRet creature_in_prison(struct Thing *thing)
{
    struct Room *room;
    //return _DK_creature_in_prison(thing);
    TRACE_THING(thing);
    room = get_room_thing_is_on(thing);
    if (creature_work_in_room_no_longer_possible(room, RoK_PRISON, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    if (room->total_capacity < room->used_capacity)
    {
        if (is_my_player_number(room->owner))
            output_message(SMsg_PrisonTooSmall, 0, true);
        set_start_state(thing);
        return CrStRet_ResetOk;
    }
    switch (process_prison_function(thing))
    {
    case CrCkRet_Deleted:
        return CrStRet_Deleted;
    case CrCkRet_Available:
        process_prison_visuals(thing, room);
        return CrStRet_Modified;
    default:
        return CrStRet_ResetOk;
    }
}

TbBool prison_convert_creature_to_skeleton(struct Room *room, struct Thing *thing)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *crthing;
  long crmodel;
  cctrl = creature_control_get_from_thing(thing);
  crmodel = get_room_create_creature_model(room->kind); // That normally returns skeleton breed
  crthing = create_creature(&thing->mappos, crmodel, room->owner);
  if (thing_is_invalid(crthing))
  {
      ERRORLOG("Couldn't create creature %s in prison", creature_code_name(crmodel));
      return false;
  }
  init_creature_level(crthing, cctrl->explevel);
  set_start_state(crthing);
  if (creature_model_bleeds(thing->model))
    create_effect_around_thing(thing, TngEff_Unknown10);
  kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
  dungeon = get_dungeon(room->owner);
  if (!dungeon_invalid(dungeon))
      dungeon->lvstats.skeletons_raised++;
  return true;
}

TbBool process_prisoner_skelification(struct Thing *thing, struct Room *room)
{
  struct CreatureStats *crstat;
  crstat = creature_stats_get_from_thing(thing);
  if ( (thing->health >= 0) || (!crstat->humanoid_creature) )
    return false;
  //TODO [config] Allow skeletification only if spent specific amount of turns in prison (set low value)
  if (ACTION_RANDOM(101) > game.prison_skeleton_chance)
    return false;
  if (is_my_player_number(room->owner))
    output_message(SMsg_PrisonMadeSkeleton, 0, true);
  prison_convert_creature_to_skeleton(room,thing);
  return true;
}

long process_prison_food(struct Thing *thing, struct Room *room)
{
  return _DK_process_prison_food(thing, room);
}

/**
 * Does a step of being imprisoned.
 * Informs if the imprisoning cycle should end.
 * @param thing
 */
CrCheckRet process_prison_function(struct Thing *thing)
{
  struct Room *room;
  //return _DK_process_prison_function(thing);
  room = get_room_creature_works_in(thing);
  if ( !room_still_valid_as_type_for_thing(room, RoK_PRISON, thing) )
  {
      WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->owner);
      set_start_state(thing);
      return CrCkRet_Continue;
  }
  process_creature_hunger(thing);
  if ( process_prisoner_skelification(thing,room) )
    return CrCkRet_Deleted;
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(thing);
  if ((cctrl->instance_id == CrInst_NULL) && process_prison_food(thing, room) )
    return CrCkRet_Continue;
  // Breaking from jail is only possible once per some amount of turns
  if ((game.play_gameturn % gameadd.time_between_prison_break) == 0)
  {
      if (jailbreak_possible(room, thing->owner))
      {
          if (is_my_player_number(room->owner))
              output_message(SMsg_PrisonersEscaping, 40, true);
          else if (is_my_player_number(room->owner))
              output_message(SMsg_CreatrFreedPrison, 40, true);
          set_start_state(thing);
          return CrCkRet_Continue;
      }
  }
  return CrCkRet_Available;
}

/******************************************************************************/
