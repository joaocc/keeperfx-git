/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_instances.c
 *     Player instances support and switching code.
 * @par Purpose:
 *     Supports various states of a player, and switching between them.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_instances.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "bflib_planar.h"

#include "creature_control.h"
#include "creature_states.h"
#include "creature_graphics.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "thing_stats.h"
#include "light_data.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_traps.h"
#include "thing_physics.h"
#include "front_simple.h"
#include "frontend.h"
#include "power_hand.h"
#include "player_utils.h"
#include "room_workshop.h"
#include "magic.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "engine_arrays.h"
#include "engine_redraw.h"
#include "sounds.h"
#include "config_settings.h"
#include "config_terrain.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, long *n);
long pinstfm_hand_grab(struct PlayerInfo *player, long *n);
long pinstfe_hand_grab(struct PlayerInfo *player, long *n);
long pinstfs_hand_drop(struct PlayerInfo *player, long *n);
long pinstfe_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip(struct PlayerInfo *player, long *n);
long pinstfm_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfs_direct_control_creature(struct PlayerInfo *player, long *n);
long pinstfs_passenger_control_creature(struct PlayerInfo *player, long *n);
long pinstfm_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
long pinstfm_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
long pinstfe_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_query_creature(struct PlayerInfo *player, long *n);
long pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo player_instance_info[] = {
  { 0, 0, NULL,                        NULL,                        NULL,                                {0}, {0}, 0, 0},
  { 3, 1, pinstfs_hand_grab,           pinstfm_hand_grab,           pinstfe_hand_grab,                   {0}, {0}, 0, 0},
  { 3, 1, pinstfs_hand_drop,           pinstfm_hand_drop,           pinstfe_hand_drop,                   {0}, {0}, 0, 0},
  { 4, 0, pinstfs_hand_whip,           NULL,                        pinstfe_hand_whip,                   {0}, {0}, 0, 0},
  { 5, 0, pinstfs_hand_whip_end,       NULL,                        pinstfe_hand_whip_end,               {0}, {0}, 0, 0},
  {12, 1, pinstfs_direct_control_creature,pinstfm_control_creature, pinstfe_direct_control_creature,     {0}, {0}, 0, 0},
  {12, 1, pinstfs_passenger_control_creature,pinstfm_control_creature,pinstfe_passenger_control_creature,{0}, {0}, 0, 0},
  {12, 1, pinstfs_direct_leave_creature,pinstfm_leave_creature,     pinstfe_leave_creature,              {0}, {0}, 0, 0},
  {12, 1, pinstfs_passenger_leave_creature,pinstfm_leave_creature,  pinstfe_leave_creature,              {0}, {0}, 0, 0},
  { 0, 1, pinstfs_query_creature,      NULL,                        NULL,                                {0}, {0}, 0, 0},
  { 0, 1, pinstfs_unquery_creature,    NULL,                        NULL,                                {0}, {0}, 0, 0},
  {16, 1, pinstfs_zoom_to_heart,       pinstfm_zoom_to_heart,       pinstfe_zoom_to_heart,               {0}, {0}, 0, 0},
  {16, 1, pinstfs_zoom_out_of_heart,   pinstfm_zoom_out_of_heart,   pinstfe_zoom_out_of_heart,           {0}, {0}, 0, 0},
  {12, 1, NULL,                        pinstfm_control_creature_fade,pinstfe_control_creature_fade,      {0}, {0}, 0, 0},
  { 8, 1, pinstfs_fade_to_map,         pinstfm_fade_to_map,         pinstfe_fade_to_map,                 {0}, {0}, 0, 0},
  { 8, 1, pinstfs_fade_from_map,       pinstfm_fade_from_map,       pinstfe_fade_from_map,               {0}, {0}, 0, 0},
  {-1, 1, pinstfs_zoom_to_position,    pinstfm_zoom_to_position,    pinstfe_zoom_to_position,            {0}, {0}, 0, 0},
  { 0, 0, NULL,                        NULL,                        NULL,                                {0}, {0}, 0, 0},
  { 0, 0, NULL,                        NULL,                        NULL,                                {0}, {0}, 0, 0},
};

/******************************************************************************/

DLLIMPORT long _DK_pinstfs_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_query_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT void _DK_process_player_instance(struct PlayerInfo *player);
DLLIMPORT void _DK_process_player_instances(void);
DLLIMPORT void _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);
DLLIMPORT void _DK_leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfs_hand_grab(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->hand_thing_idx);
  if (dungeon->num_things_in_hand)
  {
    dungeon->field_43 = 60;
    dungeon->field_53 = 40;
  }
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 783, 256);
  return 0;

}

