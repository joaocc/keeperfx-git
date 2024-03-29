/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_jobs.c
 *     Creature job assign and verify functions.
 * @par Purpose:
 *     Defines creature jobs configuration and various job-related routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     27 Aug 2013 - 07 Oct 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_jobs.h"
#include "globals.h"

#include "bflib_math.h"

#include "thing_data.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_magic.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "room_jobs.h"
#include "power_hand.h"
#include "spdigger_stack.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "gui_soundmsgs.h"

#include "creature_states_prisn.h"
#include "creature_states_rsrch.h"
#include "creature_states_scavn.h"
#include "creature_states_spdig.h"
#include "creature_states_tortr.h"
#include "creature_states_train.h"
#include "creature_states_wrshp.h"
#include "creature_states_lair.h"
#include "creature_states_pray.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_creature_find_and_perform_anger_job(struct Thing *creatng);
DLLIMPORT long _DK_attempt_job_preference(struct Thing *creatng, long jobpref);
DLLIMPORT long _DK_attempt_anger_job_destroy_rooms(struct Thing *creatng);
DLLIMPORT long _DK_attempt_anger_job_steal_gold(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_do_job_always_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_training_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_research_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_manufacturing_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_scavenging_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_freeze_prisoners_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_join_fight_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_barracking_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);

TbBool attempt_job_work_in_room_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_in_state_on_room_content_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_move_to_event_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_in_state_internal_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);

TbBool creature_can_do_job_always_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_research_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_training_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_manufacturing_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_scavenging_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_place_in_vault_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_take_salary_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_take_sleep_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);

TbBool attempt_job_work_in_room_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_work_in_room_and_cure_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_sleep_in_lair_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_in_state_internal_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);

const struct NamedCommand creature_job_player_check_func_type[] = {
  {"can_do_job_always",        1},
  {"can_do_training",          2},
  {"can_do_research",          3},
  {"can_do_manufacturing",     4},
  {"can_do_scavenging",        5},
  {"can_freeze_prisoners",     6},
  {"can_join_fight",           7},
  {"can_do_barracking",        8},
  {"none",                     9},
  {NULL,                       0},
};

