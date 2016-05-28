//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot_combat.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "studio.h"

#include "bot.h"
#include "bot_func.h"
#include "bot_weapons.h"

extern int mod_id;
extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern bool b_observer_mode;
extern int team_allies[4];
extern edict_t *pent_info_ctfdetect;
extern float is_team_play;
extern bool checked_teamplay;

FILE *fp;

typedef struct
{
	int iId;  // the weapon ID value
	char weapon_name[64];  // name of the weapon when selecting it
	float fPrimaryRate;			// primary rate of fire
	float fSecondaryRate;		// secondary rate of fire
	float primary_min_distance;    // 0 = no minimum
	float primary_max_distance;    // 9999 = no maximum
	float secondary_min_distance; // 0 = no minimum
	float secondary_max_distance; // 9999 = no maximum
	int use_percent;    // times out of 100 to use this weapon when available
	bool can_use_underwater;      // can use this weapon underwater
	int primary_fire_percent;    // times out of 100 to use primary fire
	int min_primary_ammo;         // minimum ammout of primary ammo needed to fire
	int min_secondary_ammo;      // minimum ammout of seconday ammo needed to fire
	bool primary_fire_hold;        // hold down primary fire button to use?
	bool secondary_fire_hold;     // hold down secondary fire button to use?
	bool primary_fire_charge;     // charge weapon using primary fire?
	bool secondary_fire_charge;  // charge weapon using secondary fire?
	float primary_charge_delay;    // time to charge weapon
	float secondary_charge_delay; // time to charge weapon
	// int iPrimaryAmmoIndex;
	// int iSecondaryAmmoIndex;
} bot_weapon_select_t;

// weapons are stored in priority order, most desired weapon should be at
// the start of the array and least desired should be at the end
// presumably the no ammo ones are at the top so they can always be selected

