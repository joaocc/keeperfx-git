/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_data.c
 *     Rooms support functions.
 * @par Purpose:
 *     Functions to create and maintain the game rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Apr 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_data.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "config_creature.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void count_slabs(struct Room *room);
void count_gold_slabs_with_efficiency(struct Room *room);
void count_gold_hoardes_in_room(struct Room *room);
void count_slabs_div2(struct Room *room);
void count_books_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_slabs_with_efficiency(struct Room *room);
void count_crates_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_bodies_in_room(struct Room *room);
void count_capacity_in_garden(struct Room *room);
void count_food_in_room(struct Room *room);
void count_lair_occupants(struct Room *room);
/******************************************************************************/

RoomKind look_through_rooms[] = {
    RoK_DUNGHEART, RoK_TREASURE, RoK_LAIR,      RoK_GARDEN,
    RoK_LIBRARY,   RoK_TRAINING, RoK_WORKSHOP,  RoK_SCAVENGER,
    RoK_PRISON,    RoK_TEMPLE,   RoK_TORTURE,   RoK_GRAVEYARD,
    RoK_BARRACKS,  RoK_BRIDGE,   RoK_GUARDPOST, RoK_ENTRANCE,
    RoK_DUNGHEART, RoK_UNKN17,};

struct RoomData room_data[] = {
  { 0,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 201, 201},
  {14,  0, count_slabs,             NULL,                   NULL,                  0, 0, 0, 598, 614},
  {16, 57, count_gold_slabs_with_efficiency, count_gold_hoardes_in_room, NULL,     1, 0, 0, 599, 615},
  {18, 61, count_slabs_div2,        count_books_in_room,    count_workers_in_room, 0, 0, 0, 600, 616},
  {20, 65, count_slabs_with_efficiency, NULL,               NULL,                  1, 0, 0, 601, 617},
  {22, 63, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 602, 619},
  {24, 67, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 603, 618},
  {26,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 604, 620},
  {28, 75, count_slabs_div2,        count_crates_in_room,   count_workers_in_room, 0, 0, 0, 605, 621},
  {30, 77, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 613, 629},
  {32, 73, count_slabs_div2,        NULL,                   NULL,                  1, 0, 0, 612, 628},
  {34, 71, count_slabs_div2,        count_bodies_in_room,   NULL,                  0, 0, 0, 606, 622},
  {40, 69, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 607, 623},
  {36, 59, count_capacity_in_garden, count_food_in_room,    NULL,                  1, 0, 0, 608, 624},
  {38, 79, count_slabs_with_efficiency, count_lair_occupants, NULL,                1, 0, 0, 609, 625},
  {51, 81, NULL,                    NULL,                   NULL,                  0, 0, 0, 610, 626},
  {53, 83, count_slabs,             NULL,                   NULL,                  0, 0, 0, 611, 627},
};

struct RoomInfo room_info[] = {
  { 0,  0,  0},
  { 0,  0,  0},
  {29, 57,  0},
  {33, 61,  0},
  {37, 65,  0},
  {35, 63,  0},
  {39, 67, 85},
  { 0,  0,  0},
  {47, 75, 86},
  {49, 77,156},
  {45, 73,155},
  {43, 71, 45},
  {41, 69,  0},
  {31, 59,  0},
  {51, 79,  0},
  {53, 81,  0},
  {55, 83,  0},
};

struct AroundLByte const room_spark_offset[] = {
  {-256,  256},
  {-256,    0},
  {-256, -256},
  {-256, -256},
  {   0, -256},
  { 256, -256},
  { 256, -256},
  { 256,    0},
  { 256,  256},
  { 256,  256},
  {   0,  256},
  {-256,  256},
};

struct Around const small_around[] = {
  { 0,-1},
  { 1, 0},
  { 0, 1},
  {-1, 0},
};

struct Around const my_around_eight[] = {
  { 0,-1},
  { 1,-1},
  { 1, 0},
  { 1, 1},
  { 0, 1},
  {-1, 1},
  {-1, 0},
  {-1,-1},
};

