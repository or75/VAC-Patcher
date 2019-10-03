#include "MemoryProcess.h"

bool ExternalProcess::Open( PCHAR szProcName )
{
	dwProcessID = dwGetProcessID( szProcName );

	if ( !dwProcessID ) {
		//MessageBoxA( 0 , "Error dwGetProcessID" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}

	hProcess = OpenProcess( PROCESS_ALL_ACCESS , false , dwProcessID );

	if ( hProcess == INVALID_HANDLE_VALUE ) {
		//MessageBoxA( 0 , "Error OpenProcess" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}

	return true;
}

bool ExternalProcess::ExecuteCode( DWORD dwAddress )
{
	HANDLE hThread = CreateRemoteThread( hProcess , 0 , 0 , (LPTHREAD_START_ROUTINE)dwAddress , 0 , 0 , NULL );

	if ( !hThread ) {
		MessageBoxA( 0 , "Error CreateRemoteThread [ExecuteCode]" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}
	else
	{
		WaitForSingleObject( hThread , INFINITE );
		return true;
	}
}

PCHAR ExternalProcess::ReadString( DWORD dwAddress , int len )
{
	char StringDat[256];
	ReadProcessMemory( hProcess , (LPCVOID)dwAddress , StringDat , len , 0 );
	return &StringDat[0];
}

BOOL ExternalProcess::WriteString( DWORD dwAddress , PCHAR str )
{
	if ( WriteAllocateMemory( (LPVOID)dwAddress , (PBYTE)str , strlen( str ) ) )
		return true;
	else
		return false;
}

DWORD ExternalProcess::dwGetProcessID( PCHAR szProcName )
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS , 0 );
	DWORD dwGetProcessID = 0;
	if ( hSnapshot != INVALID_HANDLE_VALUE )
	{
		PROCESSENTRY32 ProcEntry32 = { 0 };
		ProcEntry32.dwSize = sizeof( MODULEENTRY32 );
		if ( Process32First( hSnapshot , &ProcEntry32 ) )
		{
			do
			{
				if ( strcmp( ProcEntry32.szExeFile , szProcName ) == 0 )
				{
					dwGetProcessID = (DWORD)ProcEntry32.th32ProcessID;
					break;
				}
			} while ( Process32Next( hSnapshot , &ProcEntry32 ) );
		}
		CloseHandle( hSnapshot );
	}
	return dwGetProcessID;
}

module ExternalProcess::GetModule( PCHAR szModuleName )
{
	module mod;
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE , dwProcessID );

	if ( hSnapshot != INVALID_HANDLE_VALUE )
	{
		MODULEENTRY32 ModuleEntry32 = { 0 };
		ModuleEntry32.dwSize = sizeof( MODULEENTRY32 );
		if ( Module32First( hSnapshot , &ModuleEntry32 ) )
		{
			do
			{
				if ( strcmp( ModuleEntry32.szModule , szModuleName ) == 0 )
				{
					mod.Base = (DWORD)ModuleEntry32.modBaseAddr;
					mod.Size = ModuleEntry32.modBaseSize;
					mod.End = mod.Base + mod.Size;
					break;
				}
			} while ( Module32Next( hSnapshot , &ModuleEntry32 ) );
		}
		CloseHandle( hSnapshot );
	}
	else
		MessageBoxA( 0 , "Error dwGetModuleBaseAddress" , "Error" , MB_OK | MB_ICONERROR );

	return mod;
}

DWORD ExternalProcess::AllocateMemory( int len )
{
	LPVOID lpMemoryAlloc = VirtualAllocEx( hProcess , NULL , len , MEM_COMMIT , PAGE_EXECUTE_READWRITE );

	if ( !lpMemoryAlloc ) {
		MessageBoxA( 0 , "Error VirtualAllocEx [AllocateMemory]" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}

	return (DWORD)lpMemoryAlloc;
}

DWORD ExternalProcess::AllocateString( PCHAR Str )
{
	DWORD dwMemoryAlloc = AllocateMemory( strlen( Str ) );

	if ( WriteString( dwMemoryAlloc , Str ) )
		return dwMemoryAlloc;

	return false;
}

bool ExternalProcess::FreeAllocateMemory( LPVOID lpMemoryAlloc , int len )
{
	if ( !VirtualFreeEx( hProcess , lpMemoryAlloc , len , MEM_DECOMMIT ) )
	{
		MessageBoxA( 0 , "Error VirtualFreeEx [FreeAllocateMemory]" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}

	return true;
}

bool ExternalProcess::WriteAllocateMemory( LPVOID lpMemoryAlloc , PBYTE Code , int len )
{
	if ( !WriteProcessMemory( hProcess , lpMemoryAlloc , Code , len , NULL ) )
	{
		MessageBoxA( 0 , "Error WriteProcessMemory [WriteAllocateMemory]" , "Error" , MB_OK | MB_ICONERROR );
		return false;
	}

	return true;
}

DWORD ExternalProcess::FindPattern( PCHAR pattern , PCHAR mask , DWORD start , DWORD end , DWORD offset )
{
	int patternLength = strlen( mask );
	bool found = false;

	for ( DWORD i = start; i < end - patternLength; i++ )
	{
		found = true;
		for ( int idx = 0; idx < patternLength; idx++ )
		{
			if ( mask[idx] == 'x' && pattern[idx] != ReadMemory<CHAR>( i + idx ) )
			{
				found = false;
				break;
			}
		}
		if ( found )
		{
			return i + offset;
		}
	}

	return 0;
}