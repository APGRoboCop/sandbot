//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// h_export.cpp
//

#include "extdll.h"
#include "dllapi.h"
#include "h_export.h"
#include "meta_api.h"
#include "cbase.h"
#include "studio.h"

#include "bot.h"
#include "engine.h"
#include "dll.h"
#include "linkfunc.h"
#include "h_export.h"

#ifndef __linux__
#include <io.h>
#else
#include <sys/io.h>
#endif

#ifndef __linux__
HINSTANCE h_Library = NULL;
#else
void* h_Library = NULL;
#endif

int DispatchSpawn_Post( edict_t * pent );
const char *Cmd_Args( void );
const char *Cmd_Argv( int argc );
int Cmd_Argc( void );

enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;

char g_szLibraryPath[64];

extern DLL_FUNCTIONS other_gFunctionTable;

extern int mod_id;

extern bot_player_t *pBotData;


#ifndef __linux__
// required DLL entry point
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	if( fdwReason == DLL_PROCESS_DETACH )
	{
		if( h_Library )
		{
			FreeLibrary( h_Library );
		}
		if( pOrdinals )
		{
			delete[] pOrdinals;
		}
		if( pFunctionAddresses )
		{
			delete[] pFunctionAddresses;
		}
		if( pNameAddresses )
		{
			delete[] pNameAddresses;
		}
	}

	return TRUE;
}
#endif