Creature_Job_Player_Check_Func creature_job_player_check_func_list[] = {
  NULL,
  creature_can_do_job_always_for_player,
  creature_can_do_training_for_player,
  creature_can_do_research_for_player,
  creature_can_do_manufacturing_for_player,
  creature_can_do_scavenging_for_player,
  creature_can_freeze_prisoners_for_player,
  creature_can_join_fight_for_player,
  creature_can_do_barracking_for_player,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_player_assign_func_type[] = {
  {"work_in_room",             1},
  {"in_state_on_room_content", 2},
  {"move_to_event",            3},
  {"in_state_internal",        4},
  {"none",                     5},
  {NULL,                       0},
};

Creature_Job_Player_Assign_Func creature_job_player_assign_func_list[] = {
  NULL,
  attempt_job_work_in_room_for_player,
  attempt_job_in_state_on_room_content_for_player,
  attempt_job_move_to_event_for_player,
  attempt_job_in_state_internal_for_player,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_coords_check_func_type[] = {
  {"can_do_job_always",        1},
  {"can_do_research",          2},
  {"can_do_training",          3},
  {"can_do_manufacturing",     4},
  {"can_do_scavenging",        5},
  {"can_place_in_vault",       6},
  {"can_take_salary",          7},
  {"can_take_sleep",           8},
  {"none",                     9},
  {NULL,                       0},
};

Creature_Job_Coords_Check_Func creature_job_coords_check_func_list[] = {
  NULL,
  creature_can_do_job_always_near_pos,
  creature_can_do_research_near_pos,
  creature_can_do_training_near_pos,
  creature_can_do_manufacturing_near_pos,
  creature_can_do_scavenging_near_pos,
  creature_can_place_in_vault_near_pos,
  creature_can_take_salary_near_pos,
  creature_can_take_sleep_near_pos,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_coords_assign_func_type[] = {
  {"work_in_room",             1},
  {"work_in_room_and_cure",    2},
  {"sleep_in_lair",            3},
  {"in_state_internal",        4},
  {"none",                     5},
  {NULL,                       0},
};

Creature_Job_Coords_Assign_Func creature_job_coords_assign_func_list[] = {
  NULL,
  attempt_job_work_in_room_near_pos,
  attempt_job_work_in_room_and_cure_near_pos,
  attempt_job_sleep_in_lair_near_pos,
  attempt_job_in_state_internal_near_pos,
  NULL,
  NULL,
};

/******************************************************************************/
TbBool set_creature_assigned_job(struct Thing *thing, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("The %s index %d has invalid control",thing_model_name(thing),(int)thing->index);
        return false;
    }
    cctrl->job_assigned = new_job;
    SYNCLOG("Assigned job %s for %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(thing),(int)thing->index,(int)thing->owner);
    return true;
}

/**
 * Returns if the given job is creature's primary or secondary job.
 * @param thing
 * @param job_kind
 * @return
 */
TbBool creature_has_job(const struct Thing *thing, CreatureJob job_kind)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return ((crstat->job_primary & job_kind) != 0) || ((crstat->job_secondary & job_kind) != 0);
}

TbBool creature_free_for_anger_job(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    cctrl = creature_control_get_from_thing(creatng);
    dungeon = get_dungeon(creatng->owner);
    return ((cctrl->spell_flags & CSAfF_Unkn0800) == 0)
        && (dungeon->must_obey_turn == 0)
        && ((cctrl->spell_flags & CSAfF_Chicken) == 0)
        && !thing_is_picked_up(creatng) && !is_thing_passenger_controlled(creatng);
}

TbBool attempt_anger_job_destroy_rooms(struct Thing *creatng)
{
    //return _DK_attempt_anger_job_destroy_rooms(creatng);
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureVandaliseRooms)) {
        return false;
    }
    struct Room *room;
    struct Coord3d pos;
    room = find_nearest_room_for_thing_excluding_two_types(creatng, creatng->owner, 7, 1, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureVandaliseRooms)) {
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner)) {
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_CreatureVandaliseRooms;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_anger_job_steal_gold(struct Thing *creatng)
{
    //return _DK_attempt_anger_job_steal_gold(creatng);
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureStealGold)) {
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if (creatng->creature.gold_carried >= crstat->gold_hold) {
        return false;
    }
    struct Room *room;
    struct Coord3d pos;
    room = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_TREASURE, NavRtF_NoOwner, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureStealGold)) {
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner))
    {
        ERRORLOG("Cannot setup head for treasury.");
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_CreatureSearchForGoldToStealInRoom1;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_anger_job_kill_creatures(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureKillCreatures)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureKillCreatures)) {
        return false;
    }
    return true;
}

TbBool attempt_anger_job_leave_dungeon(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureLeaves)) {
        return false;
    }
    struct Room *room;
    room = find_nearest_room_for_thing(creatng, creatng->owner, RoK_ENTRANCE, NavRtF_Default);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureLeaves)) {
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = CrSt_CreatureLeaves;
    return true;
}

TbBool attempt_anger_job_damage_walls(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureDamageWalls)) {
        return false;
    }
    struct Coord3d pos;
    if (!get_random_position_in_dungeon_for_creature(creatng->owner, 1, creatng, &pos)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureAttemptToDamageWalls)) {
        return false;
    }
    setup_person_move_to_coord(creatng, &pos, NavRtF_Default);
    creatng->continue_state = CrSt_CreatureAttemptToDamageWalls;
    return true;
}

TbBool attempt_anger_job_mad_psycho(struct Thing *creatng)
{
    TRACE_THING(creatng);
    if (!external_set_thing_state(creatng, CrSt_MadKillingPsycho)) {
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->spell_flags |= CSAfF_MadKilling;
    cctrl->byte_9A = 0;
    return true;
}

TbBool attempt_anger_job_persuade(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->explevel <= 5) {
        return false;
    }
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreaturePersuade)) {
        return false;
    }
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(creatng->owner);
    int persuade_count;
    persuade_count = min(dungeon->num_active_creatrs-1, 5);
    if (persuade_count <= 0) {
        return false;
    }
    persuade_count = ACTION_RANDOM(persuade_count) + 1;
    if (!external_set_thing_state(creatng, CrSt_CreaturePersuade)) {
        return false;
    }
    cctrl->byte_9A = persuade_count;
    return true;
}