long pinstfm_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_grab(player, n);
  struct CreaturePickedUpOffset *pickoffs;
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfs_hand_grab(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->influenced_thing_idx);
  if (thing->class_id == TCls_Creature)
  {
      pickoffs = get_creature_picked_up_offset(thing);
      dungeon->field_43 += (pickoffs->field_4 - 60) / 4;
      dungeon->field_53 += (pickoffs->field_6 - 40) / 4;
      return 0;
  }
  else
  {
    dungeon->field_43 = 60;
    dungeon->field_53 = 40;
    return 0;
  }
}

long pinstfe_hand_grab(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_hand_grab(player, n);
  struct Thing *dsttng;
  struct Thing *grabtng;
  long i;
  SYNCDBG(8,"Starting");
  dsttng = thing_get(player->influenced_thing_idx);
  grabtng = thing_get(player->hand_thing_idx);
  if (!thing_is_pickable_by_hand(player,dsttng)
   || (dsttng->creation_turn != player->influenced_thing_creation))
  {
      player->influenced_thing_creation = 0;
      player->influenced_thing_idx = 0;
      return 0;
  }
  set_power_hand_offset(player, dsttng);
  switch (dsttng->class_id)
  {
  case TCls_Creature:
      if (!external_set_thing_state(dsttng, CrSt_InPowerHand))
          return 0;
      if (creature_affected_by_spell(dsttng, SplK_Chicken))
          i = convert_td_iso(122);
      else
          i = get_creature_anim(dsttng, 9);
      set_thing_draw(dsttng, i, 256, -1, -1, 0, 2);
      break;
  case TCls_Object:
      dsttng = process_object_being_picked_up(dsttng, grabtng->owner);
      if (thing_is_invalid(dsttng))
      {
          player->influenced_thing_creation = 0;
          player->influenced_thing_idx = 0;
          return 0;
      }
      break;
  }
  if (!thing_is_invalid(grabtng))
    set_power_hand_graphic(player->id_number, 784, 256);
  insert_thing_into_power_hand_list(dsttng, player->id_number);
  player->influenced_thing_creation = 0;
  player->influenced_thing_idx = 0;
  place_thing_in_limbo(dsttng);
  return 0;
}

long pinstfs_hand_drop(struct PlayerInfo *player, long *n)
{
    struct Thing *thing;
    struct Dungeon *dungeon;
    //return _DK_pinstfs_hand_drop(player, n);
    dungeon = get_players_dungeon(player);
    thing = thing_get(player->hand_thing_idx);
    player->influenced_thing_idx = dungeon->things_in_hand[0];
    if (!thing_is_invalid(thing))
      set_power_hand_graphic(player->id_number, 783, -256);
    return 0;
}

long pinstfe_hand_drop(struct PlayerInfo *player, long *n)
{
    struct Thing *thing;
    struct Dungeon *dungeon;
    //return _DK_pinstfe_hand_drop(player, n);
    dungeon = get_players_dungeon(player);
    thing = thing_get(player->hand_thing_idx);
    dungeon->field_43 = 60;
    dungeon->field_53 = 40;
    if (!thing_is_invalid(thing))
      set_power_hand_graphic(player->id_number, 782, 256);
    player->influenced_thing_idx = 0;
    return 0;
}

long pinstfs_hand_whip(struct PlayerInfo *player, long *n)
{
    struct Thing *thing;
    //return _DK_pinstfs_hand_whip(player, n);
    thing = thing_get(player->hand_thing_idx);
    if (!thing_is_invalid(thing))
      set_power_hand_graphic(player->id_number, 786, 256);
    return 0;
}

