/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_shots.c
 *     Shots support functions.
 * @par Purpose:
 *     Functions to support shot things.
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
#include "thing_shots.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "thing_data.h"
#include "thing_factory.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "thing_objects.h"
#include "front_simple.h"
#include "thing_stats.h"
#include "map_blocks.h"
#include "room_garden.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "power_process.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "creature_states.h"
#include "creature_groups.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_move_shot(struct Thing *shotng);
DLLIMPORT long _DK_update_shot(struct Thing *shotng);

DLLIMPORT struct Thing *_DK_get_thing_collided_with_at_satisfying_filter(struct Thing *shotng, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5);
DLLIMPORT struct Thing *_DK_get_shot_collided_with_same_type(struct Thing *shotng, struct Coord3d *nxpos);
DLLIMPORT long _DK_shot_hit_wall_at(struct Thing *shotng, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_door_at(struct Thing *shotng, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_object_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos);
DLLIMPORT void _DK_create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *trgtng);
DLLIMPORT long _DK_check_hit_when_attacking_door(struct Thing *shotng);
DLLIMPORT void _DK_process_dig_shot_hit_wall(struct Thing *shotng, long blocked_flags);
DLLIMPORT struct Thing *_DK_create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner);
/******************************************************************************/
TbBool thing_is_shot(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Shot)
        return false;
    return true;
}

TbBool shot_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    struct ShotConfigStats *shotst;
    if (thing->owner == plyr_idx)
    {
        // Normally, thing models 15 and 20 are slappable. But this is up to config file.
        shotst = get_shot_model_stats(thing->model);
        return ((shotst->model_flags & ShMF_Slappable) != 0);
    }
    return false;
}

TbBool shot_model_is_navigable(long tngmodel)
{
    // Normally, only shot model 6 is navigable
    struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(tngmodel);
    return ((shotst->model_flags & ShMF_Navigable) != 0);
}

TbBool shot_is_boulder(const struct Thing *thing)
{
    return (thing->model == 15);
}

TbBool detonate_shot(struct Thing *shotng)
{
    struct PlayerInfo *myplyr;
    struct Thing *castng;
    struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(shotng->model);
    SYNCDBG(18,"Starting for model %d",(int)shotng->model);
    castng = INVALID_THING;
    myplyr = get_my_player();
    // Identify the creator if the shot
    if (shotng->index != shotng->parent_idx) {
        castng = thing_get(shotng->parent_idx);
        TRACE_THING(castng);
    }
    // If the shot has area_range, then make area damage
    if (shotst->area_range != 0) {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(castng);
        //TODO SPELLS Spell level should be taken from within the shot, not from caster creature
        // Caster may have leveled up, or even may be already dead
        // But currently shot do not store its level, so we don't really have a choice
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(castng);
        long dist, damage;
        dist = compute_creature_attack_range(shotst->area_range*COORD_PER_STL, crstat->luck, cctrl->explevel);
        damage = compute_creature_attack_damage(shotst->area_damage, crstat->luck, cctrl->explevel);
        explosion_affecting_area(castng, &shotng->mappos, dist, damage, shotst->area_blow, shotst->area_hit_type, shotst->damage_type);
    }
    switch (shotng->model)
    {
    case 4://SHOT_LIGHTNING
    case 16://SHOT_GOD_LIGHTNING
    case 24://SHOT_LIGHTNING_BALL
        PaletteSetPlayerPalette(myplyr, engine_palette);
        break;
    case 11://SHOT_GRENADE
        create_effect(&shotng->mappos, TngEff_Unknown50, shotng->owner);
        create_effect(&shotng->mappos,  TngEff_Unknown09, shotng->owner);
        break;
    case 15://SHOT_BOULDER
        create_effect_around_thing(shotng, TngEff_DirtRubble);
        break;
    default:
        break;
    }
    delete_thing_structure(shotng, 0);
    return true;
}

struct Thing *get_shot_collided_with_same_type(struct Thing *thing, struct Coord3d *nxpos)
{
    return _DK_get_shot_collided_with_same_type(thing, nxpos);
}

TbBool give_gold_to_creature_or_drop_on_map_when_digging(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long damage)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    long gold;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    dungeon = get_dungeon(creatng->owner);
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    gold = calculate_gold_digged_out_of_slab_with_single_hit(damage, creatng->owner, cctrl->explevel, slb);
    creatng->creature.gold_carried += gold;
    if (dungeon_invalid(dungeon)) {
        dungeon->lvstats.gold_mined += gold;
    }
    if (crstat->gold_hold <= creatng->creature.gold_carried)
    {
        drop_gold_pile(creatng->creature.gold_carried, &creatng->mappos);
        creatng->creature.gold_carried = 0;
    }
    return true;
}