TbBool attempt_anger_job_join_enemy(struct Thing *creatng)
{
    struct Thing *heartng;
    int i, n;
    n = ACTION_RANDOM(PLAYERS_COUNT);
    for (i=0; i < PLAYERS_COUNT; i++, n=(n+1)%PLAYERS_COUNT)
    {
        if ((n == game.neutral_player_num) || (n == creatng->owner))
            continue;
        struct PlayerInfo *player;
        player = get_player(n);
        if (!player_exists(player) || (player->field_2C != 1))
            continue;
        heartng = get_player_soul_container(n);
        if (thing_exists(heartng) && (heartng->active_state != 3))
        {
            TRACE_THING(heartng);
            if (creature_can_navigate_to(creatng, &heartng->mappos, NavRtF_Default)) {
                change_creature_owner(creatng, n);
                anger_set_creature_anger_all_types(creatng, 0);
            }
        }
    }
    return false;
}

long attempt_anger_job(struct Thing *creatng, long ajob_kind)
{
    switch (ajob_kind)
    {
    case 1:
        if (!attempt_anger_job_kill_creatures(creatng))
            break;
        return true;
    case 2:
        if (!attempt_anger_job_destroy_rooms(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatrDestroyRooms, 500, 1);
        return true;
    case 4:
        if (!attempt_anger_job_leave_dungeon(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatureLeaving, 500, 1);
        return true;
    case 8:
        if (!attempt_anger_job_steal_gold(creatng))
            break;
        return true;
    case 16:
        if (!attempt_anger_job_damage_walls(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatrDestroyRooms, 500, 1);
        return true;
    case 32:
        if (!attempt_anger_job_mad_psycho(creatng))
            break;
        return true;
    case 64:
        if (!attempt_anger_job_persuade(creatng)) {
            // If can't init persuade, then leave alone
            if (!attempt_anger_job_leave_dungeon(creatng))
                break;
            if (is_my_player_number(creatng->owner))
                output_message(SMsg_CreatureLeaving, 500, 1);
        }
        return true;
    case 128:
        if (!attempt_anger_job_join_enemy(creatng))
            break;
        return true;
    default:
        break;
    }
    return false;
}

TbBool creature_find_and_perform_anger_job(struct Thing *creatng)
{
    //return _DK_creature_find_and_perform_anger_job(creatng);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    int i, k, n;
    // Count the amount of jobs set
    i = 0;
    k = crstat->jobs_anger;
    while (k != 0)
    {
        if ((k & 1) != 0)
            i++;
        k >>= 1;
    }
    if (i <= 0) {
        return false;
    }
    // Select a random job as a starting point
    n = ACTION_RANDOM(i) + 1;
    i = 0;
    for (k = 0; k < crtr_conf.angerjobs_count; k++)
    {
        if ((crstat->jobs_anger & (1 << k)) != 0) {
            n--;
        }
        if (n <= 0) {
            i = k;
            break;
        }
    }
    // Go through all jobs, starting at randomly selected one, attempting to start each one
    for (k = 0; k < crtr_conf.angerjobs_count; k++)
    {
        if ((crstat->jobs_anger & (1 << i)) != 0)
        {
          if (attempt_anger_job(creatng, 1 << i))
              return 1;
        }
        i = (i+1) % crtr_conf.angerjobs_count;
    }
    return 0;
}

/** Returns if a creature will refuse to do specific job.
 *
 * @param creatng The creature which is planned for the job.
 * @param jobpref The job to be checked.
 * @return
 */
TbBool creature_will_reject_job(const struct Thing *creatng, CreatureJob jobpref)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return (jobpref & crstat->jobs_not_do) != 0;
}

TbBool is_correct_owner_to_perform_job(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // Note that room required for job is not checked here on purpose.
    // We need to check for it later in upper function, because lack of related room may generate message for the player
    if (creatng->owner == plyr_idx)
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            if ((get_flags_for_job(new_job) & JoKF_OwnedDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(new_job) & JoKF_OwnedCreatures) == 0)
                return false;
        }
    } else
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            if ((get_flags_for_job(new_job) & JoKF_EnemyDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(new_job) & JoKF_EnemyCreatures) == 0)
                return false;
        }
    }
    return true;
}

