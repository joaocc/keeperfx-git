/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_soundmsgs.c
 *     Allows to play sound messages during the game.
 * @par Purpose:
 *     Functions to manage queue of speeches to be played.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Jun 2010 - 11 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_soundmsgs.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_math.h"
#include "game_merge.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_process_messages(void);
/******************************************************************************/
DLLIMPORT struct MessageQueueEntry _DK_message_queue[MESSAGE_QUEUE_COUNT];
#define message_queue _DK_message_queue
DLLIMPORT unsigned long _DK_message_playing;
#define message_playing _DK_message_playing
/******************************************************************************/
Phrase phrases[] = {
    0,  1,  2, 3,  4,   5,  6,  7,  8, 9,  10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,
};

struct SMessage messages[] = {
  {  0, 0, 0},
  {  1, 1, 0},
  {  2, 1, 0},
  {  3, 1, 0},
  {  4, 1, 0},
  {  5, 1, 0},
  {  6, 1, 0},
  {  7, 1, 0},
  {  8, 1, 0},
  {  9, 1, 0},
  { 10, 1, 0},
  { 11, 1, 0},
  { 12, 1, 0},
  { 13, 1, 0},
  { 14, 1, 0},
  { 15, 1, 0},
  { 16, 1, 0},
  { 17, 1, 0},
  { 18, 1, 0},
  { 19, 1, 0},
  { 20, 1, 0},
  { 21, 1, 0},
  { 22, 1, 0},
  { 23, 1, 0},
  { 24, 1, 0},
  { 25, 1, 0},
  { 26, 1, 0},
  { 27, 1, 0},
  { 28, 1, 0},
  { 29, 1, 0},
  { 30, 1, 0},
  { 31, 1, 0},
  { 32, 1, 0},
  { 33, 1, 0},
  { 34, 1, 0},
  { 35, 1, 0},
  { 36, 1, 0},
  { 37, 1, 0},
  { 38, 1, 0},
  { 39, 1, 0},
  { 40, 1, 0},
  { 41, 1, 0},
  { 42, 1, 0},
  { 43, 1, 0},
  { 44, 1, 0},
  { 45, 1, 0},
  { 46, 1, 0},
  { 47, 1, 0},
  { 48, 1, 0},
  { 49, 1, 0},
  { 50, 1, 0},
  { 51, 1, 0},
  { 52, 1, 0},
  { 53, 1, 0},
  { 54, 1, 0},
  { 55, 1, 0},
  { 56, 1, 0},
  { 57, 1, 0},
  { 58, 1, 0},
  { 59, 1, 0},
  { 60, 1, 0},
  { 61, 1, 0},
  { 62, 1, 0},
  { 63, 1, 0},
  { 64, 1, 0},
  { 65, 1, 0},
  { 66, 1, 0},
  { 67, 1, 0},
  { 68, 1, 0},
  { 69, 1, 0},
  { 70, 1, 0},
  { 71, 1, 0},
  { 72, 1, 0},
  { 73, 1, 0},
  { 74, 1, 0},
  { 75, 1, 0},
  { 76, 1, 0},
  { 77, 1, 0},
  { 78, 1, 0},
  { 79, 1, 0},
  { 80, 1, 0},
  { 81, 1, 0},
  { 82, 1, 0},
  { 83, 1, 0},
  { 84, 1, 0},
  { 85, 1, 0},
  { 86, 1, 0},
  { 87, 1, 0},
  { 88, 1, 0},
  { 89, 1, 0},
  { 90, 1, 0},
  { 91, 1, 0},
  { 92, 1, 0},
  { 93, 1, 0},
  { 94, 1, 0},
  { 95, 1, 0},
  { 96, 1, 0},
  { 97, 1, 0},
  { 98, 1, 0},
  { 99, 1, 0},
  {100, 1, 0},
  {101, 1, 0},
  {102, 1, 0},
  {103, 1, 0},
  {104, 1, 0},
  {105, 1, 0},
  {106, 1, 0},
  {107, 1, 0},
  {108, 1, 0},
  {109, 1, 0},
  {110, 1, 0},
  {111, 1, 0},
  {112, 1, 0},
  {113, 1, 0},
  {114, 1, 0},
  {115, 1, 0},
  {116, 1, 0},
  {117, 1, 0},
  {118, 1, 0},
  {119, 1, 0},
  {120, 1, 0},
  {121, 1, 0},
  {122, 1, 0},
  {123, 1, 0},
  {124, 1, 0},
  {125, 1, 0},
};

