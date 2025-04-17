#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "evtpatch.h"
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
#include <spm/map_data.h>
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
using namespace spm::evt_npc;

namespace mod {

NPCTribeAnimDef animsDoopliss[] = {
    {0, "S_1"},
    {1, "W_1"},
    {2, "R_1"},
    {3, "T_1"},
    {4, "D_1"},
    {6, "A_1A"},
    {7, "A_1B"},
    {8, "A_2"},
    {11, "A_3"},
    {-1, nullptr}
  };

  NPCTribeAnimDef animsWhipGuy[] = {
    {0, "n_stg2_kansyuS_1"},
    {1, "n_stg2_kansyuW_1"},
    {2, "n_stg2_kansyuR_1"},
    {3, "n_stg2_kansyuT_1"},
    {-1, nullptr}
  };

  NPCTribeAnimDef * getDooplissAnims()
  {
    return animsDoopliss;
  }

  EVT_BEGIN(doopliss_cutscene)
    RUN_CHILD_EVT(spm::evt_door::door_init_evt)
    USER_FUNC(evt_npc_entry, PTR("kansyu"), PTR("n_stg2_kansyu"), 0)
    USER_FUNC(evt_npc_set_position, PTR("kansyu"), 450, 0, 0)
    USER_FUNC(evt_npc_set_property, PTR("kansyu"), 14, (s32)animsWhipGuy)
    USER_FUNC(evt_npc_set_anim, PTR("kansyu"), 0, 1)
    USER_FUNC(spm::evt_npc::evt_npc_entry, PTR("doopliss"), PTR("c_ranpel"), 0)
    USER_FUNC(spm::evt_npc::evt_npc_set_position, PTR("doopliss"), 0, -100, 0)
  RETURN()
  EVT_END()

  void doopliss_main()
  {
    npcTribes[529].animDefs = animsDoopliss;
    npcTribes[529].animPoseName = "c_ranpel";
    npcTribes[529].maxHp = 25;
    npcTribes[529].killXp = 3500;
    spm::map_data::MapData * mi3_03_md = spm::map_data::mapDataPtr("mi3_03");
    evtpatch::hookEvtReplace(mi3_03_md->initScript, 32, (spm::evtmgr::EvtScriptCode*)doopliss_cutscene);
  }

}
