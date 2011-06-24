/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_train.c
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
#include "creature_states_train.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_instances.h"
#include "thing_list.h"
#include "creature_control.h"
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

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_training_room(struct Thing *creatng);
DLLIMPORT short _DK_training(struct Thing *creatng);
DLLIMPORT long _DK_creature_can_be_trained(struct Thing *creatng);
DLLIMPORT long _DK_player_can_afford_to_train_creature(struct Thing *creatng);
DLLIMPORT void _DK_process_creature_in_training_room(struct Thing *creatng, struct Room *room);
DLLIMPORT void _DK_setup_move_to_new_training_position(struct Thing *creatng, struct Room *room, unsigned long restart);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Returns if the creature meets conditions to be trained.
 *
 * @param thing The creature thing to be tested.
 * @return
 */
TbBool creature_can_be_trained(struct Thing *thing)
{
    struct CreatureStats *crstat;
    //return _DK_creature_can_be_trained(thing);
    crstat = creature_stats_get_from_thing(thing);
    // Creatures without training value can't be trained
    if (crstat->training_value <= 0)
        return false;
    // If its model can train, check if this one can gain more experience
    return creature_can_gain_experience(thing);
}

TbBool player_can_afford_to_train_creature(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    //return _DK_player_can_afford_to_train_creature(thing);
    dungeon = get_dungeon(thing->owner);
    crstat = creature_stats_get_from_thing(thing);
    return (dungeon->total_money_owned >= crstat->training_cost);
}

void setup_training_move(struct Thing *creatng, SubtlCodedCoords stl_num)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->moveto_pos.x.val = subtile_coord_center(stl_num_decode_x(stl_num));
    cctrl->moveto_pos.y.val = subtile_coord_center(stl_num_decode_y(stl_num));
    cctrl->moveto_pos.z.val = get_thing_height_at(creatng, &cctrl->moveto_pos);
    if (thing_in_wall_at(creatng, &cctrl->moveto_pos))
    {
        ERRORLOG("Illegal setup to wall at (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(creatng);
    }
}

void setup_training_move_near(struct Thing *creatng, SubtlCodedCoords stl_num)
{
    SubtlCodedCoords near_stl_num;
    long dist_x,dist_y;
    long stl_x,stl_y;
    stl_x = stl_num_decode_x(stl_num);
    stl_y = stl_num_decode_y(stl_num);
    // Select a subtile closer to current position
    dist_x = stl_x - (long)creatng->mappos.x.stl.num;
    dist_y = stl_y - (long)creatng->mappos.y.stl.num;
    if (abs(dist_x) > abs(dist_y))
    {
        if (dist_x > 0) {
            stl_x -= 1;
        } else {
            stl_x += 1;
        }
    } else
    {
        if (dist_y > 0) {
            stl_y -= 1;
        } else {
            stl_y += 1;
        }
    }
    near_stl_num = get_subtile_number(stl_x,stl_y);
    setup_training_move(creatng, near_stl_num);
}

struct Thing *get_creature_in_training_room_which_could_accept_partner(struct Room *room, struct Thing *partnertng)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (thing != partnertng)
        {
            if ( (get_creature_state_besides_move(thing) == CrSt_Training) && (cctrl->training.partner_idx == 0) )
            {
                if (subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num) == room) {
                    return thing;
                } else {
                    WARNLOG("The %s pretends to be in room but it's not.",thing_model_name(thing));
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return INVALID_THING;
}

void setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long restart)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *prtng;
    struct CreatureControl *prctrl;
    struct Coord3d pos;
    long i;
    SYNCDBG(8,"Starting for %s",thing_model_name(thing));
    //_DK_setup_move_to_new_training_position(thing, room, a3);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    if ( restart )
      cctrl->training.search_timeout = 50;
    // Try partner training
    if ((crstat->partner_training > 0) && (ACTION_RANDOM(100) < crstat->partner_training))
    {
        prtng = get_creature_in_training_room_which_could_accept_partner(room, thing);
        if (!thing_is_invalid(prtng))
        {
            SYNCDBG(7,"The %s found %s as training partner.",thing_model_name(thing),thing_model_name(prtng));
            prctrl = creature_control_get_from_thing(prtng);
            prctrl->training.mode = CrTrMd_PartnerTraining;
            prctrl->training.train_timeout = 75;
            prctrl->training.partner_idx = thing->index;
            prctrl->training.partner_field9 = thing->field_9;
            cctrl->training.mode = CrTrMd_PartnerTraining;
            cctrl->training.train_timeout = 75;
            cctrl->training.partner_idx = prtng->index;
            cctrl->training.partner_field9 = prtng->field_9;
            return;
      }
    }
    // No partner - train at some random position
    cctrl->training.mode = CrTrMd_SearchForTrainPost;
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(8,"Going to train at (%d,%d)",(int)pos.x.stl.num,(int)pos.y.stl.num);
        i = get_subtile_number(pos.x.stl.num,pos.y.stl.num);
        setup_training_move(thing, i);
    } else {
        SYNCDBG(8,"No new position found, staying at (%d,%d)",(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.x.stl.num);
    }
    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
    }
}

