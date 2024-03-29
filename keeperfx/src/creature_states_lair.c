/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_lair.c
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
#include "creature_states_lair.h"
#include "globals.h"

#include "bflib_math.h"
#include "thing_physics.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_states_mood.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_lair.h"
#include "room_util.h"
#include "engine_arrays.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_lair_to_sleep(struct Thing *creatng);
DLLIMPORT short _DK_cleanup_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_going_home_to_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_at_changed_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_at_new_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_change_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_choose_room_for_lair_site(struct Thing *creatng);
DLLIMPORT long _DK_creature_add_lair_to_room(struct Thing *creatng, struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns if given creature is able to heal by sleeping.
 * Does not take into consideration if the creature has a lair, checks only if
 * the creature model is able to heal in its lair in general.
 * @param creatng
 * @return
 */
TbBool creature_can_do_healing_sleep(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return ((crstat->heal_requirement > 0) && (crstat->lair_size > 0));
}

TbBool creature_is_sleeping(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if ((i == CrSt_CreatureSleep))
        return true;
    return false;
}

TbBool creature_is_doing_lair_activity(const struct Thing *thing)
{
    CrtrStateId i;
    i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureSleep) || (i == CrSt_CreatureGoingHomeToSleep) || (i == CrSt_AtLairToSleep)
      || (i == CrSt_CreatureChooseRoomForLairSite) || (i == CrSt_CreatureAtNewLair) || (i == CrSt_CreatureWantsAHome)
      || (i == CrSt_CreatureChangeLair) || (i == CrSt_CreatureAtChangedLair))
        return true;
    return false;
}

TbBool creature_requires_healing(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    long maxhealth, minhealth;
    maxhealth = compute_creature_max_health(crstat->health,cctrl->explevel);
    minhealth = crstat->heal_requirement * maxhealth / 256;
    if ((long)thing->health <= minhealth)
        return true;
    return false;
}

TbBool creature_move_to_home_lair(struct Thing *creatng)
{
    if (!creature_has_lair_room(creatng)) {
        return false;
    }
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    cctrl = creature_control_get_from_thing(creatng);
    lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_is_invalid(lairtng)) {
        return false;
    }
    return setup_person_move_to_coord(creatng, &lairtng->mappos, NavRtF_Default);

}

long creature_will_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    long dist_x,dist_y;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
    if (thing_is_invalid(lairtng))
        return false;
    dist_x = (long)thing->mappos.x.stl.num - (long)lairtng->mappos.x.stl.num;
    dist_y = (long)thing->mappos.y.stl.num - (long)lairtng->mappos.y.stl.num;
    return (abs(dist_x) < 1) && (abs(dist_y) < 1);
}