long pinstfe_hand_whip(struct PlayerInfo *player, long *n)
{
  struct Thing *efftng;
  struct Thing *thing;
  struct Camera *cam;
  struct Coord3d pos;
  //return _DK_pinstfe_hand_whip(player, n);

  thing = thing_get(player->influenced_thing_idx);
  if (!thing_exists(thing) || (thing->creation_turn != player->influenced_thing_creation) || (!thing_slappable(thing, player->id_number)))
  {
      player->influenced_thing_creation = 0;
      player->influenced_thing_idx = 0;
      return 0;
  }
  switch (thing->class_id)
  {
  case TCls_Creature:
      if (creature_affected_by_spell(thing, SplK_Freeze))
      {
          kill_creature(thing, INVALID_THING, thing->owner, CrDed_Default);
      } else
      {
          slap_creature(player, thing);
          pos.x.val = thing->mappos.x.val;
          pos.y.val = thing->mappos.y.val;
          pos.z.val = thing->mappos.z.val + (thing->field_58 >> 1);
          if ( creature_model_bleeds(thing->model) )
              create_effect(&pos, TngEff_Unknown06, thing->owner);
          thing_play_sample(thing, 75, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
          cam = player->acamera;
          if (cam != NULL)
          {
            thing->pos_2C.x.val += distance_with_angle_to_coord_x(64, cam->orient_a);
            thing->pos_2C.y.val += distance_with_angle_to_coord_y(64, cam->orient_a);
          }
      }
      break;
  case TCls_Shot:
      if (thing->model == 15)
      {
          thing->field_52 = player->acamera->orient_a;
          thing->health -= game.boulder_reduce_health_slap;
      } else
      if (thing->model == 20)
      {
          thing->field_52 = player->acamera->orient_a;
      }
      break;
  case TCls_Trap:
      if (thing->model == 1)
        external_activate_trap_shot_at_angle(thing, player->acamera->orient_a);
      break;
  case TCls_Object:
      if (object_is_slappable(thing, player->id_number))
      {
        efftng = create_effect(&thing->mappos, TngEff_Unknown49, thing->owner);
        if (!thing_is_invalid(efftng))
          thing_play_sample(efftng, 75, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        slap_object(thing);
      }
      break;
  }
  set_player_instance(player, PI_WhipEnd, false);
  return 0;
}

long pinstfm_hand_drop(struct PlayerInfo *player, long *n)
{
  struct Dungeon *dungeon;
  long i;
  //return _DK_pinstfm_hand_drop(player, n);
  dungeon = get_players_dungeon(player);
  i = player->field_4B1+1;
  if (i < 1) i = 1;
  dungeon->field_43 += (60 - dungeon->field_43) / i;
  dungeon->field_53 += (40 - dungeon->field_53) / i;
  return 0;
}

long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_hand_whip_end(player, n);
  thing = thing_get(player->hand_thing_idx);
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 787, 256);
  return 0;
}

long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfe_hand_whip_end(player, n);
  thing = thing_get(player->hand_thing_idx);
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 785, 256);
  return 0;
}

long pinstfs_passenger_control_creature(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfs_control_creature(player, n);
  player->field_0 |= 0x80;
  if (is_my_player(player))
  {
    player->field_4C5 = 1;
    turn_off_all_window_menus();
    turn_off_menu(GMnu_CREATURE_QUERY1);
    turn_off_menu(GMnu_CREATURE_QUERY2);
    game.field_15038E = 0;
    game.flags_font |= FFlg_unk04;
  }
  cam = player->acamera;
  player->field_0 |= 0x10;
  player->dungeon_camera_zoom = get_camera_zoom(cam);
  // Play possession sound
  if (is_my_player(player))
      play_non_3d_sample(39);
  return 0;
}

long pinstfs_direct_control_creature(struct PlayerInfo *player, long *n)
{
    // Reset state of the thing being possessed
    struct Thing *thing;
    thing = thing_get(player->influenced_thing_idx);
    if (thing_is_creature(thing)) {
        SYNCDBG(8,"Cleaning up state %s of %s index %d",creature_state_code_name(thing->active_state),thing_model_name(thing),(int)thing->index);
        initialise_thing_state(thing, CrSt_ManualControl);
    }
    return pinstfs_passenger_control_creature(player, n);
}

long pinstfm_control_creature(struct PlayerInfo *player, long *n)
{
    struct CreatureStats *crstat;
    struct Thing *thing;
    struct Camera *cam;
    long mv_x,mv_y,mv_a;
//    return _DK_pinstfm_control_creature(player, n);
    cam = player->acamera;
    if (cam == NULL)
        return 0;
    thing = thing_get(player->influenced_thing_idx);
    if (thing_is_invalid(thing) || (thing->class_id == TCls_DeadCreature) || creature_is_dying(thing))
    {
        set_camera_zoom(cam, player->dungeon_camera_zoom);
        if (is_my_player(player))
            PaletteSetPlayerPalette(player, engine_palette);
        player->influenced_thing_idx = 0;
        player->field_0 &= ~0x10;
        player->field_0 &= ~0x80;
        set_player_instance(player, PI_Unset, true);
        return 0;
    }
    if (player->view_mode != PVM_FrontView)
    {
        view_zoom_camera_in(cam, 30000, 6000);
        // Compute new camera angle
        mv_a = (thing->field_52 - cam->orient_a) & LbFPMath_AngleMask;
        if (mv_a > LbFPMath_PI)
          mv_a -= 2*LbFPMath_PI;
        if (mv_a < -170)
        {
            mv_a = -170;
        } else
        if (mv_a > 170)
        {
            mv_a = 170;
        }
        cam->orient_a += mv_a;
        cam->orient_a &= LbFPMath_AngleMask;
        thing = thing_get(player->influenced_thing_idx);
        crstat = creature_stats_get_from_thing(thing);
        // Now mv_a becomes a circle radius
        mv_a = crstat->eye_height + thing->mappos.z.val;
        mv_x = thing->mappos.x.val + distance_with_angle_to_coord_x(mv_a ,cam->orient_a) - (MapCoordDelta)cam->mappos.x.val;
        mv_y = thing->mappos.y.val + distance_with_angle_to_coord_y(mv_a ,cam->orient_a) - (MapCoordDelta)cam->mappos.y.val;
        if (mv_x < -128)
        {
            mv_x = -128;
        } else
        if (mv_x > 128)
        {
            mv_x = 128;
        }
        if (mv_y < -128)
        {
            mv_y = -128;
        } else
        if (mv_y > 128)
        {
            mv_y = 128;
        }
        cam->mappos.x.val += mv_x;
        cam->mappos.y.val += mv_y;
        if (cam->orient_a < 0)
        {
          cam->orient_a += 2*LbFPMath_PI;
        }
        if (cam->orient_a >= 2*LbFPMath_PI)
        {
          cam->orient_a -= 2*LbFPMath_PI;
        }
    }
    return 0;
}