/**
 *  Finds a random training post near to the current position of given creature.
 *  Used when finding a training post seems to be taking too long; in that case, creature should start training with a nearest post.
 *  Note that this routine does not always select the nearest post - it is enough if it's 3 subtiles away.
 *
 * @param creatng The creature who wish to train with training post.
 */
void setup_training_search_for_post(struct Thing *creatng)
{
    struct Room *room;
    struct Thing *traintng;
    struct Thing *thing;
    long start_slab;
    long min_distance,dist;
    long slb_x,slb_y;
    long i,k;
    room = subtile_room_get(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    // Let's start from a random slab
    slb_x = -1;
    slb_y = -1;
    min_distance = LONG_MAX;
    traintng = INVALID_THING;
    start_slab = ACTION_RANDOM(room->slabs_count);
    k = start_slab;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (k <= 0)
            break;
        k--;
    }
    // Got random starting slab, now sweep room slabs from it
    thing = INVALID_THING;
    k = room->slabs_count;
    i = get_slab_number(slb_x,slb_y);
    while (k > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (i == 0)
          i = room->slabs_list;
        // Per room tile code - find a nearest training post
        thing = get_object_at_subtile_of_model_and_owned_by(3*slb_x+1, 3*slb_y+1, 31, creatng->owner);
        if (!thing_is_invalid(thing))
        {
            dist = get_2d_distance(&creatng->mappos, &thing->mappos);
            if (dist < min_distance) {
                traintng = thing;
                min_distance = dist;
                if (min_distance < (3<<8))
                    break;
            }
        }
        // Per room tile code ends
        k--;
    }
    // Got trainer (or not...), now do the correct action
    if (thing_is_invalid(thing))
    {
        SYNCDBG(6,"Room no longer have training post, moving somewhere else.");
        setup_move_to_new_training_position(creatng, room, true);
    } else
    {
        i = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
        setup_training_move_near(creatng, i);
    }
}

struct Thing *find_training_post_just_next_to_creature(struct Thing *creatng)
{
    long i;
    struct Thing *traintng;
    traintng = INVALID_THING;
    for (i=0; i < 4; i++)
    {
        long stl_x,stl_y;
        stl_x = creatng->mappos.x.stl.num + (long)small_around[i].delta_x;
        stl_y = creatng->mappos.y.stl.num + (long)small_around[i].delta_y;
        traintng = get_object_at_subtile_of_model_and_owned_by(stl_x, stl_y, 31, creatng->owner);
        if (!thing_is_invalid(traintng))
            break;
    }
    return traintng;
}