long process_lair_enemy(struct Thing *thing, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *enemytng;
    long combat_factor;
    cctrl = creature_control_get_from_thing(thing);
    // Shouldn't be possible. But just for sure.
    if (room_is_invalid(room))
    {
        return 0;
    }
    // If the room changed during creature's journey, end
    if ((room->kind != RoK_LAIR) || (room->owner != thing->owner) || (room->index != cctrl->lair_room_id))
    {
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    // End if the creature has no lair enemy
    if (crstat->lair_enemy == 0)
    {
        return 0;
    }
    // Search for enemies no often than every 64 turns
    if (((game.play_gameturn + thing->index) & 0x3F) != 0)
    {
        return 0;
    }
    combat_factor = find_fellow_creature_to_fight_in_room(thing,room,crstat->lair_enemy,&enemytng);
    if (combat_factor < 1)
        return 0;
    if (!set_creature_in_combat_to_the_death(thing, enemytng, combat_factor))
        return 0;
    return 1;
}

long creature_add_lair_to_room(struct Thing *creatng, struct Room *room)
{
    struct Thing *lairtng;
    if (!room_has_enough_free_capacity_for_creature(room, creatng))
        return 0;
    //return _DK_creature_add_lair_to_room(thing, room);
    // Make sure we don't already have a lair on that position
    lairtng = find_creature_lair_at_subtile(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, 0);
    if (!thing_is_invalid(lairtng))
        return 0;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    crstat = creature_stats_get_from_thing(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    room->content_per_model[creatng->model]++;
    room->used_capacity += crstat->lair_size;
    if ((cctrl->lair_room_id > 0) && (cctrl->lairtng_idx > 0))
    {
        struct Room *room;
        room = room_get(cctrl->lair_room_id);
        creature_remove_lair_from_room(creatng, room);
    }
    cctrl->lair_room_id = room->index;
    // Create the lair thing
    struct CreatureData *crdata;
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    crdata = creature_data_get_from_thing(creatng);
    lairtng = create_object(&pos, crdata->field_1, creatng->owner, -1);
    if (thing_is_invalid(lairtng))
    {
        ERRORLOG("Could not create lair totem");
        remove_thing_from_mapwho(creatng);
        place_thing_in_mapwho(creatng);
        return 1; // Return that so we won't try to redo the action over and over
    }
    lairtng->mappos.z.val = get_thing_height_at(lairtng, &lairtng->mappos);
    // Associate creature with the lair
    cctrl->lairtng_idx = lairtng->index;
    lairtng->word_13 = creatng->index;
    lairtng->word_15 = 1;
    // Lair size depends on creature level
    lairtng->word_17 = 300 * cctrl->explevel / 20 + 300;
    lairtng->field_52 = ACTION_RANDOM(0x800);
    struct Objects *objdat;
    unsigned long i;
    objdat = get_objects_data_for_thing(lairtng);
    i = convert_td_iso(objdat->field_5);
    set_thing_draw(lairtng, i, objdat->field_7, lairtng->word_15, 0, -1, objdat->field_11);
    thing_play_sample(creatng, 158, NORMAL_PITCH, 0, 3, 1, 2, FULL_LOUDNESS);
    create_effect(&pos, imp_spangle_effects[creatng->owner], creatng->owner);
    anger_set_creature_anger(creatng, 0, AngR_NoLair);
    remove_thing_from_mapwho(creatng);
    place_thing_in_mapwho(creatng);
    return 1;
}

CrStateRet creature_at_changed_lair(struct Thing *creatng)
{
    struct Room *room;
    TRACE_THING(creatng);
    //return _DK_creature_at_changed_lair(thing);
    if (!thing_is_on_own_room_tile(creatng))
    {
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    room = get_room_thing_is_on(creatng);
    if (!room_initially_valid_as_type_for_thing(room, RoK_LAIR, creatng))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng));
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    if (!creature_add_lair_to_room(creatng, room)) {
        internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return CrStRet_Modified;
    }
    // All done - finish the state
    set_start_state(creatng);
    return CrStRet_ResetOk;
}

CrStateRet creature_at_new_lair(struct Thing *creatng)
{
    struct Room *room;
    TRACE_THING(creatng);
    //return _DK_creature_at_new_lair(thing);
    room = get_room_thing_is_on(creatng);
    if ( !room_still_valid_as_type_for_thing(room, RoK_LAIR, creatng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->owner);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    if (!creature_add_lair_to_room(creatng, room))
    {
        internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return CrStRet_Modified;
    }
    set_start_state(creatng);
    return CrStRet_ResetOk;
}

short creature_change_lair(struct Thing *thing)
{
    TRACE_THING(thing);
    return _DK_creature_change_lair(thing);
}

TbBool setup_head_for_random_unused_lair_slab(struct Thing *creatng, struct Room *room)
{
    SlabCodedCoords slbnum;
    long n;
    unsigned long k;
    n = ACTION_RANDOM(room->slabs_count);
    slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    k = 0;
    while (1)
    {
        MapSlabCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(slbnum);
        slb_y = slb_num_decode_y(slbnum);
        struct Thing *lairtng;
        lairtng = find_creature_lair_at_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), 0);
        if (thing_is_invalid(lairtng))
        {
            if (setup_person_move_to_position(creatng, slab_subtile_center(slb_x), slab_subtile_center(slb_y), NavRtF_Default)) {
                return true;
            }
            WARNLOG("Cannot get somewhere in room");
        }
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
        k++;
        if (k >= room->slabs_count) {
            break;
        }
    }
    return false;
}