long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n)
{
    //return _DK_pinstfe_direct_control_creature(player, n);
    struct Thing *thing;
    thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (!control_creature_as_controller(player, thing)) {
            thing = INVALID_THING;
        }
    }
    if (thing_is_invalid(thing))
    {
        set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
        if (is_my_player(player)) {
            PaletteSetPlayerPalette(player, engine_palette);
        }
        player->field_0 &= ~0x10;
        player->field_0 &= ~0x80;
        return 0;
    }
    set_player_instance(player, PI_CrCtrlFade, false);
    if (thing->class_id == TCls_Creature)
    {
        load_swipe_graphic_for_creature(thing);
        if (is_my_player(player))
        {
            if (creature_affected_by_spell(thing, SplK_Freeze)) {
                PaletteSetPlayerPalette(player, blue_palette);
            }
        }
        creature_choose_first_available_instance(thing);
    }
    if (is_my_player(player))
      turn_on_menu(GMnu_CREATURE_QUERY1);
    return 0;
}

long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n)
{
//  return _DK_pinstfe_passenger_control_creature(player, n);
    struct Thing *thing;
    thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_invalid(thing))
      control_creature_as_passenger(player, thing);
    set_player_instance(player, PI_CrCtrlFade, false);
    return 0;
}

long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_direct_leave_creature(player, n);
  if (player->influenced_thing_idx == 0)
  {
    set_player_instance(player, PI_Unset, true);
    return 0;
  }
  player->field_0 |= 0x80;
  thing = thing_get(player->influenced_thing_idx);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
      PaletteSetPlayerPalette(player, engine_palette);
      player->field_4C5 = 11;
      turn_off_all_window_menus();
      turn_off_menu(GMnu_CREATURE_QUERY1);
      turn_off_menu(GMnu_CREATURE_QUERY2);
      turn_off_menu(GMnu_CREATURE_QUERY3);
      turn_on_main_panel_menu();
      set_flag_byte(&game.numfield_C, 0x40, (game.numfield_C & 0x20) != 0);
  }
  thing = thing_get(player->influenced_thing_idx);
  leave_creature_as_controller(player, thing);
  player->field_0 |= 0x10;
  player->influenced_thing_idx = 0;
  light_turn_light_on(player->field_460);
  play_non_3d_sample(177);
  return 0;
}

long pinstfm_leave_creature(struct PlayerInfo *player, long *n)
{
    //return _DK_pinstfm_leave_creature(player, n);
    if (player->view_mode != PVM_FrontView)
    {
        view_zoom_camera_out(player->acamera, 30000, 6000);
        if (get_camera_zoom(player->acamera) < player->dungeon_camera_zoom) {
            set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
        }
    }
    return 0;
}

long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_passenger_leave_creature(player, n);
  if (player->influenced_thing_idx == 0)
  {
      set_player_instance(player, PI_Unset, true);
      return 0;
  }
  player->field_0 |= 0x80;
  thing = thing_get(player->influenced_thing_idx);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
    PaletteSetPlayerPalette(player, engine_palette);
    player->field_4C5 = 11;
    turn_off_all_window_menus();
    turn_off_menu(GMnu_CREATURE_QUERY1);
    turn_off_menu(GMnu_CREATURE_QUERY2);
    turn_off_menu(GMnu_CREATURE_QUERY3);
    turn_on_main_panel_menu();
    set_flag_byte(&game.numfield_C, 0x40, (game.numfield_C & 0x20) != 0);
  }
  leave_creature_as_passenger(player, thing);
  player->field_0 |= 0x10;
  player->influenced_thing_idx = 0;
  light_turn_light_on(player->field_460);
  play_non_3d_sample(177);
  return 0;
}

