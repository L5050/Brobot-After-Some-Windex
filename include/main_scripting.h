#pragma once
#include "evt_cmd.h"

#include <common.h>

namespace mod {


  EVT_DECLARE_USER_FUNC(npcEntryFromTribeId, 1)
  EVT_DECLARE_USER_FUNC(increaseAttack, 1)
  EVT_DECLARE_USER_FUNC(rpg_npc_setup, 0)
  EVT_DECLARE_USER_FUNC(rpg_off, 0)
  EVT_DECLARE_USER_FUNC(getFP, 1)
  EVT_DECLARE_USER_FUNC(setFP, 1)
  EVT_DECLARE_USER_FUNC(addFP, 1)
  EVT_DECLARE_USER_FUNC(subtractFP, 1)
  EVT_DECLARE_USER_FUNC(osReportLW, 1)
  EVT_DECLARE_USER_FUNC(calc_peach_heal, 1)
  EVT_DECLARE_USER_FUNC(check_pressed_2_ac, 1)
  EVT_DECLARE_USER_FUNC(check_ac_success, 1)
  EVT_DECLARE_USER_FUNC(ac_success_toggle, 0)
  EVT_DECLARE_USER_FUNC(ac_success_reset, 0)
  EVT_DECLARE_USER_FUNC(displayDamage, 4)

  EVT_DECLARE(parentOfBeginRPG)
  EVT_DECLARE(deleteAttackedEnemy)

}
