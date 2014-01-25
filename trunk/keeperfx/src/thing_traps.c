/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.c
 *     Traps support functions.
 * @par Purpose:
 *     Functions to support trap things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_traps.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"

#include "thing_data.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "thing_effects.h"
#include "thing_shots.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "room_util.h"
#include "game_legacy.h"
#include "frontend.h"
#include "engine_render.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_trap(struct Coord3d *pos, unsigned short trpkind, unsigned short plyr_idx);
DLLIMPORT struct Thing *_DK_get_trap_for_position(long pos_x, long pos_y);
DLLIMPORT struct Thing *_DK_get_trap_for_slab_position(long slb_x, long slb_y);
DLLIMPORT long _DK_update_trap(struct Thing *traptng);
DLLIMPORT void _DK_update_trap_trigger(struct Thing *traptng);
DLLIMPORT void _DK_external_activate_trap_shot_at_angle(struct Thing *traptng, long plyr_idx);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_trap(unsigned char trpkind, long plyr_idx, long a3);
DLLIMPORT long _DK_update_trap_trigger_line_of_sight_90(struct Thing *traptng);
DLLIMPORT void _DK_activate_trap(struct Thing *traptng, struct Thing *creatng);
/******************************************************************************/
TbBool destroy_trap(struct Thing *traptng)
{
    if ((traptng->trap.num_shots == 0) && !is_neutral_thing(traptng) && !is_hero_thing(traptng)) {
        readd_workshop_item_to_amount_placeable(traptng->owner, traptng->class_id, traptng->model);
    }
    delete_thing_structure(traptng, 0);
    return true;
}

TbBool trap_is_active(const struct Thing *thing)
{
    return ((thing->trap.num_shots > 0) && (thing->trap.long_14t <= game.play_gameturn));
}

TbBool trap_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx)
    {
        return (thing->model == 1) && trap_is_active(thing);
    }
    return false;
}

struct Thing *get_trap_for_position(long pos_x, long pos_y)
{
    return _DK_get_trap_for_position(pos_x, pos_y);
}

struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapCoord pos_x,pos_y;
    //return _DK_get_trap_for_slab_position(slb_x, slb_y);
    pos_x = subtile_coord_center(slab_subtile_center(slb_x));
    pos_y = subtile_coord_center(slab_subtile_center(slb_y));
    return get_trap_around_of_model_and_owned_by(pos_x, pos_y, -1, -1);
}

TbBool slab_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing *traptng;
    traptng = get_trap_for_slab_position(slb_x, slb_y);
    return !thing_is_invalid(traptng);
}

TbBool thing_is_deployed_trap(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Trap)
        return false;
    return true;
}

TbBool update_trap_trigger_line_of_sight_90(struct Thing *thing)
{
    //TODO modify to disallow activation by spectators
    return _DK_update_trap_trigger_line_of_sight_90(thing);
}

void activate_trap_shot_head_for_target90(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats *trapstat;
    trapstat = &trap_stats[traptng->model];
    if (trapstat->field_1A <= 0)
    {
        ERRORLOG("Trap activation of bad shot kind %d",(int)trapstat->field_1A);
        return;
    }
    struct Thing *shotng;
    shotng = create_shot(&traptng->mappos, trapstat->field_1A, traptng->owner);
    if (!thing_is_invalid(shotng))
    {
        {
            MapCoord crpos_x, crpos_y;
            MapCoord trpos_x, trpos_y;
            trpos_x = traptng->mappos.x.val;
            trpos_y = traptng->mappos.y.val;
            crpos_x = creatng->mappos.x.val;
            crpos_y = creatng->mappos.y.val;
            if (abs(trpos_x - crpos_x) <= abs(trpos_y - crpos_y))
            {
                if (crpos_y >= trpos_y)
                    shotng->field_52 = 1024;
                else
                    shotng->field_52 = 0;
            } else
            {
                if (crpos_x >= trpos_x)
                    shotng->field_52 = 512;
                else
                    shotng->field_52 = 1536;
            }
        }
        shotng->field_54 = 0;
        struct ShotConfigStats *shotst;
        shotst = get_shot_model_stats(trapstat->field_1A);
        struct ComponentVector cvect;
        angles_to_vector(shotng->field_52, 0, shotst->old->speed, &cvect);
        shotng->acceleration.x.val += cvect.x;
        shotng->acceleration.y.val += cvect.y;
        shotng->acceleration.z.val += cvect.z;
        shotng->field_1 |= 0x04;
        shotng->byte_16 = trapstat->field_1B;
        if (shotst->old->firing_sound > 0) {
            thing_play_sample(traptng, shotst->old->firing_sound+UNSYNC_RANDOM(shotst->old->firing_sound_variants), 100, 0, 3, 0, 6, 256);
        }
        if (shotst->old->shot_sound > 0) {
            thing_play_sample(shotng, shotst->old->shot_sound, 100, 0, 3, 0, 2, 256);
        }
    }
}