short const around_map[] = {-257, -256, -255, -1, 0, 1, 255, 256, 257};

unsigned char const slabs_to_centre_peices[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
};

/**
 * Should contain values encoded with get_subtile_number(). */
const unsigned short small_around_pos[] = {
  0xFF00, 0x0001, 0x0100, 0xFFFF, 0x0000,
  0xFF00, 0x0001, 0x0100, 0x00FF, 0xFFFF,
  0x0FF01, 0x01FF, 0x0101
};

unsigned short const room_effect_elements[] = { 55, 56, 57, 58, 0, 0 };
const short slab_around[] = { -85, 1, 85, -1 };
/******************************************************************************/
DLLIMPORT unsigned char _DK_find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT void _DK_count_gold_slabs_with_efficiency(struct Room *room);
DLLIMPORT void _DK_count_gold_hoardes_in_room(struct Room *room);
DLLIMPORT void _DK_count_books_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_crates_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_bodies_in_room(struct Room *room);
DLLIMPORT void _DK_count_food_in_room(struct Room *room);
DLLIMPORT void _DK_count_lair_occupants(struct Room *room);
DLLIMPORT short _DK_room_grow_food(struct Room *room);
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long capac);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT struct Room *_DK_link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_create_room_flag(struct Room *room);
DLLIMPORT struct Room *_DK_allocate_free_room_structure(void);
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);
DLLIMPORT struct Room *_DK_find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2);
DLLIMPORT long _DK_create_workshop_object_in_workshop_room(long a1, long a2, long a3);
DLLIMPORT unsigned char _DK_find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT struct Room* _DK_find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing,
    signed char a2, signed char a3, unsigned char a4, long a5);
DLLIMPORT struct Room* _DK_find_room_with_spare_capacity(unsigned char a1, signed char a2, long a3);
DLLIMPORT short _DK_delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3);
DLLIMPORT long _DK_calculate_room_efficiency(struct Room *room);
DLLIMPORT void _DK_kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3);
DLLIMPORT void _DK_free_room_structure(struct Room *room);
DLLIMPORT void _DK_reset_creatures_rooms(struct Room *room);
DLLIMPORT void _DK_replace_room_slab(struct Room *room, long a2, long a3, unsigned char a4);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4);
/******************************************************************************/
struct Room *room_get(long room_idx)
{
  if ((room_idx < 1) || (room_idx > ROOMS_COUNT))
    return &game.rooms[0];
  return &game.rooms[room_idx];
}

struct Room *subtile_room_get(long stl_x, long stl_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_for_subtile(stl_x,stl_y);
  if (slabmap_block_invalid(slb))
    return INVALID_ROOM;
  return room_get(slb->room_index);
}

struct Room *slab_room_get(long slb_x, long slb_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_block(slb_x,slb_y);
  if (slabmap_block_invalid(slb))
    return INVALID_ROOM;
  return room_get(slb->room_index);
}

TbBool room_is_invalid(const struct Room *room)
{
  if (room == NULL)
    return true;
  if (room == INVALID_ROOM)
    return true;
  return (room <= &game.rooms[0]);
}

struct RoomData *room_data_get_for_kind(long room_kind)
{
  if ((room_kind < 1) || (room_kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room_kind];
}

struct RoomData *room_data_get_for_room(const struct Room *room)
{
  if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room->kind];
}

long get_room_look_through(RoomKind rkind)
{
  const long arr_length = sizeof(look_through_rooms)/sizeof(look_through_rooms[0]);
  long i;
  for (i=0; i < arr_length; i++)
  {
    if (look_through_rooms[i] == rkind)
      return i;
  }
  return -1;
}

long get_room_slabs_count(long plyr_idx, unsigned short rkind)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned long k;
  long i;
  long count;
  dungeon = get_players_num_dungeon(plyr_idx);
  count = 0;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    // Per-room code
    count += room->slabs_count;
    // Per-room code ends
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return count;
}

