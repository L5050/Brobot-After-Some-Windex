#include "mod.h"
#include "patch.h"
#include "evtpatch.h"

#include <spm/camdrv.h>
#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/seqdef.h>
#include <spm/mario.h>
#include <spm/evt_mario.h>
#include <spm/evtmgr.h>
#include <spm/npcdrv.h>
#include <spm/npc_dimeen_l.h>
#include <spm/map_data.h>
#include <spm/eff_zunbaba.h>
#include <spm/evt_eff.h>
#include <spm/evt_seq.h>
#include <spm/evt_npc.h>
#include <spm/evt_snd.h>
#include <spm/wpadmgr.h>
#include <wii/os/OSError.h>
#include <wii/gx.h>

namespace mod {
/*
    Title Screen Custom Text
    Prints "SPM Rel Loader" at the top of the title screen
*/

static spm::seqdef::SeqFunc *seq_titleMainReal;
static void seq_titleMainOverride(spm::seqdrv::SeqWork *wp)
{
    wii::gx::GXColor green = {0, 255, 0, 255};
    f32 scale = 0.8f;
    const char * msg = "SPM-Bringle-IDK";
    spm::fontmgr::FontDrawStart();
    spm::fontmgr::FontDrawEdge();
    spm::fontmgr::FontDrawColor(&green);
    spm::fontmgr::FontDrawScale(scale);
    spm::fontmgr::FontDrawNoiseOff();
    spm::fontmgr::FontDrawRainbowColorOff();
    f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
    spm::fontmgr::FontDrawString(x, 200.0f, msg);
    seq_titleMainReal(wp);
}
static void titleScreenCustomTextPatch()
{
    seq_titleMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main;
    spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main = &seq_titleMainOverride;
}

// Patches Dimentio to have a dynamic movement zone rather than being hardcoded for one room. thanks yme!
s32 dimen_determine_move_pos_new(spm::evtmgr::EvtEntry *entry, bool isFirstCall)
{
    spm::mario::MarioWork *marioWork = spm::mario::marioGetPtr();
    spm::npcdrv::NPCEntry *npc = entry->ownerNPC;
    double destYPos = 0;
    f32 marioZ = ((marioWork->position).z);
    f32 destXPos = 0;
    u32 dimenMoveRand = 0;
    wii::mtx::Vec3 min;
    wii::mtx::Vec3 max;
    spm::hitdrv::hitGetMapEntryBbox(0, &min, &max);
    s32 i = 0;
    do
    {
        while (true)
        {
            do
            {
                i = i + 1;
                dimenMoveRand = spm::system::irand(400);
                destXPos = ((marioWork->position).x + (f32)dimenMoveRand - 200);
                if (i > 50)
                {
                    destXPos = npc->position.x;
                    goto outOfBounds;
                }
            } while ((destXPos <= (min.x + 25)) || ((max.x - 25) <= destXPos));
        outOfBounds:
            u32 yMoveBehavior = spm::system::irand(100);
            if (yMoveBehavior < 67)
            {
                dimenMoveRand = spm::system::irand(4);
                destYPos = (10.0 * (f32)dimenMoveRand + 20.0);
            }
            else
            {
                dimenMoveRand = spm::system::irand(3);
                destYPos = (32.0 * (f32)dimenMoveRand + 40.0);
            }
            if (npc->flippedTo3d != 0)
                break;
            if ((100.0 < __builtin_abs((destXPos - (marioWork->position).x))) || (80.0 < destYPos))
                goto setFloats;
        }
        destYPos = spm::system::distABf(destXPos, marioZ, ((marioWork->position).x), marioZ);
    } while ((destYPos <= 120.0) && (destYPos <= 80.0));
setFloats:
    spm::evtmgr::EvtVar *args = (spm::evtmgr::EvtVar *)entry->pCurData;
    spm::evtmgr_cmd::evtSetFloat(entry, args[0], destXPos);
    spm::evtmgr_cmd::evtSetFloat(entry, args[1], destYPos + (marioWork->position).y);
    spm::evtmgr_cmd::evtSetFloat(entry, args[2], marioZ);
    return 2;
}

s32 check_pressed_b_brognle(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    u32 pressed = spm::wpadmgr::wpadGetButtonsPressed(0);
    if (pressed & 0x400) {
      wii::os::OSReport("pressed B!\n");
      return 2;
    }
    return 0;
  }

