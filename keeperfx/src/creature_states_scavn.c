/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_scavn.c
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
#include "creature_states_scavn.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_sound.h"

#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_states_prisn.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "player_utils.h"
#include "player_instances.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_scavenge.h"
#include "room_entrance.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_scavenger_room(struct Thing *calltng);
DLLIMPORT short _DK_creature_being_scavenged(struct Thing *scavtng);
DLLIMPORT short _DK_creature_scavenged_disappear(struct Thing *scavtng);
DLLIMPORT short _DK_creature_scavenged_reappear(struct Thing *scavtng);
DLLIMPORT long _DK_process_scavenge_function(struct Thing *calltng);
DLLIMPORT short _DK_scavengering(struct Thing *calltng);
DLLIMPORT long _DK_turn_creature_to_scavenger(struct Thing *scavtng, struct Thing *calltng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_do_scavenging(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return (crstat->scavenge_value > 0);
}

short at_scavenger_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_scavenger_room(thing);
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_SCAVENGER, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (crstat->scavenger_cost >= dungeon->total_money_owned)
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToScavenge, MESSAGE_DELAY_TREASURY, true);
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room))
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Scavengering);
    cctrl->field_82 = 0;
    return 1;
}

short creature_being_scavenged(struct Thing *creatng)
{
    //return _DK_creature_being_scavenged(creatng);
    struct Thing *fellowtng;
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    //return _DK_make_all_players_creatures_angry(plyr_idx);
    dungeon = get_players_num_dungeon(creatng->owner);
    fellowtng = INVALID_THING;
    if (dungeon->num_active_creatrs <= 1)
    {
        SYNCDBG(19,"No other creatures");
        return 0;
    }
    int n;
    n = ACTION_RANDOM(dungeon->num_active_creatrs-1);
    unsigned long k;
    int i;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if ((n <= 0) && (thing->index != creatng->index)) {
            fellowtng = thing;
            break;
        }
        n--;
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    if (thing_is_invalid(fellowtng))
    {
        SYNCDBG(19,"Cannot get creature");
        return 0;
    }
    if (setup_person_move_to_coord(creatng, &fellowtng->mappos, NavRtF_Default) <= 0)
    {
        SYNCDBG(19,"Cannot move to coord");
        return 0;
    }
    creatng->continue_state = CrSt_CreatureBeingScavenged;
    if (!S3DEmitterIsPlayingSample(creatng->snd_emitter_id, 156, 0))
        thing_play_sample(creatng, 156, NORMAL_PITCH, 0, 3, 1, 2, FULL_LOUDNESS);
    SYNCDBG(19,"Finished");
    return 1;
}

short creature_scavenged_disappear(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    struct Coord3d pos;
    long stl_x, stl_y;
    long i;
    //return _DK_creature_scavenged_disappear(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->byte_9A--;
    if (cctrl->byte_9A > 0)
    {
      if ((cctrl->byte_9A == 7) && (cctrl->byte_9B < PLAYERS_COUNT))
      {
        create_effect(&thing->mappos, get_scavenge_effect_element(cctrl->byte_9B), thing->owner);
      }
      return 0;
    }
    // We don't really have to convert coordinates into numbers and back to XY.
    i = get_subtile_number(cctrl->scavenge.stl_9D_x, cctrl->scavenge.stl_9D_y);
    stl_x = stl_num_decode_x(i);
    stl_y = stl_num_decode_y(i);
    room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room) || (room->kind != RoK_SCAVENGER))
    {
        ERRORLOG("Room %s at (%d,%d) disappeared.",room_code_name(RoK_SCAVENGER),(int)stl_x,(int)stl_y);
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
        return -1;
    }
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        move_thing_in_map(thing, &pos);
        anger_set_creature_anger_all_types(thing, 0);
        dungeon = get_dungeon(cctrl->byte_9B);
        dungeon->creatures_scavenge_gain++;
        if (is_my_player_number(thing->owner))
          output_message(SMsg_MinionScanvenged, 0, true);
        cctrl->byte_9C = thing->owner;
        change_creature_owner(thing, cctrl->byte_9B);
        internal_set_thing_state(thing, CrSt_CreatureScavengedReappear);
        return 0;
    } else
    {
        ERRORLOG("No valid position inside %s room for %s.",room_code_name(room->kind),thing_model_name(thing));
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
        return -1;
    }
}

