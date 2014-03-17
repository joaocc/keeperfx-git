/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_senses.c
 *     Functions to check vision, hearing and other senses of creatures.
 * @par Purpose:
 *     Creature senses checks and handling.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     27 Nov 2011 - 22 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_senses.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_settings.h"
#include "map_blocks.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
DLLIMPORT long _DK_line_of_sight_3d_ignoring_specific_door(const struct Coord3d *frpos, const struct Coord3d *topos, const struct Thing *doortng);
DLLIMPORT long _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(const struct Coord3d *frpos, const struct Coord3d *topos, const struct Thing *doortng);
DLLIMPORT long _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *frpos, const struct Coord3d *topos, long plyr_idx);
DLLIMPORT unsigned char _DK_line_of_sight_3d(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT long _DK_jonty_creature_can_see_thing_including_lava_check(const struct Thing * creatng, const struct Thing * thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool sibling_line_of_sight_ignoring_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, const struct Thing *doortng)
{
    // If we don't want to ignore any doors
    if (thing_is_invalid(doortng))
    {
        // Check for door at central subtile
        if (subtile_is_door(stl_slab_center_subtile(nextpos->x.stl.num),stl_slab_center_subtile(nextpos->y.stl.num))) {
            return false;
        }
    }
    // If only one dimensions changed, allow the pass
    // (in that case the outcome has been decided before this call)
    if ((nextpos->x.stl.num == prevpos->x.stl.num) ||
        (nextpos->y.stl.num == prevpos->y.stl.num)) {
        // change is (x,0) or (0,x)
        return true;
    }
    struct Coord3d posmvx;
    struct Coord3d posmvy;
    MapSubtlDelta subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - (MapSubtlDelta)prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - (MapSubtlDelta)prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3:
        posmvx.x.val = prevpos->x.val - COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (point_in_map_is_solid_ignoring_door(&posmvx, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) X",(int)posmvx.x.stl.num,(int)posmvx.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        if (point_in_map_is_solid_ignoring_door(&posmvy, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) Y",(int)posmvy.x.stl.num,(int)posmvy.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        break;

    case -1:
        posmvx.x.val = prevpos->x.val + COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (point_in_map_is_solid_ignoring_door(&posmvx, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) X",(int)posmvx.x.stl.num,(int)posmvx.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        if (point_in_map_is_solid_ignoring_door(&posmvy, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) Y",(int)posmvy.x.stl.num,(int)posmvy.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        break;

    case 1:
        posmvx.x.val = prevpos->x.val - COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (point_in_map_is_solid_ignoring_door(&posmvx, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) X",(int)posmvx.x.stl.num,(int)posmvx.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        if (point_in_map_is_solid_ignoring_door(&posmvy, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) Y",(int)posmvy.x.stl.num,(int)posmvy.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        break;

    case 3:
        posmvx.x.val = prevpos->x.val + COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (point_in_map_is_solid_ignoring_door(&posmvx, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) X",(int)posmvx.x.stl.num,(int)posmvx.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        if (point_in_map_is_solid_ignoring_door(&posmvy, doortng)) {
            SYNCDBG(17, "Cannot see through (%d,%d) with delta (%d,%d) Y",(int)posmvy.x.stl.num,(int)posmvy.y.stl.num,(int)subdelta_x,(int)subdelta_y);
            return false;
        }
        break;

    default:
        ERRORDBG(8,"Invalid use of sibling function, delta (%d,%d)",(int)subdelta_x,(int)subdelta_y);
        break;
    }
    return true;
}


TbBool line_of_sight_3d_ignoring_specific_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, const struct Thing *doortng)
{
    MapCoordDelta dx,dy,dz;
    dx = topos->x.val - (MapCoordDelta)frpos->x.val;
    dy = topos->y.val - (MapCoordDelta)frpos->y.val;
    dz = topos->z.val - (MapCoordDelta)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_line_of_sight_3d_ignoring_specific_door(frpos, topos, doortng);
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = COORD_PER_STL;
    } else {
        dx = -dx;
        increase_x = -COORD_PER_STL;
    }
    if (dy >= 0) {
        increase_y = COORD_PER_STL;
    } else {
        dy = -dy;
        increase_y = -COORD_PER_STL;
    }
    if (dz >= 0) {
        increase_z = COORD_PER_STL;
    } else {
        dz = -dz;
        increase_z = -COORD_PER_STL;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (point_in_map_is_solid_ignoring_door(&nextpos, doortng)) {
            return false;
        }
        if (!sibling_line_of_sight_ignoring_door(&prevpos, &nextpos, doortng)) {
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

TbBool sibling_line_of_sight_3d_including_lava_check_ignoring_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, const struct Thing *doortng)
{
    // If we don't want to ignore any doors
    if (thing_is_invalid(doortng))
    {
        // Check for door at central subtile
        if (subtile_is_door(stl_slab_center_subtile(nextpos->x.stl.num),stl_slab_center_subtile(nextpos->y.stl.num))) {
            return false;
        }
    }
    // If only one dimensions changed, allow the pass
    // (in that case the outcome has been decided before this call)
    if ((nextpos->x.stl.num == prevpos->x.stl.num) ||
        (nextpos->y.stl.num == prevpos->y.stl.num)) {
        // change is (x,0) or (0,x)
        return true;
    }
    struct Coord3d posmvx;
    struct Coord3d posmvy;
    MapSubtlDelta subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - (MapSubtlDelta)prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - (MapSubtlDelta)prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3:
        posmvx.x.val = prevpos->x.val - COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_door(&posmvx, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&posmvy, doortng) & 0x01) {
            return false;
        }
        break;

    case -1:
        posmvx.x.val = prevpos->x.val + COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_door(&posmvx, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&posmvy, doortng) & 0x01) {
            return false;
        }
        break;

    case 1:
        posmvx.x.val = prevpos->x.val - COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_door(&posmvx, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&posmvy, doortng) & 0x01) {
            return false;
        }
        break;

    case 3:
        posmvx.x.val = prevpos->x.val + COORD_PER_STL;
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + COORD_PER_STL;
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_door(&posmvx, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&posmvy, doortng) & 0x01) {
            return false;
        }
        break;

    default:
        ERRORDBG(8,"Invalid use of sibling function, delta (%d,%d)",(int)subdelta_x,(int)subdelta_y);
        break;
    }
    return true;
}

