/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_data.c
 *     Map array data management functions.
 * @par Purpose:
 *     Functions to support the map data array, which stores map blocks information.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     15 May 2009 - 12 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_data.h"
#include "globals.h"

#include "bflib_math.h"
#include "slab_data.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Map bad_map_block;
const long map_to_slab[] = {
   0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,
   6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11,
  12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17,
  18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
  24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29,
  30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35,
  36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41,
  42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47,
  48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53,
  54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59,
  60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65,
  66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71,
  72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77,
  78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 82, 82, 82, 83, 83, 83,
  84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 88, 88, 88, 89, 89, 89,
};

/** Map subtiles, X dimension.
 *  @note The subtile indexed [map_subtiles_x] should exist
 *      in the map, so there really is map_subtiles_x+1 subtiles. */
int map_subtiles_x = 255;
/** Map subtiles, Y dimension.
 *  @note The subtile indexed [map_subtiles_y] should exist
 *      in the map, so there really is map_subtiles_y+1 subtiles. */
int map_subtiles_z = 8;
/** Map subtiles, Z dimension.
 */
int map_subtiles_y = 255;
/** Map tiles, X dimension.
 *  Equals to tiles (slabs) count; The last slab has index map_tiles_x-1. */
int map_tiles_x = 85;
/** Map tiles, Y dimension.
 *  Equals to tiles (slabs) count; The last slab has index map_tiles_y-1. */
int map_tiles_y = 85;

long navigation_map_size_x = 256;
long navigation_map_size_y = 256;

unsigned char *IanMap = NULL;
long nav_map_initialised = 0;
/******************************************************************************/
/**
 * Returns if the subtile coords are in range of subtiles which have slab entry.
 */
TbBool subtile_has_slab(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  if ((stl_x >= 0) && (stl_x < 3*map_tiles_x))
    if ((stl_y >= 0) && (stl_y < 3*map_tiles_y))
      return true;
  return false;
}

/**
 * Returns if the subtile coords are in range map subtiles.
 */
TbBool subtile_coords_invalid(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  if ((stl_x < 0) || (stl_x > map_subtiles_x))
      return true;
  if ((stl_y < 0) || (stl_y > map_subtiles_y))
      return true;
  return false;
}

struct Map *get_map_block_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  if ((stl_x < 0) || (stl_x > map_subtiles_x))
      return INVALID_MAP_BLOCK;
  if ((stl_y < 0) || (stl_y > map_subtiles_y))
      return INVALID_MAP_BLOCK;
  return &game.map[get_subtile_number(stl_x,stl_y)];
}

struct Map *get_map_block_at_pos(long stl_num)
{
  if ((stl_num < 0) || (stl_num > get_subtile_number(map_subtiles_x,map_subtiles_y)))
      return INVALID_MAP_BLOCK;
  return &game.map[stl_num];
}

TbBool map_block_invalid(const struct Map *map)
{
  if (map == NULL)
    return true;
  if (map == INVALID_MAP_BLOCK)
    return true;
  return (map < &game.map[0]);
}

unsigned long get_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  if ((stl_x < 0) || (stl_x > map_subtiles_x))
      return 0;
  if ((stl_y < 0) || (stl_y > map_subtiles_y))
      return 0;
  return game.navigation_map[navmap_tile_number(stl_x,stl_y)];
}

void set_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long navcolour)
{
  if ((stl_x < 0) || (stl_x > map_subtiles_x))
      return;
  if ((stl_y < 0) || (stl_y > map_subtiles_y))
      return;
  game.navigation_map[navmap_tile_number(stl_x,stl_y)] = navcolour;
}

long get_ceiling_height(const struct Coord3d *pos)
{
  long i;
  i = get_subtile_number(pos->x.stl.num,pos->y.stl.num);
  return ((game.map[i].data & 0xF000000u) >> 24) << 8;
}

long get_mapwho_thing_index(const struct Map *map)
{
  return ((map->data >> 11) & 0x7FF);
  //could also be ((map->data & 0x3FF800) >> 11);
}

void set_mapwho_thing_index(struct Map *map, long thing_idx)
{
  // Check if new value is correct
  if ((unsigned long)thing_idx > 0x7FF)
  {
      ERRORLOG("Tried to set invalid thing %ld",thing_idx);
      return;
  }
  // Clear previous and set new
  map->data ^= (map->data ^ ((unsigned long)thing_idx << 11)) & 0x3FF800;
}

long get_mapblk_column_index(const struct Map *map)
{
  return ((map->data) & 0x7FF);
}

void set_mapblk_column_index(struct Map *map, long column_idx)
{
  // Check if new value is correct
  if ((unsigned long)column_idx > 0x7FF)
  {
      ERRORLOG("Tried to set invalid column %ld",column_idx);
      return;
  }
  // Clear previous and set new
  map->data ^= (map->data ^ ((unsigned long)column_idx)) & 0x7FF;
}