short creature_scavenged_reappear(struct Thing *thing)
{
    return _DK_creature_scavenged_reappear(thing);
}

/**
 * Returns if a player to whom the creature belongs can afford the creature to go scavenging.
 * @param creatng
 * @return
 */
TbBool player_can_afford_to_scavenge_creature(const struct Thing *creatng)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(creatng->owner);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return (crstat->scavenger_cost < dungeon->total_money_owned);
}

TbBool reset_scavenge_counts(struct Dungeon *dungeon)
{
    memset(dungeon->creatures_scavenging, 0, CREATURE_TYPES_COUNT);
    dungeon->scavenge_counters_turn = game.play_gameturn;
    return true;
}

TbBool can_scavenge_creature_from_pool(const struct Dungeon * dungeon, ThingModel crtr_kind)
{
    SYNCDBG(11, "Starting for creature kind %s", creature_code_name(crtr_kind));

    if (game.pool.crtr_kind[crtr_kind] <= 0) {
        return false;
    }
    if (dungeon->num_active_creatrs >= dungeon->max_creatures_attracted) {
        return false;
    }
    return true;
}

TbBool thing_is_valid_scavenge_target(const struct Thing *calltng, const struct Thing *scavtng)
{
    if (!thing_is_creature(scavtng) || (scavtng->model != calltng->model)) {
        return false;
    }
    if (!is_neutral_thing(scavtng))
    {
        if (!players_are_enemies(calltng->owner, scavtng->owner)) {
            return false;
        }
    }
    if (thing_is_picked_up(scavtng)) {
        return false;
    }
    if (is_thing_passenger_controlled(scavtng) || creature_is_kept_in_custody(scavtng)) {
        return false;
    }
    if (is_hero_thing(scavtng) && (!gameadd.scavenge_good_allowed)) {
        return false;
    }
    if (is_neutral_thing(scavtng) && (!gameadd.scavenge_neutral_allowed)) {
        return false;
    }
    struct PlayerInfo *scavplyr;
    scavplyr = INVALID_PLAYER;
    if (!is_neutral_thing(scavtng)) {
        scavplyr = get_player(scavtng->owner);
    }
    if (scavplyr->controlled_thing_idx != scavtng->index)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(scavtng);
        if (game.play_gameturn - cctrl->temple_cure_gameturn > game.temple_scavenge_protection_turns)
        {
            return true;
        }
    }
    return false;
}

struct Thing *select_scavenger_target(const struct Thing *calltng)
{
    long weakpts;
    struct Thing *weaktng;
    weaktng = INVALID_THING;
    weakpts = LONG_MAX;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_valid_scavenge_target(calltng, thing))
        {
            SYNCDBG(18,"The %s index %d owner %d is valid target for %s index %d owner %d",
                thing_model_name(thing),(int)thing->index,(int)thing->owner,
                thing_model_name(calltng),(int)calltng->index,(int)calltng->owner);
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);
            if (game.play_gameturn - cctrl->temple_cure_gameturn > game.temple_scavenge_protection_turns)
            {
                long thingpts;
                thingpts = calculate_correct_creature_scavenge_required(thing, calltng->owner);
                if (weakpts > thingpts)
                {
                    weakpts = thingpts;
                    weaktng = thing;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(18,"The weakest valid target for %s index %d owner %d is %s index %d owner %d",
        thing_model_name(calltng),(int)calltng->index,(int)calltng->owner,
        thing_model_name(weaktng),(int)weaktng->index,(int)weaktng->owner);
    return weaktng;
}

struct Thing *get_scavenger_target(const struct Thing *calltng)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(calltng->owner);
    { // Check if last scavenged creature of that kind is still good for scavenging
        struct Thing *lastng;
        if (dungeon->scavenge_targets[calltng->model] != 0) {
            lastng = thing_get(dungeon->scavenge_targets[calltng->model]);
        } else {
            lastng = INVALID_THING;
        }
        if (thing_is_valid_scavenge_target(calltng, lastng))
        {
            SYNCDBG(8,"The last target, %s index %d owner %d, is still valid",thing_model_name(lastng),(int)lastng->index,(int)lastng->owner);
            return lastng;
        }
    }
    return select_scavenger_target(calltng);
}

