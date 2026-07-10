// bdlb_guidformatter.h                                               -*-C++-*-
#ifndef INCLUDED_BDLB_GUIDFORMATTER
#define INCLUDED_BDLB_GUIDFORMATTER

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide `bsl::formatter` specialization for `bdlb::Guid`.
//
//@CLASSES:
//  bsl::formatter<bdlb::Guid, t_CHAR>: specialization to format `bdlb::Guid`
//
//@SEE_ALSO: bdlb_guid, bslfmt_format
//
//@DESCRIPTION: This component provides a partial specialization of the
// `bsl::formatter` class template for `bdlb::Guid`, enabling `bdlb::Guid`
// values to be used with `bsl::format` and its variants.
//
///Format Specification
///--------------------
// The supported format specification is a subset of the standard `std::format`
// grammar:
// ```
//   format-spec ::= [ [fill] align ] [ width ]
//   fill        ::= any character other than '{' or '}'
//   align       ::= '<' | '>' | '^'
//   width       ::= non-negative-integer | '{' [ arg-id ] '}'
// ```
// No sign, alternate-form (`#`), zero-pad (`0`), precision, or type
// specifiers are supported.  The default alignment is left, matching the
// convention for string-like types.  The formatted representation is always
// the canonical 36-character lowercase hexadecimal form
// `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Formatting a `bdlb::Guid`
/// - - - - - - - - - - - - - - - - - -
// Suppose we have a `bdlb::Guid` object and want to render it as a string:
// ```
//   const unsigned char raw[bdlb::Guid::k_GUID_NUM_BYTES] = {
//       0x5c, 0x9d, 0x4e, 0x53, 0x0d, 0xf1, 0x11, 0xe4,
//       0x91, 0x91, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 };
//   bdlb::Guid guid(raw);
//
//   bsl::string s = bsl::format("{}", guid);
//   assert(s == "5c9d4e53-0df1-11e4-9191-0800200c9a66");
// ```
// Width, alignment, and fill are supported as they would be for a string:
// ```
//   bsl::string padded = bsl::format("{:*^40}", guid);
//   assert(padded == "**5c9d4e53-0df1-11e4-9191-0800200c9a66**");
// ```

#include <bdlscm_version.h>

#include <bdlb_guid.h>

#include <bslfmt_format.h>
#include <bslfmt_formatspecificationparser.h>
#include <bslfmt_formattercharutil.h>
#include <bslfmt_padutil.h>

#include <bslmf_assert.h>
#include <bslmf_issame.h>

#include <bsls_keyword.h>
#include <bsls_libraryfeatures.h>

#include <bsl_iterator.h>
#include <bsl_string_view.h>

#include <cstddef>  // for `std::ptrdiff_t`

namespace BloombergLP {
namespace bdlb {

                    // ==============================
                    // class Guid_BslFmtFormatterImpl
                    // ==============================

/// This component-private class template provides the implementation of the
/// `bsl::formatter` specialization for `bdlb::Guid` values.  The specified
/// `t_CHAR` template parameter determines both the format-string and the
/// output character type.  The behavior is undefined unless `t_CHAR` is
/// `char` or `wchar_t`.
template <class t_CHAR>
class Guid_BslFmtFormatterImpl {
  private:
    // PRIVATE TYPES

    /// A type alias for the format-specification parser.
    typedef bslfmt::FormatSpecificationParser<t_CHAR> Specification;

    // DATA
    Specification d_spec;  // parsed specification

  public:
    // MANIPULATORS

    /// Parse the format specification contained in the specified
    /// `parseContext` and return an iterator, pointing to the beginning of the
    /// unparsed section of the format string.  Throw a `bsl::format_error`
    /// exception if the specification does not conform to the grammar
    /// documented at the top of this component.
    template <class t_PARSE_CONTEXT>
    BSLS_KEYWORD_CONSTEXPR_CPP20 typename t_PARSE_CONTEXT::iterator parse(
                                                t_PARSE_CONTEXT& parseContext);

    // ACCESSORS