TbBool jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, const struct Thing *doortng)
{
    MapCoordDelta dx,dy,dz;
    dx = topos->x.val - (MapCoordDelta)frpos->x.val;
    dy = topos->y.val - (MapCoordDelta)frpos->y.val;
    dz = topos->z.val - (MapCoordDelta)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(frpos, topos, doortng);
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = COORD_PER_STL;
    } else {
        dx = -dx;
        increase_x = -COORD_PER_STL;
    }
    if (dy >= 0) {
        increase_y = COORD_PER_STL;
    } else {
        dy = -dy;
        increase_y = -COORD_PER_STL;
    }
    if (dz >= 0) {
        increase_z = COORD_PER_STL;
    } else {
        dz = -dz;
        increase_z = -COORD_PER_STL;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (get_point_in_map_solid_flags_ignoring_door(&nextpos, doortng) & 0x01) {
            return false;
        }
        if (!sibling_line_of_sight_3d_including_lava_check_ignoring_door(&prevpos, &nextpos, doortng)) {
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

TbBool sibling_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, PlayerNumber plyr_idx)
{
    // Check for door at central subtile
    if (subtile_is_door(stl_slab_center_subtile(nextpos->x.stl.num), stl_slab_center_subtile(nextpos->y.stl.num))) {
        return false;
    }
    // If only one dimensions changed, allow the pass
    // (in that case the outcome has been decided before this call)
    if ((nextpos->x.stl.num == prevpos->x.stl.num)
     || (nextpos->y.stl.num == prevpos->y.stl.num)) {
        // change is (x,0) or (0,x)
        return true;
    }
    struct Coord3d posmvy;
    struct Coord3d posmvx;
    int subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - (MapSubtlDelta)prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - (MapSubtlDelta)prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3: // change is (-1,-1)
        posmvx.x.val = prevpos->x.val - subtile_coord(1,0);
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - subtile_coord(1,0);
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvy, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvx, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case -1: // change is (1,-1) as (-1,0) was eliminated earlier
        posmvx.x.val = prevpos->x.val + subtile_coord(1,0);
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val - subtile_coord(1,0);
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvy, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvx, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case 1: // change is (-1,1) as (1,0) was eliminated earlier
        posmvx.x.val = prevpos->x.val - subtile_coord(1,0);
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + subtile_coord(1,0);
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvy, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvx, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case 3: // change is (1,1)
        posmvx.x.val = prevpos->x.val + subtile_coord(1,0);
        posmvx.y.val = prevpos->y.val;
        posmvx.z.val = prevpos->z.val;
        posmvy.x.val = prevpos->x.val;
        posmvy.y.val = prevpos->y.val + subtile_coord(1,0);
        posmvy.z.val = prevpos->z.val;
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvy, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&posmvx, plyr_idx) & 0x01) {
            return false;
        }
        break;

    default:
        ERRORDBG(8,"Invalid use of sibling function, delta (%d,%d)",(int)subdelta_x,(int)subdelta_y);
        break;
    }
    return true;
}