bot_weapon_select_t valve_weapon_select[] = {
    {VALVE_WEAPON_CROWBAR, "weapon_crowbar", 0.3, 0.0,
	0.0, 32.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_HANDGRENADE, "weapon_handgrenade", 0.1, 0.0,
	250.0, 750.0, 0.0, 0.0,
    30, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_SNARK, "weapon_snark", 0.1, 0.0,
	150.0, 500.0, 0.0, 0.0,
    50, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_EGON, "weapon_egon", 0.0, 0.0,
	128.0, 2048.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_GAUSS, "weapon_gauss", 0.2, 1.0,
	0.0, 8192.0, 0.0, 8192.0,
    100, FALSE, 80, 1, 10,
	FALSE, FALSE, FALSE, TRUE, 0.0, 0.8},
    {VALVE_WEAPON_SHOTGUN, "weapon_shotgun", 0.75, 1.5,
	0.0, 150.0, 0.0, 150.0,
    100, FALSE, 70, 1, 2,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_PYTHON, "weapon_357", 0.75, 0.0, 0.0,
	8192.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_HORNETGUN, "weapon_hornetgun", 0.25, 0.0,
	0.0, 1000.0, 0.0, 1000.0,
    100, TRUE, 50, 1, 4,
	FALSE, TRUE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_MP5, "weapon_9mmAR", 0.1, 1.0,
	0.0, 8192.0, 250.0, 600.0,
    100, FALSE, 90, 1, 1,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_CROSSBOW, "weapon_crossbow", 0.75, 0.0,
	128.0, 8192.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_RPG, "weapon_rpg", 1.5, 0.0,
	250.0, 9999.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_GLOCK, "weapon_9mmhandgun", 0.3, 0.2,
	0.0, 8192.0, 0.0, 1024.0,
    100, TRUE, 70, 1, 1,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    /* terminator */
    {0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t gearbox_weapon_select[] = {
	{GEARBOX_WEAPON_PIPEWRENCH, "weapon_pipewrench", 0.5, 0.0,
	 0.0, 50.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_KNIFE, "weapon_knife", 0.4, 0.0,
	 0.0, 50.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_CROWBAR, "weapon_crowbar", 0.3, 0.0,
	 0.0, 50.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_DISPLACER, "weapon_displacer", 5.0, 0.0,
	 100.0, 1000.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_SPORELAUNCHER, "weapon_sporelauncher", 0.5, 0.0,
	 500.0, 1000.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_SHOCKRIFLE, "weapon_shockrifle", 0.1, 0.0,
	 50.0, 800.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_SNIPERRIFLE, "weapon_sniperrifle", 1.5, 0.0,
	 50.0, 1500.0, 0.0, 0.0,
	 100, FALSE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_HANDGRENADE, "weapon_handgrenade", 0.1, 0.0,
	 250.0, 750.0, 0.0, 0.0,
	 30, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_SNARK, "weapon_snark", 0.1, 0.0,
	 150.0, 500.0, 0.0, 0.0,
	 50, FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_EGON, "weapon_egon", 0.0, 0.0,
	 0.0, 9999.0, 0.0, 0.0,
	 100, FALSE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_GAUSS, "weapon_gauss", 0.2, 1.0,
	 0.0, 9999.0, 0.0, 9999.0,
	 100, FALSE, 80, 1, 10, FALSE, FALSE, FALSE, TRUE, 0.0, 0.8},
	{GEARBOX_WEAPON_M249, "weapon_m249", 0.1, 0.0,
	 0.0, 400.0, 0.0, 0.0,
	 100, FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_SHOTGUN, "weapon_shotgun", 0.75, 1.5,
	 30.0, 150.0, 30.0, 150.0,
	 100, FALSE, 70, 1, 2, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_EAGLE, "weapon_eagle", 0.25, 0.0,
	 0.0, 1200.0, 0.0, 0.0,
	 100, FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_PYTHON, "weapon_357", 0.75, 0.0,
	 30.0, 700.0, 0.0, 0.0,
	 100, FALSE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_HORNETGUN, "weapon_hornetgun", 0.25, 0.0,
	 30.0, 1000.0, 30.0, 1000.0,
	 100, TRUE, 50, 1, 4, FALSE, TRUE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_MP5, "weapon_9mmAR", 0.1, 1.0,
	 0.0, 250.0, 300.0, 600.0,
	 100, FALSE, 90, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_CROSSBOW, "weapon_crossbow", 0.75, 0.0,
	 100.0, 1000.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_RPG, "weapon_rpg", 1.5, 0.0,
	 300.0, 9999.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GEARBOX_WEAPON_GLOCK, "weapon_9mmhandgun", 0.3, 0.2,
	 0.0, 1200.0, 0.0, 1200.0,
	 100, TRUE, 70, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	/* terminator */
	{0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t cs_weapon_select[] = {
	{CS_WEAPON_KNIFE, "weapon_knife", 0.3, 0.0,
	 0.0, 50.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0,
	 FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{CS_WEAPON_USP, "weapon_usp", 0.3, 0.2,
	 0.0, 1200.0, 0.0, 1200.0,
	 100, TRUE, 100, 1, 0,
	 FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{CS_WEAPON_GLOCK18, "weapon_glock18", 0.3, 0.2,
	 0.0, 1200.0, 0.0, 1200.0,
	 100, TRUE, 100, 1, 0,
	 FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	/* terminator */
	{0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t tfc_weapon_select[] = {
	{TF_WEAPON_AXE, "tf_weapon_axe", 0.3, 0.0,
	 0.0, 50.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_KNIFE, "tf_weapon_knife", 0.3, 0.0,
	 0.0, 40.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_SPANNER, "tf_weapon_knife", 0.3, 0.0,
	 0.0, 40.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_MEDIKIT, "tf_weapon_medikit", 0.3, 0.0,
	 0.0, 40.0, 0.0, 0.0,
	 100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_SNIPERRIFLE, "tf_weapon_sniperrifle", 1.0, 0.0,
	 300.0, 2500.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, TRUE, FALSE, 1.0, 0.0},
	{TF_WEAPON_FLAMETHROWER, "tf_weapon_flamethrower", 0.0, 0.0,
	 100.0, 500.0, 0.0, 0.0,
	 100, FALSE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_AC, "tf_weapon_ac", 0.0, 0.0,
	 50.0, 1000.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_GL, "tf_weapon_gl", 0.6, 0.0,
	 300.0, 900.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_RPG, "tf_weapon_rpg", 0.5, 0.0,
	 300.0, 900.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_IC, "tf_weapon_ic", 2.0, 0.0,
	 300.0, 800.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_TRANQ, "tf_weapon_tranq", 1.5, 0.0,
	 40.0, 1000.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_RAILGUN, "tf_weapon_railgun", 0.4, 0.0,
	 40.0, 800.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_SUPERNAILGUN, "tf_weapon_superng", 0.0, 0.0,
	 40.0, 800.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_SUPERSHOTGUN, "tf_weapon_supershotgun", 0.6, 0.0,
	 40.0, 500.0, 0.0, 0.0,
	 100, TRUE, 100, 2, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_AUTORIFLE, "tf_weapon_autorifle", 0.1, 0.0,
	 0.0, 800.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_SHOTGUN, "tf_weapon_shotgun", 0.5, 0.0,
	 40.0, 400.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{TF_WEAPON_NAILGUN, "tf_weapon_ng", 0.1, 0.0,
	 40.0, 600.0, 0.0, 0.0,
	 100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	/* terminator */
	{0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t gunman_weapon_select[] = {
	{GUNMAN_WEAPON_DMLGRENADE, "weapon_dmlgrenade", 1.0, 0.0,
	250.0, 750.0, 0.0, 0.0,
	30, true, 100, 1, 0,
	false, false, false, false, 0.0, 0.0},
	{GUNMAN_WEAPON_BEAMGUN, "weapon_beamgun", 0.0, 0.0,
	0.0, 500.0, 0.0, 0.0,
	100, false, 100, 1, 0,
	true, false, false, false, 0.0, 0.0},
	{GUNMAN_WEAPON_MINIGUN, "weapon_minigun", 0.1, 0.0,
	0.0, 1024.0, 0.0, 0.0,
	100, true, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GUNMAN_WEAPON_SHOTGUN, "weapon_shotgun", 0.75, 0.0,
	0.0, 150.0, 0.0, 0.0,
    100, true, 100, 2, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GUNMAN_WEAPON_CHEMGUN, "weapon_chemgun", 1.0, 0.0,
	100.0, 400.0, 0.0, 0.0,
	100, true, 100, 2, 0,
	false, false, false, false, 0.0, 0.0},
	{GUNMAN_WEAPON_DML, "weapon_dml", 1.5, 0.0, 250.0,
	9999.0, 0.0, 0.0,
	100, true, 100, 1, 0,
	false, false, false, false, 0.0, 0.0},
	{GUNMAN_WEAPON_GAUSSPISTOL, "weapon_gausspistol", 0.2, 0.0,
	0.0, 1024.0, 0.0, 0.0,
    100, true, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GUNMAN_WEAPON_FISTS, "weapon_fists", 0.3, 0.0,
	0.0, 32.0, 0.0, 0.0,
    100, true, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{GUNMAN_WEAPON_AICORE, "weapon_aicore", 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0, true, 100, 0, 0,
	false, false, false, false, 0.0, 0.0},
	/* terminator */
    {0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t ns_weapon_select[] = {
    {NS_WEAPON_GRENADE, "weapon_grenade", 1.0, 0.0,
	250.0, 750.0, 0.0, 0.0,
    40, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {NS_WEAPON_HEAVYMACHINEGUN, "weapon_heavymachinegun", 0.0, 0.0,
	0.0, 250.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_SHOTGUN, "weapon_shotgun", 1.3, 0.0,
	30.0, 150.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_MACHINEGUN, "weapon_machinegun", 0.0, 0.0,
	0.0, 250.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
    {NS_WEAPON_GRENADEGUN, "weapon_grenadegun", 1.2, 0.0,
	300.0, 500.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {NS_WEAPON_PISTOL, "weapon_pistol", 0.5, 0.0,
	0.0, 1200.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_PISTOL, "weapon_welder", 0.0, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_KNIFE, "weapon_knife", 0.7, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

	// skulk
	{NS_WEAPON_BITEGUN, "weapon_bitegun", 0.8, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

	// gorge
	{NS_WEAPON_SPIT, "weapon_spit", 0.8, 0.0,
	0.0, 250.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

	// lerk
	{NS_WEAPON_BITE2GUN, "weapon_bite2gun", 0.7, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_SPORE, "weapon_spore", 0.5, 0.0,
	0.0, 225.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

	// fade
	{NS_WEAPON_SWIPE, "weapon_swipe", 0.9, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_BLINK, "weapon_blink", 0.0, 0.0,
	50.0, 9999.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_ACIDROCKETGUN, "weapon_acidrocketgun", 1.0, 0.0, 0.0, 1200.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

	// onos
	{NS_WEAPON_CLAWS, "weapon_claws", 0.9, 0.0,
	0.0, 50.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_STOMP, "weapon_stomp", 1.0, 0.0,
	0.0, 250.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{NS_WEAPON_CHARGE, "weapon_charge", 1.0, 0.0,
	0.0, 100.0, 0.0, 0.0,
    30, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},

    /* terminator */
    {0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t hunger_weapon_select[] = {
    {VALVE_WEAPON_CROWBAR, "weapon_crowbar", 0.3, 0.0,
	0.0, 32.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_SPANNER, "weapon_th_shovel", 0.3, 0.0,
	0.0, 32.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_SPANNER, "weapon_th_spanner", 0.3, 0.0,
	0.0, 32.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_HANDGRENADE, "weapon_handgrenade", 0.1, 0.0,
	250.0, 750.0, 0.0, 0.0,
    30, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_SNARK, "weapon_snark", 0.1, 0.0,
	150.0, 500.0, 0.0, 0.0,
    50, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_EGON, "weapon_egon", 0.0, 0.0,
	128.0, 2048.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_GAUSS, "weapon_gauss", 0.2, 1.0,
	0.0, 8192.0, 0.0, 8192.0,
    100, FALSE, 80, 1, 10,
	FALSE, FALSE, FALSE, TRUE, 0.0, 0.8},
    {VALVE_WEAPON_SHOTGUN, "weapon_shotgun", 0.75, 1.5,
	0.0, 150.0, 0.0, 150.0,
    100, FALSE, 70, 1, 2,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_PYTHON, "weapon_357", 0.75, 0.0,
	0.0, 8192.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_TAURUS, "weapon_th_taurus", 0.75, 0.0,
	0.0, 8192.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_MP5, "weapon_9mmAR", 0.1, 1.0,
	0.0, 8192.0, 250.0, 600.0,
    100, FALSE, 90, 1, 1,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_CHAINGUN, "weapon_th_chaingun", 0.1, 0.0,
	0.0, 8192.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_AP9, "weapon_th_ap9", 0.1, 1.0,
	0.0, 8192.0, 0.0, 8192.0,
    100, FALSE, 70, 1, 3,
	true, FALSE, FALSE, FALSE, 0.0, 0.0},
	{HUNGER_WEAPON_TH_SNIPER, "weapon_th_sniper", 0.1, 0.0,
	0.0, 8192.0, 0.0, 0.0,
    100, FALSE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_CROSSBOW, "weapon_crossbow", 0.75, 0.0,
	128.0, 8192.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_RPG, "weapon_rpg", 1.5, 0.0, 250.0, 9999.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    {VALVE_WEAPON_GLOCK, "weapon_9mmhandgun", 0.3, 0.2,
	0.0, 8192.0, 0.0, 1024.0,
    100, TRUE, 70, 1, 1,
	FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
    /* terminator */
    {0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};

bot_weapon_select_t frontline_weapon_select[] = {
   {FLF_WEAPON_HEGRENADE, "weapon_hegrenade", 0.3, 0.0,
   200.0, 1000.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_FLASHBANG, "weapon_flashbang", 0.3, 0.0,
   100.0, 800.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_KNIFE, "weapon_knife", 0.3, 0.2,
   0.0, 60.0, 0.0, 0.0,
    100, TRUE, 100, 0, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_HK21, "weapon_hk21", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_UMP45, "weapon_ump45", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_FAMAS, "weapon_famas", 0.0, 0.0,
   0.0, 500.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_MSG90, "weapon_msg90", 1.2, 0.0,
   0.0, 2500.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_MP5A2, "weapon_mp5a2", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_AK5, "weapon_ak5", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_MP5SD, "weapon_mp5sd", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_M4, "weapon_m4", 0.0, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_SPAS12, "weapon_spas12", 0.9, 0.0,
   0.0, 900.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_MAC10, "weapon_mac10", 0.0, 0.0,
   0.0, 500.0, 0.0, 0.0,
    100, TRUE, 100, 1, 0, TRUE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_BERETTA, "weapon_beretta", 0.4, 0.0,
   0.0, 1200.0, 0.0, 1200.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   {FLF_WEAPON_MK23, "weapon_mk23", 0.4, 0.0,
   0.0, 1200.0, 0.0, 1200.0,
    100, TRUE, 100, 1, 0, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0},
   /* terminator */
   {0, "", 0, 0.0, 0.0, 0.0, 0.0, 0, TRUE, 0, 1, 1, FALSE, FALSE, FALSE, FALSE, 0.0, 0.0}
};


void BotCheckTeamplay(void)
{
	// is this a teamplay mod
	if( ((mod_id == GEARBOX_DLL) && pent_info_ctfdetect) || (mod_id == DECAY_DLL) || (mod_id == CSTRIKE_DLL) || (mod_id == CZERO_DLL) || (mod_id == DOD_DLL) || (mod_id == TFC_DLL) || (mod_id == NS_DLL) || (mod_id == FRONTLINE_DLL))
	{
		is_team_play = 1.0;
	}
	// otherwise fall back on the mp_teamplay cvar
	else
	{
		is_team_play = CVAR_GET_FLOAT("mp_teamplay");  // teamplay enabled?
	}

	checked_teamplay = TRUE;
}


edict_t *BotFindEnemy( bot_t *pBot )
{
   Vector vecEnd;
   static bool flag=TRUE;
   edict_t *pent = NULL;
   edict_t *pNewEnemy;
   float nearestdistance;
   int i;

   edict_t *pEdict = pBot->pEdict;

   if (pBot->pBotEnemy != NULL)  // does the bot already have an enemy?
   {
      vecEnd = pBot->pBotEnemy->v.origin + pBot->pBotEnemy->v.view_ofs;

      // if the enemy is dead?
      if (!IsAlive(pBot->pBotEnemy))  // is the enemy dead?, assume bot killed it
      {
         // the enemy is dead, jump for joy about 10% of the time
         if (RANDOM_LONG(1, 100) <= 10)
            pEdict->v.button |= IN_JUMP;

         // don't have an enemy anymore so null out the pointer...
         pBot->pBotEnemy = NULL;
      }
      else if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ))
      {
         if ((mod_id == TFC_DLL) && (pEdict->v.playerclass == TFC_CLASS_MEDIC))
         {
            if (pBot->pBotEnemy->v.health >= pBot->pBotEnemy->v.max_health)
            {
               pBot->pBotEnemy = NULL;  // player is healed, null out pointer
            }
         }
         else
         {
            // if enemy is still visible and in field of view, keep it

            // face the enemy
            Vector v_enemy = pBot->pBotEnemy->v.origin - pEdict->v.origin;
            Vector bot_angles = UTIL_VecToAngles( v_enemy );

            pEdict->v.ideal_yaw = bot_angles.y;

            BotFixIdealYaw(pEdict);

            // keep track of when we last saw an enemy
            pBot->f_bot_see_enemy_time = gpGlobals->time;

            return (pBot->pBotEnemy);
         }
      }
   }

   pent = NULL;
   pNewEnemy = NULL;
   nearestdistance = 1000;

   if (mod_id == TFC_DLL)
   {
      if (pEdict->v.playerclass == TFC_CLASS_MEDIC)
      {
         // search the world for players...
         for (i = 1; i <= gpGlobals->maxClients; i++)
         {
            edict_t *pPlayer = INDEXENT(i);

            // skip invalid players and skip self (i.e. this bot)
            if ((pPlayer) && (!pPlayer->free) && (pPlayer != pEdict))
            {
               // skip this player if not alive (i.e. dead or dying)
               if (!IsAlive(pPlayer))
                  continue;

               if ((b_observer_mode) && !(pPlayer->v.flags & FL_FAKECLIENT))
                  continue;

               int player_team = UTIL_GetTeam(pPlayer);
               int bot_team = UTIL_GetTeam(pEdict);

               // don't target your enemies...
               if ((bot_team != player_team) && !(team_allies[bot_team] & (1<<player_team)))
                  continue;

               // check if player needs to be healed...
               if ((pPlayer->v.health / pPlayer->v.max_health) > 0.50)
                  continue;  // health greater than 50% so ignore

               vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

               // see if bot can see the player...
               if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ))
               {
                  float distance = (pPlayer->v.origin - pEdict->v.origin).Length();

                  if (distance < nearestdistance)
                  {
                     nearestdistance = distance;
                     pNewEnemy = pPlayer;

                     pBot->pBotUser = NULL;  // don't follow user when enemy found
                  }
               }
            }
         }
      }

      if (pNewEnemy == NULL)
      {
         while ((pent = UTIL_FindEntityByClassname( pent, "building_sentrygun" )) != NULL)
         {
            int sentry_team = -1;
            int bot_team = UTIL_GetTeam(pEdict);

            if (pent->v.colormap == 0xA096)
               sentry_team = 0;  // blue team's sentry
            else if (pent->v.colormap == 0x04FA)
               sentry_team = 1;  // red team's sentry
            else if (pent->v.colormap == 0x372D)
               sentry_team = 2;  // yellow team's sentry
            else if (pent->v.colormap == 0x6E64)
               sentry_team = 3;  // green team's sentry

            // don't target your own team's sentry guns...
            if (bot_team == sentry_team)
               continue;

            // don't target your allie's sentry guns either...
            if (team_allies[bot_team] & (1<<sentry_team))
               continue;

            vecEnd = pent->v.origin + pent->v.view_ofs;

            // is this sentry gun visible?
            if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ))
            {
               float distance = (pent->v.origin - pEdict->v.origin).Length();

               // is this the closest sentry gun?
               if (distance < nearestdistance)
               {
                  nearestdistance = distance;
                  pNewEnemy = pent;

                  pBot->pBotUser = NULL;  // don't follow user when enemy found
               }
            }
         }
      }
   }

   if (pNewEnemy == NULL)
   {
      nearestdistance = 2500;

      // search the world for players...
      for (i = 1; i <= gpGlobals->maxClients; i++)
      {
         edict_t *pPlayer = INDEXENT(i);

         // skip invalid players and skip self (i.e. this bot)
         if ((pPlayer) && (!pPlayer->free) && (pPlayer != pEdict))
         {
            // skip this player if not alive (i.e. dead or dying)
            if (!IsAlive(pPlayer))
               continue;

            if ((b_observer_mode) && !(pPlayer->v.flags & FL_FAKECLIENT))
               continue;

            if (!checked_teamplay)  // check for team play...
               BotCheckTeamplay();

            // is team play enabled?
            if (is_team_play > 0.0)
            {
               int player_team = UTIL_GetTeam(pPlayer);
               int bot_team = UTIL_GetTeam(pEdict);

               // don't target your teammates...
               if (bot_team == player_team)
                  continue;

               if (mod_id == TFC_DLL)
               {
                  // don't target your allies either...
                  if (team_allies[bot_team] & (1<<player_team))
                     continue;
               }
            }

            vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

            // see if bot can see the player...
            if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ))
            {
               float distance = (pPlayer->v.origin - pEdict->v.origin).Length();
               if (distance < nearestdistance)
               {
                  nearestdistance = distance;
                  pNewEnemy = pPlayer;

                  pBot->pBotUser = NULL;  // don't follow user when enemy found
               }
            }
         }
      }
   }

   if (pNewEnemy)
   {
      // face the enemy
      Vector v_enemy = pNewEnemy->v.origin - pEdict->v.origin;
      Vector bot_angles = UTIL_VecToAngles( v_enemy );

      pEdict->v.ideal_yaw = bot_angles.y;

      BotFixIdealYaw(pEdict);

      // keep track of when we last saw an enemy
      pBot->f_bot_see_enemy_time = gpGlobals->time;
   }

   // has the bot NOT seen an ememy for at least 5 seconds (time to reload)?
   if ((pBot->f_bot_see_enemy_time > 0) && ((pBot->f_bot_see_enemy_time + 5.0) <= gpGlobals->time))
   {
      pBot->f_bot_see_enemy_time = -1;  // so we won't keep reloading

	  // TODO: is this check needed for all mods where players reload? check where else IN_RELOAD is set
      if ((mod_id == VALVE_DLL) || (mod_id == GEARBOX_DLL) || (mod_id == REWOLF_DLL) || (mod_id == HUNGER_DLL))
      {
         pEdict->v.button |= IN_RELOAD;  // press reload button
      }
   }

   return (pNewEnemy);
}


Vector BotBodyTarget( edict_t *pBotEnemy, bot_t *pBot )
{
   Vector target;
   float f_distance;
   float f_scale;
   int d_x, d_y, d_z;

   edict_t *pEdict = pBot->pEdict;

   f_distance = (pBotEnemy->v.origin - pEdict->v.origin).Length();

   if (f_distance > 1000)
      f_scale = 1.0;
   else if (f_distance > 100)
      f_scale = f_distance / 1000.0;
   else
      f_scale = 0.1;

   switch (pBot->bot_skill)
   {
      case 0:
         // VERY GOOD, same as from CBasePlayer::BodyTarget (in player.h)
         target = pBotEnemy->v.origin + pBotEnemy->v.view_ofs * RANDOM_FLOAT( 0.5, 1.1 );
         d_x = 0;  // no offset
         d_y = 0;
         d_z = 0;
         break;
      case 1:
         // GOOD, offset a little for x, y, and z
         target = pBotEnemy->v.origin + pBotEnemy->v.view_ofs;  // aim for the head (if you can find it)
         d_x = RANDOM_FLOAT(-5, 5) * f_scale;
         d_y = RANDOM_FLOAT(-5, 5) * f_scale;
         d_z = RANDOM_FLOAT(-10, 10) * f_scale;
         break;
      case 2:
         // FAIR, offset somewhat for x, y, and z
         target = pBotEnemy->v.origin;  // aim for the body
         d_x = RANDOM_FLOAT(-10, 10) * f_scale;
         d_y = RANDOM_FLOAT(-10, 10) * f_scale;
         d_z = RANDOM_FLOAT(-18, 18) * f_scale;
         break;
      case 3:
         // POOR, offset for x, y, and z
         target = pBotEnemy->v.origin;  // aim for the body
         d_x = RANDOM_FLOAT(-20, 20) * f_scale;
         d_y = RANDOM_FLOAT(-20, 20) * f_scale;
         d_z = RANDOM_FLOAT(-32, 32) * f_scale;
         break;
      case 4:
         // BAD, offset lots for x, y, and z
         target = pBotEnemy->v.origin;  // aim for the body
         d_x = RANDOM_FLOAT(-35, 35) * f_scale;
         d_y = RANDOM_FLOAT(-35, 35) * f_scale;
         d_z = RANDOM_FLOAT(-50, 50) * f_scale;
         break;
   }

   target = target + Vector(d_x, d_y, d_z);

   return target;
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)
// BotFireWeapon will return TRUE if weapon was fired, FALSE otherwise

bool BotFireWeapon( Vector v_enemy, bot_t *pBot, int weapon_choice)
{
   bot_weapon_select_t *pSelect = NULL;
   int select_index;
   int iId;
   bool use_primary;
   bool use_secondary;
   int use_percent;
   int primary_percent;

   edict_t *pEdict = pBot->pEdict;

   float distance = v_enemy.Length();  // how far away is the enemy?

   if (mod_id == VALVE_DLL)
   {
      pSelect = &valve_weapon_select[0];
   }
   else if (mod_id == TFC_DLL)
   {
      pSelect = &tfc_weapon_select[0];
   }
   else if (mod_id == CSTRIKE_DLL)
   {
      pSelect = &cs_weapon_select[0];
   }
   else if (mod_id == GEARBOX_DLL)
   {
      pSelect = &gearbox_weapon_select[0];
   }
   else if (mod_id == REWOLF_DLL)
   {
      pSelect = &gunman_weapon_select[0];
   }
   else if (mod_id == NS_DLL)
   {
      pSelect = &ns_weapon_select[0];
   }
   else if (mod_id == HUNGER_DLL)
   {
      pSelect = &hunger_weapon_select[0];
   }
   else if (mod_id == FRONTLINE_DLL)
   {
      pSelect = &frontline_weapon_select[0];
   }

   if (pSelect)
   {
      // are we charging the primary fire?
      if (pBot->f_primary_charging > 0)
      {
         iId = pBot->charging_weapon_id;

         if (mod_id == TFC_DLL)
         {
            if (iId == TF_WEAPON_SNIPERRIFLE)
            {
               pBot->f_move_speed = 0;  // don't move while using sniper rifle
            }
         }

         // is it time to fire the charged weapon?
         if (pBot->f_primary_charging <= gpGlobals->time)
         {
            // we DON'T set pEdict->v.button here to release the
            // fire button which will fire the charged weapon

            pBot->f_primary_charging = -1;  // -1 means not charging

            // find the correct fire delay for this weapon
            select_index = 0;

            while ((pSelect[select_index].iId) &&
                   (pSelect[select_index].iId != iId))
               select_index++;

            pBot->f_shoot_time = gpGlobals->time + pSelect[select_index].fPrimaryRate;

            return TRUE;
         }
         else
         {
            pEdict->v.button |= IN_ATTACK;   // charge the weapon
            pBot->f_shoot_time = gpGlobals->time;  // keep charging

            return TRUE;
         }
      }

      // are we charging the secondary fire?
      if (pBot->f_secondary_charging > 0)
      {
         iId = pBot->charging_weapon_id;

         // is it time to fire the charged weapon?
         if (pBot->f_secondary_charging <= gpGlobals->time)
         {
            // we DON'T set pEdict->v.button here to release the
            // fire button which will fire the charged weapon

            pBot->f_secondary_charging = -1;  // -1 means not charging

            // find the correct fire delay for this weapon
            select_index = 0;

            while ((pSelect[select_index].iId) &&
                   (pSelect[select_index].iId != iId))
               select_index++;

            pBot->f_shoot_time = gpGlobals->time +  pSelect[select_index].fSecondaryRate;

            return TRUE;
         }
         else
         {
            pEdict->v.button |= IN_ATTACK2;  // charge the weapon
            pBot->f_shoot_time = gpGlobals->time;  // keep charging

            return TRUE;
         }
      }

      select_index = 0;

      // loop through all the weapons until terminator is found...
      while (pSelect[select_index].iId)
      {
         // was a weapon choice specified? (and if so do they NOT match?)
         if ((weapon_choice != 0) &&
             (weapon_choice != pSelect[select_index].iId))
         {
            select_index++;  // skip to next weapon
            continue;
         }

         // is the bot NOT carrying this weapon?
         if (!(pEdict->v.weapons & (1<<pSelect[select_index].iId)))
         {
            select_index++;  // skip to next weapon
            continue;
         }   

#if 0
         // is the bot NOT skilled enough to use this weapon?
         if ((pBot->bot_skill+1) > pSelect[select_index].skill_level)
         {
            select_index++;  // skip to next weapon
            continue;
         }
#endif

         // is the bot underwater and does this weapon NOT work under water?
         if ((pEdict->v.waterlevel == 3) &&
             !(pSelect[select_index].can_use_underwater))
         {
            select_index++;  // skip to next weapon
            continue;
         }

         use_percent = RANDOM_LONG(1, 100);

         // is use percent greater than weapon use percent?
         if (use_percent > pSelect[select_index].use_percent)
         {
            select_index++;  // skip to next weapon
            continue;
         }

         iId = pSelect[select_index].iId;
         use_primary = FALSE;
         use_secondary = FALSE;
         primary_percent = RANDOM_LONG(1, 100);

         // is primary percent less than weapon primary percent AND
         // no ammo required for this weapon OR
            // enough ammo available to fire AND
         // the bot is far enough away to use primary fire AND
         // the bot is close enough to the enemy to use primary fire

         if ((primary_percent <= pSelect[select_index].primary_fire_percent) &&
             ((weapon_defs[iId].iAmmo1 == -1) ||
              (pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] >=
               pSelect[select_index].min_primary_ammo)) &&
             (distance >= pSelect[select_index].primary_min_distance) &&
             (distance <= pSelect[select_index].primary_max_distance))
         {
            use_primary = TRUE;
         }

         // otherwise see if there is enough secondary ammo AND
         // the bot is far enough away to use secondary fire AND
         // the bot is close enough to the enemy to use secondary fire

         else if (((weapon_defs[iId].iAmmo2 == -1) ||
                   (pBot->m_rgAmmo[weapon_defs[iId].iAmmo2] >=
                    pSelect[select_index].min_secondary_ammo)) &&
                  (distance >= pSelect[select_index].secondary_min_distance) &&
                  (distance <= pSelect[select_index].secondary_max_distance))
         {
            use_secondary = TRUE;
         }

         // see if there wasn't enough ammo to fire the weapon...
         if ((use_primary == FALSE) && (use_secondary == FALSE))
         {
            select_index++;  // skip to next weapon
            continue;
         }

         // select this weapon if it isn't already selected
         if (pBot->current_weapon.iId != iId)
            UTIL_SelectItem(pEdict, pSelect[select_index].weapon_name);

         if (mod_id == TFC_DLL)
         {
            if (iId == TF_WEAPON_SNIPERRIFLE)
            {
               pBot->f_move_speed = 0;  // don't move while using sniper rifle

               if (pEdict->v.velocity.Length() > 50)
               {
                  return FALSE;  // don't press attack key until velocity is < 50
               }
            }

            if (pEdict->v.playerclass == TFC_CLASS_MEDIC)
            {
               int player_team = UTIL_GetTeam(pBot->pBotEnemy);
               int bot_team = UTIL_GetTeam(pEdict);

               // only heal your teammates or allies...
               if (((bot_team == player_team) ||
                    (team_allies[bot_team] & (1<<player_team))) &&
                   (iId != TF_WEAPON_MEDIKIT))
               {
                  return FALSE;  // don't "fire" unless weapon is medikit
               }
            }
         }

         if (use_primary)
         {
            pEdict->v.button |= IN_ATTACK;  // use primary attack

            if (pSelect[select_index].primary_fire_charge)
            {
               pBot->charging_weapon_id = iId;

               // release primary fire after the appropriate delay...
               pBot->f_primary_charging = gpGlobals->time +
                              pSelect[select_index].primary_charge_delay;

               pBot->f_shoot_time = gpGlobals->time;  // keep charging
            }
            else
            {
               // set next time to shoot
               if (pSelect[select_index].primary_fire_hold)
                  pBot->f_shoot_time = gpGlobals->time;  // don't let button up
               else
               {
                  pBot->f_shoot_time = gpGlobals->time + pSelect[select_index].fPrimaryRate;
               }
            }
         }
         else  // MUST be use_secondary...
         {
            pEdict->v.button |= IN_ATTACK2;  // use secondary attack

            if (pSelect[select_index].secondary_fire_charge)
            {
               pBot->charging_weapon_id = iId;

               // release secondary fire after the appropriate delay...
               pBot->f_secondary_charging = gpGlobals->time +
                              pSelect[select_index].secondary_charge_delay;

               pBot->f_shoot_time = gpGlobals->time;  // keep charging
            }
            else
            {
               // set next time to shoot
               if (pSelect[select_index].secondary_fire_hold)
                  pBot->f_shoot_time = gpGlobals->time;  // don't let button up
               else
               {
                  pBot->f_shoot_time = gpGlobals->time + pSelect[select_index].fSecondaryRate;
               }
            }
         }

         return TRUE;  // weapon was fired
      }
   }

   // didn't have any available weapons or ammo, return FALSE
   return FALSE;
}


void BotShootAtEnemy( bot_t *pBot )
{
   float f_distance;

   edict_t *pEdict = pBot->pEdict;

   // aim for the head and/or body
   Vector v_enemy = BotBodyTarget( pBot->pBotEnemy, pBot ) - GetGunPosition(pEdict);

   pEdict->v.v_angle = UTIL_VecToAngles( v_enemy );

   if (pEdict->v.v_angle.y > 180)
      pEdict->v.v_angle.y -=360;

   // Paulo-La-Frite - START bot aiming bug fix
   if (pEdict->v.v_angle.x > 180)
      pEdict->v.v_angle.x -=360;

   // set the body angles to point the gun correctly
   pEdict->v.angles.x = pEdict->v.v_angle.x / 3;
   pEdict->v.angles.y = pEdict->v.v_angle.y;
   pEdict->v.angles.z = 0;

   // adjust the view angle pitch to aim correctly (MUST be after body v.angles stuff)
   pEdict->v.v_angle.x = -pEdict->v.v_angle.x;
   // Paulo-La-Frite - END

   float x = pEdict->v.v_angle.y;
   if (x > 180) x -= 360;
   if (abs(pEdict->v.ideal_yaw - x) > 2.0)
      fp = NULL;

   pEdict->v.ideal_yaw = pEdict->v.v_angle.y;

   BotFixIdealYaw(pEdict);


   v_enemy.z = 0;  // ignore z component (up & down)

   f_distance = v_enemy.Length();  // how far away is the enemy scum?

   if (f_distance > 200)      // run if distance to enemy is far
      pBot->f_move_speed = pBot->f_max_speed;
   else if (f_distance > 20)  // walk if distance is closer
      pBot->f_move_speed = pBot->f_max_speed / 2;
   else                     // don't move if close enough
      pBot->f_move_speed = 0.0;


   // is it time to shoot yet?
   if (pBot->f_shoot_time <= gpGlobals->time)
   {
      // select the best weapon to use at this distance and fire...
      BotFireWeapon(v_enemy, pBot, 0);
   }
}


bool BotShootTripmine( bot_t *pBot )
{
   edict_t *pEdict = pBot->pEdict;

   if (pBot->b_shoot_tripmine != TRUE)
      return FALSE;

   // aim at the tripmine and fire the glock...

   Vector v_enemy = pBot->v_tripmine - GetGunPosition( pEdict );

   pEdict->v.v_angle = UTIL_VecToAngles( v_enemy );

   if (pEdict->v.v_angle.y > 180)
      pEdict->v.v_angle.y -=360;

   // Paulo-La-Frite - START bot aiming bug fix
   if (pEdict->v.v_angle.x > 180)
      pEdict->v.v_angle.x -=360;

   // set the body angles to point the gun correctly
   pEdict->v.angles.x = pEdict->v.v_angle.x / 3;
   pEdict->v.angles.y = pEdict->v.v_angle.y;
   pEdict->v.angles.z = 0;

   // adjust the view angle pitch to aim correctly (MUST be after body v.angles stuff)
   pEdict->v.v_angle.x = -pEdict->v.v_angle.x;
   // Paulo-La-Frite - END

   pEdict->v.ideal_yaw = pEdict->v.v_angle.y;

   BotFixIdealYaw(pEdict);

   return (BotFireWeapon( v_enemy, pBot, VALVE_WEAPON_GLOCK ));
}