void process_creature_in_training_room(struct Thing *thing, struct Room *room)
{
    static const struct Around corners[] = {
        {1, 2},
        {0, 1},
        {1, 0},
        {2, 1},
    };
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *traintng;
    struct Thing *crtng;
    struct CreatureControl *cctrl2;
    struct Coord3d pos;
    long speed,dist;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    SYNCDBG(8,"Starting %s mode %d",thing_model_name(thing),(int)cctrl->training.mode);
    //_DK_process_creature_in_training_room(thing, room); return;
    cctrl->field_4A = 0;
    switch (cctrl->training.mode)
    {
    case CrTrMd_SearchForTrainPost:
        // While we're in an instance, just wait
        if (cctrl->instance_id != CrInst_NULL)
            break;
        // On timeout, search for nearby training posts to start training ASAP
        if (cctrl->training.search_timeout < 1)
        {
            SYNCDBG(6,"Search timeout - selecting post nearest to (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
            setup_training_search_for_post(thing);
            cctrl->training.search_timeout = 100;
            break;
        }
        // Do a moving step
        cctrl->training.search_timeout--;
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // Move target is reached - find a training post which is supposed to be around here
            traintng = find_training_post_just_next_to_creature(thing);
            if (thing_is_invalid(traintng))
            {
                SYNCDBG(6,"Reached (%d,%d) but there's no training post there",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
                setup_move_to_new_training_position(thing, room, false);
                break;
            }
            // Found - go to next mode
            cctrl->training.mode = CrTrMd_SelectPositionNearTrainPost;
            cctrl->training.search_timeout = 50;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get to (%d,%d) in the training room",(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.y.stl.num);
            set_start_state(thing);
        }
        break;
    case CrTrMd_SelectPositionNearTrainPost:
        for (i=0; i < 4; i++)
        {
            long slb_x,slb_y;
            long stl_x,stl_y;
            struct SlabMap *slb;
            slb_x = map_to_slab[thing->mappos.x.stl.num] + (long)small_around[i].delta_x;
            slb_y = map_to_slab[thing->mappos.y.stl.num] + (long)small_around[i].delta_y;
            slb = get_slabmap_block(slb_x,slb_y);
            if ((slb->kind != SlbT_TRAINING) || (slabmap_owner(slb) != thing->owner))
                continue;
            stl_x = 3*slb_x + (long)corners[i].delta_x;
            stl_y = 3*slb_y + (long)corners[i].delta_y;
            traintng = INVALID_THING;
            // Check if any other creature is using that post; allow only unused posts
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(stl_x, stl_y, -1, thing->owner, thing->index);
            if (thing_is_invalid(crtng))
            {
                traintng = get_object_at_subtile_of_model_and_owned_by(3*slb_x+1, 3*slb_y+1, 31, thing->owner);
            }
            if (!thing_is_invalid(traintng))
            {
                cctrl->training.pole_stl_x = 3 * map_to_slab[thing->mappos.x.stl.num] + 1;
                cctrl->training.pole_stl_y = 3 * map_to_slab[thing->mappos.y.stl.num] + 1;
                cctrl->moveto_pos.x.stl.num = stl_x;
                cctrl->moveto_pos.y.stl.num = stl_y;
                cctrl->moveto_pos.x.stl.pos = 128;
                cctrl->moveto_pos.y.stl.pos = 128;
                cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
                if (thing_in_wall_at(thing, &cctrl->moveto_pos))
                {
                    ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
                    break;
                }
                cctrl->training.mode = CrTrMd_MoveToTrainPost;
                break;
            }
        }
        if (cctrl->training.mode == CrTrMd_SelectPositionNearTrainPost)
          setup_move_to_new_training_position(thing, room, 1);
        break;
    case CrTrMd_MoveToTrainPost:
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // If there's already someone training at that position, go somewhere else
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, -1, thing->owner, thing->index);
            if (!thing_is_invalid(crtng))
            {
                setup_move_to_new_training_position(thing, room, 1);
                break;
            }
            // Otherwise, train at this position
            cctrl->training.mode = CrTrMd_TurnToTrainPost;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case CrTrMd_TurnToTrainPost:
        pos.x.val = ((long)cctrl->training.pole_stl_x << 8) + 128;
        pos.y.val = ((long)cctrl->training.pole_stl_y << 8) + 128;
        if (creature_turn_to_face(thing, &pos) < 56)
        {
          cctrl->training.mode = CrTrMd_DoTrainWithTrainPost;
          cctrl->training.train_timeout = 75;
        }
        break;
    case CrTrMd_PartnerTraining:
        if (cctrl->training.partner_idx == 0)
        {
            setup_move_to_new_training_position(thing, room, false);
            return;
        }
        crtng = thing_get(cctrl->training.partner_idx);
        if (!thing_exists(crtng) || (get_creature_state_besides_move(crtng) != CrSt_Training) || (crtng->field_9 != cctrl->training.partner_field9))
        {
            SYNCDBG(8,"The %s cannot start partner training - creature to train with is gone.",thing_model_name(thing));
            setup_move_to_new_training_position(thing, room, false);
            return;
        }
        cctrl2 = creature_control_get_from_thing(crtng);
        if (cctrl2->training.partner_idx != thing->index)
        {
            SYNCDBG(6,"The %s cannot start partner training - %s changed the partner.",thing_model_name(thing),thing_model_name(crtng));
            cctrl->training.partner_idx = 0;
            setup_move_to_new_training_position(thing, room, false);
            break;
        }
        if (subtile_room_get(crtng->mappos.x.stl.num, crtng->mappos.y.stl.num) != room)
        {
            SYNCDBG(8,"The %s cannot start partner training - partner has left the room.",thing_model_name(thing));
            cctrl->training.partner_idx = 0;
            cctrl2->training.partner_idx = 0;
            setup_move_to_new_training_position(thing, room, false);
            break;
        }
        crstat = creature_stats_get_from_thing(thing);
        dist = get_combat_distance(thing, crtng);
        if (dist > 284)
        {
            if (creature_move_to(thing, &crtng->mappos, get_creature_speed(thing), 0, 0) == -1)
            {
              WARNLOG("The %s cannot navigate to training partner",thing_model_name(thing));
              setup_move_to_new_training_position(thing, room, false);
              cctrl->training.partner_idx = 0;
            }
        } else
        if (dist >= 156)
        {
            if (creature_turn_to_face(thing, &crtng->mappos) < 56)
            {
              cctrl->training.train_timeout--;
              if (cctrl->training.train_timeout > 0)
              {
                if ((cctrl->instance_id == CrInst_NULL) && ((cctrl->training.train_timeout % 8) == 0))
                {
                    set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
                }
              } else
              {
                if (cctrl->instance_id == CrInst_NULL)
                {
                    setup_move_to_new_training_position(thing, room, false);
                    cctrl->training.partner_idx = 0;
                } else
                {
                    cctrl->training.train_timeout = 1;
                }
                cctrl->exp_points += (room->efficiency * crstat->training_value);
              }
            }
        } else
        {
            creature_retreat_from_combat(thing, crtng, 33, 0);
        }
        break;
    case CrTrMd_DoTrainWithTrainPost:
    default:
        if (cctrl->training.train_timeout > 0)
        {
            // While training timeout is positive, continue initiating the train instances
            cctrl->training.train_timeout--;
            if ((cctrl->instance_id == CrInst_NULL) && ((cctrl->training.train_timeout % 8) == 0))
            {
                set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
            }
        } else
        {
            // Wait for the instance to end, then select new move position
            if (cctrl->instance_id != CrInst_NULL)
            {
                cctrl->training.train_timeout = 0;
            } else
            {
                cctrl->training.train_timeout = 0;
                setup_move_to_new_training_position(thing, room, true);
            }
        }
        break;
    }
    SYNCDBG(18,"End");
}

