/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.h
 *     Header file for player_computer.cpp.
 *     Note that this file is a C header, while its code is CPP.
 * @par Purpose:
 *     Computer player definitions and activities.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_COMPUT_H
#define DK_PLYR_COMPUT_H

#include "bflib_basics.h"
#include "globals.h"

#include "config.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define COMPUTER_TASKS_COUNT        100
#define COMPUTER_PROCESSES_COUNT     20
#define COMPUTER_CHECKS_COUNT        15
#define COMPUTER_EVENTS_COUNT        12
#define COMPUTER_PROCESS_LISTS_COUNT 14
#define COMPUTER_TRAP_LOC_COUNT      20

#define COMPUTER_PROCESS_TYPES_COUNT 26
#define COMPUTER_CHECKS_TYPES_COUNT  51
#define COMPUTER_EVENTS_TYPES_COUNT  31
#define COMPUTER_SPARK_POSITIONS_COUNT 64
#define COMPUTER_SOE_GRID_SIZE        8

/** How strong should be the preference to dig glod from treasure room and not other rooms. Originally was 22 subtiles. */
#define TREASURE_ROOM_PREFERENCE_WHILE_DIGGING_GOLD 16

/** How often to check for possible gold veins which could be digged by computer */
#define GOLD_DEMAND_CHECK_INTERVAL 5000
/** How long to wait for diggers to prepare a place for room before dropping the task and assuming it failed */
#define COMPUTER_DIG_ROOM_TIMEOUT 7500
#define COMPUTER_TOOL_DIG_LIMIT 356

enum ComputerTaskTypes {
    CTT_None = 0,
    CTT_DigRoomPassage,
    CTT_DigRoom,
    CTT_CheckRoomDug,
    CTT_PlaceRoom,
    CTT_DigToEntrance,
    CTT_DigToGold,
    CTT_DigToAttack,
    CTT_MagicCallToArms,
    CTT_PickupForAttack,
    CTT_MoveCreatureToRoom, // 10
    CTT_MoveCreatureToPos,
    CTT_MoveCreaturesToDefend,
    CTT_SlapImps,
    CTT_DigToNeutral,
    CTT_MagicSpeedUp,
    CTT_WaitForBridge,
    CTT_AttackMagic,
    CTT_SellTrapsAndDoors,
    CTT_MoveGoldToTreasury,
};

enum TrapDoorSellingCategory {
    TDSC_EndList = 0,
    TDSC_DoorCrate,
    TDSC_TrapCrate,
    TDSC_DoorPlaced,
    TDSC_TrapPlaced,
};

enum GameActionTypes {
    GA_None = 0,
    GA_Unk01,
    GA_UsePwrHandPick,
    GA_UsePwrHandDrop,
    GA_UseMkDigger,
    GA_UseSlap,
    GA_UsePwrSight,
    GA_UsePwrObey,
    GA_UsePwrHealCrtr,
    GA_UsePwrCall2Arms,
    GA_UsePwrCaveIn,
    GA_StopPwrCall2Arms,
    GA_Unk12,
    GA_Unk13,
    GA_MarkDig,
    GA_Unk15,
    GA_PlaceRoom,
    GA_SetTendencies,
    GA_PlaceTrap,
    GA_PlaceDoor,
    GA_UsePwrLightning,
    GA_UsePwrSpeedUp,
    GA_UsePwrArmour,
    GA_UsePwrConceal,
    GA_UsePwrHoldAudnc,
    GA_UsePwrDisease,
    GA_UsePwrChicken,
    GA_Unk27,
    GA_UsePwrSlap,
    GA_SellTrap,
    GA_SellDoor,
};

enum ToolDigFlags {
    ToolDig_BasicOnly = 0x00, /**< Allows to dig only through basic earth slabs. */
    ToolDig_AllowValuable = 0x01, /**< Allows to dig through valuable slabs. */
    ToolDig_AllowLiquidWBridge = 0x02, /**< Allows to dig through liquid slabs, if only player has ability to build bridges through them.
                                            Also allows to dig through valuable slabs(which should be later changed)). */
};

