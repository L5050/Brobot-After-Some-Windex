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

npc_rpg_data npcDataTable[NPC_TABLE_MAX];

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

  NPCTribeAnimDef animsOcta2[] = {
    {0, "S_1"},
    {1, "W_1"},
    {2, "R_1"},
    {3, "T_1"},
    {4, "D_1"},
    {7, "D_1"},
    {6, "D_1"},
    {10, "Z_1"},
    {11, "Z_1"},
    {14, "N_1"},
    {-1, nullptr}
  };

  static inline s32 getDataTableIndex(s32 tribeId)
  {
    for (s32 i = 0; i < NPC_TABLE_MAX; i++)
    {
      if (npcDataTable[i].tribeId == tribeId) {
        return i;
      }
    }
    wii::os::OSReport("Invalid TribeId Loaded, loading goomba instead\n");
    return 0;
  }

  s32 get_rpg_enemy_info(spm::evtmgr::EvtEntry * evtEntry, bool firstRun)
  {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    s32 tribeId = getRpgTribeID(args[0]);

    if (!IsNpcActive(args[0])) {
      s32 index = args[0];
      //debug only wii::os::OSReport("NPC is inactive, killing NPC\n");
      spm::an2_08::an2_08_wp.rpgNpcInfo[index].maxHp = -1;
      spm::an2_08::an2_08_wp.rpgNpcInfo[index].killXp = 0;
      spm::evtmgr_cmd::evtSetValue(evtEntry, args[1], 1);
      spm::an2_08::evt_rpg_enemy_take_damage(evtEntry, firstRun);
      spm::evtmgr_cmd::evtSetValue(evtEntry, args[1], -1);
      spm::evtmgr_cmd::evtSetValue(evtEntry, args[2], -1);
      return 2;
    }
    s32 index = getDataTableIndex(tribeId);
    spm::evtmgr_cmd::evtSetValue(evtEntry, args[1], npcDataTable[index].animDefs);
    spm::evtmgr_cmd::evtSetValue(evtEntry, args[2], npcTribes[tribeId].animPoseName);

    if (firstRun == false) {}
    return 2;
  }

  void npc_rpgdrv_main()
  {
    npcDataTable[0] = {0, 2, animsKuribo, nullptr, nullptr, nullptr};
    npcDataTable[1] = {125, 250, animsOcta2, nullptr, nullptr, nullptr};
  }

}