  s32 patchAsserts(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    writeWord(spm::evtmgr::evtEntryType, 0x80, 0x60000000);
    writeWord(spm::evtmgr::evtEntry, 0x7C, 0x60000000);
    writeWord(spm::evtmgr::evtChildEntry, 0x7C, 0x60000000);
    writeWord(spm::evtmgr::evtBrotherEntry, 0x7C, 0x60000000);
    return 2;
  }

  EVT_DECLARE_USER_FUNC(check_pressed_b_brognle, 0)
  EVT_DECLARE_USER_FUNC(patchAsserts, 0)
  
    EVT_BEGIN(hampter2)
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 426)
    USER_FUNC(check_pressed_b_brognle)
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    SET(LW(10), 1)
    DO(0)
    LBL(25)
    USER_FUNC(spm::evt_npc::evt_npc_spawn_sammer_guy, 0, LW(10), LW(0), LW(1), LW(2), 0)
    ADD(LW(10), 1)
    IF_LARGE_EQUAL(LW(10), 26)
        DO_BREAK()
    END_IF()
    WAIT_FRM(30)
    GOTO(25)
    WHILE()
    USER_FUNC(check_pressed_b_brognle)
    DO(0)
    LBL(50)
    USER_FUNC(spm::evt_npc::evt_npc_spawn_sammer_guy, 0, LW(10), LW(0), LW(1), LW(2), 0)
    ADD(LW(10), 1)
    IF_LARGE_EQUAL(LW(10), 51)
        DO_BREAK()
    END_IF()
    WAIT_FRM(30)
    GOTO(50)
    WHILE()
    USER_FUNC(check_pressed_b_brognle)
    DO(0)
    LBL(75)
    USER_FUNC(spm::evt_npc::evt_npc_spawn_sammer_guy, 0, LW(10), LW(0), LW(1), LW(2), 0)
    ADD(LW(10), 1)
    IF_LARGE_EQUAL(LW(10), 76)
        DO_BREAK()
    END_IF()
    WAIT_FRM(30)
    GOTO(75)
    WHILE()
    USER_FUNC(check_pressed_b_brognle)
    DO(0)
    LBL(100)
    USER_FUNC(spm::evt_npc::evt_npc_spawn_sammer_guy, 0, LW(10), LW(0), LW(1), LW(2), 0)
    ADD(LW(10), 1)
    IF_LARGE_EQUAL(LW(10), 101)
        DO_BREAK()
    END_IF()
    WAIT_FRM(30)
    GOTO(100)
    WHILE()
    USER_FUNC(check_pressed_b_brognle)
    USER_FUNC(spm::evt_snd::evt_snd_bgmon, 0, PTR("BGM_BTL_BOSS_STG1"))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(patchAsserts)
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 44)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 44, LW(0), LW(1), LW(2), LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
LBL(1)
    USER_FUNC(check_pressed_b_brognle)
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 189)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 189, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 150)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 150 , 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))/*
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 132)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 132, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))*/
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 142)
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 142, 0, LW(1), 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 226)
USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 226, 0, LW(1), 0, LW(10), EVT_NULLPTR)
USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))/*
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 134)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 134, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 137)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 137, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))*/
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 182)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 182, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 174) // o chunk veg
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 174, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 187)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 187, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 213)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 213, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 183)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 183, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 426)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 426, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))/*
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 255)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 255, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 287)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 287, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))*/
    USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 286)
    USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 286, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
    USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_tribe_agb_async, 422)
USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, 422, 0, -100, 0, LW(10), EVT_NULLPTR)
USER_FUNC(spm::evt_npc::evt_npc_set_anim, LW(10), 0, 1)
USER_FUNC(spm::evt_npc::func_80107c38, LW(10), 0)
USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
USER_FUNC(spm::evt_npc::evt_npc_flip_to, LW(10), 1)
USER_FUNC(spm::evt_npc::evt_npc_finish_flip_instant, LW(10))
USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
ADD(LW(1), FLOAT(20.0))
USER_FUNC(spm::evt_npc::evt_npc_set_position, LW(10), FLOAT(0.0), LW(1), FLOAT(0.0))
USER_FUNC(spm::evt_npc::func_800ff8f8, LW(10), LW(11), LW(12), LW(13))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
USER_FUNC(spm::evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
USER_FUNC(spm::evt_npc::evt_npc_flip, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_wait_flip_finished, LW(10))
USER_FUNC(spm::evt_npc::evt_npc_restart_evt_id, LW(10))
    RETURN()
    EVT_END()

    EVT_BEGIN(hampter)
    RUN_EVT(hampter2)
    RETURN()
    EVT_END()

    static s32 chunkFartId;

    s32 grabChonkyFarts(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar *args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    chunkFartId = spm::evtmgr_cmd::evtGetValue(evtEntry, args[0]);
    return 2;
    }

    s32 returnChonkyFarts(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar *args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    chunkFartId = spm::evtmgr_cmd::evtSetValue(evtEntry, args[0], chunkFartId);
    return 2;
    }

    s32 evt_npc_halt(spm::evtmgr::EvtEntry * evtEntry, bool firstRun)
    {
        return 0;
    }

    EVT_DECLARE_USER_FUNC(grabChonkyFarts, 1)
    EVT_DECLARE_USER_FUNC(returnChonkyFarts, 1)
    EVT_DECLARE_USER_FUNC(evt_npc_halt, 0)

    EVT_BEGIN(returnChunksDeathScript)
        USER_FUNC(grabChonkyFarts, LW(10))
    RETURN_FROM_CALL()

    EVT_BEGIN(hookChunksDeathScript)
        USER_FUNC(returnChonkyFarts, LW(10))
        DELETE_EVT(LW(10))
        USER_FUNC(spm::evt_npc::evt_npc_set_move_mode, PTR("me"), 0)
        WAIT_MSEC(1000)
        USER_FUNC(spm::evt_npc::evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
        USER_FUNC(spm::evt_snd::evt_snd_sfxon, PTR("SFX_E_ENEMY_DIE1"))
        USER_FUNC(spm::evt_npc::evt_npc_animflag_onoff, PTR("me"), 0, 128)
        USER_FUNC(spm::evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_position, PTR("me"), FLOAT(0.0), FLOAT(500.0), FLOAT(0.0))
        USER_FUNC(evt_npc_halt)
    RETURN()
    EVT_END()

    EVT_BEGIN(dimentioDeathScript)
        RUN_EVT(spm::npcdrv::npcEnemyTemplates[142].unkScript3)
        WAIT_MSEC(1200)
        USER_FUNC(spm::evt_npc::evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
        USER_FUNC(spm::evt_snd::evt_snd_sfxon, PTR("SFX_E_ENEMY_DIE1"))
        USER_FUNC(spm::evt_npc::evt_npc_animflag_onoff, PTR("me"), 0, 128)
        USER_FUNC(spm::evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_move_mode, PTR("me"), 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_position, PTR("me"), FLOAT(0.0), FLOAT(500.0), FLOAT(-5000.0))
        USER_FUNC(evt_npc_halt)
    RETURN()
    EVT_END()

    EVT_BEGIN(longLiveTheKing)
        USER_FUNC(spm::evt_npc::evt_npc_delete, PTR("me"))
        USER_FUNC(evt_npc_halt)
    RETURN()
    EVT_END()

    EVT_BEGIN(otherDeathScript)
        WAIT_MSEC(1200)
        USER_FUNC(spm::evt_npc::evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
        USER_FUNC(spm::evt_snd::evt_snd_sfxon, PTR("SFX_E_ENEMY_DIE1"))
        USER_FUNC(spm::evt_npc::evt_npc_animflag_onoff, PTR("me"), 0, 128)
        USER_FUNC(spm::evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_move_mode, PTR("me"), 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_position, PTR("me"), FLOAT(0.0), FLOAT(500.0), FLOAT(-5000.0))
        USER_FUNC(evt_npc_halt)
    RETURN()
    EVT_END()

    EVT_BEGIN(bonechillSucks)
        USER_FUNC(spm::evt_npc::evt_npc_set_unitwork, PTR("me"), 14, PTR(otherDeathScript))
    RETURN_FROM_CALL()

    EVT_BEGIN(dimentioPatch)
        USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(5), LW(6), LW(7))
        ADD(LW(1), LW(6))
    RETURN_FROM_CALL()

    spm::npcdrv::NPCEntry * dimi_npcEntryFromSetupEnemy(s32 setupFileIndex, wii::mtx::Vec3 * pos, s32 nTemplateNo, spm::npcdrv::MiscSetupDataV6 * miscSetupData)
    {
        spm::mario::MarioWork *marioWork = spm::mario::marioGetPtr();
        pos->y += marioWork->position.y;
        return spm::npcdrv::npcEntryFromSetupEnemy(setupFileIndex, pos, nTemplateNo, miscSetupData);
    }

    EVT_BEGIN(hampter_cutscene)
        USER_FUNC(spm::evt_seq::evt_seq_set_seq, spm::seqdrv::SEQ_MAPCHANGE, PTR("mi3_03"), PTR("doa_l"))
    RETURN()
    EVT_END()
    
    EVT_BEGIN(dumbo)
        SET(LW(1), LW(1))
    RETURN_FROM_CALL()

    EVT_BEGIN(dumbo2)
        USER_FUNC(spm::evt_npc::evt_npc_set_property, PTR("me"), 6, PTR(otherDeathScript))
    RETURN_FROM_CALL()

/*
    General mod functions
*/
void bringle_main()
{
    wii::os::OSReport("SPM Rel Loader: the mod has ran!\n");
    titleScreenCustomTextPatch();
    //evtpatch::evtmgrExtensionInit(); // Initialize EVT scripting extension
    patch::hookFunction(spm::evt_npc::evt_npc_dimen_determine_move_pos, dimen_determine_move_pos_new);
    evtpatch::hookEvt(spm::npcdrv::npcEnemyTemplates[183].onSpawnScript, 85, (spm::evtmgr::EvtScriptCode*)returnChunksDeathScript);
    //evtpatch::hookEvt(spm::npcdrv::npcEnemyTemplates[183].unkScript6, 1, (spm::evtmgr::EvtScriptCode*)hookChunksDeathScript); //Fix for if O'Chunks is killed outside of boss rooms
    spm::npcdrv::npcEnemyTemplates[174].unkScript6 = hookChunksDeathScript;
    spm::npcdrv::npcEnemyTemplates[182].unkScript6 = hookChunksDeathScript;
    spm::npcdrv::npcEnemyTemplates[183].unkScript6 = hookChunksDeathScript;
    spm::npcdrv::npcEnemyTemplates[226].unkScript6 = dimentioDeathScript;
    spm::npcdrv::npcEnemyTemplates[142].unkScript6 = dimentioDeathScript;
    //spm::npcdrv::npcEnemyTemplates[150].unkScript6 = otherDeathScript;

    spm::evtmgr::EvtScriptCode* mainLogicCroacus = getInstructionEvtArg(spm::npcdrv::npcEnemyTemplates[150].unkScript3, 11, 0);
    evtpatch::hookEvtReplace(mainLogicCroacus, 197, (spm::evtmgr::EvtScriptCode*)otherDeathScript);

    #ifdef SPM_EU0
    evtpatch::hookEvtReplace(spm::npcdrv::npcEnemyTemplates[44].unkScript3, 77, (spm::evtmgr::EvtScriptCode*)hampter_cutscene);
    #else
    evtpatch::hookEvtReplace(spm::npcdrv::npcEnemyTemplates[44].unkScript3, 81, (spm::evtmgr::EvtScriptCode*)hampter_cutscene);
    #endif

    //evtpatch::hookEvtReplace(spm::eff_zunbaba::wracktail_defeat, 214, (spm::evtmgr::EvtScriptCode*)hampter_cutscene);

    spm::map_data::MapData * mapData = spm::map_data::mapDataPtr("he1_04");
    evtpatch::hookEvt(mapData->initScript, 42, (spm::evtmgr::EvtScriptCode*)hampter);
    writeBranchLink( & spm::npc_dimeen_l::func_801e5fd0, 0x7D4, dimi_npcEntryFromSetupEnemy);
    writeBranchLink( & spm::npc_dimeen_l::func_801e5fd0, 0x7B8, dimi_npcEntryFromSetupEnemy);
    writeBranchLink( & spm::npc_dimeen_l::func_801e5fd0, 0x7F0, dimi_npcEntryFromSetupEnemy);
    spm::evtmgr::EvtScriptCode* dimentioOnSpawn = spm::npcdrv::npcEnemyTemplates[142].onSpawnScript;
    spm::evtmgr::EvtScriptCode* mainLogic = getInstructionEvtArg(dimentioOnSpawn, 5, 3);
    spm::evtmgr::EvtScriptCode* clones = getInstructionEvtArg(mainLogic, 58, 0);
    evtpatch::hookEvt(clones, 43, (spm::evtmgr::EvtScriptCode*)dimentioPatch);
    evtpatch::hookEvt(spm::npcdrv::npcEnemyTemplates[189].unkScript6, 4, (spm::evtmgr::EvtScriptCode*)bonechillSucks);
//other functions
}

}