extern "C" EXPORT int GetEngineFunctions( enginefuncs_t * pengfuncsFromEngine, int *interfaceVersion )
{
	if( g_bIsMMPlugin )
		memset( pengfuncsFromEngine, 0, sizeof( enginefuncs_t ) );

	// and now we need to pass engine functions table to the game DLL (in fact it's our own
	// functions we are passing here, but the game DLL won't notice)...

	pengfuncsFromEngine->pfnPrecacheModel = pfnPrecacheModel;
	pengfuncsFromEngine->pfnPrecacheSound = pfnPrecacheSound;
	pengfuncsFromEngine->pfnSetModel = pfnSetModel;
	pengfuncsFromEngine->pfnModelIndex = pfnModelIndex;
	pengfuncsFromEngine->pfnModelFrames = pfnModelFrames;
	pengfuncsFromEngine->pfnSetSize = pfnSetSize;
	pengfuncsFromEngine->pfnChangeLevel = pfnChangeLevel;
	pengfuncsFromEngine->pfnGetSpawnParms = pfnGetSpawnParms;
	pengfuncsFromEngine->pfnSaveSpawnParms = pfnSaveSpawnParms;
	pengfuncsFromEngine->pfnVecToYaw = pfnVecToYaw;
	pengfuncsFromEngine->pfnVecToAngles = pfnVecToAngles;
	pengfuncsFromEngine->pfnMoveToOrigin = pfnMoveToOrigin;
	pengfuncsFromEngine->pfnChangeYaw = pfnChangeYaw;
	pengfuncsFromEngine->pfnChangePitch = pfnChangePitch;
	pengfuncsFromEngine->pfnFindEntityByString = pfnFindEntityByString;
	pengfuncsFromEngine->pfnGetEntityIllum = pfnGetEntityIllum;
	pengfuncsFromEngine->pfnFindEntityInSphere = pfnFindEntityInSphere;
	pengfuncsFromEngine->pfnFindClientInPVS = pfnFindClientInPVS;
	pengfuncsFromEngine->pfnEntitiesInPVS = pfnEntitiesInPVS;
	pengfuncsFromEngine->pfnMakeVectors = pfnMakeVectors;
	pengfuncsFromEngine->pfnAngleVectors = pfnAngleVectors;
	pengfuncsFromEngine->pfnCreateEntity = pfnCreateEntity;
	pengfuncsFromEngine->pfnRemoveEntity = pfnRemoveEntity;
	pengfuncsFromEngine->pfnCreateNamedEntity = pfnCreateNamedEntity;
	pengfuncsFromEngine->pfnMakeStatic = pfnMakeStatic;
	pengfuncsFromEngine->pfnEntIsOnFloor = pfnEntIsOnFloor;
	pengfuncsFromEngine->pfnDropToFloor = pfnDropToFloor;
	pengfuncsFromEngine->pfnWalkMove = pfnWalkMove;
	pengfuncsFromEngine->pfnSetOrigin = pfnSetOrigin;
	pengfuncsFromEngine->pfnEmitSound = pfnEmitSound;
	pengfuncsFromEngine->pfnEmitAmbientSound = pfnEmitAmbientSound;
	pengfuncsFromEngine->pfnTraceLine = pfnTraceLine;
	pengfuncsFromEngine->pfnTraceToss = pfnTraceToss;
	pengfuncsFromEngine->pfnTraceMonsterHull = pfnTraceMonsterHull;
	pengfuncsFromEngine->pfnTraceHull = pfnTraceHull;
	pengfuncsFromEngine->pfnTraceModel = pfnTraceModel;
	pengfuncsFromEngine->pfnTraceTexture = pfnTraceTexture;
	pengfuncsFromEngine->pfnTraceSphere = pfnTraceSphere;
	pengfuncsFromEngine->pfnGetAimVector = pfnGetAimVector;
	pengfuncsFromEngine->pfnServerCommand = pfnServerCommand;
	pengfuncsFromEngine->pfnServerExecute = pfnServerExecute;
	pengfuncsFromEngine->pfnClientCommand = pfnClientCommand;
	pengfuncsFromEngine->pfnParticleEffect = pfnParticleEffect;
	pengfuncsFromEngine->pfnLightStyle = pfnLightStyle;
	pengfuncsFromEngine->pfnDecalIndex = pfnDecalIndex;
	pengfuncsFromEngine->pfnPointContents = pfnPointContents;
	pengfuncsFromEngine->pfnMessageBegin = pfnMessageBegin;
	pengfuncsFromEngine->pfnMessageEnd = pfnMessageEnd;
	pengfuncsFromEngine->pfnWriteByte = pfnWriteByte;
	pengfuncsFromEngine->pfnWriteChar = pfnWriteChar;
	pengfuncsFromEngine->pfnWriteShort = pfnWriteShort;
	pengfuncsFromEngine->pfnWriteLong = pfnWriteLong;
	pengfuncsFromEngine->pfnWriteAngle = pfnWriteAngle;
	pengfuncsFromEngine->pfnWriteCoord = pfnWriteCoord;
	pengfuncsFromEngine->pfnWriteString = pfnWriteString;
	pengfuncsFromEngine->pfnWriteEntity = pfnWriteEntity;
	pengfuncsFromEngine->pfnCVarRegister = pfnCVarRegister;
	pengfuncsFromEngine->pfnCVarGetFloat = pfnCVarGetFloat;
	pengfuncsFromEngine->pfnCVarGetString = pfnCVarGetString;
	pengfuncsFromEngine->pfnCVarSetFloat = pfnCVarSetFloat;
	pengfuncsFromEngine->pfnCVarSetString = pfnCVarSetString;
	pengfuncsFromEngine->pfnPvAllocEntPrivateData = pfnPvAllocEntPrivateData;
	pengfuncsFromEngine->pfnPvEntPrivateData = pfnPvEntPrivateData;
	pengfuncsFromEngine->pfnFreeEntPrivateData = pfnFreeEntPrivateData;
	pengfuncsFromEngine->pfnSzFromIndex = pfnSzFromIndex;
	pengfuncsFromEngine->pfnAllocString = pfnAllocString;
	pengfuncsFromEngine->pfnGetVarsOfEnt = pfnGetVarsOfEnt;
	pengfuncsFromEngine->pfnPEntityOfEntOffset = pfnPEntityOfEntOffset;
	pengfuncsFromEngine->pfnEntOffsetOfPEntity = pfnEntOffsetOfPEntity;
	pengfuncsFromEngine->pfnIndexOfEdict = pfnIndexOfEdict;
	pengfuncsFromEngine->pfnPEntityOfEntIndex = pfnPEntityOfEntIndex;
	pengfuncsFromEngine->pfnFindEntityByVars = pfnFindEntityByVars;
	pengfuncsFromEngine->pfnGetModelPtr = pfnGetModelPtr;
	pengfuncsFromEngine->pfnRegUserMsg = pfnRegUserMsg;
	pengfuncsFromEngine->pfnAnimationAutomove = pfnAnimationAutomove;
	pengfuncsFromEngine->pfnGetBonePosition = pfnGetBonePosition;
	pengfuncsFromEngine->pfnFunctionFromName = pfnFunctionFromName;
	pengfuncsFromEngine->pfnNameForFunction = pfnNameForFunction;
	pengfuncsFromEngine->pfnClientPrintf = pfnClientPrintf;
	pengfuncsFromEngine->pfnServerPrint = pfnServerPrint;
	pengfuncsFromEngine->pfnCmd_Args = Cmd_Args;
	pengfuncsFromEngine->pfnCmd_Argv = Cmd_Argv;
	pengfuncsFromEngine->pfnCmd_Argc = Cmd_Argc;
	pengfuncsFromEngine->pfnGetAttachment = pfnGetAttachment;
	pengfuncsFromEngine->pfnCRC32_Init = pfnCRC32_Init;
	pengfuncsFromEngine->pfnCRC32_ProcessBuffer = pfnCRC32_ProcessBuffer;
	pengfuncsFromEngine->pfnCRC32_ProcessByte = pfnCRC32_ProcessByte;
	pengfuncsFromEngine->pfnCRC32_Final = pfnCRC32_Final;
	pengfuncsFromEngine->pfnRandomLong = pfnRandomLong;
	pengfuncsFromEngine->pfnRandomFloat = pfnRandomFloat;
	pengfuncsFromEngine->pfnSetView = pfnSetView;
	pengfuncsFromEngine->pfnTime = pfnTime;
	pengfuncsFromEngine->pfnCrosshairAngle = pfnCrosshairAngle;
	pengfuncsFromEngine->pfnLoadFileForMe = pfnLoadFileForMe;
	pengfuncsFromEngine->pfnFreeFile = pfnFreeFile;
	pengfuncsFromEngine->pfnEndSection = pfnEndSection;
	pengfuncsFromEngine->pfnCompareFileTime = pfnCompareFileTime;
	pengfuncsFromEngine->pfnGetGameDir = pfnGetGameDir;
	pengfuncsFromEngine->pfnCvar_RegisterVariable = pfnCvar_RegisterVariable;
	pengfuncsFromEngine->pfnFadeClientVolume = pfnFadeClientVolume;
	pengfuncsFromEngine->pfnSetClientMaxspeed = pfnSetClientMaxspeed;
	pengfuncsFromEngine->pfnCreateFakeClient = pfnCreateFakeClient;
	pengfuncsFromEngine->pfnRunPlayerMove = pfnRunPlayerMove;
	pengfuncsFromEngine->pfnNumberOfEntities = pfnNumberOfEntities;
	pengfuncsFromEngine->pfnGetInfoKeyBuffer = pfnGetInfoKeyBuffer;
	pengfuncsFromEngine->pfnInfoKeyValue = pfnInfoKeyValue;
	pengfuncsFromEngine->pfnSetKeyValue = pfnSetKeyValue;
	pengfuncsFromEngine->pfnSetClientKeyValue = pfnSetClientKeyValue;
	pengfuncsFromEngine->pfnIsMapValid = pfnIsMapValid;
	pengfuncsFromEngine->pfnStaticDecal = pfnStaticDecal;
	pengfuncsFromEngine->pfnPrecacheGeneric = pfnPrecacheGeneric;
	pengfuncsFromEngine->pfnGetPlayerUserId = pfnGetPlayerUserId;
	pengfuncsFromEngine->pfnBuildSoundMsg = pfnBuildSoundMsg;
	pengfuncsFromEngine->pfnIsDedicatedServer = pfnIsDedicatedServer;
	pengfuncsFromEngine->pfnCVarGetPointer = pfnCVarGetPointer;
	pengfuncsFromEngine->pfnGetPlayerWONId = pfnGetPlayerWONId;
	pengfuncsFromEngine->pfnInfo_RemoveKey = pfnInfo_RemoveKey;
	pengfuncsFromEngine->pfnGetPhysicsKeyValue = pfnGetPhysicsKeyValue;
	pengfuncsFromEngine->pfnSetPhysicsKeyValue = pfnSetPhysicsKeyValue;
	pengfuncsFromEngine->pfnGetPhysicsInfoString = pfnGetPhysicsInfoString;
	pengfuncsFromEngine->pfnPrecacheEvent = pfnPrecacheEvent;
	pengfuncsFromEngine->pfnPlaybackEvent = pfnPlaybackEvent;
	pengfuncsFromEngine->pfnSetFatPVS = pfnSetFatPVS;
	pengfuncsFromEngine->pfnSetFatPAS = pfnSetFatPAS;
	pengfuncsFromEngine->pfnCheckVisibility = pfnCheckVisibility;
	pengfuncsFromEngine->pfnDeltaSetField = pfnDeltaSetField;
	pengfuncsFromEngine->pfnDeltaUnsetField = pfnDeltaUnsetField;
	pengfuncsFromEngine->pfnDeltaAddEncoder = pfnDeltaAddEncoder;
	pengfuncsFromEngine->pfnGetCurrentPlayer = pfnGetCurrentPlayer;
	pengfuncsFromEngine->pfnCanSkipPlayer = pfnCanSkipPlayer;
	pengfuncsFromEngine->pfnDeltaFindField = pfnDeltaFindField;
	pengfuncsFromEngine->pfnDeltaSetFieldByIndex = pfnDeltaSetFieldByIndex;
	pengfuncsFromEngine->pfnDeltaUnsetFieldByIndex = pfnDeltaUnsetFieldByIndex;
	pengfuncsFromEngine->pfnSetGroupMask = pfnSetGroupMask;
	pengfuncsFromEngine->pfnCreateInstancedBaseline = pfnCreateInstancedBaseline;
	pengfuncsFromEngine->pfnCvar_DirectSet = pfnCvar_DirectSet;
	pengfuncsFromEngine->pfnForceUnmodified = pfnForceUnmodified;
	pengfuncsFromEngine->pfnGetPlayerStats = pfnGetPlayerStats;
	pengfuncsFromEngine->pfnAddServerCommand = pfnAddServerCommand;
	pengfuncsFromEngine->pfnVoice_GetClientListening = pfnVoice_GetClientListening;
	pengfuncsFromEngine->pfnVoice_SetClientListening = pfnVoice_SetClientListening;
	pengfuncsFromEngine->pfnGetPlayerAuthId = pfnGetPlayerAuthId;
	pengfuncsFromEngine->pfnSequenceGet = pfnSequenceGet;
	pengfuncsFromEngine->pfnSequencePickSentence = pfnSequencePickSentence;
	pengfuncsFromEngine->pfnGetFileSize = pfnGetFileSize;
	pengfuncsFromEngine->pfnGetApproxWavePlayLen = pfnGetApproxWavePlayLen;
	pengfuncsFromEngine->pfnIsCareerMatch = pfnIsCareerMatch;
	pengfuncsFromEngine->pfnGetLocalizedStringLength = pfnGetLocalizedStringLength;
	pengfuncsFromEngine->pfnRegisterTutorMessageShown = pfnRegisterTutorMessageShown;
	pengfuncsFromEngine->pfnGetTimesTutorMessageShown = pfnGetTimesTutorMessageShown;
	pengfuncsFromEngine->ProcessTutorMessageDecayBuffer = pfnProcessTutorMessageDecayBuffer;
	pengfuncsFromEngine->ConstructTutorMessageDecayBuffer = pfnConstructTutorMessageDecayBuffer;
	pengfuncsFromEngine->ResetTutorMessageDecayData = pfnResetTutorMessageDecayData;

	return TRUE;
}