enum CompProcessFlags {
    ComProc_Unkn0001 = 0x0001,
    ComProc_Unkn0002 = 0x0002,
    ComProc_Unkn0004 = 0x0004,
    ComProc_Unkn0008 = 0x0008,
    ComProc_Unkn0010 = 0x0010,
    ComProc_Unkn0020 = 0x0020,
    ComProc_Unkn0040 = 0x0040,
    ComProc_Unkn0080 = 0x0080,
    ComProc_Unkn0100 = 0x0100,
    ComProc_Unkn0200 = 0x0200,
    ComProc_Unkn0400 = 0x0400,
    ComProc_Unkn0800 = 0x0800,
};

enum CompCheckFlags {
    ComChk_Unkn0001 = 0x0001,
    ComChk_Unkn0002 = 0x0002,
    ComChk_Unkn0004 = 0x0004,
    ComChk_Unkn0008 = 0x0008,
    ComChk_Unkn0010 = 0x0010,
    ComChk_Unkn0020 = 0x0020,
    ComChk_Unkn0040 = 0x0040,
    ComChk_Unkn0080 = 0x0080,
    ComChk_Unkn0100 = 0x0100,
    ComChk_Unkn0200 = 0x0200,
    ComChk_Unkn0400 = 0x0400,
    ComChk_Unkn0800 = 0x0800,
};

enum CompTaskFlags {
    ComTsk_Unkn0001 = 0x0001,
    ComTsk_Unkn0002 = 0x0002,
    ComTsk_Unkn0004 = 0x0004,
    ComTsk_Unkn0008 = 0x0008,
    ComTsk_Unkn0010 = 0x0010,
    ComTsk_Unkn0020 = 0x0020,
    ComTsk_Unkn0040 = 0x0040,
    ComTsk_Unkn0080 = 0x0080,
};

enum CompTaskStates {
    CTaskSt_None = 0,
    CTaskSt_Wait, /**< Waiting some game turns before starting a new task. */
    CTaskSt_Select, /**< Choosing a task to be performed. */
    CTaskSt_Perform, /**< Performing the task. */
};

/** Return values for computer task functions. */
enum CompTaskRet {
    CTaskRet_Unk0 = 0,
    CTaskRet_Unk1,
    CTaskRet_Unk2,
    CTaskRet_Unk3,
    CTaskRet_Unk4,
};

enum ItemAvailabilityRet {
    IAvail_Never         = 0,
    IAvail_Now           = 1,
    IAvail_NeedResearch  = 4,
};

//TODO COMPUTER This returns NULL, which is unsafe
#define INVALID_COMPUTER_PLAYER NULL
#define INVALID_COMPUTER_PROCESS NULL
#define INVALID_COMPUTER_TASK &game.computer_task[0]
/******************************************************************************/
#pragma pack(1)

struct Computer2;
struct ComputerProcess;
struct ComputerCheck;
struct ComputerEvent;
struct Event;
struct Thing;
struct ComputerTask;
struct GoldLookup;

typedef unsigned char ComputerType;
typedef unsigned short ComputerTaskType;
typedef char ComputerName[LINEMSG_SIZE];

typedef long (*Comp_Process_Func)(struct Computer2 *, struct ComputerProcess *);
typedef long (*Comp_Check_Func)(struct Computer2 *, struct ComputerCheck *);
typedef long (*Comp_Event_Func)(struct Computer2 *, struct ComputerEvent *,struct Event *);
typedef long (*Comp_EvntTest_Func)(struct Computer2 *, struct ComputerEvent *);
typedef long (*Comp_Task_Func)(struct Computer2 *, struct ComputerTask *);

struct Comp_Check_Func_ListItem {
  const char *name;
  Comp_Check_Func func;
};

struct TaskFunctions {
  const char *name;
  Comp_Task_Func func;
};

struct ComputerProcess { // sizeof = 72
  char *name;
  long priority;
  unsigned long confval_2;
  unsigned long confval_3;
  unsigned long confval_4;
  unsigned long confval_5;
  Comp_Process_Func func_check;
  Comp_Process_Func func_setup;
  Comp_Process_Func func_task;
  Comp_Process_Func func_complete;
  Comp_Process_Func func_pause;
  struct ComputerProcess *parent;
  unsigned long param_1;
  unsigned long param_2;
  unsigned long param_3;
  unsigned long param_4;
  unsigned long param_5;
  unsigned long flags; /**< Values from ComProc_* enumeration. */
};

struct ComputerCheck { // sizeof = 32
  char *name;
  unsigned long flags;
  long turns_interval;
  Comp_Check_Func func;
  long param1;
  long param2;
  long param3;
  long param4;
};