TbBool is_correct_position_to_perform_job(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    const struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    const struct Room *room;
    room = subtile_room_get(stl_x, stl_y);
    RoomKind job_rkind;
    job_rkind = get_room_for_job(new_job);
    if (job_rkind != RoK_NONE)
    {
        if (room_is_invalid(room)) {
            return false;
        }
        if (room->kind != job_rkind) {
            return false;
        }
    }
    if (!is_correct_owner_to_perform_job(creatng, slabmap_owner(slb), new_job)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_job_always_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return true;
}

TbBool creature_can_do_training_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_be_trained(creatng) && player_can_afford_to_train_creature(creatng);
}

TbBool creature_can_do_research_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_research(creatng);
}

TbBool creature_can_do_manufacturing_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_manufacturing(creatng);
}

TbBool creature_can_do_scavenging_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_scavenging(creatng) && player_can_afford_to_scavenge_creature(creatng);
}

TbBool creature_can_freeze_prisoners_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // To freeze prisoners, our prison can't be empty
    struct Room *room;
    room = find_room_for_thing_with_used_capacity(creatng, creatng->owner, get_room_for_job(Job_FREEZE_PRISONERS), NavRtF_Default, 1);
    return creature_instance_is_available(creatng, CrInst_FREEZE) && !room_is_invalid(room);
}

TbBool creature_can_join_fight_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct Event *event;
    event = get_event_of_type_for_player(EvKind_EnemyFight, creatng->owner);
    if (event_is_invalid(event)) {
        event = get_event_of_type_for_player(EvKind_HeartAttacked, creatng->owner);
    }
    return !event_is_invalid(event);
}

TbBool creature_can_do_barracking_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // Grouping or barracking only makes sense if we have more than one creature
    const struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->num_active_creatrs > 1);
}

/** Returns if a creature can do specific job for the player.
 *
 * @param creatng The creature which is planned for the job.
 * @param plyr_idx Player for whom the job is to be done.
 * @param new_job Job selection with single job flag set.
 * @param flags Function behavior adjustment flags.
 * @return True if the creature can do the job specified, false otherwise.
 * @note this should be used instead of person_will_do_job_for_room()
 * @note this function will never change state of the input thing, even if appropriate flags are set
 * @see creature_can_do_job_near_position() similar function for use when target position is known
 */