#ifndef __linux__
void WINAPI GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
#else
extern "C" void GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
#endif
{
	// get the engine functions from the engine...
	memcpy( &g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t) );
	// get the globals from the engine
	gpGlobals = pGlobals;

	// find the directory name of the currently running mod
	// this returns just the mod directory's name: http://metamod.org/engine_notes.html#GetGameDir
	char game_dir[256];
	char *szLibraryPath = "";
	GET_GAME_DIR(game_dir);

	if (strcmpi(game_dir, "valve") == 0)
	{
		mod_id = VALVE_DLL;

#ifndef __linux__
		szLibraryPath = "valve/dlls/hl.dll";
#else
		szLibraryPath = "valve/dlls/hl.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "bshift") == 0)
	{
		mod_id = BSHIFT_DLL;

#ifndef __linux__
		szLibraryPath = "bshift/dlls/hl.dll";
#else
		szLibraryPath = "bshift/dlls/bshift.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "gearbox") == 0)
	{
		mod_id = GEARBOX_DLL;

#ifndef __linux__
		szLibraryPath = "gearbox/dlls/opfor.dll";
#else
		szLibraryPath = "gearbox/dlls/opfor.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "decay") == 0)
	{
		mod_id = DECAY_DLL;

#ifndef __linux__
		szLibraryPath = "decay/dlls/decay.dll";
#else
		szLibraryPath = "decay/dlls/decay_i386.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "cstrike") == 0)
	{
		mod_id = CSTRIKE_DLL;

#ifndef __linux__
		szLibraryPath = "cstrike/dlls/mp.dll";
#else
		szLibraryPath = "cstrike/dlls/cs.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "czero") == 0)
	{
		mod_id = CZERO_DLL;

#ifndef __linux__
		szLibraryPath = "czero/dlls/mp.dll";
#else
		szLibraryPath = "czero/dlls/cs.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "czeror") == 0)
	{
		mod_id = CZEROR_DLL;

#ifndef __linux__
		szLibraryPath = "czeror/dlls/cz.dll";
#else
		szLibraryPath = "czeror/dlls/cz.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "dod") == 0)
	{
		mod_id = DOD_DLL;

#ifndef __linux__
		szLibraryPath = "dod/dlls/dod.dll";
#else
		szLibraryPath = "dod/dlls/dod.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "tfc") == 0)
	{
		mod_id = TFC_DLL;

#ifndef __linux__
		szLibraryPath = "tfc/dlls/tfc.dll";
#else
		szLibraryPath = "tfc/dlls/tfc.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "rewolf") == 0)
	{
		mod_id = REWOLF_DLL;

#ifndef __linux__
		szLibraryPath = "rewolf/dlls/gunman.dll";
#else
		szLibraryPath = nullptr;
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = nullptr;
#endif
		}
	}
	else if (strcmpi(game_dir, "hunger") == 0)
	{
		mod_id = HUNGER_DLL;

#ifndef __linux__
		szLibraryPath = "hunger/dlls/einar.dll";
#else
		szLibraryPath = nullptr;
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = nullptr;
#endif
		}
	}
	else if (strcmpi(game_dir, "ns") == 0)
	{
		mod_id = NS_DLL;

#ifndef __linux__
		szLibraryPath = "ns/dlls/ns.dll";
#else
		szLibraryPath = "ns/dlls/ns_i386.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}
	else if (strcmpi(game_dir, "ship") == 0)
	{
		mod_id = SHIP_DLL;

#ifndef __linux__
		szLibraryPath = "ship/dlls/ship.dll";
#else
		szLibraryPath = "ship/dlls/ship_i386.so";
#endif

		if( !g_bIsMMPlugin )
		{
#ifndef __linux__
			h_Library = LoadLibrary( szLibraryPath );
#else
			h_Library = dlopen( szLibraryPath, RTLD_NOW );
#endif
		}
	}

	strncpy( g_szLibraryPath, szLibraryPath, strlen( szLibraryPath ) );

	if( !g_bIsMMPlugin && h_Library == NULL )
	{
		ALERT( at_error, "Library not found or not supported!" );
	}

	extern bot_player_t g_valveBots[];
	extern bot_player_t g_gearboxBots[];
	extern bot_player_t g_dodBots[];
	extern bot_player_t g_gunmanBots[];
	extern bot_player_t g_nsBots[];
	extern bot_player_t g_hungerBots[];
	extern bot_player_t g_shipBots[];

	if( mod_id == VALVE_DLL || mod_id == TFC_DLL )
	{
		pBotData = g_valveBots;
	}
	else if( mod_id == GEARBOX_DLL )
	{
		pBotData = g_gearboxBots;
	}
	else if( mod_id == DOD_DLL )
	{
		pBotData = g_dodBots;
	}
	else if( mod_id == REWOLF_DLL )
	{
		pBotData = g_gunmanBots;
	}
	else if( mod_id == NS_DLL )
	{
		pBotData = g_nsBots;
	}
	else if( mod_id == HUNGER_DLL )
	{
		pBotData = g_hungerBots;
	}
	else if( mod_id == SHIP_DLL )
	{
		pBotData = g_shipBots;
	}

	LoadExtraExports();

	if( g_bIsMMPlugin )
	{
		return;
	}

	GetEngineFunctions( pengfuncsFromEngine, NULL );

	// give the engine functions to the other DLL...
	(*(GIVEFNPTRSTODLL)GetProcAddress( h_Library, "GiveFnptrsToDll" ))( pengfuncsFromEngine, pGlobals );

	// finished, interfacing from gamedll to engine complete
	return;
}

gamedll_funcs_t gGameDLLFunc;

extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
   // check if engine's pointer is valid and version is correct...
   if ((pFunctionTable == NULL) || (interfaceVersion != INTERFACE_VERSION))
      return FALSE;

   memset( pFunctionTable, 0, sizeof( DLL_FUNCTIONS ) );

   if( !g_bIsMMPlugin )
   {
	   // pass other DLLs engine callbacks to function table...
	   if( !(*(GETENTITYAPI)GetProcAddress( h_Library, "GetEntityAPI" )) (&other_gFunctionTable, INTERFACE_VERSION) )
		   return FALSE;  // error initializing function table!!!

	   gGameDLLFunc.dllapi_table = &other_gFunctionTable;
	   gpGamedllFuncs = &gGameDLLFunc;

	   memcpy( pFunctionTable, &other_gFunctionTable, sizeof( DLL_FUNCTIONS ) );
   }

   // pass gamedll functions table to engine (in fact it's our own functions we are passing
   // here, but the engine won't notice)...

   pFunctionTable->pfnGameInit = GameDLLInit;
   pFunctionTable->pfnSpawn = DispatchSpawn;
   pFunctionTable->pfnThink = DispatchThink;
   pFunctionTable->pfnUse = DispatchUse;
   pFunctionTable->pfnTouch = DispatchTouch;
   pFunctionTable->pfnBlocked = DispatchBlocked;
   pFunctionTable->pfnKeyValue = DispatchKeyValue;
   pFunctionTable->pfnSave = DispatchSave;
   pFunctionTable->pfnRestore = DispatchRestore;
   pFunctionTable->pfnSetAbsBox = DispatchObjectCollisionBox;
   pFunctionTable->pfnSaveWriteFields = SaveWriteFields;
   pFunctionTable->pfnSaveReadFields = SaveReadFields;
   pFunctionTable->pfnSaveGlobalState = SaveGlobalState;
   pFunctionTable->pfnRestoreGlobalState = RestoreGlobalState;
   pFunctionTable->pfnResetGlobalState = ResetGlobalState;
   pFunctionTable->pfnClientConnect = ClientConnect;
   pFunctionTable->pfnClientDisconnect = ClientDisconnect;
   pFunctionTable->pfnClientKill = ClientKill;
   pFunctionTable->pfnClientPutInServer = ClientPutInServer;
   pFunctionTable->pfnClientCommand = ClientCommand;
   pFunctionTable->pfnClientUserInfoChanged = ClientUserInfoChanged;
   pFunctionTable->pfnServerActivate = ServerActivate;
   pFunctionTable->pfnServerDeactivate = ServerDeactivate;
   pFunctionTable->pfnPlayerPreThink = PlayerPreThink;
   pFunctionTable->pfnPlayerPostThink = PlayerPostThink;
   pFunctionTable->pfnStartFrame = StartFrame;
   pFunctionTable->pfnParmsNewLevel = ParmsNewLevel;
   pFunctionTable->pfnParmsChangeLevel = ParmsChangeLevel;
   pFunctionTable->pfnGetGameDescription = GetGameDescription;
   pFunctionTable->pfnPlayerCustomization = PlayerCustomization;
   pFunctionTable->pfnSpectatorConnect = SpectatorConnect;
   pFunctionTable->pfnSpectatorDisconnect = SpectatorDisconnect;
   pFunctionTable->pfnSpectatorThink = SpectatorThink;
   pFunctionTable->pfnSys_Error = Sys_Error;
   pFunctionTable->pfnPM_Move = PM_Move;
   pFunctionTable->pfnPM_Init = PM_Init;
   pFunctionTable->pfnPM_FindTextureType = PM_FindTextureType;
   pFunctionTable->pfnSetupVisibility = SetupVisibility;
   pFunctionTable->pfnUpdateClientData = UpdateClientData;
   pFunctionTable->pfnAddToFullPack = AddToFullPack;
   pFunctionTable->pfnCreateBaseline = CreateBaseline;
   pFunctionTable->pfnRegisterEncoders = RegisterEncoders;
   pFunctionTable->pfnGetWeaponData = GetWeaponData;
   pFunctionTable->pfnCmdStart = CmdStart;
   pFunctionTable->pfnCmdEnd = CmdEnd;
   pFunctionTable->pfnConnectionlessPacket = ConnectionlessPacket;
   pFunctionTable->pfnGetHullBounds = GetHullBounds;
   pFunctionTable->pfnCreateInstancedBaselines = CreateInstancedBaselines;
   pFunctionTable->pfnInconsistentFile = InconsistentFile;
   pFunctionTable->pfnAllowLagCompensation = AllowLagCompensation;

   return TRUE; // finished, interfacing from engine to gamedll complete
}

extern "C" EXPORT int GetEntityAPI_Post( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	memset( pFunctionTable, 0, sizeof( DLL_FUNCTIONS ) );

	pFunctionTable->pfnSpawn = DispatchSpawn_Post;

	return (TRUE);
}

extern "C" EXPORT int GetNewDLLFunctions( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion )
{
   if( !g_bIsMMPlugin )
   {
	   static GETNEWDLLFUNCTIONS other_GetNewDLLFunctions = NULL;
	   static bool missing = FALSE;

	   // if the new DLL functions interface has been formerly reported as missing, give up
	   if( missing )
		   return FALSE;

	   // do we NOT know if the new DLL functions interface is provided ? if so, look for its address
	   if( other_GetNewDLLFunctions == NULL )
		   other_GetNewDLLFunctions = (GETNEWDLLFUNCTIONS)GetProcAddress( h_Library, "GetNewDLLFunctions" );

	   // have we NOT found it ?
	   if( other_GetNewDLLFunctions == NULL )
	   {
		   missing = TRUE; // then mark it as missing, no use to look for it again in the future
		   return FALSE; // and give up
	   }

	   gGameDLLFunc.newapi_table = pFunctionTable;

	   // else call the function that provides the new DLL functions interface on request
	   return (!(*other_GetNewDLLFunctions) (pFunctionTable, interfaceVersion));
   }

   return TRUE;
}

extern "C" EXPORT int Server_GetBlendingInterface( int version, struct sv_blending_interface_s **ppinterface, struct engine_studio_api_s *pstudio, float (*rotationmatrix)[3][4], float (*bonetransform)[MAXSTUDIOBONES][3][4] )
{
	static SERVER_GETBLENDINGINTERFACE pServer_GetBlendingInterface = nullptr;
	static bool bIsMissing = false;

	// if the game .dll has no Server_GetBlendingInterface exported
	if( bIsMissing )
	{
		return 0;
	}

	// if the server blending function is unknown
	if( !pServer_GetBlendingInterface )
	{
		pServer_GetBlendingInterface = (SERVER_GETBLENDINGINTERFACE)GetProcAddress(h_Library, "Server_GetBlendingInterface");
	}

	// if it wasn't found
	if( !pServer_GetBlendingInterface )
	{
		bIsMissing = true;
		return 0;
	}

	return (pServer_GetBlendingInterface)(version, ppinterface, pstudio, rotationmatrix, bonetransform);
}

extern "C" EXPORT void SV_SaveGameComment( char *pBuffer, int maxLength )
{
	static SV_SAVEGAMECOMMENT pSV_SaveGameComment = nullptr;
	static bool bIsMissing = false;

	// TODO: does GameUI.dll localise these names?
	if( mod_id == VALVE_DLL )
	{
		if( !strncmp( STRING( gpGlobals->mapname ), "t0a0", strlen( "t0a0" ) ) ) // a, b, b1, b2, c, d
		{
			strncpy( pBuffer, "HAZARD COURSE", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c0a0", strlen( "c0a0" ) ) ) // a, b, c, d, e
		{
			strncpy( pBuffer, "BLACK MESA INBOUND", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0" ) )
		{
			strncpy( pBuffer, "ANOMOLOUS MATERIALS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0a" ) )
		{
			strncpy( pBuffer, "ANOMOLOUS MATERIALS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0b" ) )
		{
			strncpy( pBuffer, "ANOMOLOUS MATERIALS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0c" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0d" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a0e" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1a" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1b" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1c" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1d" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c1a1f" ) )
		{
			strncpy( pBuffer, "UNFORSEEN CONSEQUENCES", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c1a2", strlen( "c1a2" ) ) ) // a, b, c, d
		{
			strncpy( pBuffer, "OFFICE COMPLEX", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c1a3", strlen( "c1a3" ) ) ) // a, b, c, d
		{
			strncpy( pBuffer, "\"WE'VE GOT HOSTILES\"", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c1a4", strlen( "c1a4" ) ) ) // b, d, e, f, g, i, j, k
		{
			strncpy( pBuffer, "BLAST PIT", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c2a1", strlen( "c2a1" ) ) ) // a, b
		{
			strncpy( pBuffer, "POWER UP", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c2a2" , strlen( "c2a2" ) ) ) // a, b1, b2, c, d, e, f, g, h
		{
			strncpy( pBuffer, "ON A RAIL", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3a" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3b" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3c" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3d" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a3e" ) )
		{
			strncpy( pBuffer, "APPREHENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4" ) )
		{
			strncpy( pBuffer, "RESIDUE PROCESSING", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4a" ) )
		{
			strncpy( pBuffer, "RESIDUE PROCESSING", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4b" ) )
		{
			strncpy( pBuffer, "RESIDUE PROCESSING", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4c" ) )
		{
			strncpy( pBuffer, "RESIDUE PROCESSING", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4d" ) )
		{
			strncpy( pBuffer, "QUESTIONABLE ETHICS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4e" ) )
		{
			strncpy( pBuffer, "QUESTIONABLE ETHICS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4f" ) )
		{
			strncpy( pBuffer, "QUESTIONABLE ETHICS", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c2a4g" ) )
		{
			strncpy( pBuffer, "QUESTIONABLE ETHICS", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c2a5", strlen( "c2a5" ) ) ) // a, b, c, d, e, f, g, w, x
		{
			strncpy( pBuffer, "SURFACE TENSION", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a1" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a1a" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a1b" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2a" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2b" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2c" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2d" ) )
		{
			strncpy( pBuffer, "\"FORGET ABOUT FREEMAN\"", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2e" ) )
		{
			strncpy( pBuffer, "LAMBDA CORE", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c3a2f" ) )
		{
			strncpy( pBuffer, "LAMBDA CORE", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1" ) )
		{
			strncpy( pBuffer, "XEN", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1a" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1b" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1c" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1d" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1e" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a1f" ) )
		{
			strncpy( pBuffer, "INTERLOPER", maxLength );
		}
		else if( !strncmp( STRING( gpGlobals->mapname ), "c4a2", strlen( "c4a2" ) ) ) // a, b
		{
			strncpy( pBuffer, "GONARCH'S LAIR", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c4a3" ) )
		{
			strncpy( pBuffer, "NIHILANTH", maxLength );
		}
		else if( !strcmp( STRING( gpGlobals->mapname ), "c5a1" ) )
		{
			strncpy( pBuffer, "NIHILANTH", maxLength );
		}
		else
		{
			strncpy( pBuffer, STRING( gpGlobals->mapname ), maxLength );
		}

		return;
	}
	else if( mod_id == GEARBOX_DLL )
	{
		return;
	}
	else if( mod_id == BSHIFT_DLL )
	{
		return;
	}

	// if the game .dll has no SV_SaveGameComment exported
	if( bIsMissing )
	{
		// go with the mapname as a sensible default
		strncpy( pBuffer, STRING(gpGlobals->mapname), maxLength );
		// don't look again
		return;
	}

	// if the save game comment function is unknown
	if( !pSV_SaveGameComment )
	{
		pSV_SaveGameComment = (SV_SAVEGAMECOMMENT)GetProcAddress(h_Library, "SV_SaveGameComment");
	}

	// if it wasn't found
	if( !pSV_SaveGameComment )
	{
		// mark it as missing
		bIsMissing = true;

		return;
	}

	(pSV_SaveGameComment)(pBuffer, maxLength);
}