    /// Create a string representation of the specified `value` in accordance
    /// with the format specification previously parsed by `parse` (see grammar
    /// documented at the top of this component) and the specified
    /// `formatContext`, and copy it to the output iterator held by
    /// `formatContext`.  Return the updated output iterator.  The behavior is
    /// undefined unless `t_FORMAT_CONTEXT::char_type` is `t_CHAR`.
    template <class t_FORMAT_CONTEXT>
    typename t_FORMAT_CONTEXT::iterator format(
                                        const Guid&       value,
                                        t_FORMAT_CONTEXT& formatContext) const;
};

}  // close package namespace
}  // close enterprise namespace


// FORMATTER SPECIALIZATIONS
namespace bsl {

/// This template partial specialization defines `bsl::formatter` for
/// `bdlb::Guid` values for both `char` and `wchar_t` character types.
template <class t_CHAR>
struct formatter<BloombergLP::bdlb::Guid, t_CHAR>
: BloombergLP::bdlb::Guid_BslFmtFormatterImpl<t_CHAR> {
};

}  // close namespace bsl

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP23_RANGE_FORMAT
// Opt `bdlb::Guid` out of `std::formatter`'s range specialization; without
// this, C++23 standard libraries would consider both this component's
// `bsl::formatter<Guid, ...>` (via `bsl`'s `std::formatter<t_ARG, t_CHAR>`
// bridge) and `std::formatter<Range, CharT>` as viable partial
// specializations, because `bdlb::Guid` satisfies `std::ranges::input_range`.
template <>
inline constexpr std::range_format
    std::format_kind<BloombergLP::bdlb::Guid> = std::range_format::disabled;
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP23_RANGE_FORMAT


// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

namespace BloombergLP {
namespace bdlb {

                    // ------------------------------
                    // class Guid_BslFmtFormatterImpl
                    // ------------------------------

// MANIPULATORS
template <class t_CHAR>
template <class t_PARSE_CONTEXT>
BSLS_KEYWORD_CONSTEXPR_CPP20 typename t_PARSE_CONTEXT::iterator
Guid_BslFmtFormatterImpl<t_CHAR>::parse(t_PARSE_CONTEXT& parseContext)
{
    const typename Specification::Sections sections =
                        static_cast<typename Specification::Sections>(
                             Specification::e_SECTIONS_FILL_ALIGN
                           | Specification::e_SECTIONS_WIDTH);

    d_spec.parse(&parseContext, sections);

    return parseContext.begin();
}

// ACCESSORS
template <class t_CHAR>
template <class t_FORMAT_CONTEXT>
typename t_FORMAT_CONTEXT::iterator
Guid_BslFmtFormatterImpl<t_CHAR>::format(const Guid&       value,
                                         t_FORMAT_CONTEXT& formatContext) const
{
    BSLMF_ASSERT((bsl::is_same<t_CHAR,
                             typename t_FORMAT_CONTEXT::char_type>::value));

    typedef bslfmt::PadUtil<t_CHAR>           PadUtil;
    typedef bslfmt::FormatterCharUtil<t_CHAR> CharUtil;

    // Postprocess a *copy* of the parsed spec so that this formatter object
    // may be reused across multiple `format` invocations with different
    // format contexts (each carrying different nested-argument values for
    // width).

    Specification finalSpec(d_spec);
    finalSpec.postprocess(formatContext);

    const bsl::basic_string_view<t_CHAR> filler(
                                              finalSpec.filler(),
                                              finalSpec.numFillerCharacters());

    std::ptrdiff_t leftPad  = 0;
    std::ptrdiff_t rightPad = 0;
    PadUtil::computePadding(&leftPad,
                            &rightPad,
                            finalSpec.postprocessedWidth(),
                            Guid::k_GUID_NUM_CHARS,
                            finalSpec.alignment(),
                            Specification::e_ALIGN_LEFT);

    // Render the guid as ASCII.  `Guid::format` always writes exactly
    // `k_GUID_NUM_CHARS` characters and never a trailing null.

    char buffer[Guid::k_GUID_NUM_CHARS];
    value.format(buffer);

    typename t_FORMAT_CONTEXT::iterator outIt = formatContext.out();

    outIt = PadUtil::pad(outIt, leftPad, filler);
    outIt = CharUtil::outputFromChar(bsl::begin(buffer),
                                     bsl::end(buffer),
                                     outIt);
    outIt = PadUtil::pad(outIt, rightPad, filler);

    return outIt;
}

}  // close package namespace
}  // close enterprise namespace

#endif  // INCLUDED_BDLB_GUIDFORMATTER

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
