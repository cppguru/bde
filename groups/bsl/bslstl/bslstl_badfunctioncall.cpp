// bslstl_badfunctioncall.cpp                                         -*-C++-*-
#include <bslstl_badfunctioncall.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#ifdef BDE_BUILD_TARGET_EXC
namespace bsl {
#if !BSLS_COMPILERFEATURES_FULL_CPP11

                        // -----------------------
                        // class bad_function_call
                        // -----------------------

bad_function_call::bad_function_call()
: std::exception()
{
}

const char *bad_function_call::what() const BSLS_EXCEPTION_VIRTUAL_NOTHROW
{
    return "bad_function_call";
}
#endif  // !BSLS_COMPILERFEATURES_FULL_CPP11

}  // close namespace bsl
#endif  // BDE_BUILD_TARGET_EXC

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