long pinstfe_leave_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_leave_creature(player, n);
  set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
  if (is_my_player(player))
    PaletteSetPlayerPalette(player, engine_palette);
  player->field_0 &= ~0x10;
  player->field_0 &= ~0x80;
  return 0;
}

long pinstfs_query_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_query_creature(player, n);
  thing = thing_get(player->influenced_thing_idx);
  player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
  set_selected_creature(player, thing);
  set_player_state(player, PSt_Unknown15, 0);
  return 0;
}

long pinstfs_unquery_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfs_unquery_creature(player, n);
  set_player_state(player, PSt_CtrlDungeon, 0);
  player->field_31 = 0;
  clear_selected_creature(player);
  return 0;
}

long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    struct Coord3d mappos;
    ThingModel spectator_breed;
    SYNCDBG(6,"Starting for player %d",(int)player->id_number);
    //return _DK_pinstfs_zoom_to_heart(player, n);
    LbPaletteDataFillWhite(zoom_to_heart_palette);
    light_turn_light_off(player->field_460);
    thing = get_player_soul_container(player->id_number);
    spectator_breed = get_players_spectator_breed(player->id_number);
    mappos.x.val = thing->mappos.x.val;
    mappos.y.val = thing->mappos.y.val + subtile_coord(7,0);
    mappos.z.val = thing->mappos.z.val + subtile_coord(1,0);
    thing = create_and_control_creature_as_controller(player, spectator_breed, &mappos);
    if (!thing_is_invalid(thing))
    {
        cctrl = creature_control_get_from_thing(thing);
        cctrl->flgfield_1 |= CCFlg_NoCompControl;
        player->field_0 |= 0x10;
        player->field_0 |= 0x80;
        game.numfield_D |= 0x08;
    }
    return 0;
}

long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_zoom_to_heart(player, n);
  struct Thing *thing;
  struct Coord3d pos;
  thing = thing_get(player->controlled_thing_idx);
  if (!thing_is_invalid(thing))
  {
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val - subtile_coord(7,0)/16;
    pos.z.val = thing->mappos.z.val;
    move_thing_in_map(thing, &pos);
  }
  if (player->field_4B1 <= 8)
    LbPaletteFade(zoom_to_heart_palette, 8, Lb_PALETTE_FADE_OPEN);
  return 0;
}


long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_zoom_to_heart(player, n);
  set_player_instance(player, PI_HeartZoomOut, false);
  LbPaletteStopOpenFade();
  return 0;
}

long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  struct Camera *cam;
  //return _DK_pinstfs_zoom_out_of_heart(player, n);
  thing = thing_get(player->controlled_thing_idx);
  if (!thing_is_invalid(thing))
    leave_creature_as_controller(player, thing);
  set_player_mode(player, PVT_DungeonTop);
  cam = player->acamera;
  if (cam == NULL) return 0;
  thing = get_player_soul_container(player->id_number);
  if (thing_is_invalid(thing))
  {
      cam->mappos.x.val = (map_subtiles_x << 8)/2;
      cam->mappos.y.val = (map_subtiles_y << 8)/2;
      cam->zoom = 24000;
      cam->orient_a = 0;
      return 0;
  }
  cam->mappos.x.val = thing->mappos.x.val;
  if (player->view_mode == PVM_FrontView)
  {
    cam->mappos.y.val = thing->mappos.y.val;
    cam->zoom = 65536;
  } else
  {
    cam->mappos.y.val = thing->mappos.y.val - (thing->field_58 >> 1) -  thing->mappos.z.val;
    cam->zoom = 24000;
  }
  cam->orient_a = 0;
  return 0;
}

long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
    struct Thing *thing;
    struct Camera *dstcam;
    struct Camera *cam;
    long deltax,deltay;
    unsigned long addval;
    //return _DK_pinstfm_zoom_out_of_heart(player, n);
    if (player->view_mode != PVM_FrontView)
    {
        cam = player->acamera;
        thing = get_player_soul_container(player->id_number);
        if (cam != NULL)
        {
          cam->zoom -= 988;
          cam->orient_a += 16;
          addval = (thing->field_58 >> 1);
          deltax = distance_with_angle_to_coord_x((long)thing->mappos.z.val+addval, cam->orient_a);
          deltay = distance_with_angle_to_coord_y((long)thing->mappos.z.val+addval, cam->orient_a);
        } else
        {
          addval = (thing->field_58 >> 1);
          deltax = addval;
          deltay = -addval;
        }
        dstcam = &player->cameras[0];
        dstcam->mappos.x.val = thing->mappos.x.val + deltax;
        dstcam->mappos.y.val = thing->mappos.y.val + deltay;
        dstcam = &player->cameras[3];
        dstcam->mappos.x.val = thing->mappos.x.val + deltax;
        dstcam->mappos.y.val = thing->mappos.y.val + deltay;
    }
    if (player->field_4B1 >= 8)
      LbPaletteFade(engine_palette, 8, Lb_PALETTE_FADE_OPEN);
    return 0;
}