long turn_creature_to_scavenger(struct Thing *scavtng, struct Thing *calltng)
{
    return _DK_turn_creature_to_scavenger(scavtng, calltng);
}

TbBool process_scavenge_creature_from_level(struct Thing *scavtng, struct Thing *calltng, long work_value)
{
    struct Dungeon *calldngn;
    long num_prayers;
    calldngn = get_dungeon(calltng->owner);
    if (dungeon_invalid(calldngn)) {
        ERRORLOG("The %s owner %d can't do scavenging - has no dungeon",thing_model_name(calltng),(int)calltng->owner);
        return false;
    }
    // Compute amount of creatures praying against the scavenge
    if (!is_neutral_thing(scavtng)) {
        struct Dungeon *scavdngn;
        scavdngn = get_dungeon(scavtng->owner);
        num_prayers = scavdngn->creatures_praying[scavtng->model];
    } else {
        num_prayers = 0;
    }
    // Increase scavenging counter, used to break the prayers counter
    calldngn->creatures_scavenging[scavtng->model]++;
    // If scavenge is blocked by prayers, return
    if (calldngn->creatures_scavenging[calltng->model] < 2 * num_prayers) {
        SYNCDBG(8, "Player %d prayers (%d) are blocking player %d scavenging (%d) of %s", (int)scavtng->owner,
            (int)num_prayers, (int)calltng->owner, (int)calldngn->creatures_scavenging[calltng->model], thing_model_name(calltng));
        return false;
    }
    SYNCDBG(18,"The %s index %d scavenges %s index %d",thing_model_name(calltng),(int)calltng->index,thing_model_name(scavtng),(int)scavtng->index);
    // If we're starting to scavenge a new creature, do the switch
    if (calldngn->scavenge_targets[calltng->model] != scavtng->index)
    {
        calldngn->scavenge_turn_points[calltng->model] = work_value;
        if (calldngn->scavenge_targets[calltng->model] > 0)
        {
            // Stop scavenging old creature
            struct Thing *thing;
            thing = thing_get(calldngn->scavenge_targets[calltng->model]);
            if (thing_is_creature(thing) && (thing->model == calltng->model))
            {
                if (creature_is_being_scavenged(thing)) {
                    set_start_state(thing);
                }
            }
        }
        // Start the new scavenging
        calldngn->scavenge_targets[calltng->model] = scavtng->index;
        if (is_my_player_number(scavtng->owner)) {
            output_message(SMsg_CreatureScanvenged, 500, 1);
        }
        event_create_event(scavtng->mappos.x.val, scavtng->mappos.y.val, EvKind_CreatrScavenged, scavtng->owner, scavtng->index);
    } else
    {
        calldngn->scavenge_turn_points[calltng->model] += work_value;
    }
    // Make sure the scavenged creature is in correct state
    if (!creature_is_being_scavenged(scavtng))
    {
        if (!is_neutral_thing(scavtng)) {
            external_set_thing_state(scavtng, CrSt_CreatureBeingScavenged);
        }
    }
    long scavpts;
    scavpts = calculate_correct_creature_scavenge_required(scavtng, calltng->owner);
    if ((scavpts << 8) < calldngn->scavenge_turn_points[calltng->model])
    {
        SYNCDBG(8,"The %s index %d owner %d accumulated enough points to turn to scavenger",thing_model_name(scavtng),(int)scavtng->index,(int)scavtng->owner);
        turn_creature_to_scavenger(scavtng, calltng);
        calldngn->scavenge_turn_points[calltng->model] -= (scavpts << 8);
        return true;
    }
    return false;
}

TbBool creature_scavenge_from_creature_pool(struct Thing *calltng)
{
    struct Room *room;
    struct Coord3d pos;
    room = get_room_thing_is_on(calltng);
    if (!room_initially_valid_as_type_for_thing(room, RoK_SCAVENGER, calltng)) {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->owner);
        return false;
    }
    if (game.pool.crtr_kind[calltng->model] <= 0) {
        ERRORLOG("Tried to generate %s but it is not in pool",thing_model_name(calltng));
        return false;
    }
    if ( !find_random_valid_position_for_thing_in_room(calltng, room, &pos) ) {
        ERRORLOG("Could not find valid position for thing to be generated");
        return false;
    }
    struct Thing *scavtng;
    scavtng = create_creature(&pos, calltng->model, calltng->owner);
    if (thing_is_invalid(scavtng)) {
        ERRORLOG("Tried to generate %s but creation failed",thing_model_name(calltng));
        return false;
    }
    if (!remove_creature_from_generate_pool(calltng->model))
    {
        ERRORLOG("Could not remove %s from pool",thing_model_name(calltng));
        return false;
    }
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(calltng->owner);
        dungeon->creatures_scavenge_gain++;
    }
    internal_set_thing_state(scavtng, CrSt_CreatureScavengedReappear);
    return true;
}

