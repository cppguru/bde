// bdlsb_memoutstreambuf.cpp                                          -*-C++-*-
#include <bdlsb_memoutstreambuf.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdlsb_memoutstreambuf_cpp,"$Id$ $CSID$")

#include <bslmf_assert.h>
#include <bsls_assert.h>

#include <bsls_performancehint.h>

#include <bsl_algorithm.h>
#include <bsl_cstring.h>
#include <bsl_limits.h>

namespace BloombergLP {
namespace bdlsb {
                         // --------------------------
                         // class MemOutStreamBuf_Util
                         // --------------------------

// CLASS DATA

const bsl::size_t MemOutStreamBuf_Util::k_INITIAL_BUFFER_SIZE;
const bsl::size_t MemOutStreamBuf_Util::k_GEOMETRIC_GROWTH_FACTOR;
const bsl::size_t MemOutStreamBuf_Util::k_MAX_GEOMETRIC_GROWTH_LENGTH =
           bsl::numeric_limits<bsl::size_t>::max() / k_GEOMETRIC_GROWTH_FACTOR;

// CLASS METHODS

bsl::size_t MemOutStreamBuf_Util::computeNewCapacity(
                                                   bsl::size_t newLength,
                                                   bsl::size_t currentCapacity)
{
    // Our first estimate of the needed capacity is the current capacity, or
    // `k_INITIAL_BUFFER_SIZE` if there is no current capacity.
    bsl::size_t newCapacity = 0 == currentCapacity
                            ? k_INITIAL_BUFFER_SIZE
                            : currentCapacity;

    // If that first estimate is insufficient for the current need
    // (`newLength`) grow that estimate.

    if (newCapacity < newLength) {
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                  k_MAX_GEOMETRIC_GROWTH_LENGTH < newLength)) {

            // Cannot apply `k_GEOMETRIC_GROWTH_FACTOR` without overflow.
            // Instead, claim half of the remaining range.  Exercise of this
            // code is very unlikely on 64-bit builds where
            // `k_MAX_GEOMETRIC_GROWTH_LIMIT` is (roughly) `9 x 10^18` bytes.

            newCapacity =
                     newLength +
                     (bsl::numeric_limits<bsl::size_t>::max() - newLength) / 2;
        }
        else {
            do {
                newCapacity *= k_GEOMETRIC_GROWTH_FACTOR;
            } while (newCapacity < newLength);
        }
    }

    return newCapacity;
}

                         // ---------------------
                         // class MemOutStreamBuf
                         // ---------------------

// PRIVATE MANIPULATORS
void MemOutStreamBuf::grow(bsl::size_t newLength)
{
    const bsl::size_t newCapacity =
                          MemOutStreamBuf_Util::computeNewCapacity(newLength,
                                                                   capacity());

    BSLS_ASSERT(newCapacity >= newLength);

    reserveCapacity(newCapacity);
}

// PROTECTED MANIPULATORS
int MemOutStreamBuf::overflow(int_type insertionChar)
{
    if (traits_type::eof() == insertionChar) {
        return traits_type::not_eof(insertionChar);                   // RETURN
    }

    if (length() == capacity()) {
        grow(capacity() + 1);
    }
    return sputc(static_cast<char_type>(insertionChar));
}

MemOutStreamBuf::pos_type
MemOutStreamBuf::seekpos(MemOutStreamBuf::pos_type position,
                         bsl::ios_base::openmode   which)
{
    return seekoff(off_type(position), bsl::ios_base::beg, which);
}

MemOutStreamBuf::pos_type
MemOutStreamBuf::seekoff(MemOutStreamBuf::off_type offset,
                         bsl::ios_base::seekdir    way,
                         bsl::ios_base::openmode   which)
{
    // This is an output-only buffer, so cannot "seek" in "get" area.
    if (!(which & bsl::ios_base::out)) {
        return pos_type(-1);                                          // RETURN
    }

    // Compute offset from the current position.  In this stream, 'pptr()'
    // defines both the current position and the end of the logical byte
    // stream.  Thus, 'bsl::ios_base::curr' and 'bsl::ios_base::end' are
    // handled identically.

    off_type currOffset = bsl::ios_base::beg == way
                          ? offset - length()
                          : offset;

    // 'currOffset' is invalid if it is positive or has an absolute-value
    // greater than 'length()'.

    if (currOffset > 0 || static_cast<unsigned>(-currOffset) > length()) {
        return pos_type(-1);                                          // RETURN
    }

    pbump(static_cast<int>(currOffset));

    return pos_type(length());
}

bsl::streamsize MemOutStreamBuf::xsputn(const char_type *source,
                                        bsl::streamsize  numChars)
{
    BSLMF_ASSERT(bsl::numeric_limits<bsl::streamsize>::is_signed);
    BSLS_ASSERT((source && 0 < numChars) || 0 == numChars);

    if (0 == numChars) {
        return numChars;                                              // RETURN
    }

    const bsl::size_t newLength =
                                 static_cast<bsl::size_t>(length() + numChars);
    BSLS_ASSERT(newLength >= length());

    if (newLength > capacity()) {
        grow(newLength);
    }
    bsl::copy(source, source + numChars, pptr());

    // `pbump` accepts an `int` so (in the unlikely case) that
    // `remainingLength` is greater than `INT_MAX`, we must call `pbump`
    // multiple times.

    const bsl::streamsize intMax = static_cast<bsl::streamsize>(
                                              bsl::numeric_limits<int>::max());
    bsl::streamsize       remainingLength = numChars;

    do {
        int bumpLength = static_cast<int>(bsl::min(intMax, remainingLength));
        pbump(bumpLength);
        remainingLength -= bumpLength;
    } while (remainingLength > 0);

    return numChars;
}

// MANIPULATORS
void MemOutStreamBuf::reserveCapacity(bsl::size_t numCharacters)
{
    if (numCharacters <= capacity()) {
        return;                                                       // RETURN
    }

    char_type *oldBuffer = pbase();
    char_type *newBuffer = static_cast<char_type *>(
                                       d_allocator_p->allocate(numCharacters));

    // Copy over existing characters.

    const bsl::size_t oldLength = length();
    bsl::copy(oldBuffer, oldBuffer + oldLength * sizeof(char_type), newBuffer);
    d_allocator_p->deallocate(oldBuffer);

    // Reset data members appropriately.
    setp(newBuffer, newBuffer + numCharacters);

    // `pbump` accepts an `int` so (in the unlikely case) that
    // `remainingLength` is greater than `INT_MAX`, we must call `pbump`
    // multiple times.

    const bsl::size_t intMax = static_cast<bsl::size_t>(
                                              bsl::numeric_limits<int>::max());

    bsl::size_t remainingLength = oldLength;
    do {
        int bumpLength = static_cast<int>(bsl::min(intMax, remainingLength));
        pbump(bumpLength);
        remainingLength -= bumpLength;
    } while (remainingLength > 0);
}

}  // close package namespace
}  // close enterprise namespace

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