short creature_choose_room_for_lair_site(struct Thing *thing)
{
    TRACE_THING(thing);
    //return _DK_creature_choose_room_for_lair_site(thing);
    struct Room *room;
    room = get_best_new_lair_for_creature(thing);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_LAIR);
        set_start_state(thing);
        return 0;
    }
    if (!setup_head_for_random_unused_lair_slab(thing, room))
    {
        ERRORLOG("Chosen lair is not valid, internal inconsistency.");
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_CreatureAtNewLair;
    return 1;
}

short at_lair_to_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    struct Room *room;
    TRACE_THING(thing);
    //return _DK_at_lair_to_sleep(thing);
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
    cctrl->target_room_id = 0;
    if (thing_is_invalid(lairtng) || (cctrl->slap_turns != 0))
    {
        set_start_state(thing);
        return 0;
    }
    if (!creature_will_sleep(thing))
    {
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_LAIR, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ((cctrl->lair_room_id != room->index))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !creature_turn_to_face_angle(thing, lairtng->field_52) )
    {
        internal_set_thing_state(thing, CrSt_CreatureSleep);
        cctrl->field_82 = 200;
        thing->movement_flags &= ~TMvF_Flying;
    }
    process_lair_enemy(thing, room);
    return 1;
}

short cleanup_sleep(struct Thing *creatng)
{
    //return _DK_cleanup_sleep(creatng);
    restore_creature_flight_flag(creatng);
    return 1;
}

short creature_going_home_to_sleep(struct Thing *thing)
{
  return _DK_creature_going_home_to_sleep(thing);
}

long room_has_slab_adjacent(const struct Room *room, long slbkind)
{
    long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        // Per room tile code
        long n;
        for (n=0; n < AROUND_SLAB_LENGTH; n++)
        {
            long slab_num;
            slab_num = i + around_slab[n];
            struct SlabMap *slb;
            slb = get_slabmap_direct(slab_num);
            if (!slabmap_block_invalid(slb))
            {
                if (slb->kind == slbkind) {
                    return true;
                }
            }
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    return 0;
}

short creature_sleep(struct Thing *thing)
{
    //return _DK_creature_sleep(thing);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->slap_turns > 0) || !creature_will_sleep(thing)) {
        set_start_state(thing);
        return 0;
    }
    struct Room *room;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (room->kind != RoK_LAIR)
        || (cctrl->lair_room_id != room->index) || (room->owner != thing->owner)) {
        set_start_state(thing);
        return 0;
    }
    thing->movement_flags &= ~0x0020;
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    if (((game.play_gameturn + thing->index) % game.recovery_frequency) == 0)
    {
        HitPoints recover;
        recover = compute_creature_max_health(crstat->sleep_recovery, cctrl->explevel);
        apply_health_to_thing_and_display_health(thing, recover);
    }
    anger_set_creature_anger(thing, 0, AngR_NoLair);
    anger_apply_anger_to_creature(thing, crstat->annoy_sleeping, AngR_Other, 1);
    if (cctrl->field_82 > 0) {
        cctrl->field_82--;
    }
    if (((game.play_gameturn + thing->index) & 0x3F) == 0)
    {
        if (ACTION_RANDOM(100) < 5) {
            struct Dungeon *dungeon;
            dungeon = get_dungeon(thing->owner);
            dungeon->lvstats.backs_stabbed++;
        }
    }
    if (crstat->sleep_exp_slab != SlbT_ROCK)
    {
        if (creature_can_gain_experience(thing) && room_has_slab_adjacent(room, crstat->sleep_exp_slab))
        {
            cctrl->exp_points += crstat->sleep_experience;
            check_experience_upgrade(thing);
        }
    }
    {
        HitPoints health_max;
        health_max = compute_creature_max_health(crstat->health, cctrl->explevel);
        if ((crstat->heal_threshold * health_max / 256 <= thing->health) && (!cctrl->field_82))
        {
            set_start_state(thing);
            return 1;
        }
    }
    process_lair_enemy(thing, room);
    return 0;
}

/******************************************************************************/