long get_player_rooms_count(long plyr_idx, unsigned short rkind)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned long k;
  long i;
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return 0;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return k;
}

void set_room_capacity(struct Room *room, long capac)
{
    SYNCDBG(7,"Starting");
    _DK_set_room_capacity(room, capac);
}

void set_room_efficiency(struct Room *room)
{
  _DK_set_room_efficiency(room);
}

void count_slabs(struct Room *room)
{
  room->total_capacity = room->slabs_count;
}

void count_gold_slabs_with_efficiency(struct Room *room)
{
  _DK_count_gold_slabs_with_efficiency(room);
}

void count_gold_hoardes_in_room(struct Room *room)
{
  _DK_count_gold_hoardes_in_room(room);
}

void count_slabs_div2(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = ((count/256) >> 1);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;

}

void count_books_in_room(struct Room *room)
{
  _DK_count_books_in_room(room);
}

void count_workers_in_room(struct Room *room)
{
  _DK_count_workers_in_room(room);
}

void count_slabs_with_efficiency(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_crates_in_room(struct Room *room)
{
  _DK_count_crates_in_room(room);
}

void count_bodies_in_room(struct Room *room)
{
  _DK_count_bodies_in_room(room);
}

void count_capacity_in_garden(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_food_in_room(struct Room *room)
{
  _DK_count_food_in_room(room);
}

void count_lair_occupants(struct Room *room)
{
  _DK_count_lair_occupants(room);
}


void delete_room_structure(struct Room *room)
{
  //_DK_delete_room_structure(room); return;
  struct Dungeon *dungeon;
  unsigned short *wptr;
  if (room == NULL)
    return;
  if (room->field_0 & 0x01)
  {
    if (game.neutral_player_num != room->owner)
    {
        dungeon = get_players_num_dungeon(room->owner);
        wptr = &dungeon->room_kind[room->kind];
        if (room->index == *wptr)
        {
          *wptr = room->next_of_owner;
          game.rooms[room->next_of_owner].prev_of_owner = 0;
        }
        else
        {
          game.rooms[room->next_of_owner].prev_of_owner = room->prev_of_owner;
          game.rooms[room->prev_of_owner].next_of_owner = room->next_of_owner;
        }
    }
    memset(room, 0, sizeof(struct Room));
  }
}

void delete_all_room_structures(void)
{
  struct Room *room;
  long i;
  for (i=1; i < ROOMS_COUNT; i++)
  {
    room = &game.rooms[i];
    delete_room_structure(room);
  }
}

struct Room *link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind)
{
  // TODO: rework! may lead to hang on map borders
  return _DK_link_adjacent_rooms_of_type(owner, x, y, rkind);
}

void count_room_slabs(struct Room *room)
{
    struct SlabMap *slb;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    room->slabs_count = 0;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        slb = get_slabmap_block(slb_x,slb_y);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        room->slabs_count++;
        slb->room_index = room->index;
        // Per room tile code ends
        k++;
        if (k >= map_tiles_x*map_tiles_y)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

void update_room_ensign_position(struct Room *room)
{
    struct MapOffset *sstep;
    struct SlabMap *slb;
    unsigned long tot_x,tot_y;
    long cx,cy;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    tot_x = 0;
    tot_y = 0;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        slb = get_slabmap_block(slb_x,slb_y);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        // Per room tile code
        tot_x += slb_x;
        tot_y += slb_y;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    if (room->slabs_count > 1)
    {
      tot_x /= room->slabs_count;
      tot_y /= room->slabs_count;
    }
    for (i=0; i < 256; i++)
    {
      sstep = &spiral_step[i];
      cx = 3 * (tot_x + sstep->h) + 1;
      cy = 3 * (tot_y + sstep->v) + 1;
      slb_x = map_to_slab[cx%(map_subtiles_x+1)];
      slb_y = map_to_slab[cy%(map_subtiles_y+1)];
      slb = get_slabmap_block(slb_x,slb_y);
      if (&game.rooms[slb->room_index] == room)
      {
        room->stl_x = cx;
        room->stl_y = cy;
        return;
      }
    }
    ERRORLOG("Cannot find position to place an ensign.");
}

struct Room *create_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y)
{
  struct Dungeon *dungeon;
  struct SlabMap *slb;
  struct Room *room,*nxroom;
  long slb_x,slb_y;
  long i;
  SYNCDBG(7,"Starting");
  // room = _DK_create_room(owner, rkind, x, y); return room;
  room = link_adjacent_rooms_of_type(owner, x, y, rkind);
  if (room != NULL)
  {
      count_room_slabs(room);
  } else
  {
    if ( !i_can_allocate_free_room_structure() )
    {
        ERRORLOG("Cannot allocate any more rooms.");
        return NULL;
    }
    room = allocate_free_room_structure();
    room->owner = owner;
    room->kind = rkind;
    if (rkind == 1)
    {
      if ((game.field_14E938 > 0) && (game.field_14E938 < ROOMS_COUNT))
      {
        room->word_19 = game.field_14E938;
        nxroom = room_get(game.field_14E938);
        nxroom->word_17 = room->index;
      }
      game.field_14E938 = room->index;
      game.field_14E93A++;
    }
    if (owner != game.neutral_player_num)
    {
        // note that we can't get_players_num_dungeon() because players
        // may be uninitialized yet when this is called.
        dungeon = get_dungeon(owner);
        i = dungeon->room_kind[room->kind%ROOM_TYPES_COUNT];
        room->next_of_owner = i;
        game.rooms[i].prev_of_owner = room->index;
        dungeon->room_kind[room->kind%ROOM_TYPES_COUNT] = room->index;
        dungeon->room_slabs_count[room->kind%ROOM_TYPES_COUNT]++;
    }
    slb_x = map_to_slab[x%(map_subtiles_x+1)];
    slb_y = map_to_slab[y%(map_subtiles_y+1)];
    i = get_slab_number(slb_x, slb_y);
    room->slabs_list = i;
    room->field_39 = i;
    slb = get_slabmap_direct(i);
    slb->next_in_room = 0;
    count_room_slabs(room);
    create_room_flag(room);
  }
  update_room_ensign_position(room);
  SYNCDBG(7,"Done");
  return room;
}

void create_room_flag(struct Room *room)
{
  struct Thing *thing;
  struct Coord3d pos;
  long x,y;
  //_DK_create_room_flag(room);
  x = 3 * slb_num_decode_x(room->slabs_list) + 1;
  y = 3 * slb_num_decode_y(room->slabs_list) + 1;
  if ((room->kind != RoK_DUNGHEART) && (room->kind != RoK_ENTRANCE)
     && (room->kind != RoK_GUARDPOST) && (room->kind != RoK_BRIDGE))
  {
    pos.z.val = 2 << 8;
    pos.x.val = x << 8;
    pos.y.val = y << 8;
    thing = find_base_thing_on_mapwho(1, 25, x, y);
    if (thing == NULL)
    {
      thing = create_object(&pos, 25, room->owner, -1);
    }
    if (thing == NULL)
    {
      ERRORLOG("Cannot create room_flag");
      return;
    }
    thing->word_13 = room->index;
  }
}

void delete_room_flag(struct Room *room)
{
    struct Thing *thing;
    long stl_x,stl_y;
    stl_x = 3 * slb_num_decode_x(room->slabs_list) + 1;
    stl_y = 3 * slb_num_decode_y(room->slabs_list) + 1;
    if ((room->kind != RoK_DUNGHEART) && (room->kind != RoK_ENTRANCE))
    {
        thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
        if (!thing_is_invalid(thing))
          delete_thing_structure(thing, 0);
    }
}

struct Room *allocate_free_room_structure(void)
{
  return _DK_allocate_free_room_structure();
}

unsigned short i_can_allocate_free_room_structure(void)
{
  unsigned short ret = _DK_i_can_allocate_free_room_structure();
  if (ret == 0)
      SYNCDBG(3,"No slot for next room");
  return ret;
}

void reinitialise_treaure_rooms(void)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned int i,k,n;
  for (n=0; n < DUNGEONS_COUNT; n++)
  {
    dungeon = get_dungeon(n);
    i = dungeon->room_kind[RoK_TREASURE];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_owner;
      set_room_capacity(room, 1);
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
  }
}