long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfe_zoom_out_of_heart(player, n);
  LbPaletteStopOpenFade();
  cam = player->acamera;
  if ((player->view_mode != PVM_FrontView) && (cam != NULL))
  {
    cam->zoom = 8192;
    cam->orient_a = 256;
  }
  light_turn_light_on(player->field_460);
  player->field_0 &= ~0x10;
  player->field_0 &= ~0x80;
  game.numfield_D &= ~0x08;
  if (is_my_player(player))
    PaletteSetPlayerPalette(player, engine_palette);
  return 0;
}

long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_control_creature_fade(player, n);
  player->field_0 |= 0x80;
  return 0;
}

long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_control_creature_fade(player, n);
  if (is_my_player(player))
  {
    if ((player->field_3 & 0x04) == 0)
      PaletteSetPlayerPalette(player, _DK_blue_palette);
    else
      PaletteSetPlayerPalette(player, engine_palette);
  }
  player->field_0 &= ~0x10;
  light_turn_light_off(player->field_460);
  player->field_0 &= ~0x80;
  return 0;
}

long pinstfs_fade_to_map(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfs_fade_to_map(player, n);
  cam = player->acamera;
  player->field_4BD = 0;
  player->field_0 |= 0x80;
  player->field_4B5 = cam->field_6;
  if (is_my_player(player))
  {
    set_flag_byte(&player->field_1, 0x02, settings.tooltips_on);
    settings.tooltips_on = 0;
    set_flag_byte(&player->field_1, 0x01, toggle_status_menu(0));
  }
  set_engine_view(player, 6);
  return 0;

}

long pinstfm_fade_to_map(struct PlayerInfo *player, long *n)
{
  return 0;
}

long pinstfe_fade_to_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_fade_to_map(player, n);
  set_player_mode(player, PVT_MapScreen);
  if (is_my_player(player))
    settings.tooltips_on = ((player->field_1 & 0x02) != 0);
  player->field_0 &= ~0x80;
  return 0;
}

long pinstfs_fade_from_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfs_fade_from_map(player, n);
  player->field_0 |= 0x80;
  if (is_my_player(player))
  {
    set_flag_byte(&player->field_1, 0x02, settings.tooltips_on);
    settings.tooltips_on = 0;
    game.numfield_C &= ~0x0040;
  }
  player->field_4BD = 32;
  set_player_mode(player, PVT_DungeonTop);
  set_engine_view(player, 7);
  return 0;
}

long pinstfm_fade_from_map(struct PlayerInfo *player, long *n)
{
  return 0;
}

long pinstfe_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_fade_from_map(player, n);
}

long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_zoom_to_position(player, n);
}

long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  long x,y;
  //return _DK_pinstfm_zoom_to_position(player, n);
  cam = player->acamera;
  if (abs(cam->mappos.x.val - player->zoom_to_pos_x) >= abs(player->field_4DB))
    x = player->field_4DB + cam->mappos.x.val;
  else
    x = player->zoom_to_pos_x;
  if (abs(cam->mappos.y.val - player->zoom_to_pos_y) >= abs(player->field_4DF))
    y = player->field_4DF + cam->mappos.y.val;
  else
    y = player->zoom_to_pos_y;
  if ((player->zoom_to_pos_x == x) && (player->zoom_to_pos_y == y))
      player->field_4B1 = 0;
  cam->mappos.x.val = x;
  cam->mappos.y.val = y;
  return 0;
}

long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n)
{
    //return _DK_pinstfe_zoom_to_position(player, n);
    player->field_0 &= ~0x80;
    player->field_0 &= ~0x10;
    return 0;
}

void set_player_instance(struct PlayerInfo *player, long ninum, TbBool force)
{
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
  long inum;
  inum = player->instance_num;
  if (inum >= PLAYER_INSTANCES_COUNT)
    inum = 0;
  if ((inum == 0) || (player_instance_info[inum].field_4 != 1) || (force))
  {
    player->instance_num = ninum%PLAYER_INSTANCES_COUNT;
    inst_info = &player_instance_info[player->instance_num];
    player->field_4B1 = inst_info->field_0;
    callback = inst_info->start_cb;
    if (callback != NULL)
      callback(player, &inst_info->field_14[0]);
  }
}

