//-----------------------------------------------------------------------------
//
// Skulltag Source
// Copyright (C) 2007 Brad Carney
// Copyright (C) 2007-2012 Skulltag Development Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the Skulltag Development Team nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
// 4. Redistributions in any form must be accompanied by information on how to
//    obtain complete source code for the software and any accompanying
//    software that uses the software. The source code must either be included
//    in the distribution or be available for no more than the cost of
//    distribution plus a nominal fee, and must be freely redistributable
//    under reasonable conditions. For an executable file, complete source
//    code means the source code for all modules it contains. It does not
//    include source code for modules or files that typically accompany the
//    major components of the operating system on which the executable file
//    runs.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Date created:  7/12/07
//
//
// Filename: gamemode.cpp
//
// Description: 
//
//-----------------------------------------------------------------------------

#include "cooperative.h"
#include "deathmatch.h"
#include "doomstat.h"
#include "gamemode.h"
#include "team.h"
#include "network.h"
#include "sv_commands.h"
#include "g_game.h"
#include "joinqueue.h"
#include "cl_demo.h"
#include "survival.h"
// [BB] The next includes are only needed for GAMEMODE_DisplayStandardMessage
#include "sbar.h"
#include "v_video.h"

//*****************************************************************************
//	CONSOLE VARIABLES

CVAR( Bool, instagib, false, CVAR_SERVERINFO | CVAR_LATCH | CVAR_CAMPAIGNLOCK );
CVAR( Bool, buckshot, false, CVAR_SERVERINFO | CVAR_LATCH | CVAR_CAMPAIGNLOCK );

CVAR( Bool, sv_suddendeath, true, CVAR_SERVERINFO | CVAR_LATCH );

//*****************************************************************************
//	VARIABLES

// Data for all our game modes.
static	GAMEMODE_s				g_GameModes[NUM_GAMEMODES];

// Our current game mode.
static	GAMEMODE_e				g_CurrentGameMode;

//*****************************************************************************
//	FUNCTIONS

