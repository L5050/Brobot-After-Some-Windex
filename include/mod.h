#pragma once

#include <common.h>
#include "evt_cmd.h"
#include "spm/evtmgr.h"
#include <wii/tpl.h>
namespace mod {

#define MOD_VERSION "SPM-RPG-Battles"

extern "C" {

EVT_DECLARE(evt_memcard_save)

}

extern s32 fp;
extern bool gIsDolphin;
extern bool gIsRiivolution;
extern bool gIsPatchedDisc;
extern bool gIs4_3;
extern bool succeededActionCommand;
extern bool superGuard;

s32 getRpgTribeID(s32 index);
bool IsNpcActive(s32 index);
spm::evtmgr::EvtScriptCode* getInstructionEvtArg(spm::evtmgr::EvtScriptCode* script, s32 line, int instruction);
EVT_DECLARE_USER_FUNC(compareStrings, 3)
EVT_DECLARE_USER_FUNC(mario_set_scale, 3)
EVT_DECLARE_USER_FUNC(mario_chg_paper, 1)

void patchTpl(u32 destId, u32 srcId, wii::tpl::TPLHeader *destTpl, wii::tpl::TPLHeader *srcTpl, const char *filePath = nullptr, bool free = false);
void main();

}