TbBool creature_can_do_job_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job, unsigned long flags)
{
    SYNCDBG(16,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    if (creature_will_reject_job(creatng, new_job))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; in not do jobs list",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!is_correct_owner_to_perform_job(creatng, plyr_idx, new_job))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; not correct owner for job",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Don't allow creatures changed to chickens to have any job assigned, besides those specifically marked
    if (creature_affected_by_spell(creatng, SplK_Chicken) && ((get_flags_for_job(new_job) & JoKF_AllowChickenized) == 0))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; under chicken spell",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Check if the job is related to correct player
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(new_job);
    if (jobcfg->func_plyr_check == NULL)
    {
        return false;
    }
    if (!jobcfg->func_plyr_check(creatng, plyr_idx, new_job))
    {
        return false;
    }
    RoomKind job_rkind;
    job_rkind = get_room_for_job(new_job);
    if (job_rkind != RoK_NONE)
    {
        if (!player_has_room(plyr_idx, job_rkind))
        {
            SYNCDBG(3,"Cannot assign %s in player %d room for %s index %d owner %d; no required room built",creature_job_code_name(new_job),(int)plyr_idx,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            if ((flags & JobChk_PlayMsgOnFail) != 0) {
                const struct RoomConfigStats *roomst;
                roomst = get_room_kind_stats(get_room_for_job(new_job));
                if (is_my_player_number(plyr_idx) && (roomst->msg_needed > 0)) {
                    output_message_room_related_from_computer_or_player_action(roomst->msg_needed);
                }
            }
            return false;
        }
        if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0)
        {
            struct Room *room;
            room = find_room_with_spare_capacity(plyr_idx, job_rkind, 1);
            if (room_is_invalid(room))
            {
                SYNCDBG(3,"Cannot assign %s in player %d room for %s index %d owner %d; not enough room capacity",creature_job_code_name(new_job),(int)plyr_idx,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                if ((flags & JobChk_PlayMsgOnFail) != 0) {
                    const struct RoomConfigStats *roomst;
                    roomst = get_room_kind_stats(get_room_for_job(new_job));
                    if (is_my_player_number(plyr_idx) && (roomst->msg_too_small > 0)) {
                        output_message_room_related_from_computer_or_player_action(roomst->msg_too_small);
                    }
                }
                return false;
            }
        }
    }
    return true;
}

TbBool send_creature_to_job_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    SYNCDBG(6,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(new_job);
    if (jobcfg->func_plyr_assign != NULL)
    {
        if (jobcfg->func_plyr_assign(creatng, plyr_idx, new_job))
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(creatng);
            // Set computer control accordingly to job flags
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->flgfield_1 |= CCFlg_NoCompControl;
            } else {
                cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
            }
            // If a new task isn't a work-in-group thing, remove the creature from group
            if ((get_flags_for_job(new_job) & JoKF_NoGroups) != 0)
            {
                if (creature_is_group_member(creatng)) {
                    remove_creature_from_group(creatng);
                }
            }
            return true;
        }
    } else
    {
        ERRORLOG("Cannot start %s for %s (owner %d); job has no player-based assign",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->owner);
    }
    return false;
}

TbBool creature_can_do_job_always_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    return true;
}

TbBool creature_can_do_research_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_research(creatng))
    {
        struct Room *room;
        room = subtile_room_get(stl_x, stl_y);
        struct Dungeon *dungeon;
        dungeon = get_dungeon(room->owner);
        if (!is_neutral_thing(creatng) && (dungeon->current_research_idx < 0))
        {
            if (is_my_player_number(dungeon->owner) && ((flags & JobChk_PlayMsgOnFail) != 0)) {
                output_message(SMsg_NoMoreReseach, 500, true);
            }
        }
        return false;
    }
    return true;
}

TbBool creature_can_do_training_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_be_trained(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_manufacturing_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_manufacturing(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_scavenging_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_scavenging(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_place_in_vault_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (creatng->creature.gold_carried < 1) {
        return false;
    }
    return true;
}

TbBool creature_can_take_salary_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    struct Room *room;
    room = subtile_room_get(stl_x, stl_y);
    if (room->used_capacity < 1) {
        return false;
    }
    return true;
}

TbBool creature_can_take_sleep_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return false;
    }
    return true;
}

 /** Returns if a creature can do specific job at given map position.
 *
 * @param creatng The creature which is planned for the job.
 * @param stl_x Target map position, x coord.
 * @param stl_y Target map position, y coord.
 * @param new_job Job selection with single job flag set.
 * @return True if the creature can do the job specified, false otherwise.
 * @see creature_can_do_job_for_player() similar function for use when only target player is known
 */
TbBool creature_can_do_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(6,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if (creature_will_reject_job(creatng, new_job))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; in not-do-jobs list",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        if ((flags & JobChk_SetStateOnFail) != 0) {
            anger_apply_anger_to_creature(creatng, crstat->annoy_will_not_do_job, AngR_Other, 1);
            external_set_thing_state(creatng, CrSt_CreatureMoan);
            cctrl->field_282 = 50;
        }
        return false;
    }
    // Don't allow creatures changed to chickens to have any job assigned, besides those specifically marked
    if (creature_affected_by_spell(creatng, SplK_Chicken) && ((get_flags_for_job(new_job) & JoKF_AllowChickenized) == 0))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; under chicken spell",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Check if the job is related to correct map place (room,slab)
    if (!is_correct_position_to_perform_job(creatng, stl_x, stl_y, new_job))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; not correct place for job",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(new_job);
    if (jobcfg->func_cord_check == NULL)
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; job has no coord check function",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!jobcfg->func_cord_check(creatng, stl_x, stl_y, new_job, flags))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; coord check not passed",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // If other tests pass, check if related room (if is needed) has capacity to be used for that job
    if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0)
    {
        struct Room *room;
        room = subtile_room_get(stl_x, stl_y);
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; not enough room capacity",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            if ((flags & JobChk_PlayMsgOnFail) != 0) {
                const struct RoomConfigStats *roomst;
                roomst = get_room_kind_stats(room->kind);
                if (is_my_player_number(room->owner) && (roomst->msg_too_small > 0)) {
                    output_message_room_related_from_computer_or_player_action(roomst->msg_too_small);
                }
            }
            return false;
        }
    }
    return true;
}