void GAMEMODE_Construct( void )
{
	// Regular co-op.
	g_GameModes[GAMEMODE_COOPERATIVE].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS;
	strncpy( g_GameModes[GAMEMODE_COOPERATIVE].szShortName, "CO-OP", 8 );
	strncpy( g_GameModes[GAMEMODE_COOPERATIVE].szF1Texture, "F1_COOP", 8 );

	// Survival co-op.
	g_GameModes[GAMEMODE_SURVIVAL].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_USEMAXLIVES;
	strncpy( g_GameModes[GAMEMODE_SURVIVAL].szShortName, "SURV", 8 );
	strncpy( g_GameModes[GAMEMODE_SURVIVAL].szF1Texture, "F1_SCP", 8 );

	// Invasion.
	g_GameModes[GAMEMODE_INVASION].ulFlags = GMF_COOPERATIVE|GMF_PLAYERSEARNKILLS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_USEMAXLIVES;
	strncpy( g_GameModes[GAMEMODE_INVASION].szShortName, "INVAS", 8 );
	strncpy( g_GameModes[GAMEMODE_INVASION].szF1Texture, "F1_INV", 8 );

	// Regular deathmatch.
	g_GameModes[GAMEMODE_DEATHMATCH].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS;
	strncpy( g_GameModes[GAMEMODE_DEATHMATCH].szShortName, "DM", 8 );
	strncpy( g_GameModes[GAMEMODE_DEATHMATCH].szF1Texture, "F1_DM", 8 );

	// Teamplay DM.
	g_GameModes[GAMEMODE_TEAMPLAY].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_TEAMPLAY].szShortName, "TDM", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMPLAY].szF1Texture, "F1_TDM", 8 );

	// Duel.
	g_GameModes[GAMEMODE_DUEL].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS|GMF_MAPRESETS;
	strncpy( g_GameModes[GAMEMODE_DUEL].szShortName, "DUEL", 8 );
	strncpy( g_GameModes[GAMEMODE_DUEL].szF1Texture, "F1_DUEL", 8 );

	// Terminator DM.
	g_GameModes[GAMEMODE_TERMINATOR].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNFRAGS;
	strncpy( g_GameModes[GAMEMODE_TERMINATOR].szShortName, "TERM", 8 );
	strncpy( g_GameModes[GAMEMODE_TERMINATOR].szF1Texture, "F1_TERM", 8 );

	// Last man standing.
	g_GameModes[GAMEMODE_LASTMANSTANDING].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNWINS|GMF_DONTSPAWNMAPTHINGS|GMF_MAPRESETS|GMF_DEADSPECTATORS;
	strncpy( g_GameModes[GAMEMODE_LASTMANSTANDING].szShortName, "LMS", 8 );
	strncpy( g_GameModes[GAMEMODE_LASTMANSTANDING].szF1Texture, "F1_LMS", 8 );

	// Team LMS.
	g_GameModes[GAMEMODE_TEAMLMS].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNWINS|GMF_DONTSPAWNMAPTHINGS|GMF_MAPRESETS|GMF_DEADSPECTATORS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_TEAMLMS].szShortName, "TM LMS", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMLMS].szF1Texture, "F1_TLMS", 8 );

	// Possession DM.
	g_GameModes[GAMEMODE_POSSESSION].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNPOINTS;
	strncpy( g_GameModes[GAMEMODE_POSSESSION].szShortName, "POSS", 8 );
	strncpy( g_GameModes[GAMEMODE_POSSESSION].szF1Texture, "F1_POSS", 8 );

	// Team possession.
	g_GameModes[GAMEMODE_TEAMPOSSESSION].ulFlags = GMF_DEATHMATCH|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_TEAMPOSSESSION].szShortName, "TM POSS", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMPOSSESSION].szF1Texture, "F1_TPOSS", 8 );

	// Regular teamgame.
	g_GameModes[GAMEMODE_TEAMGAME].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_TEAMGAME].szShortName, "TM GAME", 8 );
	strncpy( g_GameModes[GAMEMODE_TEAMGAME].szF1Texture, "F1_POSS", 8 );

	// Capture the flag.
	g_GameModes[GAMEMODE_CTF].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_CTF].szShortName, "CTF", 8 );
	strncpy( g_GameModes[GAMEMODE_CTF].szF1Texture, "F1_CTF", 8 );

	// One flag CTF.
	g_GameModes[GAMEMODE_ONEFLAGCTF].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_ONEFLAGCTF].szShortName, "OFCTF", 8 );
	strncpy( g_GameModes[GAMEMODE_ONEFLAGCTF].szF1Texture, "F1_1FCTF", 8 );

	// Skulltag.
	g_GameModes[GAMEMODE_SKULLTAG].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szShortName, "ST", 8 );
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szF1Texture, "F1_ST", 8 );

	// Domination
	g_GameModes[GAMEMODE_DOMINATION].ulFlags = GMF_TEAMGAME|GMF_PLAYERSEARNPOINTS|GMF_PLAYERSONTEAMS;
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szShortName, "DOM", 8 );
	strncpy( g_GameModes[GAMEMODE_SKULLTAG].szF1Texture, "F1_DOM", 8 );

	// Our default game mode is co-op.
	g_CurrentGameMode = GAMEMODE_COOPERATIVE;
}

//*****************************************************************************
//
ULONG GAMEMODE_GetFlags( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( 0 );

	return ( g_GameModes[GameMode].ulFlags );
}

//*****************************************************************************
//
char *GAMEMODE_GetShortName( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( NULL );

	return ( g_GameModes[GameMode].szShortName );
}

//*****************************************************************************
//
char *GAMEMODE_GetF1Texture( GAMEMODE_e GameMode )
{
	if ( GameMode >= NUM_GAMEMODES )
		return ( NULL );

	return ( g_GameModes[GameMode].szF1Texture );
}

//*****************************************************************************
//
void GAMEMODE_DetermineGameMode( void )
{
	g_CurrentGameMode = GAMEMODE_COOPERATIVE;
	if ( survival )
		g_CurrentGameMode = GAMEMODE_SURVIVAL;
	if ( invasion )
		g_CurrentGameMode = GAMEMODE_INVASION;
	if ( deathmatch )
		g_CurrentGameMode = GAMEMODE_DEATHMATCH;
	if ( teamplay )
		g_CurrentGameMode = GAMEMODE_TEAMPLAY;
	if ( duel )
		g_CurrentGameMode = GAMEMODE_DUEL;
	if ( terminator )
		g_CurrentGameMode = GAMEMODE_TERMINATOR;
	if ( lastmanstanding )
		g_CurrentGameMode = GAMEMODE_LASTMANSTANDING;
	if ( teamlms )
		g_CurrentGameMode = GAMEMODE_TEAMLMS;
	if ( possession )
		g_CurrentGameMode = GAMEMODE_POSSESSION;
	if ( teampossession )
		g_CurrentGameMode = GAMEMODE_TEAMPOSSESSION;
	if ( teamgame )
		g_CurrentGameMode = GAMEMODE_TEAMGAME;
	if ( ctf )
		g_CurrentGameMode = GAMEMODE_CTF;
	if ( oneflagctf )
		g_CurrentGameMode = GAMEMODE_ONEFLAGCTF;
	if ( skulltag )
		g_CurrentGameMode = GAMEMODE_SKULLTAG;
	if ( domination )
		g_CurrentGameMode = GAMEMODE_DOMINATION;
}