void process_player_instance(struct PlayerInfo *player)
{
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
  SYNCDBG(16,"Starting for instance %d",player->instance_num);
  //_DK_process_player_instance(player); return;
  if (player->instance_num > 0)
  {
    if (player->field_4B1 > 0)
    {
      player->field_4B1--;
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      callback = inst_info->maintain_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
    if (player->field_4B1 == 0)
    {
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      player->instance_num = 0;
      callback = inst_info->end_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
  }
}

void process_player_instances(void)
{
    //_DK_process_player_instances();return;
    int i;
    struct PlayerInfo *player;
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player))
          process_player_instance(player);
    }
    SYNCDBG(9,"Finished");
}

void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    long i,k;
    SYNCDBG(7,"Starting");
    //_DK_leave_creature_as_controller(player, thing);
    if ((thing->owner != player->id_number) || (thing->index != player->controlled_thing_idx))
    {
        set_player_instance(player, PI_Unset, 1);
        clear_selected_creature(player);
        player->field_31 = 0;
        set_player_mode(player, PVT_DungeonTop);
        player->field_0 &= ~0x08;
        set_engine_view(player, player->field_4B5);
        player->cameras[0].mappos.x.val = 0;
        player->cameras[0].mappos.y.val = 0;
        player->cameras[3].mappos.x.val = 0;
        player->cameras[3].mappos.y.val = 0;
        return;
    }
    clear_selected_creature(player);
    player->field_31 = 0;
    set_player_mode(player, PVT_DungeonTop);
    thing->alloc_flags &= ~TAlF_IsControlled;
    thing->field_4F &= ~0x01;
    player->field_0 &= ~0x08;
    set_engine_view(player, player->field_4B5);
    i = player->acamera->orient_a;
    crstat = creature_stats_get_from_thing(thing);
    k = thing->mappos.z.val + crstat->eye_height;
    player->cameras[0].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
    player->cameras[0].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
    player->cameras[3].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
    player->cameras[3].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
    if (thing->class_id == TCls_Creature)
    {
        set_start_state(thing);
        cctrl = creature_control_get_from_thing(thing);
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        if ((cctrl->flgfield_2 & 0x02) != 0) {
          delete_thing_structure(thing, 0);
        } else {
          disband_creatures_group(thing);
        }
    }
    if (thing->light_id != 0) {
        light_delete_light(thing->light_id);
        thing->light_id = 0;
    }
}

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing)
{
  struct CreatureStats *crstat;
  long i,k;
  SYNCDBG(7,"Starting");
  if ((thing->owner != player->id_number) || (thing->index != player->controlled_thing_idx))
  {
    set_player_instance(player, PI_Unset, 1);
    clear_selected_creature(player);
    player->field_31 = 0;
    set_player_mode(player, PVT_DungeonTop);
    player->field_0 &= ~0x08;
    set_engine_view(player, player->field_4B5);
    player->cameras[0].mappos.x.val = 0;
    player->cameras[0].mappos.y.val = 0;
    player->cameras[3].mappos.x.val = 0;
    player->cameras[3].mappos.y.val = 0;
    return;
  }
  set_player_mode(player, PVT_DungeonTop);
  thing->field_4F &= ~0x01;
  player->field_0 &= ~0x08;
  set_engine_view(player, player->field_4B5);
  i = player->acamera->orient_a;
  crstat = creature_stats_get_from_thing(thing);
  k = thing->mappos.z.val + crstat->eye_height;
  player->cameras[0].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
  player->cameras[0].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
  player->cameras[3].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
  player->cameras[3].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
  clear_selected_creature(player);
  player->field_31 = 0;
}

TbBool is_thing_passenger_controlled(const struct Thing *thing)
{
    struct PlayerInfo *player;
    //return _DK_is_thing_passenger_controlled(thing);
    if (is_neutral_thing(thing))
        return false;
    player = get_player(thing->owner);
    if (player->work_state != PSt_CtrlDirect)
        return false;
    switch (player->instance_num)
    {
    case PI_DirctCtrl:
        return (thing->index == player->influenced_thing_idx);
    case PI_CrCtrlFade:
        return (thing->index == player->controlled_thing_idx);
    case PI_PsngrCtLeave:
        return (thing->index == player->influenced_thing_idx);
    case PI_Unset:
        return (thing->index == player->controlled_thing_idx);
    default:
        break;
    }
    return false;
}

TbBool set_selected_creature(struct PlayerInfo *player, struct Thing *thing)
{
  if (thing->class_id == TCls_Creature)
  {
    player->controlled_thing_idx = thing->index;
    return true;
  }
  ERRORLOG("Cannot select thing for information");
  return false;
}

TbBool clear_selected_creature(struct PlayerInfo *player)
{
  player->controlled_thing_idx = 0;
  return true;
}

/** Builds room for the given player at given coords.
 * Takes money from dungeon, builds the room and updates stats.
 * Makes proper sound messages on failure.
 *
 * @param stl_x Target X coord to build on.
 * @param stl_y Target Y coord to build on.
 * @param plyr_idx Player owning the room.
 * @param rkind Kind of the room.
 * @return Returns room struct, or invalid room on error.
 */
