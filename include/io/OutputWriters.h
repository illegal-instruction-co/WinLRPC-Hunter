#pragma once

#include "core/Types.h"

#include <string>

#include <rpc.h>

namespace io {

using GUID = ::GUID;

void writeTableHeader(size_t);
void writeTableRow(const core::EndpointInfo&, const core::IfHit&, size_t);
void writeJsonLine(const core::EndpointInfo&, const core::IfHit&, const GUID&);

} // namespace io
