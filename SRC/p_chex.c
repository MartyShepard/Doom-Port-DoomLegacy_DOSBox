#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "m_cheat.h"
#include "dstrings.h"
#include "p_chex.h"

extern byte cheat_mus_seq[];
extern byte cheat_choppers_seq[];
extern byte cheat_god_seq[];
extern byte cheat_ammo_seq[];
extern byte cheat_ammonokey_seq[];
extern byte cheat_noclip_seq[];
extern byte cheat_commercial_noclip_seq[];
extern byte cheat_powerup_seq[7][10];
extern byte cheat_clev_seq[];
extern byte cheat_mypos_seq[];
extern byte cheat_amap_seq[];

#ifdef HWRENDER
		extern light_t lspr[];
#endif

void Chex1PatchEngine(void)
{

	//patch new text
	char* NEW_QUIT1 = "Don't give up now...do\nyou still wish to quit?";
	char* NEW_QUIT2 = "please don't leave we\nneed your help!";
	
	text[QUITMSG_NUM] = NEW_QUIT1;
	text[QUITMSG1_NUM] = NEW_QUIT2;
	text[QUITMSG2_NUM] = NEW_QUIT2;
	text[QUITMSG3_NUM] = NEW_QUIT2;
	text[QUITMSG4_NUM] = NEW_QUIT2;
	text[QUITMSG5_NUM] = NEW_QUIT2;
	text[QUITMSG6_NUM] = NEW_QUIT2;
	text[QUITMSG7_NUM] = NEW_QUIT2;

	text[QUIT2MSG_NUM] = NEW_QUIT1;
	text[QUIT2MSG1_NUM] = NEW_QUIT2;
	text[QUIT2MSG2_NUM] = NEW_QUIT2;
	text[QUIT2MSG3_NUM] = NEW_QUIT2;
	text[QUIT2MSG4_NUM] = NEW_QUIT2;
	text[QUIT2MSG5_NUM] = NEW_QUIT2;
	text[QUIT2MSG6_NUM] = NEW_QUIT2;

	text[HUSTR_E1M1_NUM] = "E1M1: Landing Zone";
	text[HUSTR_E1M2_NUM] = "E1M2: Storage Facility";
	text[HUSTR_E1M3_NUM] = "E1M3: Experimental Lab";
	text[HUSTR_E1M4_NUM] = "E1M4: Arboretum";
	text[HUSTR_E1M5_NUM] = "E1M5: Caverns of Bazoik";

	text[GOTARMBONUS_NUM] = "picked up slime repellant.";
	text[GOTSTIM_NUM] = "picked up bowl of fruit.";
	text[GOTHTHBONUS_NUM] = "picked up glass of water.";
	text[GOTMEDIKIT_NUM] = "picked up bowl of vegetables.";
	text[GOTMEDINEED_NUM] = "vegetables are REALLY good for you!";
	text[GOTARMOR_NUM] = "Picked up the Chex(R) Armor.";
	text[GOTMEGA_NUM] = "Picked up the Super Chex(R) Armor!";
	text[GOTSUPER_NUM] = "Supercharge Breakfast!";
	text[GOTSUIT_NUM] =	"Slimeproof Suit";
	text[GOTBERSERK_NUM] = "SPOOOON!!!!";
	text[GOTINVIS_NUM] = "Semi-invisibility suit.";
	text[GOTVISOR_NUM] = "Light amplification goggles";

	text[GOTBLUECARD_NUM] = "picked up a blue key.";
	text[GOTYELWCARD_NUM] = "picked up a yellow key.";
	text[GOTREDCARD_NUM] = "picked up a red key.";

	text[GOTCLIP_NUM] = "picked up mini zorch recharge.";
	text[GOTCLIPBOX_NUM] = "Picked up a mini zorch pack.";
	text[GOTROCKET_NUM] = "Picked up a Propulsor recharge.";
	text[GOTROCKBOX_NUM] = "Picked up a 5 pack of propulsor zorch.";
	text[GOTCELL_NUM] = "Picked up a Phasing zorcher recharge.";
	text[GOTCELLBOX_NUM] = "Picked up a phasing zorcher pack.";
	text[GOTSHELLS_NUM] = "Picked up 4 large zorcher recharges.";
	text[GOTSHELLBOX_NUM] = "Picked up a large zorcher pack (20)";
	text[GOTBACKPACK_NUM] = "Picked up a zorchpack.";
	text[GOTBFG9000_NUM] = "Oh, yes.You got the LAZ device!";
	text[GOTCHAINGUN_NUM] = "You got a rapid zorcher!";
	text[GOTCHAINSAW_NUM] = "You got the super bootspork!";
	text[GOTLAUNCHER_NUM] = "You got the zorch propulsor!";
	text[GOTPLASMA_NUM] = "You got the phasing zorcher!";
	text[GOTSHOTGUN_NUM] = "You got the large zorcher!";

	text[STSTR_DQDON_NUM] = "Invincible mode on.";
	text[STSTR_DQDOFF_NUM] = "Invincible mode off.";
	text[STSTR_FAADDED_NUM] = "Zorch Added.";
	text[STSTR_KFAADDED_NUM] = "Zorch and keys added.";
	text[STSTR_CHOPPERS_NUM] = "Eat Chex!";

	text[E1TEXT_NUM] = "mission accomplished.\n\nare you prepared for the next mission?\n\n\n\n\n\n\npress the escape key to continue...\n";
	text[NIGHTMARE_NUM] = "careful. this will be tough.\ndo you wish to continue?\n\npress y or n.";

	text[DEATHMSG_SUICIDE] = "%s needs to be more careful\n";
	text[DEATHMSG_TELEFRAG] = "%s was standing in the wrong spot\n";
	text[DEATHMSG_FIST] = "%s ate from the spoon of %s\n";
	text[DEATHMSG_GUN] = "%s was zorched by %s\n";
	text[DEATHMSG_SHOTGUN] = "%s took a large zorch from %s\n";
	text[DEATHMSG_MACHGUN] = "%s was rapidly zorched by %s\n";
	text[DEATHMSG_ROCKET] = "%s almost dodged %'s propulsor\n";
	text[DEATHMSG_GIBROCKET] = "%s was hit by %'s propulsor\n";
	text[DEATHMSG_PLASMA] = "%s was phased by %s\n";
	text[DEATHMSG_BFGBALL] = "%s couldn't escape %'s LAZ either\n";
	text[DEATHMSG_CHAINSAW] = "%s was fed from %'s bootspork\n";
	text[DEATHMSG_PLAYUNKNOW] = "%s was zorched by %s\n";
	text[DEATHMSG_HELLSLIME] = "%s can't swim in slime\n";
	text[DEATHMSG_NUKE] = "%s can't swim in slime\n";
	text[DEATHMSG_SUPHELLSLIME] = "%s can't swim in slime\n";
	text[DEATHMSG_SPECUNKNOW] = "%s needs to be more careful\n";
	text[DEATHMSG_BARRELFRAG] = "%s was zorched by %s\n";
	text[DEATHMSG_BARREL] = "%s needs to be more careful\n";
	text[DEATHMSG_POSSESSED] = "A felmoid slimed %s\n";
	text[DEATHMSG_SHOTGUY] = "%s was slimed by a bipedicus\n";
	text[DEATHMSG_TROOP] = "%s was slimed by an armored bipedicus\n";
	text[DEATHMSG_SERGEANT] = "%s was slimed by a cycloptis\n";
	text[DEATHMSG_BRUISER] = "%s fell victim to the flembrane\n";
	text[DEATHMSG_DEAD] = "%s was slimed\n";


	//patch monster changes
	mobjinfo[MT_POSSESSED].missilestate = 0;
	mobjinfo[MT_POSSESSED].meleestate = S_POSS_ATK1;

	mobjinfo[MT_SHOTGUY].missilestate = 0;
	mobjinfo[MT_SHOTGUY].meleestate = S_SPOS_ATK1;

	mobjinfo[MT_BRUISER].speed = 0;
	mobjinfo[MT_BRUISER].radius = 48*FRACUNIT; //let's try 48

	//coronas
	#ifdef HWRENDER
		lspr[1].dynamic_color = 0xff000050;
		lspr[2].dynamic_color = 0xff000050;
		lspr[3].dynamic_color = 0xff4040f7;
		lspr[4].dynamic_color = 0xff4040f7;
		lspr[5].dynamic_color = 0xff4040f7;
		lspr[6].dynamic_color = 0xff4040a0;
		lspr[7].type = 0;
		lspr[8].type = 0;
		lspr[9].type = 0;
		lspr[10].type = 0;
		lspr[11].type = 0;
		lspr[12].type = 0;
		lspr[15].light_yoffset = 3.0f;	//light
		lspr[16].type = 0;
		lspr[17].type = 0;
	#endif

	//cheat codes
	
}