void reveal_map_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
  unsigned short nflag;
  struct Map *map;
  unsigned long i;
  nflag = (1 << plyr_idx);
  map = get_map_block_at(stl_x, stl_y);
  i = (map->data >> 28) | nflag;
  map->data |= (i & 0x0F) << 28;
}

TbBool subtile_revealed(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
  unsigned short plyr_bit;
  struct Map *map;
  plyr_bit = (1 << plyr_idx);
  map = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(map))
    return false;
  if ((map->data >> 28) & plyr_bit)
    return true;
  return false;
}

void reveal_map_block(struct Map *map, long plyr_idx)
{
  unsigned short nflag;
  unsigned long i;
  nflag = (1 << plyr_idx);
  i = (map->data >> 28) | nflag;
  map->data |= (i & 0x0F) << 28;
}

TbBool map_block_revealed(const struct Map *map, long plyr_idx)
{
  unsigned short plyr_bit;
  plyr_bit = (1 << plyr_idx);
  if (map_block_invalid(map))
    return false;
  if ((map->data >> 28) & plyr_bit)
    return true;
  return false;
}

TbBool map_block_revealed_bit(const struct Map *map, long plyr_bit)
{
  if (map_block_invalid(map))
    return false;
  if ((map->data >> 28) & plyr_bit)
    return true;
  return false;
}

TbBool valid_dig_position(long plyr_idx, long stl_x, long stl_y)
{
    const struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & MapFlg_Unkn10) == 0)
    {
        /* This is original condition that was in this function.
         * I'm not sure what the author ment, but it's obviously wrong.
        if ((plyr_idx == 0)  ||  !map_block_revealed(mapblk, 0))
            if (!map_pos_is_lava(stl_x, stl_y))
                return true;
        */
        if (map_block_revealed(mapblk, plyr_idx) && !map_pos_is_lava(stl_x, stl_y))
            return true;
    }
    return false;
}
/******************************************************************************/

/** Sets map coordinates to given values, clipping them to map dimensions.
 *
 * @param pos Position to be set.
 * @param cor_x Input X coordinate.
 * @param cor_y Input Y coordinate.
 * @param cor_z Input Z coordinate.
 * @return Gives true if values were in map coords range, false if they were corrected.
 */
TbBool set_coords_with_range_check(struct Coord3d *pos, MapCoord cor_x, MapCoord cor_y, MapCoord cor_z, TbBool clip)
{
    TbBool corrected = false;
    if (cor_x >= ((map_subtiles_x+2) << 8)) {
        if (clip) cor_x = ((map_subtiles_x+2) << 8)-1;
        corrected = true;
    }
    if (cor_y >= ((map_subtiles_y+2) << 8)) {
        if (clip) cor_y = ((map_subtiles_y+2) << 8)-1;
        corrected = true;
    }
    if (cor_z >= (17 << 8)) {
        if (clip) cor_z = (17 << 8)-1;
        corrected = true;
    }
    if (cor_x < 0) {
        if (clip) cor_x = 0;
        corrected = true;
    }
    if (cor_y < 0) {
        if (clip) cor_y = 0;
        corrected = true;
    }
    if (cor_z < 0) {
        if (clip) cor_z = 0;
        corrected = true;
    }
    pos->x.val = cor_x;
    pos->y.val = cor_y;
    pos->z.val = cor_z;
    return !corrected;
}

TbBool set_coords_to_subtile_center(struct Coord3d *pos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_z)
{
    if (stl_x > map_subtiles_x+1) stl_x = map_subtiles_x+1;
    if (stl_y > map_subtiles_y+1) stl_y = map_subtiles_y+1;
    if (stl_z > 16) stl_z = 16;
    if (stl_x < 0)  stl_x = 0;
    if (stl_y < 0) stl_y = 0;
    if (stl_z < 0) stl_z = 0;
    pos->x.val = (stl_x<<8) + 128;
    pos->y.val = (stl_y<<8) + 128;
    pos->z.val = (stl_z<<8) + 128;
    return true;
}

TbBool set_coords_to_slab_center(struct Coord3d *pos, MapSubtlCoord slb_x, MapSubtlCoord slb_y)
{
    return set_coords_to_subtile_center(pos, slb_x*3+1,slb_y*3+1, 1);
}

MapCoord get_subtile_center_pos(MapSubtlCoord stl_v)
{
    return (stl_v<<8) + 128;
}

/**
 * Sets coordinates to cylindric XY shift of given source position.
 *
 * @param pos
 * @param source
 * @param radius
 * @param angle
 * @return
 */
