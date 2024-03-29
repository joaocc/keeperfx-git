/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_factory.c
 *     Things creation unified functions.
 * @par Purpose:
 *     Functions to create various things using single interface.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 22 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_factory.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_data.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "thing_shots.h"
#include "thing_traps.h"
#include "thing_corpses.h"
#include "room_util.h"
#include "sounds.h"
#include "dungeon_data.h"
#include "gui_topmsg.h"
#include "config_magic.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_thing(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long parent_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_cave_in(struct Coord3d *pos, unsigned short cimodel, unsigned short owner)
{
    struct MagicStats *magstat;
    struct Dungeon *dungeon;
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create cave in %d for player %d. There are too many things allocated.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate cave in %d for player %d, but failed.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_CaveIn;
    thing->model = 0;
    thing->parent_idx = thing->index;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->owner = owner;
    thing->creation_turn = game.play_gameturn;
    magstat = &game.keeper_power_stats[PwrK_CAVEIN];
    thing->word_15 = magstat->time;
    thing->byte_13 = pos->x.stl.num;
    thing->byte_14 = pos->y.stl.num;
    thing->byte_17 = cimodel;
    thing->health = magstat->time;
    if (owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(owner);
        dungeon->camera_deviate_quake = thing->word_15;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, unsigned short model, unsigned short owner, long parent_idx)
{
    struct Thing *thing;
    //return _DK_create_thing(pos, tngclass, model, owner, a4);
    thing = INVALID_THING;
    switch (tngclass)
    {
    case TCls_Object:
        thing = create_object(pos, model, owner, parent_idx);
        break;
    case TCls_Shot:
        thing = create_shot(pos, model, owner);
        break;
    case TCls_EffectElem:
        thing = create_effect_element(pos, model, owner);
        break;
    case TCls_DeadCreature:
        thing = create_dead_creature(pos, model, 1, owner, 0);
        break;
    case TCls_Creature:
        thing = create_creature(pos, model, owner);
        break;
    case TCls_Effect:
        thing = create_effect(pos, model, owner);
        break;
    case TCls_Trap:
        thing = create_trap(pos, model, owner);
        break;
    case TCls_AmbientSnd:
        thing = create_ambient_sound(pos, model, owner);
        break;
    case TCls_CaveIn:
        thing = create_cave_in(pos, model, owner);
        break;
    default:
        break;
    }
    return thing;
}

short thing_create_thing(struct InitThing *itng)
{
    struct Thing *thing;
    if (itng->owner == 7)
    {
        ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.hero_player_num);
        itng->owner = game.hero_player_num;
    } else
    if (itng->owner == 8)
    {
        ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.neutral_player_num);
        itng->owner = game.neutral_player_num;
    }
    if (itng->owner > 5)
    {
        ERRORLOG("Invalid owning player %d, thing discarded", (int)itng->owner);
        return false;
    }
    switch (itng->oclass)
    {
    case TCls_Object:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (!thing_is_invalid(thing))
        {
            if (itng->model == 49) //HERO_GATE
                thing->byte_13 = itng->params[1];
            check_and_asimilate_thing_by_room(thing);
            // make sure we don't have invalid pointer
            thing = INVALID_THING;
        } else
        {
            ERRORLOG("Couldn't create object model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Creature:
        thing = create_creature(&itng->mappos, itng->model, itng->owner);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create creature model %d", (int)itng->model);
            return false;
        }
        init_creature_level(thing, itng->params[1]);
        break;
    case TCls_EffectGen:
        thing = create_effect_generator(&itng->mappos, itng->model, itng->range, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create effect generator model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Trap:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create trap model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Door:
        thing = create_door(&itng->mappos, itng->model, itng->params[0], itng->owner, itng->params[1]);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create door model %d", (int)itng->model);
            return false;
        }
        break;
    case 10:
    case 11:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create thing class %d model %d", (int)itng->oclass, (int)itng->model);
            return false;
        }
        break;
    default:
        ERRORLOG("Invalid class %d, thing discarded", (int)itng->oclass);
        return false;
    }
    return true;
}
/******************************************************************************/