struct ComputerEvent { // sizeof = 44
  char *name;
  unsigned long cetype;
  unsigned long mevent_kind;
  Comp_Event_Func func_event;
  Comp_EvntTest_Func func_test;
  long test_interval;
  struct ComputerProcess *process;
  long param1;
  long param2;
  long param3;
  long last_test_gameturn; // event last checked time
};

struct ValidRooms { // sizeof = 8
  long rkind;
  struct ComputerProcess *process;
};

struct ComputerProcessMnemonic {
  char name[16];
  struct ComputerProcess *process;
};

struct ComputerCheckMnemonic {
  char name[16];
  struct ComputerCheck *check;
};

struct ComputerEventMnemonic {
  char name[16];
  struct ComputerEvent *event;
};

struct ComputerDig { // sizeof = 78
    struct Coord3d pos_E;
    struct Coord3d pos_dest;
    struct Coord3d pos_begin;
    struct Coord3d pos_next;
    long distance;
    unsigned char hug_side;
    unsigned char direction_around;
    unsigned long subfield_2C;
    long subfield_30;
    long subfield_34;
    long subfield_38;
    long subfield_3C;
    long subfield_40;
    long subfield_44;
    long subfield_48;
    long sub4C_stl_x;
    long sub4C_stl_y;
    long calls_count;
    /** Amount of valuable slabs tagged for digging during this dig process. */
    long valuable_slabs_tagged;
};

struct ComputerTask { // sizeof = 148
    unsigned char flags;
    unsigned char field_1;
    unsigned char ttype;
    unsigned char ottype;
    unsigned char rkind;
    unsigned char field_5[5];
    long created_turn;
    union {
        struct ComputerDig dig;
        struct {
            unsigned char field_E[21];
            unsigned char field_23[32];
            unsigned char field_43[6];
            unsigned char field_49[19];
        };
    };
    long lastrun_turn;
    long field_60;
    struct Coord3d pos_64;
    struct Coord3d pos_6A;
    union {
    struct {
        long items_amount;
        short field_74;
        long gold_gain;
        short field_7A;
        long gold_gain_limit;
        long total_money_limit;
        short field_84;
        /* Index of the item currently being checked in list of sellable things. */
        long sell_idx;
        unsigned char field_8A[2];
    } sell_traps_doors;
    struct {
        /* Amount of gold piles/pots to move */
        long items_amount;
        short field_74;
        long gold_gain;
        short field_7A;
        long gold_gain_limit;
        long total_money_limit;
        short field_84;
        long field_86;
        unsigned char field_8A[2];
    } move_gold;
    struct {
        struct Coord3d pos_70;
        struct Coord3d target_pos;
        long repeat_num;
        long long_80;
        short field_84;
        short word_86;
        long field_88;
    } magic_cta;
    struct {
        long splevel;
        short field_74;
        short target_thing_idx;
        short word_78;
        short field_7A;
        long repeat_num;
        long gaction;
        short field_84;
        long pwkind;
        unsigned char field_8A[2];
    } attack_magic;
    struct {
        short room_idx1;
        short word_72;
        short field_74;
        short word_76;
        short word_78;
        short field_7Ac;
        long repeat_num;
        short room_idx2;
        short word_82;
        unsigned char field_84[2];
        struct Coord3d pos_86;
    } move_to_room;
    struct {
        long field_70;
        short field_74;
        struct Coord3d target_pos;
        long repeat_num;
        short word_80;
        short word_82;
        unsigned char field_84[2];
        struct Coord3d pos_86;
    } move_to_defend;
    struct {
        long field_70;
        short field_74;
        short word_76;
        short word_78;
        short field_7Ac;
        long repeat_num;
        short word_80;
        short word_82;
        unsigned char field_84[2];
        struct Coord3d pos_86;
    } move_to_pos;
    struct {
        long field_70;
        short field_74;
        struct Coord3d target_pos;
        long repeat_num;
        short word_80;
        short word_82;
        unsigned char field_84[2];
        long long_86;
        short word_8A;
    } pickup_for_attack;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        long field_7C;
        /** Target room index. */
        short target_room_idx;
        short word_82;
        short word_84;
        short target_plyr_idx;
        long field_88;
    } dig_to_room;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        long field_7C;
        /** Target gold lookup index. */
        short target_lookup_idx;
        short word_82;
        short word_84;
        long long_86;
        short word_8A;
    } dig_to_gold;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        long field_7C;
        unsigned char byte_80;
        unsigned char byte_81;
        short word_82;
        short word_84;
        short target_plyr_idx;
        long field_88;
    } dig_somewhere;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        short width;
        short height;
        long long_80;
        short word_84;
        long area;
        short word_8A;
    } create_room;
    };
    unsigned short field_8C;
    long field_8E;
    unsigned short next_task;
};