TbBool process_scavenge_creature_from_pool(struct Thing *calltng, long work_value)
{
    struct Dungeon *calldngn;
    calldngn = get_dungeon(calltng->owner);
    calldngn->scavenge_turn_points[calltng->model] += work_value;
    long scavpts;
    scavpts = calculate_correct_creature_scavenge_required(calltng, calltng->owner);
    if ((scavpts << 8) < calldngn->scavenge_turn_points[calltng->model])
    {
        creature_scavenge_from_creature_pool(calltng);
        calldngn->scavenge_turn_points[calltng->model] -= scavpts;
        return true;
    }
    return false;
}

CrCheckRet process_scavenge_function(struct Thing *calltng)
{
    SYNCDBG(18,"Starting for %s owner %d",thing_model_name(calltng),(int)calltng->owner);
    //return _DK_process_scavenge_function(thing);
    struct CreatureControl *callctrl;
    callctrl = creature_control_get_from_thing(calltng);
    struct Dungeon *calldngn;
    struct Room *room;
    calldngn = get_dungeon(calltng->owner);
    room = get_room_creature_works_in(calltng);
    if ( !room_still_valid_as_type_for_thing(room, RoK_SCAVENGER, calltng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->owner);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(calltng);
    if (!player_can_afford_to_scavenge_creature(calltng))
    {
        if (is_my_player_number(calltng->owner))
            output_message(SMsg_NoGoldToScavenge, 500, 1);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    if (calldngn->scavenge_counters_turn != game.play_gameturn)
    {
        reset_scavenge_counts(calldngn);
    }
    long work_value;
    work_value = compute_creature_work_value(crstat->scavenge_value*256, room->efficiency, callctrl->explevel);
    work_value = process_work_speed_on_work_value(calltng, work_value);
    SYNCDBG(9,"The %s index %d owner %d produced %d scavenge points",thing_model_name(calltng),(int)calltng->index,(int)calltng->owner,(int)work_value);
    struct Thing *scavtng;
    scavtng = get_scavenger_target(calltng);
    if (!thing_is_invalid(scavtng))
    {
        process_scavenge_creature_from_level(scavtng, calltng, work_value);
    } else
    if (can_scavenge_creature_from_pool(calldngn, calltng->model))
    {
        process_scavenge_creature_from_pool(calltng, work_value);
    } else
    {
        if (crstat->entrance_force) {
          calldngn->field_1485++;
        }
        return 0;
    }
    callctrl->field_82++;
    if (callctrl->field_82 > game.scavenge_cost_frequency)
    {
        callctrl->field_82 -= game.scavenge_cost_frequency;
        if (take_money_from_dungeon(calltng->owner, crstat->scavenger_cost, 1) < 0) {
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)crstat->scavenger_cost,(int)calltng->owner);
        }
        create_price_effect(&calltng->mappos, calltng->owner, crstat->scavenger_cost);
    }
    return 0;
}

TbBool setup_scavenger_move(struct Thing *thing, struct Room *room)
{
    if (!person_move_somewhere_adjacent_in_room(thing, room)) {
        return false;
    }
    thing->continue_state = CrSt_Scavengering;
    return true;
}


CrStateRet scavengering(struct Thing *thing)
{
    // Check if we're in correct room
    struct Room *room;
    room = get_room_thing_is_on(thing);
    if (creature_work_in_room_no_longer_possible(room, RoK_SCAVENGER, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    //return _DK_scavengering(thing);
    if (process_scavenge_function(thing))
    {
        return CrStRet_Modified;
    }
    if (setup_scavenger_move(thing, room)) {
        return CrStRet_Modified;
    }
    return CrStRet_Unchanged;
}

/******************************************************************************/