//*****************************************************************************
//
void GAMEMODE_RespawnDeadSpectatorsAndPopQueue( void )
{
	// [BB] This is server side.
	if (( NETWORK_GetState( ) == NETSTATE_CLIENT ) ||
		( CLIENTDEMO_IsPlaying( )))
	{
		return;
	}

	// Respawn any players who were downed during the previous round.
	for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
	{
		if (( playeringame[ulIdx] == false ) ||
			( PLAYER_IsTrueSpectator( &players[ulIdx] )))
		{
			continue;
		}

		// We don't want to respawn players as soon as the map starts; we only
		// want to respawn dead spectators.
		if (( players[ulIdx].mo ) &&
			( players[ulIdx].mo->health > 0 ) &&
			( players[ulIdx].bDeadSpectator == false ))
		{
			continue;
		}

		players[ulIdx].bSpectating = false;
		players[ulIdx].bDeadSpectator = false;
		if ( sv_maxlives > 0 && ( GAMEMODE_GetFlags( GAMEMODE_GetCurrentMode( )) & GMF_USEMAXLIVES ) )
		{
			players[ulIdx].ulLivesLeft = sv_maxlives - 1;
		}
		players[ulIdx].playerstate = PST_REBORNNOINVENTORY;

		if (( players[ulIdx].mo ) && ( players[ulIdx].mo->health > 0 ))
		{
			if ( NETWORK_GetState( ) == NETSTATE_SERVER )
				SERVERCOMMANDS_DestroyThing( players[ulIdx].mo );

			players[ulIdx].mo->Destroy( );
			players[ulIdx].mo = NULL;
		}

		G_CooperativeSpawnPlayer( ulIdx, true );
	}

	// Let anyone who's been waiting in line join now.
	JOINQUEUE_PopQueue( -1 );
}

//*****************************************************************************
//
void GAMEMODE_RespawnAllPlayers( void )
{
	// [BB] This is server side.
	if (( NETWORK_GetState( ) != NETSTATE_CLIENT ) &&
		( CLIENTDEMO_IsPlaying( ) == false ))
	{
		// Respawn the players.
		for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
		{
			if (( playeringame[ulIdx] == false ) ||
				( PLAYER_IsTrueSpectator( &players[ulIdx] )))
			{
				continue;
			}

			if ( players[ulIdx].mo )
			{
				if ( NETWORK_GetState( ) == NETSTATE_SERVER )
					SERVERCOMMANDS_DestroyThing( players[ulIdx].mo );

				players[ulIdx].mo->Destroy( );
				players[ulIdx].mo = NULL;
			}

			players[ulIdx].playerstate = PST_ENTER;
			G_CooperativeSpawnPlayer( ulIdx, true );
		}
	}
}

//*****************************************************************************
//
void GAMEMODE_ResetPlayersKillCount( const bool bInformClients )
{
	// Reset everyone's kill count.
	for ( ULONG ulIdx = 0; ulIdx < MAXPLAYERS; ulIdx++ )
	{
		players[ulIdx].killcount = 0;
		players[ulIdx].ulRailgunShots = 0;
		// [BB] Also reset the things for DF2_AWARD_DAMAGE_INSTEAD_KILLS.
		players[ulIdx].lPointCount = 0;
		players[ulIdx].ulUnrewardedDamageDealt = 0;

		// [BB] Notify the clients about the killcount change.
		if ( playeringame[ulIdx] && bInformClients && (NETWORK_GetState() == NETSTATE_SERVER) )
		{
			SERVERCOMMANDS_SetPlayerKillCount ( ulIdx );
			SERVERCOMMANDS_SetPlayerPoints ( ulIdx );
		}
	}
}

//*****************************************************************************
//
void GAMEMODE_DisplayStandardMessage( const char *pszMessage )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut	*pMsg;

		// Display "%s WINS!" HUD message.
		pMsg = new DHUDMessageFadeOut( BigFont, pszMessage,
			160.4f,
			75.0f,
			320,
			200,
			CR_RED,
			3.0f,
			2.0f );

		StatusBar->AttachMessage( pMsg, MAKE_ID('C','N','T','R') );
	}
}

