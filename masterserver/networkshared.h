//-----------------------------------------------------------------------------
//
// Skulltag Source
// Copyright (C) 2007 Brad Carney, Benjamin Berkels
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
// Filename: networkshared.h
//
// Description: Contains network related code shared between
// Skulltag and the master server.
//
//-----------------------------------------------------------------------------

#ifndef __NETWORKSHARED_H__
#define __NETWORKSHARED_H__

// [Petteri] Check if compiling for Win32:
#if defined(__WINDOWS__) || defined(__NT__) || defined(_MSC_VER) || defined(_WIN32)
#	define __WIN32__
#endif
// Follow #ifdef __WIN32__ marks

#include <stdio.h>
#ifdef	WIN32
#include <conio.h>
#endif

// [Petteri] Use Winsock for Win32:
#ifdef __WIN32__
#	define WIN32_LEAN_AND_MEAN
// [BB] We have to use the Windows DWORD.
#	define USE_WINDOWS_DWORD
#	include <windows.h>
// [BB] Include winsock2.h instead of winsock.h
#	include <winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <errno.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/ioctl.h>
#endif

#ifndef __WIN32__
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket close
#define ioctlsocket ioctl
#define Sleep(x)        usleep (x * 1000)
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <ctype.h>
#include <math.h>

#include "../src/doomtype.h"

//*****************************************************************************
//	DEFINES

// Maximum size of the packets sent out by the server.
#define	MAX_UDP_PACKET				8192

//*****************************************************************************
typedef struct
{
   BYTE    ip[4];
   unsigned short  port;
   unsigned short  pad;
} netadr_t;

typedef struct sizebuf_s
{
	bool	allowoverflow;	// if false, do a Com_Error
	bool	overflowed;		// set to true if the buffer size failed

	// Unfortunaly, ZDaemon uses two different definitions of sizebuf_t. Attempt
	// to combine the structures here by having two sets of data.
	// Servers use this.
	BYTE	*pbData;

	// Clients use this this.
	BYTE	bData[MAX_UDP_PACKET];

	int		maxsize;
	int		cursize;
	int		readcount;

} sizebuf_t;

typedef struct
{
	// The IP address that is banned in char form. Can be a number or a wildcard.
	char		szIP[4][4];

	// Comment regarding the banned address.
	char		szComment[128];

} IPAddress_t;

bool	NETWORK_StringToAddress( char *pszString, netadr_t *pAddress );
void	NETWORK_SocketAddressToNetAddress( struct sockaddr_in *s, netadr_t *a );
bool	NETWORK_StringToIP( char *pszAddress, char *pszIP0, char *pszIP1, char *pszIP2, char *pszIP3 );

class IPFileParser{
	const unsigned int _listLength;
	char _errorMessage[1024];
public:
	IPFileParser ( const int IPListLength )
		: _listLength(IPListLength)
	{
		_errorMessage[0] = '\0';
	}
		
	const char* getErrorMessage()
	{
		return _errorMessage;
	}
	bool parseIPList( const char* FileName, IPAddress_t* IPArray ){
		FILE			*pFile;
		unsigned long	ulIdx;

		for ( ulIdx = 0; ulIdx < _listLength; ulIdx++ )
		{
			sprintf( IPArray[ulIdx].szIP[0], "0" );
			sprintf( IPArray[ulIdx].szIP[1], "0" );
			sprintf( IPArray[ulIdx].szIP[2], "0" );
			sprintf( IPArray[ulIdx].szIP[3], "0" );

			IPArray[ulIdx].szComment[0] = 0;
		}

		char curChar = 0;
		long curIPIdx = 0;
		if (( pFile = fopen( FileName, "r" )) != NULL )
		{
			while ( true )
			{
				bool parsingDone = !parseNextLine( pFile, IPArray[curIPIdx], curIPIdx );

				if ( _errorMessage[0] != '\0' )
				{
					fclose( pFile );
					return false;
				}

				if ( parsingDone == true )
					break;
			}
		}
		else
		{
			sprintf( _errorMessage, "WARNING! Could not open %s!\n", FileName );
			return false;
		}

		fclose( pFile );
		return true;
	}
private:
	char skipWhitespace( FILE *pFile )
	{
		char curChar = fgetc( pFile );
		while (( curChar == ' ' ) && ( curChar != -1 ))
			curChar = fgetc( pFile );

		return ( curChar );
	}
	char skipComment( FILE *pFile )
	{
		char curChar = fgetc( pFile );
		while (( curChar != '\r' ) && ( curChar != '\n' ) && ( curChar != -1 ))
			curChar = fgetc( pFile );

		return ( curChar );
	}
	bool parseNextLine( FILE *pFile, IPAddress_t &IP, LONG &BanIdx )
	{
		netadr_t	IPAddress;
		char		szIP[257];
		int		lPosition;

		lPosition = 0;
		szIP[0] = 0;

		char curChar = fgetc( pFile );

		// Skip whitespace.
		if ( curChar == ' ' )
		{
			curChar = skipWhitespace( pFile );

			if ( feof( pFile ))
			{
				fclose( pFile );
				return ( false );
			}
		}

		while ( 1 )
		{
			if ( curChar == '\r' || curChar == '\n' || curChar == ':' || curChar == '/' || curChar == -1 )
			{
				if ( lPosition > 0 )
				{
					if ( NETWORK_StringToIP( szIP, IP.szIP[0], IP.szIP[1], IP.szIP[2], IP.szIP[3] ))
					{
						if ( BanIdx == _listLength )
						{
							sprintf( _errorMessage, "parseNextLine: WARNING! Maximum number of IPs (%d) exceeded!\n", _listLength );
							return ( false );
						}

						BanIdx++;
						return ( true );
					}
					else if ( NETWORK_StringToAddress( szIP, &IPAddress ))
					{
						if ( BanIdx == _listLength )
						{
							sprintf( _errorMessage, "parseNextLine: WARNING! Maximum number of IPs (%d) exceeded!\n", _listLength );
							return ( false );
						}

						_itoa( IPAddress.ip[0], IP.szIP[0], 10 );
						_itoa( IPAddress.ip[1], IP.szIP[1], 10 );
						_itoa( IPAddress.ip[2], IP.szIP[2], 10 );
						_itoa( IPAddress.ip[3], IP.szIP[3], 10 );
						BanIdx++;
						return ( true );
					}
					else
					{
						IP.szIP[0][0] = 0;
						IP.szIP[1][0] = 0;
						IP.szIP[2][0] = 0;
						IP.szIP[3][0] = 0;
					}
				}

				if ( feof( pFile ))
				{
					fclose( pFile );
					return ( false );
				}
				// If we've hit a comment, skip until the end of the line (or the end of the file) and get out.
				else if ( curChar == ':' || curChar == '/' )
				{
					skipComment( pFile );
					return ( true );
				}
				else
					return ( true );
			}

			szIP[lPosition++] = curChar;
			szIP[lPosition] = 0;

			if ( lPosition == 256 )
			{
				fclose( pFile );
				return ( false );
			}

			curChar = fgetc( pFile );
		}
	}
};

#endif	// __NETWORKSHARED_H__