short room_grow_food(struct Room *room)
{
  return _DK_room_grow_food(room);
}

void update_room_efficiency(struct Room *room)
{
    struct SlabMap *slb;
    long score,peices,effic;
    long i,k,n;
    if (room->slabs_count <= 0)
    {
      ERRORLOG("How can a room exist with no slabs?");
      room->efficiency = 0;
      return;
    }
    if (room->slabs_count == 1)
        n = 0;
    else
    if (room->slabs_count == 2)
        n = 4;
    else
        n = 4 * ((long)room->slabs_count) - 4;

    peices = room->slabs_count;
    if (peices > 49)
        peices = 49;
    peices = 2 * (((long)slabs_to_centre_peices[peices]) + 4 * ((long)room->slabs_count));
    // Calculate efficiency score from slabs
    score = 0;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        // Per room tile code
        slb = get_slabmap_direct(i);
        score += calculate_effeciency_score_for_room_slab(i, room->owner);
        // Per room tile code ends
        i = get_next_slab_number_in_room(i); // It would be better to have this before per-tile block, but we need old value
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }

    if (peices > n)
        effic = ((score - n) << 8) / (peices - n);
    else
        effic = ROOM_EFFICIENCY_MAX;
    if (effic > ROOM_EFFICIENCY_MAX)
        effic = ROOM_EFFICIENCY_MAX;
    room->efficiency = effic;
}