//*****************************************************************************
// [BB] Expects pszMessage already to be colorized with V_ColorizeString.
void GAMEMODE_DisplayCNTRMessage( const char *pszMessage, const bool bInformClients, const ULONG ulPlayerExtra, const ULONG ulFlags )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut *pMsg = new DHUDMessageFadeOut( BigFont, pszMessage,
			1.5f,
			TEAM_MESSAGE_Y_AXIS,
			0,
			0,
			CR_UNTRANSLATED,
			3.0f,
			0.25f );
		StatusBar->AttachMessage( pMsg, MAKE_ID( 'C','N','T','R' ));
	}
	// If necessary, send it to clients.
	else if ( bInformClients )
	{
		SERVERCOMMANDS_PrintHUDMessageFadeOut( pszMessage, 1.5f, TEAM_MESSAGE_Y_AXIS, 0, 0, CR_UNTRANSLATED, 3.0f, 0.25f, "BigFont", false, MAKE_ID('C','N','T','R'), ulPlayerExtra, ulFlags );
	}
}

//*****************************************************************************
// [BB] Expects pszMessage already to be colorized with V_ColorizeString.
void GAMEMODE_DisplaySUBSMessage( const char *pszMessage, const bool bInformClients, const ULONG ulPlayerExtra, const ULONG ulFlags )
{
	if ( NETWORK_GetState( ) != NETSTATE_SERVER )
	{
		DHUDMessageFadeOut *pMsg = new DHUDMessageFadeOut( SmallFont, pszMessage,
			1.5f,
			TEAM_MESSAGE_Y_AXIS_SUB,
			0,
			0,
			CR_UNTRANSLATED,
			3.0f,
			0.25f );
		StatusBar->AttachMessage( pMsg, MAKE_ID( 'S','U','B','S' ));
	}
	// If necessary, send it to clients.
	else if ( bInformClients )
	{
		SERVERCOMMANDS_PrintHUDMessageFadeOut( pszMessage, 1.5f, TEAM_MESSAGE_Y_AXIS_SUB, 0, 0, CR_UNTRANSLATED, 3.0f, 0.25f, "SmallFont", false, MAKE_ID( 'S','U','B','S' ), ulPlayerExtra, ulFlags );
	}
}

//*****************************************************************************
//
GAMEMODE_e GAMEMODE_GetCurrentMode( void )
{
	return ( g_CurrentGameMode );
}

//*****************************************************************************
//
void GAMEMODE_SetCurrentMode( GAMEMODE_e GameMode )
{
	UCVarValue	 Val;
	g_CurrentGameMode = GameMode;	
	
	// [RC] Set all the CVars. We can't just use "= true;" because of the latched cvars.
	// (Hopefully Blzut's update will save us from this garbage.)

	Val.Bool = false;
	deathmatch.ForceSet( Val, CVAR_Bool );
	teamgame.ForceSet( Val, CVAR_Bool );
	instagib.ForceSet( Val, CVAR_Bool );
	buckshot.ForceSet( Val, CVAR_Bool );

	Val.Bool = true;
	switch ( GameMode )
	{
	case GAMEMODE_COOPERATIVE:

		cooperative.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_SURVIVAL:

		survival.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_INVASION:

		invasion.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DEATHMATCH:

		deathmatch.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMPLAY:

		teamplay.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DUEL:

		duel.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TERMINATOR:

		terminator.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_LASTMANSTANDING:

		lastmanstanding.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMLMS:

		teamlms.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_POSSESSION:

		possession.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMPOSSESSION:

		teampossession.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_TEAMGAME:

		teamgame.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_CTF:

		ctf.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_ONEFLAGCTF:

		oneflagctf.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_SKULLTAG:

		skulltag.ForceSet( Val, CVAR_Bool );
		break;
	case GAMEMODE_DOMINATION:

		domination.ForceSet( Val, CVAR_Bool );
		break;
	default:
		break;
	}
}

//*****************************************************************************
//
MODIFIER_e GAMEMODE_GetModifier( void )
{
	if ( instagib )
		return MODIFIER_INSTAGIB;
	else if ( buckshot )
		return MODIFIER_BUCKSHOT;
	else
		return MODIFIER_NONE;
}

//*****************************************************************************
//
void GAMEMODE_SetModifier( MODIFIER_e Modifier )
{
	UCVarValue	 Val;

	// Turn them all off.
	Val.Bool = false;
	instagib.ForceSet( Val, CVAR_Bool );
	buckshot.ForceSet( Val, CVAR_Bool );

	// Turn the selected one on.
	Val.Bool = true;
	switch ( Modifier )
	{
	case MODIFIER_INSTAGIB:
		instagib.ForceSet( Val, CVAR_Bool );
		break;
	case MODIFIER_BUCKSHOT:
		buckshot.ForceSet( Val, CVAR_Bool );
		break;
	default:
		break;
	}
}