short at_training_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_training_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    if ( !creature_can_be_trained(thing) )
    {
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (dungeon->total_money_owned < crstat->training_cost)
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_TRAINING) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Training);
    setup_move_to_new_training_position(thing, room, 1);
    cctrl->field_82 = 0;
    return 1;
}

short training(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    TbBool finish_training;
    struct Room *room;
    long i;
    SYNCDBG(18,"Starting");
    //return _DK_training(thing);
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    // Check if we should finish training
    finish_training = false;
    if (!creature_can_be_trained(thing))
    {
        SYNCDBG(9,"Ending training %s level %d; creature isn't trainable",thing_model_name(thing),(int)cctrl->explevel);
        finish_training = true;
    }
    if (!player_can_afford_to_train_creature(thing))
    {
        SYNCDBG(19,"Ending training %s index %d; cannot afford",thing_model_name(thing),(int)thing->index);
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        finish_training = true;
    }
    // Check if we're in correct room
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (room->kind != RoK_TRAINING)
     || (cctrl->work_room_id != room->index) || (room->owner != thing->owner))
    {
        SYNCDBG(9,"Ending training %s index %d; training room no longer valid",thing_model_name(thing),(int)thing->index);
        finish_training = true;
    }
    if (finish_training)
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    // Pay for the training
    cctrl->field_82++;
    if (cctrl->field_82 >= game.train_cost_frequency)
    {
        cctrl->field_82 -= game.train_cost_frequency;
        if (take_money_from_dungeon(thing->owner, crstat->training_cost, 1) < 0)
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)crstat->training_cost,(int)thing->owner);
        create_price_effect(&thing->mappos, thing->owner, crstat->training_cost);
    }
    if ((cctrl->instance_id != CrInst_NULL) || !check_experience_upgrade(thing))
    {
        i = process_work_speed_on_work_value(thing,
            (long)room->efficiency * (long)crstat->training_value);
        cctrl->exp_points += i;
        dungeon->total_experience_creatures_gained += i;
        process_creature_in_training_room(thing, room);
    } else
    {
        if (external_set_thing_state(thing, CrSt_CreatureBeHappy)) {
            cctrl->field_282 = 50;
        }
        dungeon->lvstats.creatures_trained++;
    }
    return 1;
}

/******************************************************************************/