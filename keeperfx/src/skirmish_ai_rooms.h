/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_rooms.h
 *     Header file for skirmish_ai_rooms.c
 * @par Purpose:
 *     Room/dungeon layout routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_ROOMS_H
#define SKIRMISH_AI_ROOMS_H

#include "skirmish_ai.h"
#include "room_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAI_MAX_ROOMS   500

#define SAI_ROOM_FLAG_EXTENDED  0x1
#define SAI_ROOM_FLAG_TEMP      0x2 //for algorithms requiring temp marker bit


enum SAI_RoomState
{
    SAI_ROOM_UNUSED, //no other data is valid
    SAI_ROOM_UNDUG, //not (intentionally) dug - room kind determines if anything is planned for room
    SAI_ROOM_EMPTY, //is dug but nothing built, room kind determines if anything planned
    SAI_ROOM_ACTIVE, //room has been built and is a part of dungeon
    SAI_ROOM_INCOMPLETE, //room is incomplete; meaning it does not fill up entire possible space
    SAI_ROOM_BAD, //bad room layout, SELL ASAP and move to SAI_ROOM_UNUSED
    SAI_ROOM_BEING_BUILT, //room is being built
    SAI_ROOM_BEING_DUG, //room is being dug
    SAI_ROOM_BEING_SOLD, //room has been built (in parts or what not) but is being sold

    SAI_ROOM_STATE_COUNT //counter
};

struct SAI_Room
{
    int id;
    int num_tiles_in_use; //tiles planned
    unsigned flags;
    enum RoomKinds kind;
    enum SAI_RoomState state;
    struct SAI_Room * prev_of_state;
    struct SAI_Room * next_of_state;
    SAI_Rect rect;
};

enum SAI_WallBreakOptions
{
    /**
     * We don't particularly care if we crash into enemy dungeon or not - it should
     * not stop us from pathing to where we want to be.
     */
    SAI_WB_DONT_CARE = 0,

    /**
     * We want to get into enemy player's dungeon.
     */
    SAI_WB_PREFER,

    /**
     * Avoid anything that could put us in contact with enemy (including digging to
     * lava or water - if we do we must dig all around said lava or water to be able
     * to fortify).
     */
    SAI_WB_AVOID
};

/**
 * Initializes the room layout data structures based on the current map state for
 * a given player.
 * @param plyr Index of player.
 */
void SAI_init_room_layout_for_player(int plyr);

/**
 * Sets parameters for how the room layout routines should behave in presence of
 * enemy keepers.
 * @param plyr Index of player.
 * @param wall_break How to approach the possibility of walling off.
 */
void SAI_set_room_layout_safety(int plyr, enum SAI_WallBreakOptions wall_break);

/**
 * Requests that player should have a room assigned for a certain purpose.
 * Will either return a dug (but empty) room, an undug room, or a neutral room which
 * the player is expected to dig to.
 * The room which is returned will have had its room kind assigned automatically,
 * and this function does cause state changes in module.
 * @param plyr Index of player.
 * @param kind Type of room.
 * @param size Minimal size of room; returned room may be larger.
 * @return If >= 0, the index of room. If it was not possibly to fulfill request,
 * returns -1.
 */
int SAI_request_room(int plyr, enum RoomKinds kind, int size);

/**
 * Finds an access path (or possibly more depending on room kind) that can be dug
 * in order to access a room.
 * Does not change state of the room module in any way.
 * @param plyr Index of player.
 * @param id Index of room we want to dig to.
 * @param coords Output coordinate array. Must be free()d after use by caller.
 * @return Number of coords. 0 if no path exists.
 */
int SAI_find_path_to_room(int plyr, int id, struct SAI_Point ** coords);

/**
 * Finds an access path to a gold area.
 * Does not change state of the room module in any way.
 * @param plyr Index of player.
 * @param gold_area_idx Index of targeted gold area.
 * @param coords Output coordinate array. Must be free()d after use by caller.
 * @return Number of coords. 0 if no path exists.
 */
int SAI_find_path_to_gold_area(int plyr, int gold_area_idx,
    struct SAI_Point ** coords);

/**
 * Finds an access path to a certain position.
 * Does not change state of the room module in any way.
 * @param plyr Index of player.
 * @param pos Targeted position.
 * @param coords Output coordinate array. Must be free()d after use by caller.
 * @return Number of coords. 0 if no path exists.
 */
int SAI_find_path_to_position(int plyr, SAI_Point pos,
    struct SAI_Point ** coords);

/**
 * Gets an AI room.
 * Don't modify pointed objects manually, use SAI_.. functions in this module
 * to do it; state transition are best handled by appropriate functions which
 * can error check states.
 * @param plyr Index of player.
 * @param id Identifier of used room. Must be >= 0 and < SAI_MAX_ROOMS.
 * @return A pointer to the room if the id was valid, otherwise NULL.
 */
struct SAI_Room * SAI_get_room(int plyr, int id);

/**
 * Sets the kind of a room. It is assert()ed if in a valid state.
 * @param plyr Index of player.
 * @param room_id Index of room.
 * @param kind Kind of room to set to.
 */
void SAI_set_room_kind(int plyr, int room_id, enum RoomKinds kind);

/**
 * Sets the number of tiles the AI actually intends can be used (can be less than
 * width * height).
 * @param plyr Index of player.
 * @param room_id Index of room.
 * @param tiles_used Kind of room to set to.
 */
void SAI_set_tiles_in_use(int plyr, int room_id, int tiles_used);

/**
 * Gets the head of a list of rooms by state.
 * Don't modify pointed objects manually, use SAI_.. functions in this module
 * to do it.
 * @param plyr Index of player.
 * @param state State of the rooms in the list to be returned.
 * @return
 */
struct SAI_Room * SAI_get_rooms_of_state(int plyr, enum SAI_RoomState state);

/**
 * Sets the state of an AI room and updates corresponding lists.
 * @param plyr Index of player.
 * @param room_id Index of room.
 * @param new_state New state of room.
 */
void SAI_set_room_state(int plyr, int room_id, enum SAI_RoomState new_state);

/**
 * Calculates width of rectangle. The width is always at least 1, due to definition.
 * @param rect
 * @return
 */
int SAI_rect_width(struct SAI_Rect rect);

/**
 * Calculates height of rectangle. The height is always at least 1, due to definition.
 * @param rect
 * @return
 */
int SAI_rect_height(struct SAI_Rect rect);

/**
 * Finds the center point (approximative with rounding error) of a rectangle.
 * @param rect
 * @return The center point.
 */
struct SAI_Point SAI_rect_center(struct SAI_Rect rect);

/**
 * Checks if a point is in a rectangle.
 * @param p
 * @param rect
 * @return Non-zero if point is in rectangle.
 */
int SAI_point_in_rect(struct SAI_Point p, struct SAI_Rect rect);


#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_ROOMS_H
