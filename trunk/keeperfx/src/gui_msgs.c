/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_msgs.c
 *     Game GUI Messages functions.
 * @par Purpose:
 *     Functions to display and maintain GUI Messages.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_msgs.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sprfnt.h"

#include "gui_draw.h"
#include "frontend.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_message_update(void);
DLLIMPORT void _DK_message_draw(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void message_draw(void)
{
    int i,h;
    long x,y;
    SYNCDBG(7,"Starting");
    LbTextSetFont(winfont);
    int tx_units_per_px;
    tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    int ps_units_per_px;
    {
        struct TbSprite *spr;
        spr = &gui_panel_sprites[488];
        ps_units_per_px = (22 * units_per_pixel) / spr->SHeight;
    }
    h = LbTextLineHeight();
    x = 148*units_per_pixel/16;
    y = 28*units_per_pixel/16;
    for (i=0; i < game.active_messages_count; i++)
    {
        LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
        set_flag_word(&lbDisplay.DrawFlags,Lb_TEXT_ONE_COLOR,false);
        LbTextDrawResized(x+32*units_per_pixel/16, y, tx_units_per_px, game.messages[i].text);
        draw_gui_panel_sprite_left(x, y, ps_units_per_px, 488+game.messages[i].plyr_idx);
        y += h*units_per_pixel/16;
    }
}

void message_update(void)
{
    SYNCDBG(6,"Starting");
    //_DK_message_update();
    int i;
    i = game.active_messages_count - 1;
    // Set end turn for all messages
    while (i >= 0)
    {
        struct GuiMessage *gmsg;
        gmsg = &game.messages[i];
        if (gmsg->creation_turn + 400 < game.play_gameturn)
        {
            game.active_messages_count--;
            game.messages[game.active_messages_count].text[0] = 0;
        }
        i--;
    }
}

void zero_messages(void)
{
    int i;
    game.active_messages_count = 0;
    for (i=0; i<3; i++)
    {
      memset(&game.messages[i], 0, sizeof(struct GuiMessage));
    }
}

void message_add(PlayerNumber plyr_idx)
{
    int i;
    for (i=GUI_MESSAGES_COUNT-1; i > 0; i--) {
        memcpy(&game.messages[i], &game.messages[i-1], sizeof(struct GuiMessage));
    }
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    strcpy(game.messages[0].text, player->mp_message_text);
    game.messages[0].plyr_idx = plyr_idx;
    game.messages[0].creation_turn = game.play_gameturn;
    if (game.active_messages_count < GUI_MESSAGES_COUNT) {
        game.active_messages_count++;
    }
}
/******************************************************************************/
