// bsls_stackaddressutil_plinktimestamp.cpp                           -*-C++-*-
#include <bsls_stackaddressutil_plinktimestamp.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#include <bsls_platform.h>

// BDE_VERIFY pragma: -AQb01
// BDE_VERIFY pragma: -TR04

// plink_timestamp___ :
const char *plink_timestamp___[6]={0};

// Note that the size of plink_timestamp___ must match the other weak linker
// symbol in sysutil_pwhat.c, which is 6.

#if defined(BSLS_PLATFORM_OS_AIX) ||                                         \
    defined(BSLS_PLATFORM_CMP_GNU) ||                                        \
    defined(BSLS_PLATFORM_CMP_SUN)

#pragma weak plink_timestamp___

#endif

// ----------------------------------------------------------------------------
// Copyright 2024 Bloomberg Finance L.P.
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
