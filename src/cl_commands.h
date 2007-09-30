//-----------------------------------------------------------------------------
//
// Skulltag Source
// Copyright (C) 2003 Brad Carney
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
//
//
// Filename: cl_commands.h
//
// Description: Contains prototypes for a set of functions that correspond to each
// message a client can send out. Each functions handles the send out of each message.
//
//-----------------------------------------------------------------------------

#ifndef __CL_COMMANDS_H__
#define __CL_COMMANDS_H__

#include "doomtype.h"
#include "a_pickups.h"

//*****************************************************************************
//	PROTOTYPES

void	CLIENTCOMMANDS_UserInfo( ULONG ulFlags );
void	CLIENTCOMMANDS_StartChat( void );
void	CLIENTCOMMANDS_EndChat( void );
void	CLIENTCOMMANDS_Say( ULONG ulMode, const char *pszString );
void	CLIENTCOMMANDS_ClientMove( void );
void	CLIENTCOMMANDS_MissingPacket( void );
void	CLIENTCOMMANDS_Pong( ULONG ulTime );
void	CLIENTCOMMANDS_WeaponSelect( const char *pszWeapon );
void	CLIENTCOMMANDS_Taunt( void );
void	CLIENTCOMMANDS_Spectate( void );
void	CLIENTCOMMANDS_RequestJoin( char *pszJoinPassword );
void	CLIENTCOMMANDS_RequestRCON( char *pszRCONPassword );
void	CLIENTCOMMANDS_RCONCommand( char *pszCommand );
void	CLIENTCOMMANDS_Suicide( void );
void	CLIENTCOMMANDS_ChangeTeam( char *pszJoinPassword, LONG lDesiredTeam );
void	CLIENTCOMMANDS_SpectateInfo( void );
void	CLIENTCOMMANDS_GenericCheat( LONG lCheat );
void	CLIENTCOMMANDS_GiveCheat( char *pszItem, LONG lAmount );
void	CLIENTCOMMANDS_SummonCheat( char *pszItem, bool bFriend = false );
void	CLIENTCOMMANDS_ReadyToGoOn( void );
void	CLIENTCOMMANDS_ChangeDisplayPlayer( LONG lDisplayPlayer );
void	CLIENTCOMMANDS_AuthenticateLevel( void );
void	CLIENTCOMMANDS_CallVote( LONG lVoteCommand, char *pszArgument );
void	CLIENTCOMMANDS_VoteYes( void );
void	CLIENTCOMMANDS_VoteNo( void );
void	CLIENTCOMMANDS_RequestInventoryUseAll( void );
void	CLIENTCOMMANDS_RequestInventoryUse( AInventory *item );

#endif	// __CL_COMMANDS_H__
