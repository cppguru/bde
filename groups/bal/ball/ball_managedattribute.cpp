// ball_managedattribute.cpp                                          -*-C++-*-
#include <ball_managedattribute.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(ball_managedattribute_cpp,"$Id$ $CSID$")

#include <bslim_printer.h>

#include <bsl_ostream.h>

namespace BloombergLP {
namespace ball {

                        // ----------------------
                        // class ManagedAttribute
                        // ----------------------

// ACCESSORS
bsl::ostream& ManagedAttribute::print(bsl::ostream& stream,
                                      int           level,
                                      int           spacesPerLevel) const
{
    d_attribute.print(stream, level, spacesPerLevel);
    return stream;
}

}  // close package namespace

// FREE OPERATORS
bsl::ostream& ball::operator<<(bsl::ostream&                 output,
                               const ball::ManagedAttribute& attribute)
{
    attribute.print(output, 0, -1);
    return output;
}

}  // close enterprise namespace

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
