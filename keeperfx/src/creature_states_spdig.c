/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_spdig.c
 *     Creature state machine functions for special diggers (imps).
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
#include "creature_states_spdig.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "thing_physics.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_corpses.h"
#include "thing_traps.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "room_library.h"
#include "room_graveyard.h"
#include "room_util.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "power_hand.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

/** Effects used when creating new imps. Every player color has different index. */
const int birth_effect_element[] = { 54, 79, 80, 81, 82, 82, };
const unsigned char reinforce_edges[] = { 3, 0, 0, 3, 0, 1, 2, 2, 1, };

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_imp_arrives_at_convert_dungeon(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_arrives_at_dig_or_mine(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_arrives_at_improve_dungeon(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_arrives_at_reinforce(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_birth(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_converts_dungeon(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_digs_mines(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_doing_nothing(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_drops_gold(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_improves_dungeon(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_last_did_job(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_picks_up_gold_pile(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_reinforces(struct Thing *spdigtng);
DLLIMPORT short _DK_imp_toking(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_pick_up_unconscious_body(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_picks_up_corpse(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_picks_up_spell_object(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_picks_up_trap_for_workshop(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_picks_up_trap_object(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_drops_corpse_in_graveyard(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_drops_crate_in_workshop(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_drops_spell_object_in_library(struct Thing *spdigtng);
DLLIMPORT short _DK_creature_arms_trap(struct Thing *spdigtng);
DLLIMPORT long _DK_check_out_unclaimed_unconscious_bodies(struct Thing *spdigtng, long a1);
DLLIMPORT long _DK_check_out_unclaimed_dead_bodies(struct Thing *spdigtng, long a1);
DLLIMPORT long _DK_check_out_unclaimed_spells(struct Thing *spdigtng, long a1);
DLLIMPORT long _DK_check_out_unclaimed_traps(struct Thing *spdigtng, long a1);
DLLIMPORT long _DK_check_out_unconverted_drop_place(struct Thing *spdigtng);
DLLIMPORT long _DK_check_out_undug_drop_place(struct Thing *spdigtng);
DLLIMPORT long _DK_check_out_unprettied_drop_place(struct Thing *spdigtng);
DLLIMPORT long _DK_check_out_unclaimed_gold(struct Thing *spdigtng, long a1);
DLLIMPORT long _DK_check_out_object_for_trap(struct Thing *traptng, struct Thing *spdigtng);
DLLIMPORT struct Thing *_DK_check_for_empty_trap_for_imp(struct Thing *traptng, long plyr_idx);
DLLIMPORT long _DK_take_from_gold_pile(unsigned short stl_x, unsigned short stl_y, long limit);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *check_for_empty_trap_for_imp(struct Thing *spdigtng, long tngmodel)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    TRACE_THING(spdigtng);
    //return _DK_check_for_empty_trap_for_imp(spdigtng, tngmodel);
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->model == tngmodel) && (thing->trap.num_shots == 0) && (thing->owner == spdigtng->owner))
        {
            if ( !imp_will_soon_be_arming_trap(thing) ) {
                return thing;
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

long check_out_unclaimed_unconscious_bodies(struct Thing *spdigtng, long range)
{
    //return _DK_check_out_unclaimed_unconscious_bodies(spdigtng, range);
    if (!player_has_room(spdigtng->owner, RoK_PRISON)) {
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(spdigtng);
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_capacity(spdigtng, spdigtng->owner, RoK_PRISON, NavRtF_Default, 1);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    long i;
    unsigned long k;
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (((thing->field_1 & TF1_IsDragged1) == 0) && (thing->owner != spdigtng->owner)
          && thing_revealed(thing, spdigtng->owner) && creature_is_being_unconscious(thing))
        {
            if ((range < 0) || get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                {
                    // We have a thing which we should pick - now check if the room we found is correct
                    if (room_is_invalid(room)) {
                        update_cannot_find_room_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoK_PRISON);
                        return 0;
                    }
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                        spdigtng->continue_state = CrSt_CreaturePickUpUnconsciousBody;
                        cctrl->pickup_creature_id = thing->index;
                        return 1;
                    }
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
    return 0;
}

long check_out_unclaimed_dead_bodies(struct Thing *spdigtng, long range)
{
    //return _DK_check_out_unclaimed_dead_bodies(spdigtng, range);
    if (!player_has_room(spdigtng->owner, RoK_GRAVEYARD)) {
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(spdigtng);
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_capacity(spdigtng, spdigtng->owner, RoK_GRAVEYARD, NavRtF_Default, 1);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_DeadCreature);
    long i;
    unsigned long k;
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (((thing->field_1 & TF1_IsDragged1) == 0) && (thing->active_state == DCrSt_Unknown02)
         && (thing->byte_14 == 0) && thing_revealed(thing, spdigtng->owner) && corpse_is_rottable(thing))
        {
            if ((range < 0) || get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                {
                    // We have a thing which we should pick - now check if the room we found is correct
                    if (room_is_invalid(room)) {
                        update_cannot_find_room_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoK_GRAVEYARD);
                        return 0;
                    }
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                        SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                            thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                        spdigtng->continue_state = CrSt_CreaturePicksUpCorpse;
                        cctrl->pickup_object_id = thing->index;
                        return 1;
                    }
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
    return 0;
}

long check_out_unclaimed_spells(struct Thing *spdigtng, long range)
{
    //return _DK_check_out_unclaimed_spells(spdigtng, range);
    if (!player_has_room(spdigtng->owner, RoK_LIBRARY)) {
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(spdigtng);
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_item_capacity(spdigtng, spdigtng->owner, RoK_LIBRARY, NavRtF_Default);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    long i;
    unsigned long k;
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_spellbook(thing) || thing_is_special_box(thing))
        {
            struct SlabMap *slb;
            slb = get_slabmap_thing_is_on(thing);
            if ((thing->owner != spdigtng->owner) && ((thing->alloc_flags & TAlF_IsDragged) == 0)
              && (slabmap_owner(slb) == spdigtng->owner) && thing_revealed(thing, spdigtng->owner))
            {
                if ((range < 0) || get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range)
                {
                    if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                    {
                        // We have a thing which we should pick - now check if the room we found is correct
                        if (room_is_invalid(room)) {
                            update_cannot_find_room_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoK_LIBRARY);
                            return 0;
                        }
                        if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                            SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                                thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                            if (thing_is_spellbook(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_SpellPickedUp, spdigtng->owner, thing->index);
                            } else
                            if (thing_is_special_box(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_DnSpecialFound, spdigtng->owner, thing->index);
                            }
                            spdigtng->continue_state = CrSt_CreaturePicksUpSpellObject;
                            cctrl->pickup_object_id = thing->index;
                            return 1;
                        }
                    }
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
    return 0;
}

long check_out_unclaimed_traps(struct Thing *spdigtng, long range)
{
    //return _DK_check_out_unclaimed_traps(spdigtng, range);
    if (!player_has_room(spdigtng->owner, RoK_WORKSHOP)) {
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(spdigtng);
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_item_capacity(spdigtng, spdigtng->owner, RoK_WORKSHOP, NavRtF_Default);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    long i;
    unsigned long k;
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_door_or_trap_crate(thing) && ((thing->field_1 & TF1_IsDragged1) == 0))
        {
            struct SlabMap *slb;
            slb = get_slabmap_thing_is_on(thing);
            if ((slabmap_owner(slb) == spdigtng->owner) && thing_revealed(thing, spdigtng->owner))
            {
                if ((range < 0) || get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range)
                {
                    if (!imp_will_soon_be_getting_object(spdigtng->owner, thing))
                    {
                        // If there is a trap to arm, go arming
                        struct Thing *traptng;
                        if (thing_is_trap_crate(thing)) {
                            traptng = check_for_empty_trap_for_imp(spdigtng, crate_to_workshop_item_model(thing->model));
                        } else {
                            traptng = INVALID_THING;
                        }
                        if (!thing_is_invalid(traptng))
                        {
                            if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                            {
                                spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                                cctrl->pickup_object_id = thing->index;
                                cctrl->arming_thing_id = traptng->index;
                                return 1;
                            }
                        }
                        // No trap to arm - get the crate in workshop, if it's not already on it
                        struct Room * onroom;
                        onroom = get_room_thing_is_on(thing);
                        if (room_is_invalid(onroom) || (onroom->kind != RoK_WORKSHOP))
                        {
                            // We have a thing which we should pick - now check if the room we found is correct
                            if (room_is_invalid(room)) {
                                update_cannot_find_room_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoK_WORKSHOP);
                                return 0;
                            }
                            if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                            {
                                SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                                    thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                                if (thing_is_trap_crate(thing))
                                {
                                    event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                        EvKind_TrapCrateFound, spdigtng->owner, thing->index);
                                } else
                                if (thing_is_door_crate(thing))
                                {
                                    event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                        EvKind_DoorCrateFound, spdigtng->owner, thing->index);
                                }
                                spdigtng->continue_state = CrSt_CreaturePicksUpCrateForWorkshop;
                                cctrl->pickup_object_id = thing->index;
                                return 1;
                            }
                        }
                    }
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
    return 0;
}

long check_out_unconverted_drop_place(struct Thing *thing)
{
    return _DK_check_out_unconverted_drop_place(thing);
}

long check_out_undug_drop_place(struct Thing *thing)
{
    return _DK_check_out_undug_drop_place(thing);
}

long check_out_unclaimed_gold(struct Thing *spdigtng, long range)
{
    //return _DK_check_out_unclaimed_gold(thing, a1);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(spdigtng);
    // If the creature holds more gold than its able
    if (spdigtng->creature.gold_carried >= crstat->gold_hold) {
        return 0;
    }
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    long i;
    unsigned long k;
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_object(thing) && !thing_is_picked_up(thing) && thing_revealed(thing, spdigtng->owner))
        {
            if (object_is_gold_pile(thing))
            {
                struct SlabMap *slb;
                slb = get_slabmap_thing_is_on(thing);
                if ((slabmap_owner(slb) == spdigtng->owner) || (slabmap_owner(slb) == game.neutral_player_num))
                {
                    if ((range < 0) || get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range)
                    {
                        if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                        {
                            if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                                spdigtng->continue_state = CrSt_ImpPicksUpGoldPile;
                                //cctrl->pickup_object_id = thing->index; -- don't do that; picking up gold destroys the object
                                return 1;
                            }
                        }
                    }
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
    return 0;
}

long check_out_unprettied_drop_place(struct Thing *thing)
{
    return _DK_check_out_unprettied_drop_place(thing);
}

long check_out_object_for_trap(struct Thing *spdigtng, struct Thing *traptng)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    struct SlabMap *slb;
    struct Room *room;
    long find_model,find_owner;
    long i;
    unsigned long k;
    //return _DK_check_out_object_for_trap(spdigtng, traptng);
    cctrl = creature_control_get_from_thing(spdigtng);
    // We're supposed to be in our own workshop; fail if we're not
    room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room)) {
        return 0;
    }
    if ( (room->kind != RoK_WORKSHOP) || (room->owner != spdigtng->owner) ) {
        return 0;
    }
    find_model = trap_crate_object_model(traptng->model);
    find_owner = spdigtng->owner;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == find_model)
        {
            slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            if ( (slabmap_owner(slb) == find_owner) && ((thing->field_1 & TF1_IsDragged1) == 0) )
            {
                if (!imp_will_soon_be_getting_object(find_owner, thing))
                {
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                    {
                        spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                        cctrl->pickup_object_id = thing->index;
                        cctrl->arming_thing_id = traptng->index;
                        return 1;
                    }
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
    return 0;
}

long check_out_empty_traps(struct Thing *spdigtng, long range)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->trap.num_shots == 0) && (thing->owner == spdigtng->owner))
        {
            if ( (range < 0) || (get_2d_box_distance(&thing->mappos, &spdigtng->mappos) < range) )
            {

                if ( !imp_will_soon_be_arming_trap(thing) && check_out_object_for_trap(spdigtng, thing) ) {
                    return 1;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long check_out_unreinforced_drop_place(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    MapSubtlCoord stl_x,stl_y;
    MapSlabCoord slb_x,slb_y;
    long stl_num;
    long pos_x,pos_y;
    long i,n;
    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;
    cctrl = creature_control_get_from_thing(thing);
    n = reinforce_edges[3 * (stl_y % 3) + (stl_x % 3)];
    for (i=0; i < SMALL_AROUND_LENGTH; i++)
    {
        slb_x = subtile_slab_fast(stl_x) + (long)small_around[n].delta_x;
        slb_y = subtile_slab_fast(stl_y) + (long)small_around[n].delta_y;
        if ( check_place_to_reinforce(thing, slb_x, slb_y) > 0 )
        {
            stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
            if ( check_out_uncrowded_reinforce_position(thing, stl_num, &pos_x, &pos_y) )
            {
                if ( setup_person_move_to_position(thing, pos_x, pos_y, NavRtF_Default) )
                {
                    thing->continue_state = CrSt_ImpArrivesAtReinforce;
                    cctrl->digger.working_stl = stl_num;
                    cctrl->digger.byte_93 = 0;
                    return 1;
                }
            }
        }
        n = (n + 1) % SMALL_AROUND_LENGTH;
    }
    return 0;
}

/**
 * Checks if there are crates in room the creature is on, which could be used to re-arm one of players traps.
 * If there are, setups given digger to do the task of re-arming trap.
 * @param spdigtng
 */
TbBool check_out_crates_to_arm_trap_in_room(struct Thing *spdigtng)
{
    struct Thing *thing;
    struct Thing *traptng;
    struct Room *room;
    long i;
    unsigned long k;
    room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room)) {
        return false;
    }
    if ( (room->kind != RoK_WORKSHOP) || (room->owner != spdigtng->owner) ) {
        return false;
    }

    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( thing_is_trap_crate(thing) )
        {
          if ( ((thing->field_1 & TF1_IsDragged1) == 0) && (get_room_thing_is_on(thing) == room) )
          {
              traptng = check_for_empty_trap_for_imp(spdigtng, crate_thing_to_workshop_item_model(thing));
              if (!thing_is_invalid(traptng) && !imp_will_soon_be_getting_object(spdigtng->owner, thing))
              {
                  if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                  {
                      struct CreatureControl *cctrl;
                      cctrl = creature_control_get_from_thing(spdigtng);
                      spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                      cctrl->pickup_object_id = thing->index;
                      cctrl->arming_thing_id = traptng->index;
                      return true;
                  }
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
    return false;
}

/**
 * Checks for a special digger task for a creature in its vicinity.
 * @param spdigtng
 * @note originally was check_out_available_imp_drop_tasks()
 */
long check_out_available_spdigger_drop_tasks(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(spdigtng);
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(spdigtng),(int)spdigtng->index);
    TRACE_THING(spdigtng);

    if ( check_out_unclaimed_unconscious_bodies(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_unclaimed_dead_bodies(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_unclaimed_spells(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_unclaimed_traps(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_empty_traps(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_undug_drop_place(spdigtng) )
    {
        cctrl->digger.last_did_job = SDLstJob_DigOrMine;
        return 1;
    }
    if ( check_out_unconverted_drop_place(spdigtng) )
    {
        cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
        return 1;
    }
    if ( check_out_unprettied_drop_place(spdigtng) )
    {
        cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
        return 1;
    }
    if ( check_out_unclaimed_gold(spdigtng, 768) )
    {
        return 1;
    }
    if ( check_out_unreinforced_drop_place(spdigtng) )
    {
        cctrl->digger.last_did_job = SDLstJob_ReinforceWall9;
        return 1;
    }
    if ( check_out_crates_to_arm_trap_in_room(spdigtng) )
    {
        return 1;
    }
    cctrl->digger.last_did_job = SDLstJob_None;
    return 0;
}

short imp_arrives_at_convert_dungeon(struct Thing *thing)
{
    TRACE_THING(thing);
    //return _DK_imp_arrives_at_convert_dungeon(thing);
    if (check_place_to_convert_excluding(thing,
        subtile_slab_fast(thing->mappos.x.stl.num),
        subtile_slab_fast(thing->mappos.y.stl.num)) )
    {
      internal_set_thing_state(thing, CrSt_ImpConvertsDungeon);
    } else
    {
      internal_set_thing_state(thing, CrSt_ImpLastDidJob);
    }
    return 1;
}

TbBool move_imp_to_uncrowded_dig_mine_access_point(struct Thing *spdigtng, SubtlCodedCoords stl_num)
{
    long pos_x,pos_y;
    TRACE_THING(spdigtng);
    if (!check_place_to_dig_and_get_position(spdigtng, stl_num, &pos_x, &pos_y))
        return false;
    if (!setup_person_move_to_position(spdigtng, pos_x, pos_y, NavRtF_Default))
        return false;
    spdigtng->continue_state = CrSt_ImpArrivesAtDigDirt;
    return true;
}

short imp_arrives_at_dig_or_mine(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    //return _DK_imp_arrives_at_dig_or_mine(thing);
    if ( imp_already_digging_at_excluding(spdigtng, spdigtng->mappos.x.stl.num, spdigtng->mappos.y.stl.num) )
    {
        cctrl = creature_control_get_from_thing(spdigtng);

        if ( !move_imp_to_uncrowded_dig_mine_access_point(spdigtng, cctrl->word_8F) )
        {
            internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
            return 1;
        }
    } else
    {
        if (spdigtng->active_state == CrSt_ImpArrivesAtDigDirt)
            internal_set_thing_state(spdigtng, CrSt_ImpDigsDirt);
        else
            internal_set_thing_state(spdigtng, CrSt_ImpMinesGold);
    }
    return 1;
}

short imp_arrives_at_improve_dungeon(struct Thing *spdigtng)
{
    TRACE_THING(spdigtng);
    //return _DK_imp_arrives_at_improve_dungeon(thing);
    if ( check_place_to_pretty_excluding(spdigtng,
        subtile_slab_fast(spdigtng->mappos.x.stl.num),
        subtile_slab_fast(spdigtng->mappos.y.stl.num)) )
    {
        internal_set_thing_state(spdigtng, CrSt_ImpImprovesDungeon);
    } else
    {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
    }
    return 1;
}

short imp_arrives_at_reinforce(struct Thing *thing)
{
    return _DK_imp_arrives_at_reinforce(thing);
}

short imp_birth(struct Thing *thing)
{
    struct CreatureStats *crstat;
    long i;
    TRACE_THING(thing);
    //return _DK_imp_birth(thing);
    if ( thing_touching_floor(thing) || (((thing->movement_flags & TMvF_Flying) != 0) && thing_touching_flight_altitude(thing)) )
    {
        // If the creature has flight ability, make sure it returns to flying state
        restore_creature_flight_flag(thing);
        if (!check_out_available_spdigger_drop_tasks(thing)) {
            set_start_state(thing);
        }
        return 1;
    }
    i = game.play_gameturn - thing->creation_turn;
    if ((i % 2) == 0) {
      create_effect_element(&thing->mappos, birth_effect_element[thing->owner], thing->owner);
    }
    crstat = creature_stats_get_from_thing(thing);
    thing->movement_flags &= ~TMvF_Flying;
    creature_turn_to_face_angle(thing, i * (long)crstat->max_angle_change);
    return 0;
}

short imp_converts_dungeon(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    MapSubtlCoord stl_x,stl_y;
    MapSlabCoord slb_x,slb_y;
    TRACE_THING(spdigtng);
    //return _DK_imp_converts_dungeon(thing);
    stl_x = spdigtng->mappos.x.stl.num;
    stl_y = spdigtng->mappos.y.stl.num;
    cctrl = creature_control_get_from_thing(spdigtng);
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    if ( (stl_x - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num >= 1) || (stl_y - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num >= 1) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if ( check_place_to_convert_excluding(spdigtng, slb_x, slb_y) )
    {
      if (cctrl->instance_id == CrInst_NULL)
      {
          struct SlabMap *slb;
          struct SlabAttr *slbattr;
          slb = get_slabmap_block(slb_x, slb_y);
          slbattr = get_slab_attrs(slb);
          set_creature_instance(spdigtng, CrInst_DESTROY_AREA, 0, 0, 0);
          if (slbattr->category == SlbAtCtg_RoomInterior)
          {
            room = room_get(slb->room_index);
            if (!room_is_invalid(room))
            {
                MapCoord coord_x,coord_y;
                coord_x = subtile_coord_center(room->central_stl_x);
                coord_y = subtile_coord_center(room->central_stl_y);
                event_create_event_or_update_nearby_existing_event(coord_x, coord_y,
                    EvKind_RoomUnderAttack, room->owner, 0);
                if (is_my_player_number(room->owner) ) {
                  output_message(SMsg_EnemyDestroyRooms, MESSAGE_DELAY_FIGHT, true);
                }
            }
          }
      }
      return 1;
    }
    if ( !check_place_to_pretty_excluding(spdigtng, slb_x, slb_y) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if (cctrl->instance_id != CrInst_PRETTY_PATH) {
        set_creature_instance(spdigtng, CrInst_PRETTY_PATH, 0, 0, 0);
    }
    return 1;
}

TbBool too_much_gold_lies_around_thing(const struct Thing *thing)
{
    TRACE_THING(thing);
    return gold_pile_with_maximum_at_xy(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

short imp_digs_mines(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct MapTask* mtask;
    struct SlabMap *slb;
    struct Coord3d pos;
    MapSubtlCoord stl_x,stl_y;
    MapSubtlDelta delta_x,delta_y;
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    // return _DK_imp_digs_mines(thing);
    cctrl = creature_control_get_from_thing(spdigtng);
    mtask = get_task_list_entry(spdigtng->owner, cctrl->word_91);
    stl_x = stl_num_decode_x(cctrl->word_8F);
    stl_y = stl_num_decode_y(cctrl->word_8F);
    slb = get_slabmap_for_subtile(stl_x, stl_y);

    // Check if we've arrived at the destination
    delta_x = abs(spdigtng->mappos.x.stl.num - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num);
    delta_y = abs(spdigtng->mappos.y.stl.num - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num);
    if ((mtask->coords != cctrl->word_8F) || (delta_x > 0) || (delta_y > 0))
    {
      clear_creature_instance(spdigtng);
      internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
      return 1;
    }
    // If gems are marked for digging, but there is too much gold laying around, then don't dig
    if (!slab_kind_is_indestructible(slb->kind) && too_much_gold_lies_around_thing(spdigtng))
    {
      clear_creature_instance(spdigtng);
      internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
      return 1;
    }
    // Turn to the correct direction to do the task
    pos.x.stl.num = stl_x;
    pos.y.stl.num = stl_y;
    pos.x.stl.pos = 128;
    pos.y.stl.pos = 128;
    if (creature_turn_to_face(spdigtng, &pos))
    {
      return 1;
    }

    if (mtask->kind == SDDigTask_None)
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }

    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(spdigtng, CrInst_DIG, 0, 0, 0);
    }

    if (mtask->kind == SDDigTask_MineGold)
    {
        crstat = creature_stats_get_from_thing(spdigtng);
        // If the creature holds more gold than its able
        if (spdigtng->creature.gold_carried > crstat->gold_hold)
        {
          if (game.play_gameturn - cctrl->tasks_check_turn > 128)
          {
            if (check_out_imp_has_money_for_treasure_room(spdigtng))
              return 1;
            cctrl->tasks_check_turn = game.play_gameturn;
          }
          drop_gold_pile(spdigtng->creature.gold_carried - crstat->gold_hold, &spdigtng->mappos);
          spdigtng->creature.gold_carried = crstat->gold_hold;
        }
    }
    return 1;
}

short imp_doing_nothing(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    //return _DK_imp_doing_nothing(thing);
    if (!thing_is_creature_special_digger(spdigtng))
    {
        ERRORLOG("Non digger thing %ld, %s, owner %ld - reset",(long)spdigtng->index,thing_model_name(spdigtng),(long)spdigtng->owner);
        set_start_state(spdigtng);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    cctrl = creature_control_get_from_thing(spdigtng);
    dungeon = get_dungeon(spdigtng->owner);
    if (game.play_gameturn-cctrl->idle.start_gameturn <= 1)
        return 1;
    if (check_out_imp_last_did(spdigtng))
        return 1;
    if (check_out_available_imp_tasks(spdigtng))
        return 1;
    if (check_out_imp_tokes(spdigtng))
        return 1;
    if (creature_choose_random_destination_on_valid_adjacent_slab(spdigtng))
    {
        spdigtng->continue_state = CrSt_ImpDoingNothing;
        return 1;
    }
    dungeon->lvstats.promises_broken++;
    return 1;
}

short imp_drops_gold(struct Thing *spdigtng)
{
    struct Room *room;
    struct Thing *gldtng;
    //return _DK_imp_drops_gold(thing);
    if (spdigtng->creature.gold_carried == 0)
    {
        set_start_state(spdigtng);
        return 1;
    }
    room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room) || (room->owner != spdigtng->owner) || (room->kind != RoK_TREASURE))
    {
        WARNLOG("Tried to drop gold in %s of player %d, but room %s owned by played %d is no longer valid to do that",
            room_code_name(RoK_TREASURE),(int)spdigtng->owner,room_code_name(room->kind),(int)room->owner);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    MapSubtlCoord center_stl_x,center_stl_y;
    center_stl_x = slab_subtile_center(subtile_slab_fast(spdigtng->mappos.x.stl.num));
    center_stl_y = slab_subtile_center(subtile_slab_fast(spdigtng->mappos.y.stl.num));
    struct Room *curoom;
    curoom = subtile_room_get(spdigtng->mappos.x.stl.num, spdigtng->mappos.y.stl.num);
    if (!room_exists(curoom) || (curoom->index != room->index))
    {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    gldtng = find_gold_hoard_at(center_stl_x, center_stl_y);
    if (!thing_is_invalid(gldtng))
    {
        spdigtng->creature.gold_carried -= add_gold_to_hoarde(gldtng, room, spdigtng->creature.gold_carried);
    } else
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(center_stl_x);
        pos.y.val = subtile_coord_center(center_stl_y);
        pos.z.val = spdigtng->mappos.z.val;
        gldtng = create_gold_hoarde(room, &pos, spdigtng->creature.gold_carried);
        if (!thing_is_invalid(gldtng))
            spdigtng->creature.gold_carried -= gldtng->valuable.gold_stored;
    }
    thing_play_sample(spdigtng, UNSYNC_RANDOM(3) + 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    if ( (spdigtng->creature.gold_carried == 0) || (room->used_capacity >= room->total_capacity) ) {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    if (!setup_head_for_empty_treasure_space(spdigtng, room)) {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    spdigtng->continue_state = CrSt_ImpDropsGold;
    return 1;
}

short imp_improves_dungeon(struct Thing *spdigtng)
{
    struct CreatureControl *cctrl;
    MapSubtlDelta delta_x,delta_y;
    long slb_x,slb_y;
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    //return _DK_imp_improves_dungeon(thing);
    cctrl = creature_control_get_from_thing(spdigtng);
    // Check if we've arrived at the destination
    delta_x = abs(spdigtng->mappos.x.stl.num - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num);
    delta_y = abs(spdigtng->mappos.y.stl.num - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num);
    if ( (delta_x > 0) || (delta_y > 0) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    slb_x = subtile_slab_fast(spdigtng->mappos.x.stl.num);
    slb_y = subtile_slab_fast(spdigtng->mappos.y.stl.num);
    if (!check_place_to_pretty_excluding(spdigtng, slb_x, slb_y))
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(spdigtng, CrInst_PRETTY_PATH, 0, 0, 0);
    }
    return 1;
}

short imp_last_did_job(struct Thing *thing)
{
    //return _DK_imp_last_did_job(thing);
    if (!check_out_imp_last_did(thing))
    {
        set_start_state(thing);
        return 0;
    }
    return 1;
}

GoldAmount take_from_gold_pile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long limit)
{
    //return _DK_take_from_gold_pile(stl_x, stl_y, limit);
    struct Thing *thing;
    struct Map *mapblk;
    GoldAmount total_taken;
    total_taken = 0;
    long i;
    unsigned long k;
    mapblk = get_map_block_at(stl_x,stl_y);
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
        {
            GoldAmount pot_stored;
            pot_stored = thing->valuable.gold_stored;
            if ((limit - total_taken >= pot_stored) || (limit == -1))
            {
                total_taken += pot_stored;
                delete_thing_structure(thing, 0);
            } else
            {
                thing->valuable.gold_stored += total_taken - limit;
                add_gold_to_pile(thing, 0);
                return limit;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return total_taken;
}

short imp_picks_up_gold_pile(struct Thing *spdigtng)
{
    struct CreatureStats *crstat;
    long gold_taken;
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    //return _DK_imp_picks_up_gold_pile(thing);
    crstat = creature_stats_get_from_thing(spdigtng);
    if (crstat->gold_hold > spdigtng->creature.gold_carried)
    {
        gold_taken = take_from_gold_pile(spdigtng->mappos.x.stl.num, spdigtng->mappos.y.stl.num, crstat->gold_hold - spdigtng->creature.gold_carried);
        spdigtng->creature.gold_carried += gold_taken;
    }
    internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
    return 0;
}

short imp_reinforces(struct Thing *thing)
{
    TRACE_THING(thing);
    return _DK_imp_reinforces(thing);
}

short imp_toking(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    TRACE_THING(creatng);
    //return _DK_imp_toking(thing);
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->field_282 > 0)
    {
        cctrl->field_282--;
    } else
    {
        if (cctrl->instance_id == CrInst_NULL) {
          internal_set_thing_state(creatng, creatng->continue_state);
          return 1;
        }
    }
    if (cctrl->field_282 > 0)
    {
        if (cctrl->instance_id == CrInst_NULL)
        {
            if ( ACTION_RANDOM(8) )
                set_creature_instance(creatng, CrInst_RELAXING, 0, 0, 0);
            else
                set_creature_instance(creatng, CrInst_TOKING, 0, 0, 0);
        }
    }
    if ((cctrl->instance_id == CrInst_TOKING) && (cctrl->inst_turn == cctrl->inst_action_turns))
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(creatng);
        if (crstat->toking_recovery != 0) {
            HitPoints recover;
            recover = compute_creature_max_health(crstat->toking_recovery, cctrl->explevel);
            apply_health_to_thing_and_display_health(creatng, recover);
        }
    }
    return 1;
}

/**
 * For a creature dragging a thing, this function searches for another room
 * where the thing could be placed.
 * @param thing The creature dragging another thing.
 * @return Gives true if the setup to the new room has succeeded.
 */
TbBool creature_drop_thing_to_another_room(struct Thing *thing, struct Room *skiproom, signed char rkind)
{
    struct Room *ownroom;
    struct Coord3d pos;
    TRACE_THING(thing);
    ownroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, rkind, NavRtF_Default, 1);
    if ( room_is_invalid(ownroom) || (ownroom->index == skiproom->index) )
    {
        WARNLOG("Couldn't find a new %s for object dragged by %s owned by %d",room_code_name(rkind),thing_model_name(thing),(int)thing->owner);
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room_avoiding_object(thing, ownroom, &pos) )
    {
        WARNLOG("Couldn't find a new destination in %s for object dragged by %s owned by %d",room_code_name(rkind),thing_model_name(thing),(int)thing->owner);
        return false;
    }
    if (!setup_person_move_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move %s to %s at subtile (%d,%d)",thing_model_name(thing),room_code_name(rkind),(int)pos.x.stl.num,(int)pos.y.stl.num);
        return false;
    }
    return true;
}

TbBool set_creature_being_dragged_by(struct Thing *dragtng, struct Thing *thing)
{
    struct Thing *picktng;
    struct CreatureControl *cctrl, *dragctrl;
    TRACE_THING(dragtng);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    dragctrl = creature_control_get_from_thing(dragtng);
    // Check if we're already dragging
    picktng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(picktng);
    if (!thing_is_invalid(picktng)) {
        ERRORLOG("Thing is already dragging something");
        return false;
    }
    picktng = thing_get(dragctrl->dragtng_idx);
    TRACE_THING(picktng);
    if (!thing_is_invalid(picktng)) {
        ERRORLOG("Thing is already dragged by something");
        return false;
    }
    // Set the new dragging
    cctrl->dragtng_idx = dragtng->index;
    dragtng->field_1 |= TF1_IsDragged1;
    dragctrl->dragtng_idx = thing->index;
    return false;
}

/**
 * Returns if a creature is either being dragged or is dragging something.
 * @param thing The creature to be checked.
 * @return True if the creature has something to do with dragging, false otherwise.
 * @see thing_is_dragged_or_pulled() Checks any thing if it's being dragged.
 */
TbBool creature_is_dragging_or_being_dragged(const struct Thing *thing)
{
    const struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return (cctrl->dragtng_idx != 0);
}

short creature_pick_up_unconscious_body(struct Thing *thing)
{
    struct Room *dstroom;
    struct CreatureControl *cctrl;
    struct Thing *picktng;
    struct Coord3d pos;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    //return _DK_creature_pick_up_unconscious_body(thing);
    // Check if the player has means to do such kind of action
     if ( !player_has_room(thing->owner, RoK_PRISON) || !player_creature_tends_to(thing->owner, CrTend_Imprison) )
     {
         SYNCDBG(19,"Player %d has no %s or has imprison tendency off",(int)thing->owner,room_code_name(RoK_PRISON));
         set_start_state(thing);
         return 0;
     }
    cctrl = creature_control_get_from_thing(thing);
    picktng = thing_get(cctrl->pickup_creature_id);
    TRACE_THING(picktng);
    if ( thing_is_invalid(picktng) || (picktng->active_state != CrSt_CreatureUnconscious) || ((picktng->field_1 & TF1_IsDragged1) != 0)
      || (get_2d_box_distance(&thing->mappos, &picktng->mappos) >= 512))
    {
        SYNCDBG(8,"The %s index %d to be picked up isn't in correct place or state",thing_model_name(picktng),(int)picktng->index);
        set_start_state(thing);
        return 0;
    }
    dstroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_PRISON, NavRtF_Default, 1);
    if (room_is_invalid(dstroom))
    {
        // Check why the treasure room search failed and inform the player
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_PRISON);
        set_start_state(thing);
        return 0;
    }
    if (!find_random_valid_position_for_thing_in_room(thing, dstroom, &pos))
    {
        WARNLOG("Player %d can't pick %s - no position within %s to store it",(int)thing->owner,thing_model_name(picktng),room_code_name(RoK_PRISON));
        set_start_state(thing);
        return 0;
    }
    if (!setup_person_move_backwards_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot drag %s to (%d,%d)",thing_model_name(picktng),(int)pos.x.stl.num,(int)pos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    set_creature_being_dragged_by(picktng, thing);
    thing->continue_state = CrSt_CreatureDropBodyInPrison;
    return 1;
}

short creature_picks_up_corpse(struct Thing *thing)
{
    struct Room *dstroom;
    struct CreatureControl *cctrl;
    struct Thing *picktng;
    struct Coord3d pos;
    TRACE_THING(thing);
    //return _DK_creature_picks_up_corpse(thing);
    cctrl = creature_control_get_from_thing(thing);
    picktng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(picktng);
    if ( thing_is_invalid(picktng) || ((picktng->alloc_flags & TAlF_IsDragged) != 0)
      || (get_2d_box_distance(&thing->mappos, &picktng->mappos) >= 512))
    {
        set_start_state(thing);
        return 0;
    }
    dstroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_GRAVEYARD, NavRtF_Default, 1);
    if (room_is_invalid(dstroom))
    {
        // Check why the treasure room search failed and inform the player
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_PRISON);
        set_start_state(thing);
        return 0;
    }
    if (!find_random_valid_position_for_thing_in_room_avoiding_object(thing, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - no position within %s to store it",(int)thing->owner,thing_model_name(picktng),room_code_name(RoK_GRAVEYARD));
        set_start_state(thing);
        return 0;
    }
    creature_drag_object(thing, picktng);
    if (!setup_person_move_backwards_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
    return 1;
}

/**
 * Picks up spell or special.
 * @param creatng
 */
short creature_picks_up_spell_object(struct Thing *creatng)
{
    struct Room *enmroom, *dstroom;
    struct CreatureControl *cctrl;
    struct Thing *picktng;
    struct Coord3d pos;
    TRACE_THING(creatng);
    //return _DK_creature_picks_up_spell_object(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    picktng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(picktng);
    if ( thing_is_invalid(picktng) || ((picktng->field_1 & TF1_IsDragged1) != 0)
      || (get_2d_box_distance(&creatng->mappos, &picktng->mappos) >= 512))
    {
        set_start_state(creatng);
        return 0;
    }
    enmroom = get_room_thing_is_on(picktng);
    dstroom = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_LIBRARY, NavRtF_Default, 1);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(creatng, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - doesn't have proper %s to store it",(int)creatng->owner,thing_model_name(picktng),room_code_name(RoK_LIBRARY));
        set_start_state(creatng);
        return 0;
    }
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(enmroom))
    {
        remove_spell_from_library(enmroom, picktng, creatng->owner);
    }
    // Create event to inform player about the spell or special (need to be done before pickup due to ownership changes)
    update_library_object_pickup_event(creatng, picktng);
    creature_drag_object(creatng, picktng);
    if (!setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
    return 1;
}

short creature_picks_up_crate_for_workshop(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *cratetng;
    struct Room *dstroom;
    struct Coord3d pos;
    TRACE_THING(thing);
    //return _DK_creature_picks_up_trap_for_workshop(thing);
    // Get the crate thing
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(cratetng);
    // Check if everything is right
    if ( thing_is_invalid(cratetng) || ((cratetng->field_1 & TF1_IsDragged1) != 0)
      || (get_2d_box_distance(&thing->mappos, &cratetng->mappos) >= 512) )
    {
        set_start_state(thing);
        return 0;
    }
    // Find room to drag the crate to
    dstroom = find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, RoK_WORKSHOP, NavRtF_Default);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - doesn't have proper %s to store it",(int)thing->owner,thing_model_name(cratetng),room_code_name(RoK_WORKSHOP));
        set_start_state(thing);
        return 0;
    }
    // Initialize dragging
    if ( !setup_person_move_backwards_to_coord(thing, &pos, NavRtF_Default) )
    {
        set_start_state(thing);
        return 0;
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
    return 1;
}

/** Being in workshop, pick up a trap crate to be dragged to a trap that needs re-arming.
 *
 * @param thing Special worker creature.
 * @return
 */
short creature_picks_up_trap_object(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    struct Thing *cratetng;
    struct Thing *traptng;
    TRACE_THING(thing);
    //return _DK_creature_picks_up_trap_object(thing);
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(cratetng);
    room = get_room_thing_is_on(cratetng);
    traptng = thing_get(cctrl->arming_thing_id);
    TRACE_THING(traptng);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        WARNLOG("The %s index %d or %s index %d no longer exists",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if ( ((cratetng->field_1 & TF1_IsDragged1) != 0)
      || (traptng->class_id != TCls_Trap) || (crate_thing_to_workshop_item_model(cratetng) != traptng->model))
    {
        WARNLOG("Cannot use %s index %d to refill %s index %d",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if (get_2d_box_distance(&thing->mappos, &cratetng->mappos) >= 512)
    {
        WARNLOG("The %s index %d was supposed to be near %s index %d for pickup, but it's too far",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(thing),(int)thing->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if ( !setup_person_move_backwards_to_coord(thing, &traptng->mappos, NavRtF_Default) )
    {
        WARNLOG("Cannot deliver crate to position of %s index %d",thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    SYNCDBG(18,"Moving %s index %d",thing_model_name(thing),(int)thing->index);
    if (room_exists(room))
    {
        remove_workshop_object_from_workshop(room,cratetng);
        if (!is_hero_thing(cratetng) && !is_neutral_thing(cratetng))
        {
            remove_workshop_item_from_amount_stored(cratetng->owner,
                crate_thing_to_workshop_item_class(cratetng),
                crate_thing_to_workshop_item_model(cratetng), WrkCrtF_NoOffmap);
        }
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureArmsTrap;
    return 1;
}

short creature_drops_corpse_in_graveyard(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    struct Thing *deadtng;
    TRACE_THING(thing);
    //return _DK_creature_drops_corpse_in_graveyard(thing);
    cctrl = creature_control_get_from_thing(thing);
    deadtng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(deadtng);
    // Check if corpse is ok
    if ( !thing_exists(deadtng) )
    {
        ERRORLOG("The %s index %d tried to drop a corpse, but it's gone",thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    // Check if we're on correct room
    room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(deadtng),(int)deadtng->index,room_code_name(RoK_GRAVEYARD));
        if (creature_drop_thing_to_another_room(thing, room, RoK_GRAVEYARD)) {
            thing->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }

    if ( (room->kind != RoK_GRAVEYARD) || (room->owner != thing->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room won't accept it",thing_model_name(deadtng),(int)deadtng->index,room_code_name(RoK_GRAVEYARD));
        if (creature_drop_thing_to_another_room(thing, room, RoK_GRAVEYARD)) {
            thing->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }
    // Do the dropping
    creature_drop_dragged_object(thing, deadtng);
    deadtng->owner = thing->owner;
    add_body_to_graveyard(deadtng, room);
    // The action of moving object is now finished
    set_start_state(thing);
    return 1;
}

short creature_drops_crate_in_workshop(struct Thing *thing)
{
    struct Thing *cratetng;
    struct CreatureControl *cctrl;
    struct Room *room;
    TRACE_THING(thing);
    //return _DK_creature_drops_crate_in_workshop(thing);
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(cratetng);
    // Check if crate is ok
    if ( !thing_exists(cratetng) )
    {
        ERRORLOG("The %s index %d tried to drop crate, but it's gone",thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    // Check if we're on correct room
    room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) )
    {
        SYNCDBG(7,"Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(cratetng),(int)cratetng->index,room_code_name(RoK_WORKSHOP));
        if (creature_drop_thing_to_another_room(thing, room, RoK_WORKSHOP)) {
            thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }
    if ( (room->kind != RoK_WORKSHOP) || (room->owner != thing->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
        SYNCDBG(7,"Tried to drop %s index %d in %s, but room won't accept it",thing_model_name(cratetng),(int)cratetng->index,room_code_name(RoK_WORKSHOP));
        if (creature_drop_thing_to_another_room(thing, room, RoK_WORKSHOP)) {
            thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }
    creature_drop_dragged_object(thing, cratetng);
    cratetng->owner = game.neutral_player_num;
    if (thing_is_door_or_trap_crate(cratetng))
    {
        if (!add_workshop_object_to_workshop(room, cratetng))
        {
            WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(cratetng),(int)cratetng->index,room_code_name(RoK_WORKSHOP));
            set_start_state(thing);
            return 1;
        }
        cratetng->owner = thing->owner;
        add_workshop_item_to_amounts(room->owner, crate_thing_to_workshop_item_class(cratetng),
            crate_thing_to_workshop_item_model(cratetng));
    }
    // The action of moving object is now finished
    set_start_state(thing);
    return 1;
}

/**
 * Drops a previously picked up spell into a library.
 * @param thing The creature dragging a spell.
 * @return Gives true if the action shall continue, false if it's finished.
 */
short creature_drops_spell_object_in_library(struct Thing *creatng)
{
    struct Thing *spelltng;
    struct CreatureControl *cctrl;
    struct Room *room;
    TRACE_THING(creatng);
    //return _DK_creature_drops_spell_object_in_library(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    spelltng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(spelltng);
    // Check if spell is ok
    if ( !thing_exists(spelltng) )
    {
        ERRORLOG("The %s index %d tried to drop a spell, but it's gone",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    // Check if we're on correct room
    room = get_room_thing_is_on(creatng);
    if ( room_is_invalid(room) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(spelltng),(int)spelltng->index,room_code_name(RoK_LIBRARY));
        if (creature_drop_thing_to_another_room(creatng, room, RoK_LIBRARY)) {
            creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }
    if ( (room->kind != RoK_LIBRARY) || (room->owner != creatng->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
        WARNLOG("Tried to drop %s index %d in %s room, but room won't accept it",thing_model_name(spelltng),(int)spelltng->index,room_code_name(RoK_LIBRARY));
        if (creature_drop_thing_to_another_room(creatng, room, RoK_LIBRARY)) {
            creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }
    // Do the dropping
    creature_drop_dragged_object(creatng, spelltng);
    spelltng->owner = game.neutral_player_num;
    if (thing_is_spellbook(spelltng))
    {
        if (!add_item_to_room_capacity(room, true)) {
            WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(spelltng),(int)spelltng->index,room_code_name(RoK_LIBRARY));
            set_start_state(creatng);
            return 1;
        }
        spelltng->owner = creatng->owner;
        add_spell_to_player(book_thing_to_magic(spelltng), creatng->owner);
    } else
    if (thing_is_special_box(spelltng))
    {
        spelltng->owner = creatng->owner;
    }
    // The action of moving object is now finished
    set_start_state(creatng);
    return 1;
}

short creature_arms_trap(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *traptng;
    struct Thing *postng;
    struct Thing *cratetng;
    TRACE_THING(thing);
    //return _DK_creature_arms_trap(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Creature has invalid control structure!");
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    cratetng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(cratetng);
    traptng = thing_get(cctrl->arming_thing_id);
    TRACE_THING(traptng);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        set_start_state(thing);
        return 0;
    }
    postng = get_trap_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, traptng->model, thing->owner);
    // Note that this means there can be only one trap of given kind at a subtile.
    // Otherwise it won't be possible to re-arm it, as the condition below will fail.
    if ( (postng->index != traptng->index) || (traptng->trap.num_shots > 0) )
    {
        ERRORLOG("The %s has moved or been already rearmed",thing_model_name(traptng));
        set_start_state(thing);
        return 0;
    }
    rearm_trap(traptng);
    dungeon->lvstats.traps_armed++;
    creature_drop_dragged_object(thing, cratetng);
    delete_thing_structure(cratetng, 0);
    // The action of moving object is now finished
    set_start_state(thing);
    return 1;
}

/******************************************************************************/