void process_dig_shot_hit_wall(struct Thing *thing, unsigned long blocked_flags)
{
    MapSubtlCoord stl_x, stl_y;
    struct Thing *diggertng;
    unsigned long k;
    //_DK_process_dig_shot_hit_wall(thing, a2); return;
    diggertng = INVALID_THING;
    if (thing->index != thing->parent_idx)
      diggertng = thing_get(thing->parent_idx);
    if (!thing_exists(diggertng))
    {
        ERRORLOG("Digging shot hit wall, but there's no digger creature index %d.",thing->parent_idx);
        return;
    }
    if (blocked_flags & SlbBloF_WalledX)
    {
        k = thing->field_52 & 0xFC00;
        if (k != 0)
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val) - 1);
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val));
        }
        else
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val) + 1);
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val));
        }
    } else
    if (blocked_flags & SlbBloF_WalledY)
    {
        k = thing->field_52 & 0xFE00;
        if ((k != 0) && (k != 0x0600))
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val));
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val) + 1);
        }
        else
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val));
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val) - 1);
        }
    } else
    {
        stl_x = coord_subtile(thing->mappos.x.val);
        stl_y = coord_subtile(thing->mappos.y.val);
    }
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    // You can only dig your own or neutral ground
    if ((slabmap_owner(slb) != game.neutral_player_num) && (slabmap_owner(slb) != diggertng->owner))
    {
        return;
    }
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    // Doors cannot be digged
    if ((mapblk->flags & MapFlg_IsDoor) != 0)
    {
        return;
    }
    if ((mapblk->flags & MapFlg_IsTall) == 0)
    {
        return;
    }
    int damage;
    damage = thing->word_14;
    if ((damage >= slb->health) && !slab_kind_is_indestructible(slb->kind))
    {
        if ((mapblk->flags & MapFlg_Unkn01) != 0)
        { // Valuables require counting gold
            give_gold_to_creature_or_drop_on_map_when_digging(diggertng, stl_x, stl_y, damage);
            mine_out_block(stl_x, stl_y, diggertng->owner);
            thing_play_sample(diggertng, 72+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        } else
        if ((mapblk->flags & MapFlg_IsDoor) == 0)
        { // All non-gold and non-door slabs are just destroyed
            dig_out_block(stl_x, stl_y, diggertng->owner);
            thing_play_sample(diggertng, 72+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
        check_map_explored(diggertng, stl_x, stl_y);
    } else
    {
        if (!slab_kind_is_indestructible(slb->kind))
        {
            slb->health -= damage;
        }
        if ((mapblk->flags & MapFlg_Unkn01) != 0)
        {
            give_gold_to_creature_or_drop_on_map_when_digging(diggertng, stl_x, stl_y, damage);
        }
    }
}

struct Thing *create_shot_hit_effect(struct Coord3d *effpos, long effowner, long eff_kind, long snd_idx, long snd_range)
{
    struct Thing *efftng;
    long i;
    efftng = INVALID_THING;
    if (eff_kind > 0) {
        efftng = create_effect(effpos, eff_kind, effowner);
        TRACE_THING(efftng);
    }
    if (snd_idx > 0)
    {
        if (!thing_is_invalid(efftng))
        {
            i = snd_idx;
            if (snd_range > 1)
                i += UNSYNC_RANDOM(snd_range);
            thing_play_sample(efftng, i, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    return efftng;
}

/**
 * Processes hitting a wall by given shot.
 *
 * @param thing The thing to be moved into wall.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit a wall and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_wall_at(struct Thing *shotng, struct Coord3d *pos)
{
    struct ShotConfigStats *shotst;
    struct Thing *efftng;
    struct Thing *doortng;
    unsigned long blocked_flags;
    TbBool shot_explodes;
    long i;
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);
    //return _DK_shot_hit_wall_at(thing, pos);

    efftng = INVALID_THING;
    shot_explodes = 0;
    shotst = get_shot_model_stats(shotng->model);
    blocked_flags = get_thing_blocked_flags_at(shotng, pos);
    if ( shotst->old->field_49 ) {
        process_dig_shot_hit_wall(shotng, blocked_flags);
    }

    // If blocked by a higher wall
    if ((blocked_flags & SlbBloF_WalledZ) != 0)
    {
        long cube_id;
        cube_id = get_top_cube_at(pos->x.stl.num, pos->y.stl.num, NULL);
        doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        if (!thing_is_invalid(doortng))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_31, shotst->old->field_33, shotst->old->field_35);
            if ( shotst->old->field_36 )
              shot_explodes = 1;
            i = calculate_shot_real_damage_to_door(doortng, shotng);
            apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
        } else
        if (cube_is_water(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_37, shotst->old->field_39, 1);
            if ( shotst->old->field_3B ) {
                shot_explodes = 1;
            }
        } else
        if (cube_is_lava(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_3C, shotst->old->field_3E, 1);
            if ( shotst->old->field_40 ) {
                shot_explodes = 1;
            }
        } else
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_2B, shotst->old->field_2D, shotst->old->field_2F);
            if ( shotst->old->field_30 ) {
                shot_explodes = 1;
            }
        }
    }

    if ( !shot_explodes )
    {
        if ((blocked_flags & (SlbBloF_WalledX|SlbBloF_WalledY)) != 0)
        {
            doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
            if (!thing_is_invalid(doortng))
            {
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_31, shotst->old->field_33, shotst->old->field_35);
                if (shotst->old->field_36)
                    shot_explodes = 1;
                i = calculate_shot_real_damage_to_door(doortng, shotng);
                apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
            } else
            {
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->old->field_2B, shotst->old->field_2D, shotst->old->field_2F);
                if ( shotst->old->field_30 )
                  shot_explodes = 1;
            }
        }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->byte_16 = shotst->area_hit_type;
    }
    if ( shot_explodes )
    {
        return detonate_shot(shotng);
    }
    if (shotst->old->field_D <= 0)
    {
        slide_thing_against_wall_at(shotng, pos, blocked_flags);
    } else
    {
        bounce_thing_off_wall_at(shotng, pos, blocked_flags);
    }
    return false;
}

/**
 * Processes hitting a door by given shot.
 *
 * @param thing The thing to be moved.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit a door and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
long shot_hit_door_at(struct Thing *shotng, struct Coord3d *pos)
{
    struct Thing *efftng;
    struct ShotConfigStats *shotst;
    struct Thing *doortng;
    long blocked_flags;
    int i,n;
    TbBool shot_explodes;
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);
    //return _DK_shot_hit_door_at(thing, pos);
    shot_explodes = false;
    shotst = get_shot_model_stats(shotng->model);
    efftng = INVALID_THING;
    blocked_flags = get_thing_blocked_flags_at(shotng, pos);
    if (blocked_flags != 0)
    {
      doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
      // If we did found a door to hit
      if (!thing_is_invalid(doortng))
      {
          // If the shot hit is supposed to create effect thing
          n = shotst->old->field_31;
          if (n > 0) {
              efftng = create_effect(&shotng->mappos, n, shotng->owner);
          }
          // If the shot hit is supposed to create sound
          n = shotst->old->field_33;
          if (n > 0)
          {
              if (!thing_is_invalid(efftng)) {
                  i = shotst->old->field_35;
                  thing_play_sample(efftng, n + ACTION_RANDOM(i), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
              }
          }
          // Shall the shot be destroyed on impact
          if (shotst->old->field_36) {
              shot_explodes = true;
          }
          // Apply damage to the door
          i = calculate_shot_real_damage_to_door(doortng, shotng);
          apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
      }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->byte_16 = shotst->area_hit_type;
    }
    if ( shot_explodes )
    {
        return detonate_shot(shotng);
    }
    if (shotst->old->field_D <= 0)
    {
        slide_thing_against_wall_at(shotng, pos, blocked_flags);
    } else
    {
        bounce_thing_off_wall_at(shotng, pos, blocked_flags);
    }
    return false;
}

TbBool apply_shot_experience(struct Thing *shooter, long exp_factor, long exp_increase, long shot_model)
{
    struct ShotConfigStats *shotst;
    struct CreatureControl *shcctrl;
    long exp_mag,exp_gained;
    if (!creature_can_gain_experience(shooter))
        return false;
    shcctrl = creature_control_get_from_thing(shooter);
    shotst = get_shot_model_stats(shot_model);
    exp_mag = shotst->old->experience_given_to_shooter;
    exp_gained = (exp_mag * (exp_factor + 12 * exp_factor * exp_increase / 100) << 8) / 256;
    shcctrl->prev_exp_points = shcctrl->exp_points;
    shcctrl->exp_points += exp_gained;
    if ( check_experience_upgrade(shooter) ) {
        external_set_thing_state(shooter, CrSt_CreatureBeHappy);
    }
    return true;
}

// originally was apply_shot_experience()
TbBool apply_shot_experience_from_hitting_creature(struct Thing *shooter, struct Thing *target, long shot_model)
{
    struct CreatureStats *tgcrstat;
    struct CreatureControl *tgcctrl;
    tgcctrl = creature_control_get_from_thing(target);
    tgcrstat = creature_stats_get_from_thing(target);
    return apply_shot_experience(shooter, tgcrstat->exp_for_hitting, tgcctrl->explevel, shot_model);
}

long shot_kill_object(struct Thing *shotng, struct Thing *target)
{
    if (thing_is_dungeon_heart(target))
    {
        target->active_state = 3;
        target->health = -1;
        if (is_my_player_number(shotng->owner))
        {
            struct PlayerInfo *player;
            player = get_player(target->owner);
            if (player_exists(player) && (player->field_2C == 1))
            {
                output_message(SMsg_DefeatedKeeper, 0, 1);
            }
        }
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(shotng->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.keepers_destroyed++;
        }
        return 1;
    }
    else
    if (object_is_growing_food(target))
    {
        delete_thing_structure(target, 0);
    } else
    if (object_is_mature_food(target))
    {
        thing_play_sample(shotng, 112+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        remove_food_from_food_room_if_possible(target);
        delete_thing_structure(target, 0);
    } else
    {
        WARNLOG("Killing %s by %s is not supported",thing_model_name(target),thing_model_name(shotng));
    }
    return 0;
}

long shot_hit_object_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    //return _DK_shot_hit_object_at(shotng, target, pos);
    struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(shotng->model);
    if (!thing_is_object(target)) {
        return 0;
    }
    if (shotst->old->field_28) {
        return 0;
    }
    if (target->health < 0) {
        return 0;
    }
    struct ObjectConfig *objconf;
    objconf = get_object_model_stats2(target->model);
    if (objconf->resistant_to_nonmagic && !shotst->old->deals_magic_damage) {
        return 0;
    }
    struct Thing *creatng;
    creatng = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        creatng = thing_get(shotng->parent_idx);
    }
    if (thing_is_dungeon_heart(target))
    {
        if (shotng->model == 21) //TODO CONFIG Make config option instead of model dependency
        {
            thing_play_sample(target, 134+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        } else
        if (shotng->model == 22) //TODO CONFIG Make config option instead of model dependency
        {
            thing_play_sample(target, 144+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
        event_create_event_or_update_nearby_existing_event(
          target->mappos.x.val, target->mappos.y.val,
          EvKind_HeartAttacked, target->owner, 0);
        if (is_my_player_number(target->owner)) {
            output_message(SMsg_HeartUnderAttack, 400, 1);
        }
    } else
    {
        int i;
        i = shotst->old->hit_sound;
        if (i > 0) {
            thing_play_sample(target, i, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
    }
    if (shotng->word_14)
    {
        // Drain allows caster to regain half of damage
        if (shotst->old->health_drain && thing_is_creature(creatng)) {
            apply_health_to_thing(creatng, shotng->word_14/2);
        }
        apply_damage_to_thing(target, shotng->word_14, shotst->damage_type, -1);
        target->byte_13 = 20;
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, target);
    if (target->health < 0) {
        shot_kill_object(shotng, target);
    }
    if (shotst->old->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

long get_damage_of_melee_shot(const struct Thing *shotng, const struct Thing *target)
{
    const struct CreatureStats *tgcrstat;
    const struct CreatureControl *tgcctrl;
    long crdefense,hitchance;
    tgcrstat = creature_stats_get_from_thing(target);
    tgcctrl = creature_control_get_from_thing(target);
    crdefense = compute_creature_max_defense(tgcrstat->defense,tgcctrl->explevel);
    hitchance = ((long)shotng->shot.dexterity - crdefense) / 2;
    if (hitchance < -96) {
        hitchance = -96;
    } else
    if (hitchance > 96) {
        hitchance = 96;
    }
    if (ACTION_RANDOM(256) < (128+hitchance))
      return shotng->shot.damage;
    return 0;
}

long project_damage_of_melee_shot(long shot_dexterity, long shot_damage, const struct Thing *target)
{
    const struct CreatureStats *tgcrstat;
    const struct CreatureControl *tgcctrl;
    long crdefense,hitchance;
    tgcrstat = creature_stats_get_from_thing(target);
    tgcctrl = creature_control_get_from_thing(target);
    crdefense = compute_creature_max_defense(tgcrstat->defense,tgcctrl->explevel);
    hitchance = (shot_dexterity - crdefense) / 2;
    if (hitchance < -96) {
        hitchance = -96;
    } else
    if (hitchance > 96) {
        hitchance = 96;
    }
    // If we'd return something which rounds to 0, change it to 1. Otherwise, just use hit chance in computations.
    if (abs(shot_damage) > 256/(128+hitchance))
        return shot_damage * (128+hitchance)/256;
    else if (shot_damage > 0)
        return 1;
    else if (shot_damage < 0)
        return -1;
    return 0;
}

void create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target)
{
    struct Thing *efftng;
    //_DK_create_relevant_effect_for_shot_hitting_thing(shotng, target); return;
    efftng = INVALID_THING;
    if (target->class_id == TCls_Creature)
    {
        switch (shotng->model)
        {
        case 1:
        case 2:
        case 4:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown01, shotng->owner);
            break;
        case 5:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown13, shotng->owner);
            if ( !thing_is_invalid(efftng) ) {
                efftng->byte_16 = 2;
            }
            break;
        case 6:
        case 9:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown08, shotng->owner);
            break;
        case 14:
        case 21:
        case 22:
            if (creature_affected_by_spell(target, SplK_Freeze)) {
                efftng = create_effect(&shotng->mappos, TngEff_Unknown22, shotng->owner);
            } else
            if (creature_model_bleeds(target->model)) {
                efftng = create_effect(&shotng->mappos, TngEff_Unknown06, shotng->owner);
            }
            break;
        }
    }
    TRACE_THING(efftng);
}

long check_hit_when_attacking_door(struct Thing *thing)
{
    //return _DK_check_hit_when_attacking_door(thing);
    if (!thing_is_creature(thing))
    {
        ERRORLOG("The %s in invalid for this check", thing_model_name(thing));
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->combat_flags & CmbtF_Unknown10) != 0)
    {
        CrtrStateId crstate;
        crstate = get_creature_state_besides_move(thing);
        if (crstate != CrSt_CreatureCombatFlee)
        {
            set_start_state(thing);
            return 1;
        }
    }
    return 1;
}

/**
 * Kills a creature with given shot.
 * @param shotng The shot which is killing the victim creature.
 * @param creatng The victim creature thing.
 * @return True if the creature is being killed, false if something have failed.
 * @note sometimes named shot_kills_creature().
 */
TbBool shot_kill_creature(struct Thing *shotng, struct Thing *creatng)
{
    struct ShotConfigStats *shotst;
    struct CreatureControl *cctrl;
    shotst = get_shot_model_stats(shotng->model);
    creatng->health = -1;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->shot_model = shotng->model;
    struct Thing *killertng;
    CrDeathFlags dieflags;
    if (shotng->index == shotng->parent_idx) {
        killertng = INVALID_THING;
        dieflags = CrDed_DiedInBattle;
    } else {
        killertng = thing_get(shotng->parent_idx);
        dieflags = CrDed_DiedInBattle | (shotst->old->cannot_make_target_unconscious?CrDed_NoUnconscious:0);
    }
    return kill_creature(creatng, killertng, shotng->owner, dieflags);
}

long melee_shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    struct Thing *shooter;
    struct ShotConfigStats *shotst;
    struct CreatureControl *tgcctrl;
    long damage,throw_strength;
    shotst = get_shot_model_stats(shotng->model);
    throw_strength = shotng->field_20;
    if (trgtng->health < 0)
        return 0;
    shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index)
        shooter = thing_get(shotng->parent_idx);
    tgcctrl = creature_control_get_from_thing(trgtng);
    damage = get_damage_of_melee_shot(shotng, trgtng);
    if (damage != 0)
    {
      if (shotst->old->hit_sound > 0)
      {
          thing_play_sample(trgtng, shotst->old->hit_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
          play_creature_sound(trgtng, CrSnd_Hurt, 3, 0);
      }
      if (!thing_is_invalid(shooter)) {
          apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, shooter->owner);
      } else {
          apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, -1);
      }
      if (shotst->old->field_24 != 0) {
          tgcctrl->field_B1 = shotst->old->field_24;
      }
      if ( shotst->old->field_2A )
      {
          trgtng->acceleration.x.val += (throw_strength * (long)shotng->velocity.x.val) / 16;
          trgtng->acceleration.y.val += (throw_strength * (long)shotng->velocity.y.val) / 16;
          trgtng->field_1 |= TF1_PushdByAccel;
      }
      create_relevant_effect_for_shot_hitting_thing(shotng, trgtng);
      if (trgtng->health >= 0)
      {
          if (trgtng->owner != shotng->owner) {
              check_hit_when_attacking_door(trgtng);
          }
      } else
      {
          shot_kill_creature(shotng,trgtng);
      }
    }
    if (shotst->old->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

void clear_thing_acceleration(struct Thing *thing)
{
    thing->acceleration.x.val = 0;
    thing->acceleration.y.val = 0;
    thing->acceleration.z.val = 0;
}

void set_thing_acceleration_angles(struct Thing *thing, long angle_xy, long angle_yz)
{
    struct ComponentVector cvect;
    thing->field_52 = angle_xy;
    thing->field_54 = angle_yz;
    angles_to_vector(thing->field_52, thing->field_54, 256, &cvect);
    thing->pos_2C.x.val = cvect.x;
    thing->pos_2C.y.val = cvect.y;
    thing->pos_2C.z.val = cvect.z;
}

TbBool shot_model_makes_flesh_explosion(long shot_model)
{
    if ((shot_model == ShM_Firebomb) || (shot_model == ShM_GodLightBall))
        return true;
    return false;
}

long shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    struct Thing *shooter;
    struct Thing *efftng;
    struct ShotConfigStats *shotst;
    struct Coord3d pos2;
    long i,n,amp;
    //return _DK_shot_hit_shootable_thing_at(shotng, target, pos);
    amp = shotng->field_20;
    shotst = get_shot_model_stats(shotng->model);
    shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        shooter = thing_get(shotng->parent_idx);
    }
    if (thing_is_creature(shooter))
    {
        if (thing_is_creature(trgtng))
        {
            apply_shot_experience_from_hitting_creature(shooter, trgtng, shotng->model);
        }
    }
    if (shotst->old->is_melee != 0)
    {
        return melee_shot_hit_creature_at(shotng, trgtng, pos);
    }
    if ( (shotst->old->field_28 != 0) || (trgtng->health < 0) )
        return 0;
    if ( (creature_affected_by_spell(trgtng, SplK_Rebound)) && (shotst->old->field_29 == 0) )
    {
        struct Thing *killertng;
        killertng = INVALID_THING;
        if (shotng->index != shotng->parent_idx) {
            killertng = thing_get(shotng->parent_idx);
        }
        if ( !thing_is_invalid(killertng) )
        {
            struct CreatureStats *crstat;
            crstat = creature_stats_get_from_thing(killertng);
            pos2.x.val = killertng->mappos.x.val;
            pos2.y.val = killertng->mappos.y.val;
            pos2.z.val = crstat->eye_height + killertng->mappos.z.val;
            clear_thing_acceleration(shotng);
            set_thing_acceleration_angles(shotng, get_angle_xy_to(&shotng->mappos, &pos2), get_angle_yz_to(&shotng->mappos, &pos2));
            shotng->parent_idx = trgtng->parent_idx;
            shotng->owner = trgtng->owner;
        } else
        {
            clear_thing_acceleration(shotng);
            i = (shotng->field_52 + 1024) & 0x7FF;
            n = (shotng->field_54 + 1024) & 0x7FF;
            set_thing_acceleration_angles(shotng, i, n);
            if (trgtng->class_id == TCls_Creature)
            {
                shotng->parent_idx = trgtng->parent_idx;
            }
        }
        return 1;
    }
    // Immunity to boulders
    if (shot_is_boulder(shotng))
    {
        if ((get_creature_model_flags(trgtng) & MF_ImmuneToBoulder) != 0)
        {
            efftng = create_effect(&trgtng->mappos, TngEff_Unknown14, trgtng->owner);
            if ( !thing_is_invalid(efftng) ) {
                efftng->byte_16 = 8;
            }
            shotng->health = -1;
            return 1;
        }
    }
    if (shotst->old->hit_sound != 0)
    {
        play_creature_sound(trgtng, CrSnd_Hurt, 1, 0);
        thing_play_sample(trgtng, shotst->old->hit_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    if (shotng->word_14 != 0)
    {
        if (shotst->old->health_drain) {
            give_shooter_drained_health(shooter, shotng->word_14 / 2);
        }
        if ( !thing_is_invalid(shooter) ) {
            apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, shooter->owner);
        } else {
            apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, -1);
        }
    }
    if (shotst->old->field_24 != 0)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(trgtng);
        if (cctrl->field_B1 == 0) {
            cctrl->field_B1 = shotst->old->field_24;
        }
    }
    if (shotst->old->cast_spell_kind != 0)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(shooter);
        if (!creature_control_invalid(cctrl)) {
            n = cctrl->explevel;
        } else {
            n = 0;
        }
        apply_spell_effect_to_thing(trgtng, shotst->old->cast_spell_kind, n);
    }
    if (shotst->old->field_4B != 0)
    {
        if ( !thing_is_invalid(shooter) )
        {
            if (get_no_creatures_in_group(shooter) < 8) {
                add_creature_to_group(trgtng, shooter);
            }
        }
    }
    if (shotst->old->field_2A != 0)
    {
        i = amp * (long)shotng->velocity.x.val;
        trgtng->acceleration.x.val += i / 16;
        i = amp * (long)shotng->velocity.y.val;
        trgtng->acceleration.y.val += i / 16;
        trgtng->field_1 |= TF1_PushdByAccel;
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, trgtng);
    if (shotst->old->field_45 != 0)
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(trgtng);
        shotng->health -= crstat->damage_to_boulder;
    }
    if (trgtng->health < 0)
    {
        shot_kill_creature(shotng, trgtng);
    } else
    {
        if (trgtng->owner != shotng->owner) {
            check_hit_when_attacking_door(trgtng);
        }
    }
    if (shotst->old->destroy_on_first_hit != 0) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

long shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    //return _DK_shot_hit_shootable_thing_at(shotng, target, pos);
    if (!thing_exists(target))
        return 0;
    if (target->class_id == TCls_Object) {
        return shot_hit_object_at(shotng, target, pos);
    }
    if (target->class_id == TCls_Creature) {
        return shot_hit_creature_at(shotng, target, pos);
    }
    return 0;
}

long collide_filter_thing_is_shootable_by_any_player_including_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(coltng))
        shot_owner = coltng->owner;
    return thing_is_shootable_by_any_player_including_objects(thing, shot_owner);
}

long collide_filter_thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(coltng))
        shot_owner = coltng->owner;
    return thing_is_shootable_by_any_player_excluding_objects(thing, shot_owner);
}

long collide_filter_thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(coltng))
        shot_owner = coltng->owner;
    return thing_is_shootable_by_any_player_except_own_including_objects(thing, shot_owner);
}

long collide_filter_thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(coltng))
        shot_owner = coltng->owner;
    return thing_is_shootable_by_any_player_except_own_excluding_objects(thing, shot_owner);
}

long collide_filter_thing_is_shootable_dungeon_heart_only(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(coltng))
        shot_owner = coltng->owner;
    return (thing_is_dungeon_heart(thing) && (shot_owner != thing->owner));
}

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *thing, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5)
{
    return _DK_get_thing_collided_with_at_satisfying_filter(thing, pos, filter, a4, a5);
}