TbBool update_room_contents(struct Room *room)
{
  struct RoomData *rdata;
  rdata = room_data_get_for_room(room);
  if (rdata->ofsfield_7 != NULL)
  {
    rdata->ofsfield_7(room);
  }
  if (rdata->offfield_B != NULL)
  {
    rdata->offfield_B(room);
  }
  return true;
}

void init_room_sparks(struct Room *room)
{
    struct SlabMap *slb;
    struct SlabMap *sibslb;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    if (room->kind != RoK_DUNGHEART)
    {
        k = 0;
        i = room->slabs_list;
        while (i != 0)
        {
            slb_x = slb_num_decode_x(i);
            slb_y = slb_num_decode_y(i);
            i = get_next_slab_number_in_room(i);
            // Per room tile code
            slb = get_slabmap_block(slb_x, slb_y);
            sibslb = get_slabmap_block(slb_x, slb_y-1);
            if (sibslb->room_index != slb->room_index)
            {
                room->field_43 = 1;
                room->field_44 = 0;
                room->field_41 = i;
            }
            // Per room tile code ends
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Room slabs list length exceeded when sweeping");
                break;
            }
        }
    }
}

TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_random_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2)
{
  return _DK_find_room_with_spare_room_item_capacity(a1, a2);
}

struct Room *find_room_with_spare_capacity(unsigned char owner, signed char rkind, long spare)
{
    struct Dungeon *dungeon;
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return NULL;
    dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return NULL;
    return find_room_with_spare_capacity_starting_with(dungeon->room_kind[rkind], spare);
}

struct Room *find_room_with_spare_capacity_starting_with(long room_idx, long spare)
{
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->field_10 + spare <= room->total_capacity)
        {
            return room;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return NULL;
}

struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx,long *total_spare_cap)
{
    long max_spare_cap,loc_total_spare_cap,delta;
    struct Room *max_spare_room;
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    loc_total_spare_cap = 0;
    max_spare_room = NULL;
    max_spare_cap = 0;
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->total_capacity > room->field_10)
        {
          delta = room->total_capacity - room->field_10;
          loc_total_spare_cap += delta;
          if (max_spare_cap < delta)
          {
            max_spare_cap = delta;
            max_spare_room = room;
          }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    if (total_spare_cap != NULL)
       (*total_spare_cap) = loc_total_spare_cap;
    return max_spare_room;
}

