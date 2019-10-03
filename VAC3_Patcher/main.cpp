#include "MemoryProcess.h"

#define PatternString "\x74\x47\x6A\x01\x6A\x00\xFF\x76\x18\xE8\x00\x00\x00\x00\x83\xC4\x0C\x89\x46\x08"
#define Mask "xxxxxxxxxx????xxxxxx"

void Color( int color ) //12 - red, 10 - green, 16 - white
{
	HANDLE  hConsole;
	hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	FlushConsoleInputBuffer( hConsole );
	SetConsoleTextAttribute( hConsole , color );
}

/*
55 8B EC 83 EC 28 53 56 8B 75 08 8B D9
C7 46 ? ? ? ? ? 32 C0 5E 5B 8B E5 5D C2 08 00 F6 45 0C 02
55 8B EC FF 35 ? ? ? ? 68 ? ? ? ? FF 75 0C FF 75 08 E8 ? ? ? ? 83 C4 10 5D C3 PreCheckVacModule (ModuleHandle,ModuleSize)
steamservice.dll+0x2A84C = 116(0x74) change to 235 (0xEB)
*/

void GetPrivileges()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;

	if ( OpenProcessToken( GetCurrentProcess() , TOKEN_ALL_ACCESS_P , &hToken ) )
	{
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		tp.Privileges[0].Luid.LowPart = 20;
		tp.Privileges[0].Luid.HighPart = 0;

		AdjustTokenPrivileges( hToken , FALSE , &tp , 0 , NULL , NULL );
		CloseHandle( hToken );
	}
}

int main( int argc , char** argv )
{
	DWORD Pattern = 0;
	module SteamService = { 0 };
	ExternalProcess Steam;

	char* PatternFunction = PatternString;

	Color( 10 );
	printf( "[VAC3 load by LoadLibrary for dump] by _or_75\n\n" );

	GetPrivileges();
	
	do
	{
		if ( Steam.Open( "SteamService.exe" ) )
			SteamService = Steam.GetModule( "SteamService.dll" );

		Sleep( 300 );
	} while ( !SteamService.End );

	printf( "[SteamService: 0x%X / 0x%X\n" , SteamService.Base , SteamService.End );

	Pattern = Steam.FindPattern( PatternFunction , Mask , SteamService.Base , SteamService.End , 0 );

	if ( !Pattern )
	{
		PatternFunction[0] = 0xEB;
		Pattern = Steam.FindPattern( PatternFunction , Mask , SteamService.Base , SteamService.End , 0 );
	}

	if ( Pattern )
	{
		BYTE JE = Steam.ReadMemory<BYTE>( Pattern );

		if ( JE == 0x74 )
		{
			printf( "[JE] Pattern: 0x%X\n" , Pattern );
			printf( "Press key to patch -> jmp\n" );
			getchar();
			Steam.WriteMemory<BYTE>( Pattern , 0xEB );
			printf( "[VAC BY LOAD_DLL] Patch OK\n" );
		}
		else if ( JE == 0xEB )
		{
			printf( "[JMP] Pattern: 0x%X\n" , Pattern );
			printf( "Press key to patch -> jz\n" );
			getchar();
			Steam.WriteMemory<BYTE>( Pattern , 0x74 );
			printf( "[VAC BY MANUAL_DLL] Patch OK\n" );
		}

		printf( "[ALL] Patch OK\n" );
	}
	else
	{
		Color( 12 );
		printf( "Find pattern error !\n" );
	}

	getchar();
	return 0;
}