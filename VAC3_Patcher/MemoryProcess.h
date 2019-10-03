#pragma once

#include <Windows.h>
#include <Tlhelp32.h>
#include <iostream>
#include <sstream>
#include <psapi.h>

using namespace std;

struct module
{
	DWORD Base;
	DWORD Size;
	DWORD End;
};

class ExternalProcess
{
public:
	HANDLE hProcess;
	DWORD dwProcessID;

	ExternalProcess()
	{
		hProcess = 0;
		dwProcessID = 0;
	}

	bool Open( PCHAR szProcName );
	bool ExecuteCode( DWORD dwAddress );

	PCHAR ReadString( DWORD dwAddress , int len );
	BOOL WriteString( DWORD dwAddress , PCHAR String );

	module GetModule( PCHAR szModuleName );

	DWORD AllocateMemory( int len );
	DWORD AllocateString( PCHAR Str );

	bool FreeAllocateMemory( LPVOID Memory , int len );
	bool WriteAllocateMemory( LPVOID lpMemoryAlloc , PBYTE Code , int len );

	DWORD FindPattern( PCHAR pattern , PCHAR mask , DWORD start , DWORD end , DWORD offset );

	void ShowLastError()
	{
		std::ostringstream error_code;
		DWORD dwErrorCode = GetLastError();
		error_code << dwErrorCode;
		MessageBoxA( 0 , error_code.str().c_str() , "Error" , MB_OK | MB_ICONWARNING );
	}

	template <class T>
	T ReadMemory( DWORD dwAddress )
	{
		T Value;
		BOOL isRead = ReadProcessMemory( hProcess , (LPCVOID)dwAddress , &Value , sizeof( T ) , 0 );
		if ( !isRead ) {
			ShowLastError();
			MessageBoxA( 0 , "Error ReadProcessMemory [ReadMemory]" , "Error" , MB_OK | MB_ICONERROR );
			return false;
		}
		return Value;
	}

	template <class T>
	void WriteMemory( DWORD dwAddress , T Value )
	{
		BOOL isRead = WriteProcessMemory( hProcess , (LPVOID)dwAddress , &Value , sizeof( T ) , 0 );

		if ( !isRead )
		{
			MessageBoxA( 0 , "Error WriteProcessMemory [WriteMemory]" , "Error" , MB_OK | MB_ICONERROR );
		}
	}

private:
	DWORD dwGetProcessID( PCHAR szProcName );
};