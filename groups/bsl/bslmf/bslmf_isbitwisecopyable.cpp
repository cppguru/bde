// bslmf_isbitwisecopyable.cpp                                        -*-C++-*-
#include <bslmf_isbitwisecopyable.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#include <bslmf_addconst.h>                 // for testing only
#include <bslmf_addcv.h>                    // for testing only
#include <bslmf_addlvaluereference.h>       // for testing only
#include <bslmf_addpointer.h>               // for testing only
#include <bslmf_addvolatile.h>              // for testing only
#include <bslmf_nestedtraitdeclaration.h>   // for testing only
#include <bslmf_removevolatile.h>           // for testing only

// ----------------------------------------------------------------------------
// Copyright 2023 Bloomberg Finance L.P.
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
