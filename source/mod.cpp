#include "mod.h"
#include "patch.h"
#include "scripting.cpp"

#include <spm/system.h>
#include <spm/acdrv.h>
#include <spm/camdrv.h>
#include <spm/spmario_snd.h>
#include <spm/evtmgr.h>
#include <spm/mario.h>
#include <spm/evtmgr_cmd.h>
#include <spm/msgdrv.h>
#include <spm/seq_mapchange.h>
#include <spm/effdrv.h>
#include <spm/eff_nice.h>
#include <spm/dispdrv.h>
#include <spm/animdrv.h>
#include <spm/npcdrv.h>
#include <spm/camdrv.h>
#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/seqdef.h>
#include <wii/os/OSError.h>
#include <wii/gx.h>
#include <wii/mtx.h>
#include <msl/string.h>
#include <spm/rel/an2_08.h>
#include <spm/rel/an.h>
#include <spm/rel/sp4_13.h>
USING(wii::mtx::Vec3)

//credit to rainchus for helping me out with these ASM hooks :)
extern "C" {
  char marioString[] = "Flip";
  char peachString[] = "Heal";
  char bowserString[] = "Flame";
  char luigiString[] = "Super jump";

  char * characterStrings[] = {
    marioString,
    peachString,
    bowserString,
    luigiString
  };

  s32 rpgTribeID[3] = {
    0,
    296,
    0
  };
  s32 test() {
    return 296;
  }

  void setNewFloat();
  asm
  (
    "floatValue:\n"
    ".float 299\n"

    ".global setNewFloat\n"
    "setNewFloat:\n"
        "lis 3, floatValue@ha\n"
        "ori 3, 3, floatValue@l\n"
        "lfs 1, 0x0000 (3)\n"
        "blr\n"
  );

  void getTribe();
  asm
    (
      ".global getTribe\n"
      "getTribe:\n"
      "lis 6, rpgTribeID@h\n"
      "ori 6, 6, rpgTribeID@l\n"
      "slwi 4, 4, 2\n"
      "lwzx 4, 6, 4\n"
      "blr\n"
    );

  void getTribe2();
  asm
    (
      ".global getTribe2\n"
      "getTribe2:\n"
      "lis 7, rpgTribeID@h\n"
      "ori 7, 7, rpgTribeID@l\n"
      "mr 3, 28\n"
      "slwi 3, 3, 2\n"
      "lwzx 3, 7, 3\n"
      "blr\n"
    );

  void chooseNewCharacterString();
  asm
    (
      ".global chooseNewCharacterString\n"
      "chooseNewCharacterString:\n"
      "lis 4, characterStrings@ha\n"
      "ori 4, 4, characterStrings@l\n"
      "lwzx 3, 4, 0\n" //load new string pointer
      "blr\n"
    );

}

namespace mod {
  bool rpgInProgress = false;
  bool bossFight = false;
  bool loadedStage7 = false;
  s32 fp = 0;

  /*
      Title Screen Custom Text
      Prints "Brobot After Some Windex" at the top of the title screen
  */

  static spm::seqdef::SeqFunc * seq_titleMainReal;
  static spm::seqdef::SeqFunc * seq_gameMainReal;
  static void seq_titleMainOverride(spm::seqdrv::SeqWork * wp) {
    wii::gx::GXColor green = {
      0,
      255,
      0,
      255
    };
    f32 scale = 0.8;
    const char * msg = "Brobot After Some Windex";
    spm::fontmgr::FontDrawStart();
    spm::fontmgr::FontDrawEdge();
    spm::fontmgr::FontDrawColor( & green);
    spm::fontmgr::FontDrawScale(scale);
    spm::fontmgr::FontDrawNoiseOff();
    spm::fontmgr::FontDrawRainbowColorOff();
    f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
    spm::fontmgr::FontDrawString(x, 200.0, msg);
    seq_titleMainReal(wp);
  }

  /*void drawStuff(s32 cameraId, void * param) {
    const Vec3 fpVec = {-110.0, -200.0, 0.0};
    spm::icondrv::iconDispGx(1.0, &fpVec, 4, 105);
  }*/

    static void seq_gameMainOverride(spm::seqdrv::SeqWork * wp) {
    if (rpgInProgress == true) {
      wii::gx::GXColor green = {
        0,
        255,
        0,
        255
      };
      f32 scale = 0.8;
      char buffer [50];
      sprintf(buffer, "%d", fp);
      const char * msg = buffer;
      spm::fontmgr::FontDrawStart();
      spm::fontmgr::FontDrawEdge();
      spm::fontmgr::FontDrawColor( & green);
      spm::fontmgr::FontDrawScale(scale);
      spm::fontmgr::FontDrawNoiseOff();
      spm::fontmgr::FontDrawRainbowColor();
      f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
      spm::fontmgr::FontDrawString(x+-50, -161.0, msg);
        //spm::dispdrv::dispEntry(spm::camdrv::CAM_ID_2D, 2, 300.0, drawStuff, nullptr);
      }
      if (rpgInProgress == true) {
        wii::gx::GXColor green = {
          0,
          255,
          0,
          255
        };
        f32 scale = 0.8;
        const char * msg = "FP:";
        spm::fontmgr::FontDrawStart();
        spm::fontmgr::FontDrawEdge();
        spm::fontmgr::FontDrawColor( & green);
        spm::fontmgr::FontDrawScale(scale);
        spm::fontmgr::FontDrawNoiseOff();
        spm::fontmgr::FontDrawRainbowColorOff();
        f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
        spm::fontmgr::FontDrawString(x+-90, -161.0, msg);
        }
      seq_gameMainReal(wp);
    }