TbBool send_creature_to_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    SYNCDBG(6,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(new_job);
    if (jobcfg->func_cord_assign != NULL)
    {
        if (jobcfg->func_cord_assign(creatng, stl_x, stl_y, new_job))
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(creatng);
            // Set computer control accordingly to job flags
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->flgfield_1 |= CCFlg_NoCompControl;
            } else {
                cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
            }
            // If a new task isn't a work-in-group thing, remove the creature from group
            if ((get_flags_for_job(new_job) & JoKF_NoGroups) != 0)
            {
                if (creature_is_group_member(creatng)) {
                    remove_creature_from_group(creatng);
                }
            }
            return true;
        }
    } else
    {
        ERRORLOG("Cannot start %s for %s (owner %d); job has no coord-based assign",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->owner);
    }
    return false;
}

/**
 *
 * @param creatng
 * @param plyr_idx
 * @param rkind
 * @return
 * @deprecated Room does not correspond to single job anymore, this should be avoided.
 */
TbBool creature_can_do_job_for_computer_player_in_room(const struct Thing *creatng, PlayerNumber plyr_idx, RoomKind rkind)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    CreatureJob jobpref;
    jobpref = get_job_for_room(rkind, JoKF_AssignComputerDropInRoom, crstat->job_primary|crstat->job_secondary);
    return creature_can_do_job_for_player(creatng, plyr_idx, jobpref, JobChk_None);
}

TbBool attempt_job_work_in_room_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct Coord3d pos;
    struct Room *room;
    RoomKind rkind;
    rkind = get_room_for_job(new_job);
    SYNCDBG(6,"Starting for %s (owner %d) and job %s in %s room",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job),room_code_name(rkind));
    if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0) {
        room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, rkind, NavRtF_Default, 1);
    } else {
        room = find_nearest_room_for_thing(creatng, creatng->owner, rkind, NavRtF_Default);
    }
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if (!setup_person_move_to_coord(creatng, &pos, NavRtF_Default)) {
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_job_work_in_room_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct Room *room;
    room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if (!creature_move_to_place_in_room(creatng, room, new_job)) {
        return false;
    }
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    if ((new_job == Job_TRAIN) && (creatng->model == get_players_special_digger_model(room->owner))) {
        cctrl->digger.last_did_job = SDLstJob_UseTraining4;
    }
    return true;
}

TbBool attempt_job_work_in_room_and_cure_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct Room *room;
    room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    struct Coord3d pos;
    if (!find_first_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    process_temple_cure(creatng);
    return true;
}

TbBool attempt_job_sleep_in_lair_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s (owner %d) and job %s",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(new_job));
    struct Room *room;
    room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    cctrl->slap_turns = 0;
    cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
    if (creature_has_lair_room(creatng) && (room->index == cctrl->lair_room_id))
    {
        if (creature_move_to_home_lair(creatng))
        {
            creatng->continue_state = CrSt_CreatureGoingHomeToSleep;
            return 1;
        }
    }
    struct Coord3d pos;
    if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
      && setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
    {
        creatng->continue_state = CrSt_CreatureChangeLair;
        cctrl->target_room_id = room->index;
        return 1;
    }
    return 0;
}

