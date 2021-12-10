#pragma once

#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <Siv3D/Windows/Windows.hpp>
#include <fileapi.h>
#include <string>
#include <regex>

#include <winternl.h>
#include <ntstatus.h>

#include <RestartManager.h>
#include <winerror.h>
#include <atlstr.h>
#include <Psapi.h>

#include <sysinfoapi.h>

typedef
__kernel_entry NTSYSCALLAPI
NTSTATUS
(NTAPI
* NtQueryObject_t)(
	_In_opt_ HANDLE Handle,
	_In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
	_Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
	_In_ ULONG ObjectInformationLength,
	_Out_opt_ PULONG ReturnLength
);

typedef
__kernel_entry NTSTATUS
(NTAPI
* NtQuerySystemInformation_t)(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
NtQueryObject_t fpNtQueryObject = (NtQueryObject_t)GetProcAddress(hNtDll, "NtQueryObject");
NtQuerySystemInformation_t fpNtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(hNtDll, "NtQuerySystemInformation");

typedef struct {
	PVOID Object;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR HandleValue;
	ULONG GrantedAccess;
	USHORT CreatorBackTraceIndex;
	USHORT ObjectTypeIndex;
	ULONG HansldAttributes;
	ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct {
	ULONG HandleCount;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, * PSYSTEM_HANDLE_INFORMATION_EX;

static const int SystemExtendedHandleInformation = 0x40;
