#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "main_scripting.h"
#include "npc_rpgdrv.h"

#include <spm/rel/an.h>
#include <spm/evtmgr.h>
#include <spm/evt_ac.h>
#include <spm/evt_msg.h>
#include <spm/evt_mario.h>
#include <spm/evt_pouch.h>
#include <spm/evt_fade.h>
#include <spm/evt_map.h>
#include <spm/evt_paper.h>
#include <spm/evt_img.h>
#include <spm/evt_env.h>
#include <spm/evt_eff.h>
#include <spm/evt_snd.h>
#include <spm/evt_cam.h>
#include <spm/evt_sub.h>
#include <spm/evt_npc.h>
#include <spm/evt_door.h>
#include <spm/evt_case.h>
#include <spm/evt_pouch.h>
#include <spm/evt_seq.h>
#include <spm/rel/an2_08.h>
#include <spm/rel/sp4_13.h>
#include <spm/rel/an.h>
#include <spm/wpadmgr.h>
#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/seq_game.h>
#include <spm/npcdrv.h>
#include <spm/mario.h>
#include <spm/mario_pouch.h>
#include <spm/seqdef.h>
#include <wii/os/OSError.h>
#include <patch.h>
#include <string>

using namespace spm::npcdrv;

namespace mod {


NPCTribeAnimDef animsKuribo[] = {
    {0, "kuribo_S_1"},
    {1, "kuribo_W_1"},
    {2, "kuribo_R_1"},
    {3, "kuribo_T_1"},
    {4, "kuribo_D_1"},
    {7, "kuribo_D_1"},
    {6, "kuribo_D_1"},
    {10, "kuribo_Z_1"},
    {11, "kuribo_Z_1"},
    {14, "kuribo_N_1"},
    {-1, nullptr}
  };


}