TbBool attempt_job_in_state_on_room_content_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct Room *room;
    RoomKind rkind;
    rkind = get_room_for_job(new_job);
    room = find_room_for_thing_with_used_capacity(creatng, creatng->owner, rkind, NavRtF_Default, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    internal_set_thing_state(creatng, get_initial_state_for_job(new_job));
    return true;
}

TbBool attempt_job_move_to_event_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    EventKind evkind;
    struct Event *event;
    evkind = get_event_for_job(new_job);
    event = get_event_of_type_for_player(evkind, creatng->owner);
    // Treat heart attack as enemy fight, too
    if (event_is_invalid(event) && (evkind == EvKind_EnemyFight)) {
        event = get_event_of_type_for_player(EvKind_HeartAttacked, creatng->owner);
    }
    if (event_is_invalid(event)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, coord_subtile(event->mappos_x), coord_subtile(event->mappos_y), NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = get_initial_state_for_job(new_job);
    return true;
}

TbBool attempt_job_in_state_internal_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    CrtrStateId crstate;
    crstate = get_initial_state_for_job(new_job);
    cctrl = creature_control_get_from_thing(creatng);
    internal_set_thing_state(creatng, crstate);
    // Some states need additional initialization
    if (crstate == CrSt_SeekTheEnemy) {
        cctrl->word_9A = 0;
    }
    return true;
}

TbBool attempt_job_in_state_internal_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    return attempt_job_in_state_internal_for_player(creatng, creatng->owner, new_job);
}

/**
 * Tries to assign one of creature's preferred jobs to it.
 * Starts at random job, to make sure all the jobs have equal chance of being selected.
 * @param creatng The creature to assign a job to.
 * @param jobpref Job preference flags.
 */
TbBool attempt_job_preference(struct Thing *creatng, long jobpref)
{
    //return _DK_attempt_job_preference(creatng, jobpref);
    long i,n;
    // Start checking at random job
    if (crtr_conf.jobs_count < 1) {
        return false;
    }
    n = ACTION_RANDOM(crtr_conf.jobs_count);
    for (i=0; i < crtr_conf.jobs_count; i++, n = (n+1)%crtr_conf.jobs_count)
    {
        if (n == 0)
            continue;
        CreatureJob new_job;
        new_job = 1 << (n-1);
        if ((jobpref & new_job) != 0)
        {
            SYNCDBG(19,"Check job %s",creature_job_code_name(new_job));
            if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
            {
                if (send_creature_to_job_for_player(creatng, creatng->owner, new_job)) {
                    return true;
                }
            }
        }
    }
    return false;
}

TbBool attempt_job_secondary_preference(struct Thing *creatng, long jobpref)
{
    long i;
    unsigned long k;
    // Count the amount of jobs set
    i = 0;
    k = jobpref;
    while (k)
    {
        k >>= 1;
        i++;
    }
    if (i <= 0) {
        return false;
    }
    unsigned long select_val,select_curr,select_delta;
    select_val = ACTION_RANDOM(512);
    select_delta = 512 / i;
    select_curr = select_delta;
    // For some reason, this is a bit different than attempt_job_preference().
    // Probably needs unification
    for (i=1; i < crtr_conf.jobs_count; i++)
    {
        CreatureJob new_job = 1<<(i-1);
        if ((jobpref & new_job) == 0) {
            continue;
        }
        SYNCDBG(19,"Check job %s",creature_job_code_name(new_job));
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
        {
            if (send_creature_to_job_for_player(creatng, creatng->owner, new_job)) {
                return true;
            }
        }
    }
    // If no job, give 1% chance of going to temple
    if (ACTION_RANDOM(100) == 0)
    {
        CreatureJob new_job = Job_TEMPLE_PRAY;
        if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
        {
            if (send_creature_to_job_for_player(creatng, creatng->owner, new_job)) {
                return true;
            }
        }
    }
    return 0;
}

TbBool creature_try_doing_secondary_job(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (game.play_gameturn - cctrl->job_secondary_check_turn <= 128) {
        return false;
    }
    cctrl->job_secondary_check_turn = game.play_gameturn;
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return attempt_job_secondary_preference(creatng, crstat->job_secondary);
}
/******************************************************************************/
