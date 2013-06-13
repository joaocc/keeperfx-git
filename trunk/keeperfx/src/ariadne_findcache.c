/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_findcache.c
 *     FindCache support functions for Ariadne pathfinding.
 * @par Purpose:
 *     Functions to maintain and use Find Cache.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 05 Aug 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_findcache.h"

#include "globals.h"
#include "bflib_basics.h"

#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_find_cache[4][4];
#define find_cache _DK_find_cache

DLLIMPORT long _DK_triangle_find8(long ptfind_x, long ptfind_y);
/******************************************************************************/
long triangle_find_cache_get(long pos_x, long pos_y)
{
  long cache_x,cache_y;
  long ntri;
  cache_x = (pos_x >> 14);
  if (cache_x > 3)
    cache_x = 3;
  if (cache_x < 0)
    cache_x = 0;
  cache_y = (pos_y >> 14);
  if (cache_y > 3)
    cache_y = 3;
  if (cache_y < 0)
    cache_y = 0;

  ntri = find_cache[cache_y][cache_x];
  if (get_triangle_tree_alt(ntri) == -1)
  {
    ntri = triangle_brute_find8_near(pos_x, pos_y);
    if ((ntri < 0) || (ntri > ix_Triangles))
    {
        ERRORLOG("triangles count overflow");
        ntri = -1;
    }
    find_cache[cache_y][cache_x] = ntri;
  }
  return ntri;

}

void triangle_find_cache_put(long pos_x, long pos_y, long ntri)
{
  long cache_x,cache_y;
  cache_x = (pos_x >> 14);
  if (cache_x > 3)
    cache_x = 3;
  if (cache_x < 0)
    cache_x = 0;
  cache_y = (pos_y >> 14);
  if (cache_y > 3)
    cache_y = 3;
  if (cache_y < 0)
    cache_y = 0;
  find_cache[cache_y][cache_x] = ntri;
}

long triangle_find8(long pt_x, long pt_y)
{
    int eqA,eqB,eqC;
    long ntri,ncor;
    unsigned long k;
    //TODO PATHFINDING may hang if out of points
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-sub-sub-function
    //return _DK_triangle_find8(pt_x, pt_y);
    ntri = triangle_find_cache_get(pt_x, pt_y);
    for (k=0; k < TRIANLGLES_COUNT; k++)
    {
      eqA = triangle_divide_areas_s8differ(ntri, 0, 1, pt_x, pt_y) > 0;
      eqB = triangle_divide_areas_s8differ(ntri, 1, 2, pt_x, pt_y) > 0;
      eqC = triangle_divide_areas_s8differ(ntri, 2, 0, pt_x, pt_y) > 0;

      switch ( eqA + 2 * (eqB + 2 * eqC) )
      {
      case 1:
          ntri = Triangles[ntri].tags[0];
          break;
      case 2:
          ntri = Triangles[ntri].tags[1];
          break;
      case 3:
          ncor = 1;
          pointed_at8(pt_x, pt_y, &ntri, &ncor);
          break;
      case 4:
          ntri = Triangles[ntri].tags[2];
          break;
      case 5:
          ncor = 0;
          pointed_at8(pt_x, pt_y, &ntri, &ncor);
          break;
      case 6:
      case 7:
          ncor = 2;
          pointed_at8(pt_x, pt_y, &ntri, &ncor);
          break;
      case 0:
          triangle_find_cache_put(pt_x, pt_y, ntri);
          return ntri;
      }
    }
    ERRORLOG("Infinite loop detected");
    return -1;
}

TbBool point_find(long pt_x, long pt_y, long *out_tri_idx, long *out_cor_idx)
{
    struct Point *pt;
    long tri_idx,cor_id;
    tri_idx = triangle_find8(pt_x << 8, pt_y << 8);
    if (tri_idx < 0)
    {
        return false;
    }
    for (cor_id=0; cor_id < 3; cor_id++)
    {
        pt = get_triangle_point(tri_idx,cor_id);
        if ((pt->x == pt_x) && (pt->y == pt_y))
        {
          *out_tri_idx = tri_idx;
          *out_cor_idx = cor_id;
          return true;
        }
    }
    return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