struct Comp2_UnkStr1 { // sizeof = 394
    unsigned long field_0;
    short field_4;
    long hate_amount;
    struct Coord3d pos_A[COMPUTER_SPARK_POSITIONS_COUNT];
};

struct Computer2 { // sizeof = 5322
  long task_state;
  unsigned long gameturn_delay;
  unsigned long gameturn_wait;
  unsigned long field_C;
  unsigned long tasks_did;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C; // seems to be signed long
  unsigned long field_20;
  struct Dungeon *dungeon;
  unsigned long model;
  unsigned long field_2C;
  unsigned long max_room_build_tasks;
  unsigned long field_34;
  struct ComputerProcess processes[COMPUTER_PROCESSES_COUNT+1];
  struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
  struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
  struct Comp2_UnkStr1 unkarr_A10[PLAYERS_COUNT];
  unsigned char field_11C2[394];
  struct Coord3d trap_locations[COMPUTER_TRAP_LOC_COUNT];
  /** Stores Sight Of Evil target points data. */
  unsigned long soe_targets[COMPUTER_SOE_GRID_SIZE];
  /* seem unused */
  unsigned char field_13E4[224];
  short ongoing_process;
  short task_idx;
  short held_thing_idx;
};

struct ComputerPlayerConfig {
    int processes_count;
    int checks_count;
    int events_count;
    int computers_count;
};

/**
 * Contains value of hate between players.
 */
struct THate {
    long amount;
    long plyr_idx;
    struct Coord3d *pos_near;
    long distance_near;
};

struct ExpandRooms {
    RoomKind rkind;
    short max_slabs;
};

#pragma pack()
/******************************************************************************/
extern unsigned short computer_types[];
extern struct ValidRooms valid_rooms_to_build[];

extern struct ComputerProcessMnemonic computer_process_config_list[];
extern const struct NamedCommand computer_process_func_type[];
extern Comp_Process_Func computer_process_func_list[];

extern const struct NamedCommand computer_event_func_type[];
extern Comp_Event_Func computer_event_func_list[];

extern const struct NamedCommand computer_event_test_func_type[];
extern Comp_EvntTest_Func computer_event_test_func_list[];

extern const struct NamedCommand computer_check_func_type[];
extern Comp_Check_Func computer_check_func_list[];
/******************************************************************************/
struct Computer2 *get_computer_player_f(long plyr_idx,const char *func_name);
#define get_computer_player(plyr_idx) get_computer_player_f(plyr_idx,__func__)
TbBool computer_player_invalid(const struct Computer2 *comp);
/******************************************************************************/
void shut_down_process(struct Computer2 *comp, struct ComputerProcess *cproc);
void reset_process(struct Computer2 *comp, struct ComputerProcess *cproc);
void suspend_process(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_process_index(const struct Computer2 *comp, const struct ComputerProcess *cproc);
struct ComputerProcess *get_computer_process(struct Computer2 *comp, int cproc_idx);
/******************************************************************************/
struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long a5);
TbBool is_there_an_attack_task(struct Computer2 *comp);
struct ComputerTask * able_to_build_room(struct Computer2 *comp, struct Coord3d *pos, RoomKind rkind,
    long width_slabs, long height_slabs, long a6, long a7);
long computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlookref);
void setup_dig_to(struct ComputerDig *cdig, const struct Coord3d startpos, const struct Coord3d endpos);
long move_imp_to_dig_here(struct Computer2 *comp, struct Coord3d *pos, long max_amount);
void get_opponent(struct Computer2 *comp, struct THate hate[]);
long add_to_trap_location(struct Computer2 *, struct Coord3d *);
/******************************************************************************/
long set_next_process(struct Computer2 *comp);
void computer_check_events(struct Computer2 *comp);
TbBool process_checks(struct Computer2 *comp);
GoldAmount get_computer_money_less_cost(const struct Computer2 *comp);
TbBool creature_could_be_placed_in_better_room(const struct Computer2 *comp, const struct Thing *thing);
struct Room *get_room_to_place_creature(const struct Computer2 *comp, const struct Thing *thing);
long xy_walkable(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx);
/******************************************************************************/
struct ComputerTask *get_computer_task(long idx);
struct ComputerTask *get_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype);
struct ComputerTask *get_task_in_progress_in_list(struct Computer2 *comp, const ComputerTaskType *ttypes);
TbBool is_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype);
struct ComputerTask *get_free_task(struct Computer2 *comp, long a2);
TbBool computer_task_invalid(const struct ComputerTask *ctask);
TbBool remove_task(struct Computer2 *comp, struct ComputerTask *ctask);
const char *computer_task_code_name(int ctask_type);

TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num, unsigned long evflags);
TbBool create_task_move_creatures_to_room(struct Computer2 *comp, int room_idx, long creatrs_num);
TbBool create_task_magic_battle_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long par2, long creatrs_num);
TbBool create_task_magic_support_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long par2, long par3, long creatrs_num);
TbBool create_task_pickup_for_attack(struct Computer2 *comp, struct Coord3d *pos, long par3, long creatrs_num);
TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long num_to_sell, long gold_up_to);
TbBool create_task_move_gold_to_treasury(struct Computer2 *comp, long num_to_move, long gold_up_to);
TbBool create_task_move_creature_to_subtile(struct Computer2 *comp, const struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool create_task_move_creature_to_pos(struct Computer2 *comp, const struct Thing *thing, const struct Coord3d pos);
TbBool create_task_dig_to_attack(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, PlayerNumber victim_plyr_idx, long parent_cproc_idx);
TbBool create_task_slap_imps(struct Computer2 *comp, long creatrs_num);
TbBool create_task_dig_to_neutral(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos);
TbBool create_task_dig_to_gold(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long par1, long gold_lookup_idx);
TbBool create_task_dig_to_entrance(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long entroom_idx);
TbBool create_task_magic_speed_up(struct Computer2 *comp, const struct Thing *creatng, long splevel);
TbBool create_task_attack_magic(struct Computer2 *comp, const struct Thing *creatng, PowerKind pwkind, int repeat_num, int splevel, int gaction);

long computer_able_to_use_magic(struct Computer2 *comp, PowerKind pwkind, long a3, long a4);
long computer_get_room_kind_total_capacity(struct Computer2 *comp, RoomKind room_kind);
long computer_get_room_kind_free_capacity(struct Computer2 *comp, RoomKind room_kind);
long computer_finds_nearest_room_to_pos(struct Computer2 *comp, struct Room **retroom, struct Coord3d *nearpos);
long process_tasks(struct Computer2 *comp);
TbResult game_action(PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
TbResult try_game_action(struct Computer2 *comp, PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
short tool_dig_to_pos2_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags, const char *func_name);
#define tool_dig_to_pos2(comp,cdig,simulation,digflags) tool_dig_to_pos2_f(comp,cdig,simulation,digflags,__func__)
#define search_spiral(pos, owner, i3, cb) search_spiral_f(pos, owner, i3, cb, __func__)
int search_spiral_f(struct Coord3d *pos, PlayerNumber owner, int i3, long (*cb)(MapSubtlCoord, MapSubtlCoord, long), const char *func_name);
/******************************************************************************/
ItemAvailability computer_check_room_available(struct Computer2 * comp, long rkind);
long computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos);

long count_creatures_in_dungeon(const struct Dungeon *dungeon);
long count_entrances(const struct Computer2 *comp, PlayerNumber plyr_idx);
long count_diggers_in_dungeon(const struct Dungeon *dungeon);
long check_call_to_arms(struct Computer2 *comp);
long count_creatures_for_defend_pickup(struct Computer2 *comp);
long count_creatures_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4);
long count_creatures_availiable_for_fight(struct Computer2 *comp, struct Coord3d *pos);
/******************************************************************************/
long setup_computer_attack(struct Computer2 *comp, struct ComputerProcess *cproc, struct Coord3d *pos, long victim_plyr_idx);
/******************************************************************************/
TbBool setup_a_computer_player(PlayerNumber plyr_idx, long comp_model);
void process_computer_players2(void);
void setup_computer_players2(void);
void restore_computer_player_after_load(void);
TbBool computer_force_dump_held_things_on_map(struct Computer2 *comp, const struct Coord3d *pos);
TbBool computer_force_dump_specific_held_thing(struct Computer2 *comp, struct Thing *thing, const struct Coord3d *pos);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