TbBool set_coords_to_cylindric_shift(struct Coord3d *pos, const struct Coord3d *source, long radius, long angle, long z)
{
    long px,py,pz;
    px = source->x.val + ((radius * LbSinL(angle)) >> 16);
    py = source->y.val - ((radius * LbCosL(angle)) >> 16);
    pz = source->z.val + z;
    return set_coords_with_range_check(pos, px, py, pz, true);
}

TbBool set_coords_add_velocity(struct Coord3d *pos, const struct Coord3d *source, const struct CoordDelta3d *velocity, TbBool clip)
{
    long sx,sy,sz;
    sx = velocity->x.val;
    if (sx < -MOVE_VELOCITY_LIMIT) {
        sx = -MOVE_VELOCITY_LIMIT;
    } else
    if (sx > MOVE_VELOCITY_LIMIT) {
        sx = MOVE_VELOCITY_LIMIT;
    }
    sy = velocity->y.val;
    if (sy < -MOVE_VELOCITY_LIMIT) {
        sy = -MOVE_VELOCITY_LIMIT;
    } else
    if (sy > MOVE_VELOCITY_LIMIT) {
        sy = MOVE_VELOCITY_LIMIT;
    }
    sz = velocity->z.val;
    if (sz < -MOVE_VELOCITY_LIMIT) {
        sz = -MOVE_VELOCITY_LIMIT;
    } else
    if (sz > MOVE_VELOCITY_LIMIT) {
        sz = MOVE_VELOCITY_LIMIT;
    }
    return set_coords_with_range_check(pos, source->x.val+sx, source->y.val+sy, source->z.val+sz, clip);
}

/**
 * Subtile number - stores both X and Y coords in one number.
 */
SubtlCodedCoords get_subtile_number(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  if (stl_x > map_subtiles_x+1) stl_x = map_subtiles_x+1;
  if (stl_y > map_subtiles_y+1) stl_y = map_subtiles_y+1;
  if (stl_x < 0)  stl_x = 0;
  if (stl_y < 0) stl_y = 0;
  return stl_y*(map_subtiles_x+1) + stl_x;
}

/**
 * Decodes X coordinate from subtile number.
 */
MapSubtlCoord stl_num_decode_x(SubtlCodedCoords stl_num)
{
  return stl_num % (map_subtiles_x+1);
}

/**
 * Decodes Y coordinate from subtile number.
 */
MapSubtlCoord stl_num_decode_y(SubtlCodedCoords stl_num)
{
  return (stl_num/(map_subtiles_x+1))%map_subtiles_y;
}

/**
 * Returns subtile number for center subtile on given slab.
 */
SubtlCodedCoords get_subtile_number_at_slab_center(long slb_x, long slb_y)
{
  return get_subtile_number(slb_x*3+1,slb_y*3+1);
}

/**
 * Returns subtile coordinate for central subtile on given slab.
 */
MapSubtlCoord slab_center_subtile(MapSubtlCoord stl_v)
{
  return map_to_slab[stl_v]*3+1;
}

/**
 * Returns subtile coordinate for starting subtile on given slab.
 */
MapSubtlCoord slab_starting_subtile(MapSubtlCoord stl_v)
{
  return map_to_slab[stl_v]*3;
}

/**
 * Returns subtile coordinate for ending subtile on given slab.
 */
MapSubtlCoord slab_ending_subtile(MapSubtlCoord stl_v)
{
  return map_to_slab[stl_v]*3+2;
}

long get_subtile_lightness(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (stl_x > map_subtiles_x) stl_x = map_subtiles_x;
    if (stl_y > map_subtiles_y) stl_y = map_subtiles_y;
    if (stl_x < 0)  stl_x = 0;
    if (stl_y < 0) stl_y = 0;
    return game.subtile_lightness[get_subtile_number(stl_x,stl_y)];
}
/******************************************************************************/

void clear_mapwho(void)
{
  //_DK_clear_mapwho();
  struct Map *map;
  MapSubtlCoord x,y;
  for (y=0; y < (map_subtiles_y+1); y++)
  {
      for (x=0; x < (map_subtiles_x+1); x++)
      {
          map = &game.map[get_subtile_number(x,y)];
          map->data &= 0xFFC007FFu;
      }
  }
}

void clear_mapmap_soft(void)
{
    struct Map *map;
    MapSubtlCoord x,y;
    unsigned short *wptr;
    for (y=0; y < (map_subtiles_y+1); y++)
    {
        for (x=0; x < (map_subtiles_x+1); x++)
        {
          map = &game.map[get_subtile_number(x,y)];
          wptr = &game.subtile_lightness[get_subtile_number(x,y)];
          map->data &= 0xFF3FFFFFu;
          map->data &= 0xFFFFF800u;
          map->data &= 0xFFC007FFu;
          map->data &= 0x0FFFFFFFu;
          map->flags = 0;
          *wptr = MINIMUM_LIGHTNESS;
        }
    }
}

