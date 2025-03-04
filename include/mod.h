#pragma once

#include <common.h>
#include "evt_cmd.h"

namespace mod {

#define MOD_VERSION "SPM-RPG-Battles"

extern s32 fp;
extern bool gIsDolphin;
extern bool gIsRiivolution;
extern bool gIsPatchedDisc;
extern bool gIs4_3;
extern bool succeededActionCommand;
extern bool superGuard;

s32 getRpgTribeID(s32 index);
bool IsNpcActive(s32 index);

void main();

}