void activate_trap_effect_on_trap(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats *trapstat;
    trapstat = &trap_stats[traptng->model];
    if (trapstat->field_1A <= 0)
    {
        ERRORLOG("Trap activation of bad effect kind %d",(int)trapstat->field_1A);
        return;
    }
    struct Thing *efftng;
    efftng = create_effect(&traptng->mappos, trapstat->field_1A, traptng->owner);
    if (!thing_is_invalid(efftng)) {
        efftng->byte_16 = trapstat->field_1B;
        SYNCDBG(18,"Created %s",thing_model_name(efftng));
    }
}

void activate_trap_shot_on_trap(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats *trapstat;
    trapstat = &trap_stats[traptng->model];
    if (trapstat->field_1A <= 0)
    {
        ERRORLOG("Trap activation of bad shot kind %d",(int)trapstat->field_1A);
        return;
    }
    struct Thing *shotng;
    shotng = create_shot(&traptng->mappos, trapstat->field_1A, traptng->owner);
    if (!thing_is_invalid(shotng)) {
        shotng->byte_16 = trapstat->field_1B;
        shotng->parent_idx = 0;
        shotng->acceleration.x.val += trapstat->field_30;
        shotng->acceleration.y.val += trapstat->field_32;
        shotng->acceleration.z.val += trapstat->field_34;
        shotng->field_1 |= 0x04;
    }
}

void activate_trap_slab_change(struct Thing *traptng, struct Thing *creatng)
{
    MapSubtlCoord stl_x, stl_y;
    stl_x = traptng->mappos.x.stl.num;
    stl_y = traptng->mappos.y.stl.num;
    if (subtile_is_room(stl_x, stl_y)) {
        delete_room_slab(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y), true);
    }
    place_slab_type_on_map(trap_stats[traptng->model].field_1A, stl_x, stl_y, game.neutral_player_num, 0);
    do_slab_efficiency_alteration(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
}

void activate_trap(struct Thing *traptng, struct Thing *creatng)
{
    //_DK_activate_trap(traptng, creatng); return;
    const struct TrapStats *trapstat;
    traptng->trap.byte_18t = 1;
    trapstat = &trap_stats[traptng->model];
    if (traptng->model == 2) {
        event_create_event(traptng->mappos.x.val, traptng->mappos.y.val, EvKind_AlarmTriggered, traptng->owner, 0);
    }
    thing_play_sample(traptng, 176, 100, 0, 3, 0, 2, 256);
    switch (trapstat->activation_type)
    {
    case 1:
        activate_trap_shot_head_for_target90(traptng, creatng);
        break;
    case 2:
        activate_trap_effect_on_trap(traptng, creatng);
        break;
    case 3:
        activate_trap_shot_on_trap(traptng, creatng);
        break;
    case 4:
        activate_trap_slab_change(traptng, creatng);
        break;
    default:
        ERRORLOG("Illegal trap activation type %d",(int)trapstat->activation_type);
        break;
    }
}