TbBool jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, PlayerNumber plyr_idx)
{
    MapCoordDelta dx,dy,dz;
    dx = topos->x.val - (MapCoordDelta)frpos->x.val;
    dy = topos->y.val - (MapCoordDelta)frpos->y.val;
    dz = topos->z.val - (MapCoordDelta)frpos->z.val;
    // Allow the travel to the same subtile
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        SYNCDBG(17, "Player %d can see (%d,%d) as its on same subtile",
            (int)plyr_idx,(int)topos->x.stl.num,(int)topos->y.stl.num);
        return true;
    }
    //return _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(frpos, topos, plyr_idx);
    // Initialize increases and do abs() of dx,dy and dz
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = COORD_PER_STL;
    } else {
        dx = -dx;
        increase_x = -COORD_PER_STL;
    }
    if (dy >= 0) {
        increase_y = COORD_PER_STL;
    } else {
        dy = -dy;
        increase_y = -COORD_PER_STL;
    }
    if (dz >= 0) {
        increase_z = COORD_PER_STL;
    } else {
        dz = -dz;
        increase_z = -COORD_PER_STL;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = increase_x * dx / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;

    while (distance > 0)
    {
        if (get_point_in_map_solid_flags_ignoring_own_door(&nextpos, plyr_idx) & 0x01) {
            SYNCDBG(17, "Player %d cannot see through (%d,%d) due to linear path solid flags (downcount %d)",
                (int)plyr_idx,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num,(int)distance);
            return false;
        }
        if (!sibling_line_of_sight_3d_including_lava_check_ignoring_own_door(&prevpos, &nextpos, plyr_idx)) {
            SYNCDBG(17, "Player %d cannot see through (%d,%d) due to 3D line of sight (downcount %d)",
                (int)plyr_idx,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num,(int)distance);
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    SYNCDBG(17, "Player %d can see (%d,%d)",
        (int)plyr_idx,(int)topos->x.stl.num,(int)topos->y.stl.num);
    return true;
}

TbBool jonty_creature_can_see_thing_including_lava_check(const struct Thing *creatng, const struct Thing *thing)
{
    struct CreatureStats *crstat;
    const struct Coord3d *srcpos;
    struct Coord3d pos1;
    struct Coord3d pos2;
    //return _DK_jonty_creature_can_see_thing_including_lava_check(creatng, thing);
    crstat = creature_stats_get_from_thing(creatng);
    srcpos = &creatng->mappos;
    pos1.x.val = srcpos->x.val;
    pos1.y.val = srcpos->y.val;
    pos1.z.val = srcpos->z.val;
    pos2.x.val = thing->mappos.x.val;
    pos2.y.val = thing->mappos.y.val;
    pos2.z.val = thing->mappos.z.val;
    pos1.z.val += crstat->eye_height;
    if (thing->class_id == TCls_Door)
    {
        // If we're immune to lava, or we're already on it - don't care, travel over it
        if (lava_at_position(srcpos) || creature_can_travel_over_lava(creatng))
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks w/o lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (line_of_sight_3d_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (line_of_sight_3d_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            return false;
        } else
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks with lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            return false;
        }
    } else
    {
        // If we're immune to lava, or we're already on it - don't care, travel over it
        if (lava_at_position(srcpos) || creature_can_travel_over_lava(creatng))
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks w/o lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (line_of_sight_3d(&pos1, &pos2))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (line_of_sight_3d(&pos1, &pos2))
                return true;
            return false;
        } else
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks with lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(&pos1, &pos2, creatng->owner))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(&pos1, &pos2, creatng->owner))
                return true;
            return false;
        }
    }
}

TbBool line_of_sight_3d(const struct Coord3d *frpos, const struct Coord3d *topos)
{
    MapCoordDelta dx,dy,dz;
    dx = topos->x.val - (MapCoordDelta)frpos->x.val;
    dy = topos->y.val - (MapCoordDelta)frpos->y.val;
    dz = topos->z.val - (MapCoordDelta)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_line_of_sight_3d(frpos, topos);

    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = COORD_PER_STL;
    } else {
        dx = -dx;
        increase_x = -COORD_PER_STL;
    }
    if (dy >= 0) {
        increase_y = COORD_PER_STL;
    } else {
        dy = -dy;
        increase_y = -COORD_PER_STL;
    }
    if (dz >= 0) {
        increase_z = COORD_PER_STL;
    } else {
        dz = -dz;
        increase_z = -COORD_PER_STL;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (point_in_map_is_solid(&nextpos)) {
            SYNCDBG(7, "Player cannot see through (%d,%d) due to linear path solid flags (downcount %d)",
                (int)nextpos.x.stl.num,(int)nextpos.y.stl.num,(int)distance);
            return false;
        }
        if (!sibling_line_of_sight(&prevpos, &nextpos)) {
            SYNCDBG(7, "Player cannot see through (%d,%d) due to 3D line of sight (downcount %d)",
                (int)nextpos.x.stl.num,(int)nextpos.y.stl.num,(int)distance);
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

long get_explore_sight_distance_in_slabs(const struct Thing *thing)
{
    struct PlayerInfo *player;
    if (!thing_exists(thing)) {
        return 0;
    }
    if (is_neutral_thing(thing)) {
        return 7;
    }
    player = get_player(thing->owner);
    long dist;
    if (player->controlled_thing_idx != thing->index) {
        dist = 7;
    } else {
        dist = get_creature_can_see_subtiles() / STL_PER_SLB;
        if (dist <= 7)
            dist = 7;
    }
    return dist;
}
/******************************************************************************/
