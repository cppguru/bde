// bdls_filepermissions.cpp                                           -*-C++-*-
#include <bdls_filepermissions.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdls_filepermissions_cpp,"$Id$ $CSID$")

#include <bdlb_print.h>

#include <bslim_formatguard.h>

#include <bsl_ios.h>
#include <bsl_ostream.h>

namespace BloombergLP {
namespace bdls {

                          // ----------------------
                          // struct FilePermissions
                          // ----------------------

// CLASS METHODS
bsl::ostream& FilePermissions::print(bsl::ostream&         stream,
                                     FilePermissions::Enum value,
                                     int                   level,
                                     int                   spacesPerLevel)
{
    bdlb::Print::indent(stream, level, spacesPerLevel);

    {
        const bslim::FormatGuard guard(&stream);
        stream << bsl::oct << static_cast<int>(value);
    }

    if (spacesPerLevel >= 0) {
        stream << '\n';
    }

    return stream;
}

}  // close package namespace
}  // close enterprise namespace

// ----------------------------------------------------------------------------
// Copyright 2026 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
