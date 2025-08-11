#include "core/SessionEnumerator.h"

#include <algorithm>
#include <vector>

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <wtsapi32.h>

using namespace std;

using namespace core;

vector<uint32_t> core::enumerateSessions() {
	vector<uint32_t> ids;
	PWTS_SESSION_INFOW p = nullptr;
	DWORD count = 0;

	if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &p, &count))
		return ids;

	ids.reserve(count);
	for (DWORD i = 0; i < count; i++)
		ids.push_back(p[i].SessionId);
	WTSFreeMemory(p);

	sort(ids.begin(), ids.end());
	ids.erase(unique(ids.begin(), ids.end()), ids.end());
	return ids;
}
