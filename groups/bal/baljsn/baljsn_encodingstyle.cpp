// baljsn_encodingstyle.cpp         *DO NOT EDIT*          @generated -*-C++-*-

#include <bsls_ident.h>
BSLS_IDENT_RCSID(baljsn_encoderoptions_cpp,"$Id$ $CSID$")

#include <baljsn_encodingstyle.h>

#include <bdlat_formattingmode.h>
#include <bdlat_valuetypefunctions.h>


#include <bslim_printer.h>
#include <bsls_assert.h>

#include <bsl_iomanip.h>
#include <bsl_limits.h>
#include <bsl_ostream.h>

///Implementation Note
///- - - - - - - - - -
// This file was generated from a script and was subsequently modified to add
// documentation and to make other changes.  The steps to generate and update
// this file can be found in the 'doc/generating_codec_options.txt' file.

namespace BloombergLP {
namespace baljsn {

                            // -------------------
                            // class EncodingStyle
                            // -------------------

// CONSTANTS

const char EncodingStyle::CLASS_NAME[] = "EncodingStyle";

const bdlat_EnumeratorInfo EncodingStyle::ENUMERATOR_INFO_ARRAY[] = {
    {
        EncodingStyle::e_COMPACT,
        "e_COMPACT",
        sizeof("e_COMPACT") - 1,
        ""
    },
    {
        EncodingStyle::e_PRETTY,
        "e_PRETTY",
        sizeof("e_PRETTY") - 1,
        ""
    }
};

// CLASS METHODS

int EncodingStyle::fromInt(EncodingStyle::Value *result, int number)
{
    switch (number) {
      case EncodingStyle::e_COMPACT:
      case EncodingStyle::e_PRETTY:
        *result = (EncodingStyle::Value)number;
        return 0;
      default:
        return -1;
    }
}

int EncodingStyle::fromString(
        EncodingStyle::Value *result,
        const char           *string,
        int                   stringLength)
{
    for (int i = 0; i < 2; ++i) {
        const bdlat_EnumeratorInfo& enumeratorInfo =
                    EncodingStyle::ENUMERATOR_INFO_ARRAY[i];

        if (stringLength == enumeratorInfo.d_nameLength
        &&  0 == bsl::memcmp(enumeratorInfo.d_name_p, string, stringLength))
        {
            *result = (EncodingStyle::Value)enumeratorInfo.d_value;
            return 0;
        }
    }

    return -1;
}

const char *EncodingStyle::toString(EncodingStyle::Value value)
{
    switch (value) {
      case e_COMPACT: {
        return "e_COMPACT";
      } break;
      case e_PRETTY: {
        return "e_PRETTY";
      } break;
    }

    BSLS_ASSERT(0 == "invalid enumerator");
    return 0;
}

}  // close package namespace
}  // close enterprise namespace

// GENERATED BY BLP_BAS_CODEGEN_3.8.24 Fri Feb 17 12:35:40 2017
// USING bas_codegen.pl -m msg --package baljsn --noExternalization -E --noAggregateConversion baljsn.xsd
// SERVICE VERSION
// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
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
