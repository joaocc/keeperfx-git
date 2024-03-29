/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_msgs.h
 *     Header file for gui_msgs.c.
 * @par Purpose:
 *     Game GUI Messages functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_MSGS_H
#define DK_GUI_MSGS_H

#include "globals.h"
#include "bflib_basics.h"

#define GUI_MESSAGES_COUNT      3

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct GuiMessage { // sizeof = 0x45 (69)
    char text[64];
unsigned char plyr_idx;
unsigned long creation_turn;
};

#pragma pack()
/******************************************************************************/
void message_update(void);
void message_draw(void);
void zero_messages(void);
void message_add(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
/******************************************************************************/