/******************************************************************************/
/**
 * Plays an in-game message.
 *
 * @param msg_idx Message index
 * @param delay Delay between the message can be repeated.
 * @param queue True will allow the message to queue,
 *     false will play it immediately or never.
 * @return Gives true if the message was prepared to play or queued.
 *     If the message couldn't be played nor queued, returns false.
 */
TbBool output_message(long msg_idx, long delay, TbBool queue)
{
    struct SMessage *smsg;
    long i;
    SYNCDBG(5,"Message %ld, delay %ld, queue %s",msg_idx, delay, queue?"on":"off");
    smsg = &messages[msg_idx];
    if ((long)game.play_gameturn < smsg->end_time)
    {
        SYNCDBG(8,"Delay to turn %ld didn't passed, skipping",(long)smsg->end_time);
        return false;
    }
    if (!speech_sample_playing())
    {
      i = get_phrase_sample(get_phrase_for_message(msg_idx));
      if (i == 0)
      {
          SYNCDBG(8,"No phrase sample, skipping");
          return false;
      }
      if (play_speech_sample(i))
      {
          message_playing = msg_idx;
          smsg->end_time = game.play_gameturn + delay;
          SYNCDBG(8,"Playing prepared");
          return true;
      }
    }
    if ((queue) && (msg_idx != message_playing) && (!message_already_in_queue(msg_idx)))
    {
      if (add_message_to_queue(msg_idx, delay))
      {
          SYNCDBG(8,"Playing queued");
          return true;
      }
    }
    WARNDBG(8,"Playing message %ld failed",msg_idx);
    return false;
}

void process_messages(void)
{
  SYNCDBG(17,"Starting");
  _DK_process_messages();
  SYNCDBG(19,"Finished");
}

TbBool message_already_in_queue(long msg_idx)
{
  struct MessageQueueEntry *mqentry;
  long i;
  for (i=0; i < MESSAGE_QUEUE_COUNT; i++)
  {
    mqentry = &message_queue[i];
    if ((mqentry->state == 1) && (msg_idx == mqentry->msg_idx))
      return true;
  }
  return false;
}

TbBool add_message_to_queue(long msg_idx, long delay)
{
  struct MessageQueueEntry *mqentry;
  long i;
  for (i=0; i < MESSAGE_QUEUE_COUNT; i++)
  {
    mqentry = &message_queue[i];
    if (mqentry->state == 0)
    {
      mqentry->state = 1;
      mqentry->msg_idx = msg_idx;
      mqentry->delay = delay;
      return true;
    }
  }
  return false;
}

long get_phrase_for_message(long msg_idx)
{
  struct SMessage *smsg;
  long i;
  smsg = &messages[msg_idx];
  i = UNSYNC_RANDOM(smsg->count);
  return smsg->start_idx + i;
}

/** Returns speech sample for given phrase index.
 *
 * @param phr_idx
 * @return The speech sample number; on error, trturns 0;
 */
long get_phrase_sample(long phr_idx)
{
    if ((phr_idx < 0) || (phr_idx >= sizeof(phrases)/sizeof(phrases[0])))
        phr_idx = 0;
    return phrases[phr_idx];
}

void clear_messages(void)
{
  int i;
  for (i=0; i < MESSAGE_QUEUE_COUNT; i++)
  {
    memset(&message_queue[i], 0, sizeof(struct MessageQueueEntry));
  }
  // Set end turn to 0 for all messages
  for (i=0; i < sizeof(messages)/sizeof(messages[0]); i++)
  {
      messages[i].end_time = 0;
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