TbBool find_pressure_trigger_trap_target_passing_by_subtile(const struct Thing *traptng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing **found_thing)
{
    struct Thing *thing;
    struct Map *mapblk;
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
        if (thing_is_creature(thing) && (thing->owner != traptng->owner))
        {
            if ((thing->field_1 & 0x01) == 0)
            {
                if (!creature_is_dying(thing) && !creature_is_being_unconscious(thing)
                    && ((get_creature_model_flags(thing) & MF_IsSpectator) == 0))
                {
                    if (!is_neutral_thing(thing) && !players_are_mutual_allies(traptng->owner,thing->owner))
                    {
                        *found_thing = thing;
                        return true;
                    }
                }
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
    return false;
}

TbBool update_trap_trigger_pressure(struct Thing *traptng)
{
    MapSlabCoord slb_x, slb_y;
    slb_x = subtile_slab_fast(traptng->mappos.x.stl.num);
    slb_y = subtile_slab_fast(traptng->mappos.y.stl.num);
    MapSubtlCoord stl_x, stl_y;
    MapSubtlCoord end_stl_x, end_stl_y;
    end_stl_x = slab_subtile(slb_x,2);
    end_stl_y = slab_subtile(slb_y,2);
    for (stl_y=slab_subtile(slb_y,0); stl_y <= end_stl_y; stl_y++)
    {
        for (stl_x=slab_subtile(slb_x,0); stl_x <= end_stl_x; stl_x++)
        {
            struct Thing *creatng;
            creatng = INVALID_THING;
            if (find_pressure_trigger_trap_target_passing_by_subtile(traptng, stl_x, stl_y, &creatng))
            {
                activate_trap(traptng, creatng);
                return true;
            }

        }
    }
    return false;
}

void update_trap_trigger(struct Thing *traptng)
{
    if (traptng->trap.num_shots <= 0) {
        return;
    }
    //_DK_update_trap_trigger(thing); return;
    TbBool do_trig;
    switch (trap_stats[traptng->model].trigger_type)
    {
    case 1:
        do_trig = update_trap_trigger_line_of_sight_90(traptng);
        break;
    case 2:
        do_trig = update_trap_trigger_pressure(traptng);
        break;
    default:
        ERRORLOG("Illegal trap trigger type %d",(int)trap_stats[traptng->model].trigger_type);
        do_trig = false;
        break;
    }
    if (do_trig)
    {
        const struct ManfctrConfig *mconf;
        mconf = &game.traps_config[traptng->model];
        traptng->trap.long_14t = game.play_gameturn + mconf->shots_delay;
        int n;
        n = traptng->trap.num_shots;
        if ((n > 0) && (n != 255))
        {
            traptng->trap.num_shots = n - 1;
            if (traptng->trap.num_shots == 0)
            {
                traptng->field_4F &= 0x10;
                traptng->field_4F |= 0x20;
                if (!is_neutral_thing(traptng) && !is_hero_thing(traptng)) {
                    remove_workshop_item_from_amount_placeable(traptng->owner, traptng->class_id, traptng->model);
                }
            }
        }
    }
}

TngUpdateRet update_trap(struct Thing *traptng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(traptng);
    //return _DK_update_trap(thing);
    if (traptng->health < 0)
    {
        destroy_trap(traptng);
        return TUFRet_Deleted;
    }
    if (trap_is_active(traptng))
    {
        update_trap_trigger(traptng);
    }
    if (map_pos_is_lava(traptng->mappos.x.stl.num, traptng->mappos.y.stl.num) && !thing_is_dragged_or_pulled(traptng))
    {
        destroy_trap(traptng);
        return TUFRet_Deleted;
    }
    return TUFRet_Modified;
}

struct Thing *create_trap(struct Coord3d *pos, ThingModel trpkind, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting");
    //return _DK_create_trap(pos, trpkind, plyr_idx);
    struct TrapStats *trapstat;
    trapstat = &trap_stats[trpkind];
    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots)) {
        ERRORDBG(3,"Cannot create trap %s for player %d. There are too many things allocated.",trap_code_name(trpkind),(int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct InitLight ilght;
    LbMemorySet(&ilght, 0, sizeof(struct InitLight));
    struct Thing *thing;
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate trap %s for player %d, but failed.",trap_code_name(trpkind),(int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_Trap;
    thing->model = trpkind;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->next_on_mapblk = 0;
    thing->parent_idx = thing->index;
    thing->owner = plyr_idx;
    char start_frame;
    if (trapstat->field_13) {
        start_frame = -1;
    } else {
        start_frame = 0;
    }
    set_thing_draw(thing, trapstat->field_4, trapstat->field_D, trapstat->field_8, trapstat->field_C, start_frame, 2);
    if (trapstat->field_11) {
        thing->field_4F |= 0x02;
    } else {
        thing->field_4F &= ~0x02;
    }
    if (trapstat->field_C) {
        thing->field_4F |= 0x40;
    } else {
        thing->field_4F &= ~0x40;
    }
    thing->sizexy = trapstat->field_14;
    thing->field_58 = trapstat->field_16;
    thing->field_5A = trapstat->field_14;
    thing->field_5C = trapstat->field_16;
    thing->creation_turn = game.play_gameturn;
    thing->health = trapstat->field_0;
    thing->field_4F &= ~0x10;
    thing->field_4F |= 0x20;
    thing->byte_13 = 0;
    thing->long_14 = game.play_gameturn;
    if (trapstat->field_1C != 0)
    {
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        ilght.field_0 = trapstat->field_1C;
        ilght.field_2 = trapstat->field_1E;
        ilght.is_dynamic = 1;
        ilght.field_3 = trapstat->field_1F;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id <= 0) {
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

void init_traps(void)
{
    struct Thing *thing;
    int i, k;
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per thing code
        if (thing->byte_13 == 0)
        {
            thing->byte_13 = game.traps_config[thing->model].shots;
            thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

/**
 * Removes traps on the subtile and all sibling subtiles.
 * Either treats the action as selling and removes corresponding crate from workshop,
 * or treats it as destroying and updates workshop counts to skip re-arming the destroyed trap.
 *
 * @param stl_x Central subtile X coordinate.
 * @param stl_y Central subtile Y coordinate.
 * @param sell_value Value to be added to treasury if selling traps; if not selling but just removing, should be null.
 * @return Amount of traps removed.
 */
long remove_traps_around_subtile(long stl_x, long stl_y, long *sell_value)
{
    long i,k;
    long total;
    total = 0;
    for (k=0; k < AROUND_TILES_COUNT; k++)
    {
        struct Thing *traptng;
        traptng = get_trap_for_position(stl_x+around[k].delta_x, stl_y+around[k].delta_y);
        if (!thing_is_invalid(traptng))
        {
            if (sell_value != NULL) {
                i = game.traps_config[traptng->model].selling_value;
                if (traptng->trap.num_shots == 0) {
                    if (remove_workshop_item_from_amount_stored(traptng->owner, traptng->class_id, traptng->model)) {
                        remove_workshop_object_from_player(traptng->owner, trap_crate_object_model(traptng->model));
                    }
                }
                (*sell_value) += i;
                // We don't want to increase trap_amount_placeable, so we'll not use destroy_trap() there
                delete_thing_structure(traptng, 0);
            } else {
                destroy_trap(traptng);
            }
            total++;
        }
    }
    return total;
}

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2)
{
    _DK_external_activate_trap_shot_at_angle(thing, a2);
}

TbBool tag_cursor_blocks_place_trap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(7,"Starting");
    //return _DK_tag_cursor_blocks_place_trap(plyr_idx, stl_x, stl_y);
    int par1;
    TbBool can_place;
    can_place = 0;
    MapSlabCoord slb_x, slb_y;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if (!subtile_revealed(stl_x, stl_y, plyr_idx) || ((slbattr->flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0))
    {
        par1 = temp_cluedo_mode < 1u ? 5 : 2;
    } else
    if (slab_kind_is_liquid(slb->kind))
    {
        par1 = 0;
    } else
    {
        if ((slabmap_owner(slb) == plyr_idx) && (slb->kind == SlbT_CLAIMED))
        {
            if (!slab_has_trap_on(slb_x, slb_y) && !subtile_has_door_thing_on(stl_x, stl_y))
            {
                can_place = 1;
            }
        }
        par1 = 1;
    }
    if (is_my_player_number(plyr_idx))
    {
        if (!game_is_busy_doing_gui() && (game.small_map_state != 2)) {
            // Move to first subtile on a slab
            stl_x = slab_subtile(slb_x,0);
            stl_y = slab_subtile(slb_y,0);
            draw_map_volume_box(subtile_coord(stl_x,0), subtile_coord(stl_y,0),
                subtile_coord(stl_x+STL_PER_SLB,0), subtile_coord(stl_y+STL_PER_SLB,0), par1, can_place);
        }
    }
    return can_place;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