struct Room *player_build_room_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, RoomKind rkind)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Room *room;
    struct RoomStats *rstat;
    player = get_player(plyr_idx);
    dungeon = get_players_dungeon(player);
    rstat = room_stats_get_for_kind(rkind);
    // Check if we are allowed to build the room
    if (!is_room_available(plyr_idx, rkind)) {
        // It shouldn't be possible to select unavailable room
        WARNLOG("Player %d tried to build %s which is unavailable at (%d,%d)",(int)plyr_idx,room_code_name(rkind),(int)stl_x,(int)stl_y);
        if (is_my_player(player))
            play_non_3d_sample(119);
        return INVALID_ROOM;
    }
    if (!can_build_room_at_slab(plyr_idx, rkind, subtile_slab(stl_x), subtile_slab(stl_y))) {
        // It shouldn't be possible to select unavailable room
        WARNLOG("Player %d tried to build %s on a forbidden subtile (%d,%d)",(int)plyr_idx,
            room_code_name(rkind),(int)stl_x,(int)stl_y);
        if (is_my_player(player))
            play_non_3d_sample(119);
        return INVALID_ROOM;
    }
    // Check if there's a place for new room
    if (!i_can_allocate_free_room_structure())
    {
      if (is_my_player(player))
        play_non_3d_sample(119);
      return INVALID_ROOM;
    }
    if (take_money_from_dungeon(plyr_idx, rstat->cost, 1) < 0)
    {
      if (is_my_player(player))
        output_message(SMsg_GoldNotEnough, 0, true);
      return INVALID_ROOM;
    }
    room = place_room(plyr_idx, rkind, stl_x, stl_y);
    if (!room_is_invalid(room))
    {
      if (rkind == RoK_BRIDGE)
        dungeon->lvstats.bridges_built++;
      if (is_my_player(player))
        play_non_3d_sample(77);
    }
    return room;
}

TbBool player_place_trap_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel)
{
    if (!is_trap_placeable(plyr_idx, tngmodel)) {
        WARNLOG("Player %d tried to build %s but has none to place",(int)plyr_idx,trap_code_name(tngmodel));
        return false;
    }
    struct Coord3d pos;
    set_coords_to_slab_center(&pos,subtile_slab_fast(stl_x),subtile_slab_fast(stl_y));
    delete_room_slabbed_objects(get_slab_number(subtile_slab_fast(stl_x),subtile_slab_fast(stl_y)));
    struct Thing *traptng;
    traptng = create_trap(&pos, tngmodel, plyr_idx);
    if (thing_is_invalid(traptng)) {
        return false;
    }
    traptng->mappos.z.val = get_thing_height_at(traptng, &traptng->mappos);
    traptng->trap.byte_18t = 0;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Trap, tngmodel);
    if (placing_offmap_workshop_item(plyr_idx, TCls_Trap, tngmodel)) {
        remove_workshop_item_from_amount_stored(plyr_idx, TCls_Trap, tngmodel, WrkCrtF_NoStored);
        rearm_trap(traptng);
        dungeon->lvstats.traps_armed++;
    }
    dungeon->camera_deviate_jump = 192;
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(117);
    return true;
}

TbBool player_place_door_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel)
{
    if (!is_door_placeable(plyr_idx, tngmodel)) {
        WARNLOG("Player %d tried to build %s but has none to place",(int)plyr_idx,door_code_name(tngmodel));
        return 0;
    }
    unsigned char orient;
    orient = find_door_angle(stl_x, stl_y, plyr_idx);
    struct Coord3d pos;
    set_coords_to_slab_center(&pos,subtile_slab(stl_x),subtile_slab(stl_y));
    create_door(&pos, tngmodel, orient, plyr_idx, 0);
    do_slab_efficiency_alteration(subtile_slab(stl_x), subtile_slab(stl_y));
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    int crate_source;
    crate_source = remove_workshop_item_from_amount_stored(plyr_idx, TCls_Door, tngmodel, WrkCrtF_Default);
    switch (crate_source)
    {
    case WrkCrtS_Offmap:
        remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Door, tngmodel);
        break;
    case WrkCrtS_Stored:
        remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Door, tngmodel);
        remove_workshop_object_from_player(plyr_idx, door_crate_object_model(tngmodel));
        break;
    default:
        WARNLOG("Placeable door %s amount for player %d was incorrect; fixed",door_code_name(tngmodel),(int)dungeon->owner);
        dungeon->door_amount_placeable[tngmodel] = 0;
        break;
    }
    dungeon->camera_deviate_jump = 192;
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(117);
    return 1;
}
/******************************************************************************/