  static void titleScreenCustomTextPatch() {
    seq_titleMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main;
    spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main = & seq_titleMainOverride;
    seq_gameMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_GAME].main;
    spm::seqdef::seq_data[spm::seqdrv::SEQ_GAME].main = & seq_gameMainOverride;
  }

  /*
      Various hooks
  */

  spm::evtmgr::EvtEntry * ( * evtEntry1)(const spm::evtmgr::EvtScriptCode * script, u32 priority, u8 flags);
  spm::evtmgr::EvtEntry * ( * evtChildEntry)(spm::evtmgr::EvtEntry * entry,
    const spm::evtmgr::EvtScriptCode * script, u8 flags);
  spm::evtmgr::EvtEntry * ( * evtBrotherEntry)(spm::evtmgr::EvtEntry * brother,
    const spm::evtmgr::EvtScriptCode * script, u8 flags);
  spm::evtmgr::EvtEntry * ( * evtEntryType)(const spm::evtmgr::EvtScriptCode * script, u32 priority, u8 flags, u8 type);
  spm::effdrv::EffEntry * ( * effNiceEntry)(double param_1, double param_2, double param_3, double param_4, int param_5);
  bool( * spsndBGMOn)(u32 flags,
    const char * name);
  s32( * spsndSFXOn)(const char * name);
  s32( * marioCalcDamageToEnemy)(s32 damageType, s32 tribeId);
  s32( * evt_inline_evt)(spm::evtmgr::EvtEntry * entry);
  void( * msgUnLoad)(s32 slot);
  //void ( * dispEntry)(f32 z, s8 cameraId, s8 renderMode, spm::dispdrv::DispCallback * callback, void * callbackParam);
  const char * ( * msgSearch)(const char * msgName);

  const char fileName[] = {
    "stg7"
  };

  /*
      Custom Text
  */


  const char * rpgStart = "Brobot attacks!\n"
  "<o>\n";
  const char * stg7_2_133_2_002 = "<dq>\n"
  "<p>\n"
  "What will you do?\n"
  "<o>\n";
  const char * stg7_2_133_2_003 = "<p>\n"
  "Attack who?\n"
  "<o>\n";
  const char * stg7_2_133_2_004 = "<p>\n"
  "%s attacks!\n"
  "<o>\n";
  const char * stg7_2_133_2_005 = "<p>\n"
  "You deal %d damage to\n"
  "%s!\n"
  "<k>\n"
  "<o>\n";
  const char * stg7_2_133_2_006 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But your attack is dodged!\n"
  "<k>\n"
  "<o>\n";
  const char * stg7_2_133_2_007 = "<p>\n"
  "You defeated\n"
  "%s!\n"
  "<k>\n"
  "<o>\n";
  const char * stg7_2_133_2_008 = "<p>\n"
  "You defeated Brobot!\n"
  "<k>\n"
  "<p>\n"
  "%s receives %d points!\n"
  "<k>\n"
  "<o>\n";
  const char * stg7_2_133_2_009 = "<p>\n"
  "You gained a level!\n"
  "<o>\n";
  const char * stg7_2_133_2_009_01 = "<p>\n"
  "Your attack power increased\n"
  "by %d!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_009_02 = "<p>\n"
  "Your HP increased by %d!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_010 = "<p>\n"
  "Choose a technique.\n"
  "<o>\n";

  const char * stg7_2_133_2_011 = "<p>\n"
  "Mario flips!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_012 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "...You found 2 FP!\n"
  "<k>\n"
  "<o>\n";

  const char * no_fp = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "no FP?\n"
  "*Megamind gif*"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_013 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_014 = "<p>\n"
  "Bowser breathes fire!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_015 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "...But the flames have no effect on %s!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_016 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_017 = "<p>\n"
  "Luigi glares at %s\n"
  "and launches a super jump!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_018 = "<p>\n"
  "You stomp %s for %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_019 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But the attack misses!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_020 = "<p>\n"
  "Select a Pixl.\n"
  "<o>\n";

  const char * stg7_2_133_2_021 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_022 = "<p>\n"
  "%s throws Thoreau at %s!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_023 = "<dkey><wait 500></dkey>\n"
  "<p>\n"
  "Urg... %s is too heavy to lift!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_024 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_025 = "<p>\n"
  "Take that, enemy!\n"
  "<wait 150>%s throws Boomer!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_026 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "Miss! The attack is a failure!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_027 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "Boomer explodes!\n"
  "<wait 150>%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_028 = "<p>\n"
  "%s uses Slim!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_029 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s turns sideways\n"
  "and is hard to see!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_031 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_032 = "<p>\n"
  "%s uses Thudley!\n"
  "A heavy-duty attack!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_033 = "<p>\n"
  "The attack is a success!\n"
  "%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_034 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But the attack is evaded!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_035 = "<p>\n"
  "%s hops onto Carrie!\n"
  "<dkey><wait 500></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_036 = "<p>\n"
  "You feel a little taller!\n"
  "<wait 450>...But that is the only\n"
  "effect, unfortunately.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_037 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_038 = "<p>\n"
  "%s uses Fleep on %s!\n"
  "<o>\n";

  const char * stg7_2_133_2_039 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s looks dizzy!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_040 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s avoids Fleep's effect!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_042 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But %s is asleep! It doesn't work!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_043 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_044 = "<p>\n"
  "%s uses Cudge<wait 150> to attack %s!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_045 = "<p>\n"
  "Direct hit!\n"
  "<wait 150>%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_046 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "So close!\n"
  "<wait 150>%s barely evades Cudge!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_047 = "<p>\n"
  "%s uses Dottie!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_048 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s gets tiny! Where did he go?!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_050 = "<p>\n"
  "%s uses Piccolo!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_051 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "Piccolo plays a mysterious song!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_052 = "<p>\n"
  "%s falls into a deep sleep!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_053 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_054 = "<p>\n"
  "Spinning destruction!\n"
  "<wait 150>Barry slams powerfully into %s!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_055 = "<p>\n"
  "Nice hit! <wait 150>%s\n"
  "takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_056 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But the attack is evaded!\n"
  "<wait 150>\n"
  "Your party is frustrated!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_056_01 = "<p>\n"
  "%s uses Dashell!\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_056_02 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s dashes quickly!\n"
  "<wait 450>\n"
  "...But nothing else happens.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_057 = "<p>\n"
  "Use which item?\n"
  "<o>\n";

  const char * stg7_2_133_2_058 = "<p>\n"
  "You don't have any items!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_059 = "<p>\n"
  "You can't use that right now!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_060 = "<p>\n"
  "Use it on who?\n"
  "<o>\n";

  const char * stg7_2_133_2_061 = "<p>\n"
  "%s uses\n"
  "<AN_ITEM>!\n"
  "<dkey><wait 500></dkey>\n"
  "<o>\n";

  const char * stg7_2_133_2_062 = "<p>\n"
  "But nothing happens.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_063 = "<p>\n"
  "%s recovers %d HP!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_064 = "<p>\n"
  "%s recovers from\n"
  "poison!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_065 = "<p>\n"
  "The curse on %s is\n"
  "lifted!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_066 = "<p>\n"
  "Your score increased by\n"
  "%d!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_067 = "<p>\n"
  "%s is poisoned!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_068 = "<p>\n"
  "%s's attack power\n"
  "increases!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_069 = "<p>\n"
  "%s starts to gradually\n"
  "recover HP!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_070 = "<p>\n"
  "%s gains the power to\n"
  "shock enemies on contact!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_071 = "<p>\n"
  "%s's defense increases!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_072 = "<p>\n"
  "%s is invincible\nto all attacks!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_073 = "<p>\n"
  "...The flames have no effect\n"
  "on %s!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_074 = "<p>\n"
  "%s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_075 = "<p>\n"
  "%s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_076 = "<p>\n"
  "%s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_077 = "<p>\n"
  "%s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_078 = "<p>\n"
  "A shell appears!\n"
  "<wait 150>\n"
  "%s kicks the shell!\n"
  "<dkey><wait 250></dkey>\n";

  const char * stg7_2_133_2_079 = "<p>\n"
  "%s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_080 = "<p>\n"
  "%s summons a Ghost\n"
  "Shroom! <wait 150>The Ghost Shroom\n"
  "attacks!\n"
  "<dkey><wait 250></dkey>\n";

  const char * stg7_2_133_2_081 = "<p>\n"
  "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But the Ghost Shroom's\n"
  "attacks don't reach!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_082 = "<p>\n"
  "%s falls\n"
  "into a deep sleep.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_083 = "<p>\n"
  "You got <AN_ITEM>!\n"
  "<wait 250>\n"
  "%s uses <AN_ITEM>!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_084 = "<p>\n"
  "%s's Max HP increases\n"
  "by 5!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085 = "<p>\n"
  "%s's Attack increases\n"
  "by 1!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_01 = "<p>\n"
  "%s is frozen\n"
  "solid!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_02 = "<p>\n"
  "%s is cursed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_03 = "<p>\n"
  "%s's movements are\n"
  "slowed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_04 = "<p>\n"
  "%s is tech-cursed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_05 = "<p>\n"
  "%s is heavy-cursed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_085_06 = "<p>\n"
  "%s is reverse-cursed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_086 = "<p>\n"
  "Switch with who?\n"
  "<o>\n";

  const char * stg7_2_133_2_087 = "<p>\n"
  "Tippi says, \"Go, Mario!\"\n"
  "<wait 250>\n"
  "%s switches places\n"
  "with Mario!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_088 = "<p>\n"
  "Bowser says, \"Stomping time!\"\n"
  "<wait 250>\n"
  "%s switches places\n"
  "with Bowser!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_089 = "<p>\n"
  "Luigi says, \"I'm on the job!\"\n"
  "<wait 250>\n"
  "%s switches places\n"
  "with Luigi!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_090 = "<p>\n"
  "%s runs away!\n"
  "<o>\n";
  const char * stg7_2_133_2_091 = "<p>\n"
  "<dkey>.<wait 250>.<wait 250>.<wait 250></dkey>A successful escape!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_092 = "<p>\n"
  "<dkey>.<wait 250>.<wait 250>.<wait 250></dkey>But they got noticed!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_093 = "<p>\n"
  "%s\n"
  "shoots a missile!\n"
  "<wait 300>\n"
  "<o>\n";

  const char * stg7_2_133_2_094 = "<p>\n"
  "%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_095 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "But you're invincible!\n"
  "<wait 150>\n"
  "The attack is ineffective!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_096 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s dodges like a pro!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_097 = "<p>\n"
  "Brobot shoots out\n"
  "blazing fire!\n"
  "<wait 350>\n"
  "<o>\n";

  const char * stg7_2_133_2_098 = "<p>\n"
  "Youch! %s takes\n"
  "%d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_099 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s dodges like a pro!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_100 = "<p>\n"
  "Blue Underchomp breathes\nblue fire!\n"
  "<wait 350>\n"
  "<o>\n";

  const char * stg7_2_133_2_101 = "<p>\n"
  "Now that's hot!\n"
  "<wait 150>\n"
  "%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_102 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s dodges with style!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_103 = "<p>\n"
  "Yellow Underchomp expels\nstinky breath!\n"
  "<wait 350>\n"
  "<o>\n";

  const char * stg7_2_133_2_104 = "<p>\n"
  "Oh no! It's putrid!\n"
  "<wait 150>\n"
  "%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_105 = "<dkey><wait 250></dkey>\n"
  "<p>\n"
  "%s dodges the stink!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_106 = "<p>\n"
  "%s gathers\n"
  "strength!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_107 = "<p>\n"
  "Tippi says, \"Mario! Noooo!\"\n"
  "<wait 250>\n"
  "Mario falls in battle...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_108 = "<p>\n"
  "Bowser says, \"Not again!\"\n"
  "<wait 250>\n"
  "Bowser falls in battle...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_109 = "<p>\n"
  "Luigi says, \"Bro! Forgive me...\"\n"
  "<wait 250>\n"
  "Luigi falls in battle...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_110 = "<p>\n"
  "%s falls in battle...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_111 = "<p>\n"
  "But the Life Shroom restores\n"
  "5 HP!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_112 = "<p>\n"
  "%s is taking\n"
  "a nice nap...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_113 = "<p>\n"
  "%s is looking\n"
  "for %s...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_114 = "<p>\n"
  "%s is dizzy...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_115 = "<p>\n"
  "%s wakes up!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_116 = "<p>\n"
  "%s\n"
  "regains consciousness!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_117 = "<p>\n"
  "%s recovers 1 HP!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_118 = "<p>\n"
  "The ability to recover HP\n"
  "gradually wears off!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_119 = "<p>\n"
  "Yuuuuurk! Poison...\n"
  "<wait 150>\n"
  "%s takes %d damage!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_120 = "<p>\n"
  "%s recovers from\n"
  "poison!\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_121 = "<p>\n"
  "%s's attack power\n"
  "returns to normal.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_122 = "<p>\n"
  "%s loses the ability\n"
  "to shock enemies on contact.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_123 = "<p>\n"
  "%s's defense returns\n"
  "to normal.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124 = "<p>\n"
  "%s loses the power\n"
  "of invincibility.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_01 = "<p>\n"
  "%s is frozen...\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_02 = "<p>\n"
  "%s is freed from\n"
  "the ice.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_03 = "<p>\n"
  "%s is now free\n"
  "from the curse.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_04 = "<p>\n"
  "%s is now free\n"
  "from the tech curse.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_05 = "<p>\n"
  "%s is now free\n"
  "from the heavy curse.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_124_06 = "<p>\n"
  "%s is now free\n"
  "from the reverse curse.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_125 = "<p>\n"
  "%s turns back\n"
  "frontways.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_126 = "<p>\n"
  "%s steps down off\n"
  "Carrie.\n"
  "<k>\n"
  "<o>\n";

  const char * stg7_2_133_2_127 = "<p>\n"
  "%s returns to normal\n"
  "size.\n"
  "<k>\n"
  "<o>\n";

  const char * brobot_toxic_serum = "<p>\n"
  "Brobot spews out\n"
  "a toxic serum!\n"
  "<wait 350>\n"
  "<o>\n";

  const char * peach_heal = "<p>\n"
  "Peach calls upon\n"
  "mushroom magic...\n"
  "<dkey><wait 250></dkey>\n"
  "<o>\n";

  const char * peach_heal_success = "<p>\n"
  "Success!\n"
  "Peach heals 20 HP!\n"
  "<k>\n"
  "<o>\n";

  const char * grab_fp = "<p>\n"
  "Success!\n"
  "You stole 2 FP!\n"
  "<k>\n"
  "<o>\n";

  const char * peach_special = "Heal";

  const char wang_cmd_1[] = {
    "Attack"
  };
  const char wang_cmd_2[] = {
    "Technique"
  };
  const char wang_cmd_3[] = {
    "Pixl"
  };
  const char wang_cmd_4[] = {
    "Item"
  };
  const char wang_cmd_5[] = {
    "Switch"
  };
  const char wang_cmd_6[] = {
    "Escape"
  };
  const char wang_special_1[] = {
    "Flip"
  };
  const char wang_special_2[] = {
    "Flame"
  };
  const char wang_special_3[] = {
    "Super Jump"
  };
  const char wang_level[] = {
    "LV."
  };
  const char wang_hp[] = {
    "HP"
  };
  const char wang_wang_r[] = {
    "Brobot"
  };
  const char wang_wang_b[] = {
    "Brobot"
  };
  const char wang_wang_y[] = {
    "Brobot"
  };

  const char * testCharacterStrings[] = {
    wang_special_1,
    wang_special_2,
    wang_special_3,
    peach_special
  };

  const char * newMsgSearch(const char * msgName) {
    //wii::os::OSReport("%s\n", msgName);
    if (msl::string::strcmp(msgName, "stg7_2_133_2_001") == 0)
      //Override intro
      return rpgStart;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_002") == 0)
      //Replace message
      return stg7_2_133_2_002;
    else if (msl::string::strcmp(msgName, "peach_heal") == 0)
        //Replace message
      return peach_heal;
    else if (msl::string::strcmp(msgName, "peach_heal_success") == 0)
          //Replace message
      return peach_heal_success;
    else if (msl::string::strcmp(msgName, "wang_cmd_1") == 0)
      //Replace message
      return wang_cmd_1;
    else if (msl::string::strcmp(msgName, "wang_cmd_2") == 0)
      //Replace message
      return wang_cmd_2;
    else if (msl::string::strcmp(msgName, "wang_cmd_3") == 0)
      //Replace message
      return wang_cmd_3;
    else if (msl::string::strcmp(msgName, "wang_cmd_4") == 0)
      //Replace message
      return wang_cmd_4;
    else if (msl::string::strcmp(msgName, "wang_cmd_5") == 0)
      //Replace message
      return wang_cmd_5;
    else if (msl::string::strcmp(msgName, "wang_cmd_6") == 0)
      //Replace message
      return wang_cmd_6;
    else if (msl::string::strcmp(msgName, "wang_special_1") == 0)
      //Replace message
      return wang_special_1;
    else if (msl::string::strcmp(msgName, "wang_special_2") == 0)
      //Replace message
      return wang_special_2;
    else if (msl::string::strcmp(msgName, "wang_special_3") == 0)
      //Replace message
      return wang_special_3;
    else if (msl::string::strcmp(msgName, "peach_special") == 0)
        //Replace message
      return peach_special;
    else if (msl::string::strcmp(msgName, "wang_level") == 0)
      //Replace message
      return wang_level;
    else if (msl::string::strcmp(msgName, "wang_hp") == 0)
      //Replace message
      return wang_hp;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_008") == 0)
      //Replace message
      return stg7_2_133_2_008;
    else if (msl::string::strcmp(msgName, "wang_wang_r") == 0)
      //Replace message
      return wang_wang_r;
    else if (msl::string::strcmp(msgName, "wang_wang_b") == 0)
      //Replace message
      return wang_wang_b;
    else if (msl::string::strcmp(msgName, "wang_wang_y") == 0)
      //Replace message
      return wang_wang_y;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_003") == 0)
      //Replace message
      return stg7_2_133_2_003;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_004") == 0)
      //Replace message
      return stg7_2_133_2_004;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_005") == 0)
      //Replace message
      return stg7_2_133_2_005;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_006") == 0)
      //Replace message
      return stg7_2_133_2_006;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_007") == 0)
      //Replace message
      return stg7_2_133_2_007;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_009") == 0)
      //Replace message
      return stg7_2_133_2_009;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_009_01") == 0)
      return stg7_2_133_2_009_01;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_009_02") == 0)
      return stg7_2_133_2_009_02;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_010") == 0)
      return stg7_2_133_2_010;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_011") == 0)
      return stg7_2_133_2_011;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_012") == 0)
      return stg7_2_133_2_012;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_013") == 0)
      return stg7_2_133_2_013;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_014") == 0)
      return stg7_2_133_2_014;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_015") == 0)
      return stg7_2_133_2_015;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_016") == 0)
      return stg7_2_133_2_016;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_017") == 0)
      return stg7_2_133_2_017;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_018") == 0)
      return stg7_2_133_2_018;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_019") == 0)
      return stg7_2_133_2_019;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_020") == 0)
      return stg7_2_133_2_020;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_021") == 0)
      return stg7_2_133_2_021;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_022") == 0)
      return stg7_2_133_2_022;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_023") == 0)
      return stg7_2_133_2_023;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_024") == 0)
      return stg7_2_133_2_024;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_025") == 0)
      return stg7_2_133_2_025;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_026") == 0)
      return stg7_2_133_2_026;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_027") == 0)
      return stg7_2_133_2_027;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_028") == 0)
      return stg7_2_133_2_028;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_029") == 0)
      return stg7_2_133_2_029;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_031") == 0)
      return stg7_2_133_2_031;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_032") == 0)
      return stg7_2_133_2_032;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_033") == 0)
      return stg7_2_133_2_033;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_034") == 0)
      return stg7_2_133_2_034;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_035") == 0)
      return stg7_2_133_2_035;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_036") == 0)
      return stg7_2_133_2_036;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_037") == 0)
      return stg7_2_133_2_037;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_038") == 0)
      return stg7_2_133_2_038;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_039") == 0)
      return stg7_2_133_2_039;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_040") == 0)
      return stg7_2_133_2_040;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_042") == 0)
      return stg7_2_133_2_042;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_043") == 0)
      return stg7_2_133_2_043;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_044") == 0)
      return stg7_2_133_2_044;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_045") == 0)
      return stg7_2_133_2_045;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_046") == 0)
      return stg7_2_133_2_046;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_047") == 0)
      return stg7_2_133_2_047;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_048") == 0)
      return stg7_2_133_2_048;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_050") == 0)
      return stg7_2_133_2_050;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_051") == 0)
      return stg7_2_133_2_051;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_052") == 0)
      return stg7_2_133_2_052;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_053") == 0)
      return stg7_2_133_2_053;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_054") == 0)
      return stg7_2_133_2_054;

    else if (msl::string::strcmp(msgName, "stg7_2_133_2_055") == 0)
      return stg7_2_133_2_055;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_056") == 0)
      return stg7_2_133_2_056;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_056_01") == 0)
      return stg7_2_133_2_056_01;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_056_02") == 0)
      return stg7_2_133_2_056_02;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_057") == 0)
      return stg7_2_133_2_057;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_058") == 0)
      return stg7_2_133_2_058;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_059") == 0)
      return stg7_2_133_2_059;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_060") == 0)
      return stg7_2_133_2_060;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_061") == 0)
      return stg7_2_133_2_061;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_062") == 0)
      return stg7_2_133_2_062;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_063") == 0)
      return stg7_2_133_2_063;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_064") == 0)
      return stg7_2_133_2_064;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_065") == 0)
      return stg7_2_133_2_065;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_066") == 0)
      return stg7_2_133_2_066;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_067") == 0)
      return stg7_2_133_2_067;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_068") == 0)
      return stg7_2_133_2_068;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_069") == 0)
      return stg7_2_133_2_069;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_070") == 0)
      return stg7_2_133_2_070;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_071") == 0)
      return stg7_2_133_2_071;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_072") == 0)
      return stg7_2_133_2_072;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_073") == 0)
      return stg7_2_133_2_073;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_074") == 0)
      return stg7_2_133_2_074;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_075") == 0)
      return stg7_2_133_2_075;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_076") == 0)
      return stg7_2_133_2_076;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_077") == 0)
      return stg7_2_133_2_077;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_078") == 0)
      return stg7_2_133_2_078;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_079") == 0)
      return stg7_2_133_2_079;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_080") == 0)
      return stg7_2_133_2_080;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_081") == 0)
      return stg7_2_133_2_081;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_082") == 0)
      return stg7_2_133_2_082;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_083") == 0)
      return stg7_2_133_2_083;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_084") == 0)
      return stg7_2_133_2_084;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085") == 0)
      return stg7_2_133_2_085;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_01") == 0)
      return stg7_2_133_2_085_01;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_02") == 0)
      return stg7_2_133_2_085_02;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_03") == 0)
      return stg7_2_133_2_085_03;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_04") == 0)
      return stg7_2_133_2_085_04;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_05") == 0)
      return stg7_2_133_2_085_05;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_085_06") == 0)
      return stg7_2_133_2_085_06;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_086") == 0)
      return stg7_2_133_2_086;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_087") == 0)
      return stg7_2_133_2_087;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_088") == 0)
      return stg7_2_133_2_088;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_089") == 0)
      return stg7_2_133_2_089;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_090") == 0)
      return stg7_2_133_2_090;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_092") == 0)
      return stg7_2_133_2_092;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_093") == 0)
      return stg7_2_133_2_093;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_094") == 0)
      return stg7_2_133_2_094;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_095") == 0)
      return stg7_2_133_2_095;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_096") == 0)
      return stg7_2_133_2_096;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_097") == 0)
      return stg7_2_133_2_097;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_098") == 0)
      return stg7_2_133_2_098;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_099") == 0)
      return stg7_2_133_2_099;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_100") == 0)
      return stg7_2_133_2_100;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_101") == 0)
      return stg7_2_133_2_101;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_102") == 0)
      return stg7_2_133_2_102;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_103") == 0)
      return stg7_2_133_2_103;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_104") == 0)
      return stg7_2_133_2_104;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_105") == 0)
      return stg7_2_133_2_105;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_106") == 0)
      return stg7_2_133_2_106;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_107") == 0)
      return stg7_2_133_2_107;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_108") == 0)
      return stg7_2_133_2_108;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_109") == 0)
      return stg7_2_133_2_109;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_110") == 0)
      return stg7_2_133_2_110;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_111") == 0)
      return stg7_2_133_2_111;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_112") == 0)
      return stg7_2_133_2_112;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_113") == 0)
      return stg7_2_133_2_113;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_114") == 0)
      return stg7_2_133_2_114;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_115") == 0)
      return stg7_2_133_2_115;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_116") == 0)
      return stg7_2_133_2_116;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_117") == 0)
      return stg7_2_133_2_117;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_118") == 0)
      return stg7_2_133_2_118;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_119") == 0)
      return stg7_2_133_2_119;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_120") == 0)
      return stg7_2_133_2_120;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_121") == 0)
      return stg7_2_133_2_121;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_122") == 0)
      return stg7_2_133_2_122;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_123") == 0)
      return stg7_2_133_2_123;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124") == 0)
      return stg7_2_133_2_124;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_01") == 0)
      return stg7_2_133_2_124_01;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_02") == 0)
      return stg7_2_133_2_124_02;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_03") == 0)
      return stg7_2_133_2_124_03;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_04") == 0)
      return stg7_2_133_2_124_04;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_05") == 0)
      return stg7_2_133_2_124_05;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_124_06") == 0)
      return stg7_2_133_2_124_06;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_125") == 0)
      return stg7_2_133_2_125;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_126") == 0)
      return stg7_2_133_2_126;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_127") == 0)
      return stg7_2_133_2_127;
    else if (msl::string::strcmp(msgName, "stg7_2_133_2_091") == 0)
      return stg7_2_133_2_091;
    else if (msl::string::strcmp(msgName, "brobot_toxic_serum") == 0)
      return brobot_toxic_serum;
    else if (msl::string::strcmp(msgName, "grab_fp") == 0)
      return grab_fp;
    else if (msl::string::strcmp(msgName, "no_fp") == 0)
      return no_fp;
    else
    //wii::os::OSReport("%s\n", msgName);
      return msgSearch(msgName);

  }

  spm::evtmgr::EvtEntry * newEvtEntry(const spm::evtmgr::EvtScriptCode * script, u32 priority, u8 flags) {
    spm::evtmgr::EvtEntry * entry;
    //wii::os::OSReport("%x\n", script);
    if (script == spm::sp4_13::brobot_appear_evt) {
      wii::os::OSReport("evtEntry\n");
      entry = evtEntry1(mod::parentOfBeginRPG, priority, flags);
    } else {
      entry = evtEntry1(script, priority, flags);
    }
    return entry;
  }

  spm::evtmgr::EvtEntry * newEvtChildEntry(spm::evtmgr::EvtEntry * entry,
    const spm::evtmgr::EvtScriptCode * script, u8 flags) {
    spm::evtmgr::EvtEntry * entry1;
    if (script == spm::sp4_13::brobot_appear_evt) {
      wii::os::OSReport("evtChildEntry\n");
      //wii::os::OSReport("%x\n", entry -> scriptStart);
      entry1 = evtChildEntry(entry, mod::parentOfBeginRPG, flags);
    } else {
      entry1 = evtChildEntry(entry, script, flags);
    }
    return entry1;
  }

  spm::evtmgr::EvtEntry * newEvtBrotherEntry(spm::evtmgr::EvtEntry * brother,
    const spm::evtmgr::EvtScriptCode * script, u8 flags) {
    spm::evtmgr::EvtEntry * entry;
    if (script == spm::sp4_13::brobot_appear_evt) {
      wii::os::OSReport("evtBrotherEntry\n");
      entry = evtBrotherEntry(brother, mod::parentOfBeginRPG, flags);
    } else {
      entry = evtBrotherEntry(brother, script, flags);
    }
    return entry;
  }

  spm::evtmgr::EvtEntry * newEvtEntryType(const spm::evtmgr::EvtScriptCode * script, u32 priority, u8 flags, u8 type) {
    spm::evtmgr::EvtEntry * entry;
    if (script == spm::sp4_13::brobot_appear_evt) {
      //wii::os::OSReport("evtEntryType\n");
      entry = evtEntryType(mod::parentOfBeginRPG, priority, flags, type);
    } else {
      entry = evtEntryType(script, priority, flags, type);
    }
    return entry;
  }

  s32 new_evt_inline_evt(spm::evtmgr::EvtEntry * entry) {
    //wii::os::OSReport("%x\n", entry -> scriptStart);
    if (entry -> scriptStart == spm::sp4_13::mr_l_appear_evt) {
    //  wii::os::OSReport("%x\n", entry -> scriptStart);
    }
    return evt_inline_evt(entry);
  }

  spm::effdrv::EffEntry * newEffNiceEntry(double param_1, double param_2, double param_3, double param_4, int param_5) {

    //wii::os::OSReport("%d %d %d %d %d\n", param_1, param_2, param_3, param_4, param_5);
    return effNiceEntry(param_1, param_2, param_3, param_4, param_5);

  }

  s32 newMarioCalcDamageToEnemy(s32 damageType, s32 tribeId) {
      wii::os::OSReport("%d, %d\n", damageType, tribeId);
      s32 damage = marioCalcDamageToEnemy(damageType, tribeId);
      if (tribeId == 295) damage = damage + 4;
      return damage;
  }

  /*void newDispEntry(f32 z, s8 cameraId, s8 renderMode, spm::dispdrv::DispCallback * callback, void * callbackParam) {
    wii::os::OSReport("%f\n", z);
    if (z == 601.1) {
      dispEntry(299.0, cameraId, renderMode, callback, callbackParam);
    } else {
      dispEntry(z, cameraId, renderMode, callback, callbackParam);
    }
  }*/

  void newMsgUnload(s32 slot) {
    if (slot != 7) {
      msgUnLoad(slot);
    }
  }

  bool new_spsndBGMOn(u32 flags,
    const char * name) {

    wii::os::OSReport("%s\n", name);
    return spsndBGMOn(flags, name);

  }

  s32 new_spsndSFXOn(const char * name) {

    wii::os::OSReport("%s\n", name);
    return spsndSFXOn(name);

  }

  s32 new_evt_rpg_calc_damage_to_enemy(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    s32 damageType = args[1];
    s32 damage = spm::mario::marioCalcDamageToEnemy(damageType, rpgTribeID[1]);
    if (rpgTribeID[1] == 296) damage = damage + 4;
    spm::evtmgr_cmd::evtSetValue(evtEntry, args[2], damage);
    if (firstRun == false) {}
    return 2;
  }

  s32 new_evt_rpg_calc_mario_damage(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    s32 attackStrength = spm::an2_08::an2_08_wp.rpgNpcInfo[1].attackStrength;
    if (attackStrength == 0) attackStrength = 1;
    if ((spm::an2_08::an2_08_wp.unk_54 & 0x40U) != 0) {
      if (0 < attackStrength) {
        attackStrength = attackStrength / 2;
      }
      if (attackStrength == 0) {
        attackStrength = 1;
      }
    }
    spm::evtmgr_cmd::evtSetValue(evtEntry, args[1], attackStrength);
    if (firstRun == false) {}
    return 2;
  }

  void patchWangSpecial() {
    spm::an2_08::lbl_80def2c8[1] = peach_special;
  }

  void hookEvent() {
    patch::hookFunction(spm::an2_08::evt_rpg_calc_damage_to_enemy, new_evt_rpg_calc_damage_to_enemy);
    patch::hookFunction(spm::an2_08::evt_rpg_calc_mario_damage, new_evt_rpg_calc_mario_damage);
    patchWangSpecial();

    evtEntry1 = patch::hookFunction(spm::evtmgr::evtEntry, newEvtEntry);

    evtChildEntry = patch::hookFunction(spm::evtmgr::evtChildEntry, newEvtChildEntry);

    evtBrotherEntry = patch::hookFunction(spm::evtmgr::evtBrotherEntry, newEvtBrotherEntry);

    evtEntryType = patch::hookFunction(spm::evtmgr::evtEntryType, newEvtEntryType);

    //evt_inline_evt = patch::hookFunction(spm::evtmgr_cmd::evt_inline_evt, new_evt_inline_evt);

    marioCalcDamageToEnemy = patch::hookFunction(spm::mario::marioCalcDamageToEnemy, newMarioCalcDamageToEnemy);

    //spsndBGMOn = patch::hookFunction(spm::spmario_snd::spsndBGMOn, new_spsndBGMOn);

    //spsndSFXOn = patch::hookFunction(spm::spmario_snd::spsndSFXOn, new_spsndSFXOn);

    msgSearch = patch::hookFunction(spm::msgdrv::msgSearch, newMsgSearch);

    //writeBranchLink( & spm::an2_08::rpgHandleMenu, 0x1BC, chooseNewCharacterString);
    //writeBranchLink( & spm::an2_08::evt_rpg_calc_damage_to_enemy, 0x44, test);
    writeBranchLink( & spm::an2_08::evt_rpg_npctribe_handle, 0x94, test);
    writeBranchLink( & spm::acdrv::acMain, 0x49C, setNewFloat);
    writeWord( & spm::an2_08::evt_rpg_npctribe_handle, 0xA0, 0x3B9C0004);
    writeWord( & spm::an2_08::evt_rpg_npctribe_handle, 0x8C, 0x3BA00018);
    writeWord( & spm::an2_08::evt_rpg_npctribe_handle, 0x2BC, 0x60000000);
    //writeWord( & spm::acdrv::acMain, 0x49C, 0x60000000);
  }

  /*
      Custom USER FUNCs
  */

  s32 npcEntryFromTribeId(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::npcdrv::NPCWork * npcWork = spm::npcdrv::npcGetWorkPtr();
    spm::evtmgr::EvtVar * evtVariables = evtEntry -> pCurData;
    s32 id = spm::evtmgr_cmd::evtGetValue(evtEntry, * evtVariables);
    for (int i = 0; i < 535; i++) {
      if (npcWork -> entries[i].tribeId == id) {
        spm::evtmgr_cmd::evtSetValue(evtEntry, evtEntry -> lw[0], (s32) npcWork -> entries[i].name);
      }
    }
    if (firstRun == false) {}
    return 2;
  }

  s32 increaseAttack(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    s32 newStrength = args[0];
    s32 strength = spm::an2_08::an2_08_wp.rpgNpcInfo[1].attackStrength;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].attackStrength = strength + newStrength;
    //spm::evtmgr_cmd::evtSetValue(evtEntry, (spm::evtmgr::EvtVar*)evtEntry->pCurData, 1);
    if (firstRun == false) {}
    return 2;
  }

  s32 rpg_npc_setup(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].attackStrength = spm::npcdrv::npcTribes[rpgTribeID[1]].attackStrength;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].maxHp = spm::npcdrv::npcTribes[rpgTribeID[1]].maxHp;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].killXp = spm::npcdrv::npcTribes[rpgTribeID[1]].killXp;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].flags = 0;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].unk_4 = 0;
    spm::an2_08::an2_08_wp.rpgNpcInfo[1].unk_10 = 0xff;
    rpgInProgress = true;
    fp = 5;
    if (firstRun == false) {}
    if (evtEntry->flags == 0) {}
    return 2;
  }

  s32 getFP(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    spm::evtmgr_cmd::evtSetValue(evtEntry, args[0], fp);
    if (firstRun == false) {}
    return 2;
  }

  s32 setFP(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    fp = args[0];
    if (firstRun == false) {}
    return 2;
  }

  s32 addFP(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    fp = fp + args[0];
    if (firstRun == false) {}
    return 2;
  }

  s32 subtractFP(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    spm::evtmgr::EvtVar * args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
    fp = fp - args[0];
    if (firstRun == false) {}
    return 2;
  }

  s32 rpg_off(spm::evtmgr::EvtEntry * evtEntry, bool firstRun) {
    rpgInProgress = false;
    if (firstRun == false) {}
    if (evtEntry->flags == 0) {}
    return 2;
  }

  void patchBrobot() {
    spm::npcdrv::npcTribes[295].maxHp = 1;
    spm::npcdrv::npcTribes[295].killXp = 100;
    spm::npcdrv::npcTribes[296].maxHp = 200;
  }

  void main() {
    wii::os::OSReport("SPM Rel Loader: the mod has ran!\n");
    titleScreenCustomTextPatch();
    hookEvent();
    patchBrobot();
  }

}