unsigned char find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_first_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, signed char rkind, unsigned char nav_no_owner, long spare)
{
    struct Dungeon *dungeon;
    struct Room *nearoom;
    long neardistance,distance;
    struct Coord3d pos;

    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    nearoom = NULL;
    neardistance = LONG_MAX;
    k = 0;
    i = dungeon->room_kind[rkind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        // Compute simplified distance - without use of mul or div
        distance = abs(thing->mappos.x.stl.num - room->stl_x)
                 + abs(thing->mappos.y.stl.num - room->stl_y);
        if ((neardistance > distance) && (room->field_10 + spare <= room->total_capacity))
        {
            if (find_first_valid_position_for_thing_in_room(thing, room, &pos))
            {
                if ((thing->class_id != TCls_Creature)
                  || creature_can_navigate_to(thing, &pos, nav_no_owner))
                {
                  neardistance = distance;
                  nearoom = room;
                }
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return nearoom;
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
long count_rooms_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    count = 0;
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->stl_x);
        pos.y.val = get_subtile_center_pos(room->stl_y);
        pos.z.val = 256;
        if ((room->field_10 > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            count++;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

/**
 * Gives a room of given kind and owner where the creature can navigate to.
 * Counts all possible rooms, then selects one and returns it.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
struct Room *find_random_room_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count,selected;
    SYNCDBG(18,"Starting");
    count = count_rooms_creature_can_navigate_to(thing, owner, kind, nav_no_owner);
    if (count < 1)
        return NULL;
    dungeon = get_dungeon(owner);
    selected = ACTION_RANDOM(count);
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->stl_x);
        pos.y.val = get_subtile_center_pos(room->stl_y);
        pos.z.val = 256;
        if ((room->field_10 > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            if (selected > 0)
            {
                selected--;
            } else
            {
                return room;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return NULL;
}

//TODO CREATURE_AI try to make sure the creature will do proper activity in the room.
//TODO CREATURE_AI try to select lair far away from CTA and enemies
struct Room *get_room_of_given_kind_for_thing(struct Thing *thing, struct Dungeon *dungeon, long rkind)
{
  struct Room *room;
  struct Room *retroom;
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  long retdist,dist,pay;
  unsigned long k;
  long i;
  retdist = LONG_MAX;
  retroom = NULL;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    // Per-room code
    if (room->kind == RoK_TREASURE)
    {
        cctrl = creature_control_get_from_thing(thing);
        crstat = creature_stats_get_from_thing(thing);
        dungeon = get_dungeon(thing->owner);
        if (dungeon->field_1420[thing->model] )
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel) / 2;
        } else
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel);
        }
        if (room->long_17 > pay)
          continue;
    }
    dist =  abs(thing->mappos.y.stl.num - room->stl_y);
    dist += abs(thing->mappos.x.stl.num - room->stl_x);
    if (retdist > dist)
    {
      retdist = dist;
      retroom = room;
    }
    // Per-room code ends
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return retroom;
}

long create_workshop_object_in_workshop_room(long a1, long a2, long a3)
{
  return _DK_create_workshop_object_in_workshop_room(a1, a2, a3);
}

short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3)
{
    SYNCDBG(8,"Starting");
    return _DK_delete_room_slab_when_no_free_room_structures(a1, a2, a3);
}

long calculate_room_efficiency(struct Room *room)
{
  return _DK_calculate_room_efficiency(room);
}

void kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3)
{
  _DK_kill_room_slab_and_contents(a1, a2, a3);
}

void free_room_structure(struct Room *room)
{
  _DK_free_room_structure(room);
}

void reset_creatures_rooms(struct Room *room)
{
  _DK_reset_creatures_rooms(room);
}

void replace_room_slab(struct Room *room, long a2, long a3, unsigned char a4)
{
    _DK_replace_room_slab(room, a2, a3, a4);
}

struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4)
{
    return _DK_find_nearest_room_for_thing_with_spare_item_capacity(thing, a2, a3, a4);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