void clear_mapmap(void)
{
    struct Map *map;
    unsigned long x,y;
    unsigned short *wptr;
    unsigned char *flg;
    for (y=0; y < (map_subtiles_y+1); y++)
    {
        for (x=0; x < (map_subtiles_x+1); x++)
        {
          map = get_map_block_at(x,y);
          wptr = &game.subtile_lightness[get_subtile_number(x,y)];
          flg = &game.navigation_map[get_subtile_number(x,y)];
          memset(map, 0, sizeof(struct Map));
          *wptr = MINIMUM_LIGHTNESS;
          *flg = 0;
        }
    }
}

/**
 * Clears digging operations for given player on given map slabs rectangle.
 *
 * @param plyr_idx Player index whose dig tag shall be cleared.
 * @param start_x Slabs range X starting coord.
 * @param end_x Slabs range X ending coord.
 * @param start_y Slabs range Y starting coord.
 * @param end_y Slabs range Y ending coord.
 */
void clear_dig_for_map_rect(long plyr_idx,long start_x,long end_x,long start_y,long end_y)
{
  long x,y;
  for (y = start_y; y < end_y; y++)
    for (x = start_x; x < end_x; x++)
    {
      clear_slab_dig(x, y, plyr_idx);
    }
}

/**
 * Reveals map subtiles rectangle for given player.
 * Low level function - use reveal_map_area() instead.
 */
void reveal_map_rect(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y)
{
    MapSubtlCoord x,y;
  for (y = start_y; y < end_y; y++)
    for (x = start_x; x < end_x; x++)
    {
      reveal_map_subtile(x, y, plyr_idx);
    }
}

/**
 * Reveals map subtiles rectangle for given player.
 */
void reveal_map_area(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y)
{
  start_x = slab_starting_subtile(start_x);
  start_y = slab_starting_subtile(start_y);
  end_x = slab_ending_subtile(end_x)+1;
  end_y = slab_ending_subtile(end_y)+1;
  clear_dig_for_map_rect(plyr_idx,map_to_slab[start_x],map_to_slab[end_x],
      map_to_slab[start_y],map_to_slab[end_y]);
  reveal_map_rect(plyr_idx,start_x,end_x,start_y,end_y);
  pannel_map_update(start_x,start_y,end_x,end_y);
}

TbBool map_pos_is_lava(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  unsigned long navmap;
  navmap = get_navigation_map(stl_x, stl_y);
  return ((navmap & 0x10) != 0);
}

TbBool lava_at_position(const struct Coord3d *pos)
{
    unsigned long navmap;
    navmap = get_navigation_map(pos->x.stl.num, pos->y.stl.num);
    return ((navmap & 0x10) != 0);
}

/**
 * Returns if given subtile contains room belonging to given player.
 * @param plyr_idx The player the tile shall belong to.
 * @param stl_x The subtile X coordinate.
 * @param stl_y The subtile Y coordinate.
 * @return Gives true if the tile contains any room belonging to given player, false otherwise.
 */
TbBool subtile_is_player_room(long plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    struct SlabMap *slb;
    mapblk = get_map_block_at(stl_x,stl_y);
    if (map_block_invalid(mapblk) || ((mapblk->flags & MapFlg_IsRoom) == 0))
        return false;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_owner(slb) != plyr_idx)
        return false;
    return true;
}

TbBool subtile_is_sellable_room(long plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    struct SlabMap *slb;
    mapblk = get_map_block_at(stl_x,stl_y);
    if (map_block_invalid(mapblk) || ((mapblk->flags & MapFlg_IsRoom) == 0))
        return false;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_owner(slb) != plyr_idx)
        return false;
    if ((slb->kind == SlbT_ENTRANCE) || (slb->kind == SlbT_DUNGHEART))
        return false;
    return true;
}

void neutralise_enemy_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long domn_plyr_idx)
{
    struct SlabMap *slb;
    MapSlabCoord slb_x,slb_y;
    unsigned long  wlb;
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    slb = get_slabmap_block(slb_x, slb_y);
    wlb = slabmap_wlb(slb);
    if (wlb == 1)
    {
        place_slab_type_on_map(12, 3*slb_x, 3*slb_y, game.neutral_player_num, 0);
    } else
    if (wlb == 2)
    {
        place_slab_type_on_map(13, 3*slb_x, 3*slb_y, game.neutral_player_num, 0);
    } else
    {
        place_slab_type_on_map(10, 3*slb_x, 3*slb_y, game.neutral_player_num, 1);
    }
    do_slab_efficiency_alteration(slb_x, slb_y);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
