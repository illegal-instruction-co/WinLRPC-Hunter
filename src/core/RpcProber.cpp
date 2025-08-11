#include "core/RpcProber.h"
#include "core/RpcBinding.h"

using namespace std;

using namespace core;

struct UniqueIfIdVector final {
	RPC_IF_ID_VECTOR* v{nullptr};
	inline ~UniqueIfIdVector() {
		if (v)
			RpcIfIdVectorFree(&v);
	}

	inline operator RPC_IF_ID_VECTOR*() const noexcept {
		return v;
	}
};

IfHit core::probeWithAuthSet(const wstring& endpointName, const GUID& target, unsigned long timeoutMs, const vector<AuthAttempt>& attempts,
							 const IfVerPred& pred) noexcept {
	IfHit result;

	for (auto auth : attempts) {
		UniqueRpcBinding bind;
		RPC_STATUS brs = makeBindingNcalrpc(endpointName, timeoutMs, bind);
		if (brs != RPC_S_OK)
			continue;

		if (auth != AuthAttempt::NONE)
			if (setAuth(bind, auth) != RPC_S_OK)
				continue;

		UniqueIfIdVector ifs;
		RPC_STATUS rs = RpcMgmtInqIfIds(bind, &ifs.v);

		if (rs != RPC_S_OK || !ifs.v)
			continue;

		result.acceptedAuth.push_back(auth);

		for (unsigned long i = 0; i < ifs.v->Count; i++) {
			RPC_IF_ID* iid = ifs.v->IfId[i];
			if (!iid)
				continue;

			if (InlineIsEqualGUID(iid->Uuid, target))
				if (pred.match(iid->VersMajor, iid->VersMinor)) {
					result.ok = true;
					result.vMaj = iid->VersMajor;
					result.vMin = iid->VersMinor;
				}
		}
	}
	return result;
}
