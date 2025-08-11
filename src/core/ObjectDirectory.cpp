#include "core/ObjectDirectory.h"

#include <algorithm>
#include <string>
#include <vector>

#define UNICODE
#define _UNICODE
#include <windows.h>

using namespace std;

using namespace core;

extern "C"
{

	typedef LONG NTSTATUS;

	typedef struct _UNICODE_STRING {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

	typedef struct _OBJECT_ATTRIBUTES {
		ULONG Length;
		HANDLE RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG Attributes;
		PVOID SecurityDescriptor;
		PVOID SecurityQualityOfService;
	} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

	typedef struct _OBJECT_DIRECTORY_INFORMATION {
		UNICODE_STRING Name;
		UNICODE_STRING TypeName;
	} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

	__declspec(dllimport) NTSTATUS
		__stdcall NtOpenDirectoryObject(_Out_ PHANDLE DirectoryHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes);

	__declspec(dllimport) NTSTATUS
		__stdcall NtQueryDirectoryObject(_In_ HANDLE DirectoryHandle, _Out_writes_bytes_opt_(Length) PVOID Buffer, _In_ ULONG Length,
										 _In_ BOOLEAN ReturnSingleEntry, _In_ BOOLEAN RestartScan, _Inout_ PULONG Context, _Out_opt_ PULONG ReturnLength);
}

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef OBJ_CASE_INSENSITIVE
#define OBJ_CASE_INSENSITIVE 0x00000040L
#endif
#ifndef DIRECTORY_QUERY
#define DIRECTORY_QUERY 0x0001
#endif

namespace {
struct UniqueHandle {
	HANDLE h{nullptr};
	~UniqueHandle() {
		if (h)
			CloseHandle(h);
	}
	UniqueHandle() = default;
	UniqueHandle(const UniqueHandle&) = delete;
	UniqueHandle& operator=(const UniqueHandle&) = delete;
	UniqueHandle(UniqueHandle&& o) noexcept : h(exchange(o.h, nullptr)) {}
	UniqueHandle& operator=(UniqueHandle&& o) noexcept {
		if (this != &o) {
			if (h)
				CloseHandle(h);
			h = exchange(o.h, nullptr);
		}
		return *this;
	}
	operator HANDLE() const noexcept {
		return h;
	}
};

inline void initUnicode(UNICODE_STRING& us, const wchar_t* s) noexcept {
	if (!s) {
		us = {};
		return;
	}
	size_t len = wcslen(s);
	us.Buffer = const_cast<PWSTR>(s);
	us.Length = static_cast<USHORT>(len * sizeof(wchar_t));
	us.MaximumLength = us.Length;
}

inline void initObjectAttributes(OBJECT_ATTRIBUTES& oa, UNICODE_STRING& name) noexcept {
	oa.Length = sizeof(oa);
	oa.RootDirectory = nullptr;
	oa.ObjectName = &name;
	oa.Attributes = OBJ_CASE_INSENSITIVE;
	oa.SecurityDescriptor = nullptr;
	oa.SecurityQualityOfService = nullptr;
}

inline bool isAlpcType(const UNICODE_STRING& typeName) noexcept {
	if (!typeName.Buffer || typeName.Length == 0)
		return false;

	wstring t(typeName.Buffer, typeName.Length / sizeof(wchar_t));

	return _wcsicmp(t.c_str(), L"ALPC Port") == 0;
}
} // namespace

bool core::enumObjectDirectoryAlpcPorts(const wstring& ntPath, vector<wstring>& out) {
	out.clear();

	UNICODE_STRING name{};
	initUnicode(name, ntPath.c_str());
	OBJECT_ATTRIBUTES oa{};
	initObjectAttributes(oa, name);

	UniqueHandle dir;
	if (!NT_SUCCESS(NtOpenDirectoryObject(&dir.h, DIRECTORY_QUERY, &oa)))
		return false;

	constexpr ULONG kBufSize = 64 * 1024;
	vector<unsigned char> buf(kBufSize);
	ULONG ctx = 0;
	BOOLEAN restart = TRUE;

	for (;;) {
		ULONG retLen = 0;
		NTSTATUS st = NtQueryDirectoryObject(dir, buf.data(), kBufSize, FALSE, restart, &ctx, &retLen);
		restart = FALSE;
		if (!NT_SUCCESS(st))
			break;

		auto* cur = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(buf.data());
		while (cur->Name.Buffer || cur->TypeName.Buffer) {
			if (isAlpcType(cur->TypeName) && cur->Name.Buffer && cur->Name.Length)
				out.emplace_back(cur->Name.Buffer, cur->Name.Length / sizeof(wchar_t));
			++cur;
		}
	}

	sort(out.begin(), out.end());
	out.erase(unique(out.begin(), out.end()), out.end());
	return true;
}