/**
 * Processes hitting another thing.
 *
 * @param thing The thing to be moved.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit something and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_something_while_moving(struct Thing *shotng, struct Coord3d *nxpos)
{
    struct Thing *target;
    SYNCDBG(18,"Starting for %s index %d, hit type %d",thing_model_name(shotng),(int)shotng->index, (int)shotng->shot.hit_type);
    target = INVALID_THING;
    switch ( shotng->shot.hit_type )
    {
    case 8:
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 1:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_by_any_player_including_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 2:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_by_any_player_excluding_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 3:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_by_any_player_except_own_including_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 4:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_by_any_player_except_own_excluding_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 7:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_dungeon_heart_only, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    case 9:
        target = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable_by_any_player_including_objects, 0, 0);
        if ( !thing_is_invalid(target) )
        {
            if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
              return true;
            break;
        }
        target = get_shot_collided_with_same_type(shotng, nxpos);
        if ( !thing_is_invalid(target) )
        {
            shotng->health = -1;
            return true;
        }
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    default:
        ERRORLOG("Shot has no hit thing type");
        if ( shot_hit_shootable_thing_at(shotng, target, nxpos) )
            return true;
        break;
    }
    return false;
}

TngUpdateRet move_shot(struct Thing *shotng)
{
    struct ShotConfigStats *shotst;
    struct Coord3d pos;
    TbBool move_allowed;
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);
    TRACE_THING(shotng);
    //return _DK_move_shot(shotng);

    move_allowed = get_thing_next_position(&pos, shotng);
    shotst = get_shot_model_stats(shotng->model);
    if ( !shotst->old->field_28 )
    {
        if ( shot_hit_something_while_moving(shotng, &pos) ) {
            return TUFRet_Deleted;
        }
    }
    if ((shotng->movement_flags & TMvF_Unknown10) != 0)
    {
      if ( (shotst->old->is_melee) && thing_in_wall_at(shotng, &pos) ) {
          if ( shot_hit_door_at(shotng, &pos) ) {
              return TUFRet_Deleted;
          }
      }
    } else
    {
      if ((!move_allowed) || thing_in_wall_at(shotng, &pos)) {
          if ( shot_hit_wall_at(shotng, &pos) ) {
              return TUFRet_Deleted;
          }
      }
    }
    move_thing_in_map(shotng, &pos);
    return TUFRet_Modified;
}

TngUpdateRet update_shot(struct Thing *thing)
{
    struct ShotConfigStats *shotst;
    struct PlayerInfo *myplyr;
    struct PlayerInfo *player;
    struct Thing *target;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct CoordDelta3d dtpos;
    struct ComponentVector cvect;
    long i;
    TbBool hit;
    SYNCDBG(18,"Starting for index %d, model %d",(int)thing->index,(int)thing->model);
    TRACE_THING(thing);
    //return _DK_update_shot(thing);
    hit = false;
    shotst = get_shot_model_stats(thing->model);
    myplyr = get_my_player();
    if (shotst->old->shot_sound != 0)
    {
        if (!S3DEmitterIsPlayingSample(thing->snd_emitter_id, shotst->old->shot_sound, 0))
            thing_play_sample(thing, shotst->old->shot_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    if (shotst->old->field_47)
        thing->health--;
    if (thing->health < 0)
    {
        hit = true;
    } else
    {
        switch ( thing->model )
        {
        case ShM_Firebomb:
            for (i = 2; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(127) + 63;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(127) + 63;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(127) + 63;
              create_thing(&pos1, TCls_EffectElem, 1, thing->owner, -1);
            }
            break;
        case ShM_Lightning:
            if ( lightning_is_close_to_player(myplyr, &thing->mappos) )
            {
              if (is_my_player_number(thing->owner))
              {
                  player = get_player(thing->owner);
                  if ((thing->parent_idx != 0) && (myplyr->controlled_thing_idx == thing->parent_idx))
                  {
                      PaletteSetPlayerPalette(player, lightning_palette);
                      myplyr->field_3 |= 0x08;
                  }
              }
            }
            break;
        case ShM_NaviMissile:
            target = thing_get(thing->shot.target_idx);
            if ((thing_exists(target)) && (target->class_id == TCls_Creature))
            {
                pos2.x.val = target->mappos.x.val;
                pos2.y.val = target->mappos.y.val;
                pos2.z.val = target->mappos.z.val;
                pos2.z.val += (target->field_58 >> 1);
                thing->field_52 = get_angle_xy_to(&thing->mappos, &pos2);
                thing->field_54 = get_angle_yz_to(&thing->mappos, &pos2);
                angles_to_vector(thing->field_52, thing->field_54, shotst->old->speed, &cvect);
                dtpos.x.val = cvect.x - thing->pos_2C.x.val;
                dtpos.y.val = cvect.y - thing->pos_2C.y.val;
                dtpos.z.val = cvect.z - thing->pos_2C.z.val;
                cvect.x = dtpos.x.val;
                cvect.y = dtpos.y.val;
                cvect.z = dtpos.z.val;
                i = LbSqrL(dtpos.x.val*(long)dtpos.x.val + dtpos.y.val*(long)dtpos.y.val + dtpos.z.val*(long)dtpos.z.val);
                if (i > 128)
                {
                  dtpos.x.val = ((long)cvect.x << 7) / i;
                  dtpos.y.val = ((long)cvect.y << 7) / i;
                  dtpos.z.val = ((long)cvect.z << 7) / i;
                  cvect.x = dtpos.x.val;
                  cvect.y = dtpos.y.val;
                  cvect.z = dtpos.z.val;
                }
                thing->acceleration.x.val += cvect.x;
                thing->acceleration.y.val += cvect.y;
                thing->acceleration.z.val += cvect.z;
                thing->field_1 |= TF1_PushdByAccel;
            }
            break;
        case ShM_Wind:
            for (i = 10; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(1023) + 511;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(1023) + 511;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(1023) + 511;
              create_thing(&pos1, TCls_EffectElem, 12, thing->owner, -1);
            }
            affect_nearby_enemy_creatures_with_wind(thing);
            break;
        case ShM_Grenade:
            thing->field_52 = (thing->field_52 + 113) & 0x7FF;
            break;
        case ShM_Boulder:
        case ShM_SolidBoulder:
            if ( apply_wallhug_force_to_boulder(thing) )
              hit = true;
            break;
        case ShM_GodLightning:
            draw_god_lightning(thing);
            lightning_modify_palette(thing);
            break;
        case ShM_Vortex:
            affect_nearby_stuff_with_vortex(thing);
            break;
        case ShM_Alarm:
            affect_nearby_friends_with_alarm(thing);
            break;
        case ShM_GodLightBall:
            update_god_lightning_ball(thing);
            break;
        case ShM_TrapLightning:
            if (((game.play_gameturn - thing->creation_turn) % 16) == 0)
            {
              thing->field_19 = 5;
              god_lightning_choose_next_creature(thing);
              target = thing_get(thing->shot.target_idx);
              if (thing_exists(target))
              {
                  shotst = get_shot_model_stats(24);
                  draw_lightning(&thing->mappos,&target->mappos, 96, 60);
                  apply_damage_to_thing_and_display_health(target, shotst->old->damage, shotst->damage_type, thing->owner);
              }
            }
            break;
        default:
            // All shots that do not require special processing
            break;
        }
    }
    if (!thing_exists(thing)) {
        WARNLOG("Thing disappeared during update");
        return TUFRet_Deleted;
    }
    if (hit) {
        detonate_shot(thing);
        return TUFRet_Deleted;
    }
    return move_shot(thing);
}

struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct ShotConfigStats *shotst;
    struct InitLight ilght;
    struct Thing *thing;
    //return _DK_create_shot(pos, model, owner);
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create shot %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    shotst = get_shot_model_stats(model);
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate shot %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->creation_turn = game.play_gameturn;
    thing->class_id = TCls_Shot;
    thing->model = model;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->field_22 = shotst->old->field_D;
    thing->field_20 = shotst->old->field_F;
    thing->field_21 = shotst->old->field_10;
    thing->field_23 = shotst->old->field_11;
    thing->field_24 = shotst->old->field_12;
    thing->movement_flags ^= (thing->movement_flags ^ TMvF_Unknown08 * shotst->old->field_13) & TMvF_Unknown08;
    set_thing_draw(thing, shotst->old->numfield_0, 256, shotst->old->numfield_2, 0, 0, 2);
    thing->field_4F ^= (thing->field_4F ^ 0x02 * shotst->old->field_6) & 0x02;
    thing->field_4F ^= thing->field_4F ^ ((thing->field_4F ^ 0x10 * shotst->old->field_8) & 0x30);
    thing->field_4F ^= (thing->field_4F ^ shotst->old->field_7) & 0x01;
    thing->sizexy = shotst->old->field_9;
    thing->field_58 = shotst->old->field_B;
    thing->field_5A = shotst->old->field_9;
    thing->field_5C = shotst->old->field_B;
    thing->shot.damage = shotst->old->damage;
    thing->shot.dexterity = 255;
    thing->health = shotst->health;
    if (shotst->old->field_50)
    {
        LbMemorySet(&ilght, 0, sizeof(struct InitLight));
        memcpy(&ilght.mappos,&thing->mappos,sizeof(struct Coord3d));
        ilght.field_0 = shotst->old->field_50;
        ilght.field_2 = shotst->old->field_52;
        ilght.is_dynamic = 1;
        ilght.field_3 = shotst->old->field_53;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id == 0) {
            // Being out of free lights is quite common - so info instead of warning here
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    place_thing_in_mapwho(thing);
    add_thing_to_its_class_list(thing);
    return thing;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
