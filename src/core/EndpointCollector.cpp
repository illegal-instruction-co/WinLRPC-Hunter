#include "core/EndpointCollector.h"
#include "core/ObjectDirectory.h"
#include "core/SessionEnumerator.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using namespace core;

namespace {
inline void addGlobal(vector<EndpointInfo>& out) {
	vector<wstring> names;

	if (!enumObjectDirectoryAlpcPorts(L"\\RPC Control", names))
		return;

	for (const auto& n : names) {
		EndpointInfo e{};
		e.scope = L"Global";
		e.sessionId = static_cast<uint32_t>(-1);
		e.name = n;
		e.ntPath = L"\\RPC Control";
		out.push_back(move(e));
	}
}

inline void scanSession(uint32_t sid, vector<EndpointInfo>& out) {
	vector<wstring> names;

	wchar_t path[256];
	swprintf(path, 256, L"\\Sessions\\%u\\BaseNamedObjects\\RPC Control", sid);

	if (!enumObjectDirectoryAlpcPorts(path, names))
		return;

	for (const auto& n : names) {
		EndpointInfo e{};
		e.scope = L"Session";
		e.sessionId = sid;
		e.name = n;
		e.ntPath = path;
		out.push_back(move(e));
	}
}
} // namespace

void core::collectEndpoints(bool useSpecificSession, uint32_t specificSession, uint32_t currentSession, vector<EndpointInfo>& out) {
	out.clear();

	addGlobal(out);

	if (useSpecificSession)
		scanSession(specificSession, out);
	else {
		scanSession(currentSession, out);
		for (auto sid : enumerateSessions())
			if (sid != currentSession)
				scanSession(sid, out);
	}

	sort(out.begin(), out.end(), [](const EndpointInfo& a, const EndpointInfo& b) {
		if (a.scope != b.scope)
			return a.scope < b.scope;

		if (a.sessionId != b.sessionId)
			return a.sessionId < b.sessionId;

		return a.name < b.name;
	});

	out.erase(unique(out.begin(), out.end(),
					 [](const EndpointInfo& a, const EndpointInfo& b) { return a.scope == b.scope && a.sessionId == b.sessionId && a.name == b.name; }),
			  out.end());
}
