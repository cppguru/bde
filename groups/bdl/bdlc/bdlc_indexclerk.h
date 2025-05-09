// bdlc_indexclerk.h                                                  -*-C++-*-
#ifndef INCLUDED_BDLC_INDEXCLERK
#define INCLUDED_BDLC_INDEXCLERK

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a manager of reusable, non-negative integer indices.
//
//@CLASSES:
//  bdlc::IndexClerkIter: sequential accessor to decommissioned indices
//  bdlc::IndexClerk: manager of reusable, non-negative integer indices
//
//@DESCRIPTION: This component implements an efficient, value-semantic manager
// class for reusable, non-negative integer indices.  Each new instance of a
// `bdlc::IndexClerk` will issue consecutive integers on request, beginning
// with `0, 1, 2, ...`.  Indices that are no longer needed may be returned for
// reuse.  Existing decommissioned indices are reissued before any new ones are
// created.  Value-semantic operations such as copy construction and
// assignment, equality comparison, and streaming are also provided.  Finally,
// a `bdlc::IndexClerkIter` is provided to enable sequential, read-only access
// to the currently decommissioned indices.  Note that the order of iteration
// is not defined.
//
///Performance
///-----------
// The following characterizes the performance of representative operations
// using "big-oh" notation, O[f(N,M)], where the names `N` and `M` also refer
// to the number of respective elements in the sequence of decommissioned
// indices.
// ```
//    Operation                     Worst Case
//    ---------                     ----------
//    DEFAULT CTOR                  O[1]
//    COPY CTOR(N)                  O[N]
//    N.DTOR()                      O[1]
//    N.OP=(M)                      O[M]
//    OP==(N,M)                     O[min(N,M)]
//
//    N.getIndex()                  O[1]
//    N.putIndex(index)             O[1]
//    N.removeAll()                 O[1]
//    N.numCommissionedIndices()    O[1]
//    N.numDecommissionedIndices()  O[1]
//    N.nextNewIndex()              O[1]
//    N.isInUse(index)              O[N]
// ```
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic Usage
/// - - - - - - - - - - -
// A `bdlc::IndexClerk` is commonly used in conjunction with an array to enable
// machine-address-independent referencing.  Rather than dynamically allocating
// an object and holding its address, the object is stored in the array at the
// next position dispensed by its associated `bdlc::IndexClerk`, and that index
// becomes an identifier (Id) for the new object.  Instead of destroying an
// unneeded object, its Id is merely returned to the clerk.
//
// Care must be taken to ensure that objects "created" at reused indices (i.e.,
// indices below the current length of the array) *replace* (the value of) an
// existing object in the array while objects created at new indices (i.e.,
// indices at the current length) are *appended* to the array.
//
// For example, suppose we have a security class object.  To add and remove
// security values from a security array/clerk pair, you might write the
// following two functions:
// ```
// /// Add a copy of the specified `newSecurity` to the specified
// /// `securityArray` at the index dispensed by the specified
// /// `securityClerk`.  Also update the `securityClerk`, and return the id
// /// (in `securityArray`) for the newly added security.
// int addSecurity(bsl::vector<Security> *securityArray,
//                 bdlc::IndexClerk      *securityClerk,
//                 const Security&        newSecurity)
// {
//     BSLS_ASSERT(securityArray);
//     BSLS_ASSERT(securityClerk);
//
//     int id = securityClerk->getIndex();
//
//     if (id < securityArray->size()) {
//         (*securityArray)[id] = newSecurity;
//     }
//     else {
//         securityArray->push_back(newSecurity);
//     }
//
//     return id;
// }
//
// /// Remove the security object identified by the specified `securityId`
// /// from the specified `securityArray`, and update the specified
// /// `securityClerk` (making `securityId` available for reuse).  The
// /// behavior is undefined unless `securityId` refers to an active
// /// security in `securityArray` dispensed by `securityClerk`.
// void removeSecurity(bsl::vector<Security> *securityArray,
//                     bdlc::IndexClerk      *securityClerk,
//                     int                    securityId)
// {
//     BSLS_ASSERT(securityArray);
//     BSLS_ASSERT(securityClerk);
//
//     BSLS_ASSERT(0                             <= securityId);
//     BSLS_ASSERT(securityClerk->nextNewIndex() >  securityId);
//     BSLS_ASSERT(securityArray->size()         >  securityId);
//
//     // Note that the 'isInUse' function (below) runs in linear time.
//
//     BSLS_ASSERT_SAFE(securityClerk->isInUse(securityId));
//
//     (*securityArray)[securityId] = Security();  // optional
//     securityClerk->putIndex(securityId);
// }
// ```

#include <bdlscm_version.h>

#include <bslx_instreamfunctions.h>
#include <bslx_outstreamfunctions.h>

#include <bslma_allocator.h>
#include <bslma_usesbslmaallocator.h>

#include <bslmf_isbitwisecopyable.h>
#include <bslmf_nestedtraitdeclaration.h>

#include <bsls_assert.h>
#include <bsls_platform.h>
#include <bsls_review.h>

#include <bsl_iosfwd.h>
#include <bsl_iterator.h>
#include <bsl_vector.h>

namespace BloombergLP {
namespace bdlc {

                            // ====================
                            // class IndexClerkIter
                            // ====================

/// This class defines an in-core value-semantic iterator providing
/// sequential read-only access to the decommissioned indices of a
/// `IndexClerk`.  The order of iteration is implementation dependent.
class IndexClerkIter {

    // DATA
    bsl::reverse_iterator<const int *> d_index_p;  // pointer to current
                                                   // decommissioned index

    // FRIENDS
    friend bool operator==(const IndexClerkIter& lhs,
                           const IndexClerkIter& rhs);
    friend bool operator!=(const IndexClerkIter& lhs,
                           const IndexClerkIter& rhs);
  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(IndexClerkIter, bslmf::IsBitwiseCopyable);

    // CREATORS

    /// Create an unbound iterator.
    IndexClerkIter();

    /// Create an iterator referring to the specified integer `index`.
    IndexClerkIter(const int *index);

    /// Create an iterator having the same value as the specified `original`
    /// iterator.
    IndexClerkIter(const IndexClerkIter& original);

  //~IndexClerkIter();
        // Destroy this index clerk iterator.  Note that this method is
        // generated by the compiler.

    // MANIPULATORS

    /// Create an iterator having the same value as the specified `rhs`
    /// iterator.
    IndexClerkIter& operator=(const IndexClerkIter& rhs);

    /// Increment this iterator to refer to the next index in the
    /// corresponding sequence of decommissioned indices.  Return a
    /// reference to this modifiable iterator.  The behavior is undefined
    /// unless the current index is within the range `[ begin() .. end() )`.
    IndexClerkIter& operator++();

    /// Decrement this iterator to refer to the previous index in the
    /// corresponding sequence of decommissioned indices.  Return a
    /// reference to this modifiable iterator.  The behavior is undefined
    /// unless the current index is within the range `( begin() .. end() ]`.
    IndexClerkIter& operator--();

    // ACCESSORS

    /// Return the value of the integer to which this iterator currently
    /// refers.  The behavior is undefined unless the iterator is within the
    /// range `[ begin() .. end() )`.
    int operator*() const;
};

/// Return `true` if `lhs` and `rhs` have the same value and `false`
/// otherwise.  Two iterators have the same value if they refer to the same
/// element of the same container or if they both have the end iterator
/// value for the same container.  The behavior is undefined unless `lhs`
/// and `rhs` refer to the same container and are non-singular (i.e., are
/// not default-constructed or copies of singular iterators).
bool operator==(const IndexClerkIter& lhs, const IndexClerkIter& rhs);

/// Return `true` if `lhs` and `rhs` do not have the same value and `false`
/// otherwise.  Two iterators do not have the same value if they do not
/// refer to the same element of the same container or if one has the end
/// iterator value of a container and the other refers to an element (not
/// the end) of the same container.  The behavior is undefined unless `lhs`
/// and `rhs` refer to the same container and are non-singular (i.e., are
/// not default-constructed or copies of singular iterators).
bool operator!=(const IndexClerkIter& lhs, const IndexClerkIter& rhs);

                              // ================
                              // class IndexClerk
                              // ================

/// This class defines an efficient, value-semantic manager type for
/// reusable, non-negative integer indices.  The class invariants are that
/// the all decommissioned indices must be non-negative, less than the next
/// new index, and unique.
class IndexClerk {

    // DATA
    bsl::vector<int> d_unusedStack;   // stack of decommissioned indices
    int              d_nextNewIndex;  // next unused index to be created

    // FRIENDS
    friend bool operator==(const IndexClerk&, const IndexClerk&);
    friend bool operator!=(const IndexClerk&, const IndexClerk&);

    // PRIVATE CLASS METHODS

    /// Return `true` if the class invariants of the object represented by
    /// the specified `unusedStack` are preserved and `false` otherwise.
    /// The class invariants are that all decommissioned indices are
    /// non-negative, less than the specified `nextNewIndex`, and unique.
    /// Note that the run time of this function is proportional to
    /// `numDecommissionedIndices()`, but it requires temporary space that
    /// is proportional to `nextNewIndex`.
    static bool areInvariantsPreserved(const bsl::vector<int>& unusedStack,
                                       int                     nextNewIndex);

  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(IndexClerk, bslma::UsesBslmaAllocator);

    // CLASS METHODS

    /// Return the maximum valid BDEX format version, as indicated by the
    /// specified `versionSelector`, to be passed to the `bdexStreamOut`
    /// method.  Note that it is highly recommended that `versionSelector`
    /// be formatted as "YYYYMMDD", a date representation.  Also note that
    /// `versionSelector` should be a *compile*-time-chosen value that
    /// selects a format version supported by both externalizer and
    /// unexternalizer.  See the `bslx` package-level documentation for more
    /// information on BDEX streaming of value-semantic types and
    /// containers.
    static int maxSupportedBdexVersion(int versionSelector);

    // CREATORS

    /// Create a new index clerk that dispenses consecutive non-negative
    /// integers beginning with `0, 1, 2, ...`; however, indices returned
    /// via `putIndex` will be reissued before any new ones are created.
    /// Optionally specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is 0, the currently installed default allocator is
    /// used.
    explicit IndexClerk(bslma::Allocator *basicAllocator = 0);

    /// Create a new index clerk having the value of the specified
    /// `original` index clerk.  Optionally specify a `basicAllocator` used
    /// to supply memory.  If `basicAllocator` is 0, the currently installed
    /// default allocator is used.
    IndexClerk(const IndexClerk&  original,
               bslma::Allocator  *basicAllocator = 0);

    /// Destroy this index clerk.
    ~IndexClerk();

    // MANIPULATORS

    /// Assign to this index clerk the value of the specified `rhs` index
    /// clerk, and return a reference to this modifiable index clerk.
    IndexClerk& operator=(const IndexClerk& rhs);


    /// Assign to this object the value read from the specified input
    /// `stream` using the specified `version` format, and return a
    /// reference to `stream`.  If `stream` is initially invalid, this
    /// operation has no effect.  If `version` is not supported, this object
    /// is unaltered and `stream` is invalidated, but otherwise unmodified.
    /// If `version` is supported but `stream` becomes invalid during this
    /// operation, this object has an undefined, but valid, state.  Note
    /// that no version is read from `stream`.  See the `bslx` package-level
    /// documentation for more information on BDEX streaming of
    /// value-semantic types and containers.
    template <class STREAM>
    STREAM& bdexStreamIn(STREAM& stream, int version);

    /// Return the next available unused integer index.  Existing
    /// decommissioned indices are reissued before new ones are created.
    int getIndex();

    /// Return the specified `index` to this index clerk, which indicates
    /// that `index` is no longer in use and may be reissued.  The behavior
    /// is undefined if `index` has never been generated by this clerk or is
    /// currently decommissioned.
    void putIndex(int index);

    /// Remove all of the indices from this index clerk.  Note that the
    /// following post conditions apply:
    /// ```
    /// assert(0 == numCommissionedIndices());
    /// assert(0 == numDecommissionedIndices());
    /// assert(0 == nextNewIndex());
    /// ```
    void removeAll();

    // ACCESSORS

    /// Write the value of this object, using the specified `version`
    /// format, to the specified output `stream`, and return a reference to
    /// `stream`.  If `stream` is initially invalid, this operation has no
    /// effect.  If `version` is not supported, `stream` is invalidated, but
    /// otherwise unmodified.  Note that `version` is not written to
    /// `stream`.  See the `bslx` package-level documentation for more
    /// information on BDEX streaming of value-semantic types and
    /// containers.
    template <class STREAM>
    STREAM& bdexStreamOut(STREAM& stream, int version) const;

    /// Return a `IndexClerkIter` referring to the first index returned to
    /// this `IndexClerk` that is currently unused, or `end()` if there are
    /// currently no decommissioned indices.
    IndexClerkIter begin() const;

    /// Return a `IndexClerkIter` referring to an invalid index, indicating
    /// the end of the sequence of decommissioned index.
    IndexClerkIter end() const;

    /// Return `true` if the specified `index` is currently in use, and
    /// `false` otherwise.  The behavior is undefined unless `0 <= index`
    /// and `index < nextNewIndex()`.  Note that this method runs in time
    /// proportional to the number of decommissioned indices.
    bool isInUse(int index) const;

    /// Return the number of indices currently in use.
    int numCommissionedIndices() const;

    /// Return the number of indices that are currently decommissioned.
    int numDecommissionedIndices() const;

    /// Return the smallest (non-negative) index that has not been issued by
    /// this index clerk.  Note that this function offers the client a
    /// "peek" at the next "new" index, but has no effect on the value of
    /// this index clerk.
    int nextNewIndex() const;

    /// Format this index clerk to the specified output `stream` at the
    /// (absolute value of) the optionally specified indentation `level` and
    /// return a reference to `stream`.  If `level` is specified, optionally
    /// specify `spacesPerLevel`, the number of spaces per indentation level
    /// for this and all of its nested objects.  If `level` is negative,
    /// suppress indentation of the first line.  If `spacesPerLevel` is
    /// negative, format the entire output on one line, suppressing all but
    /// the initial indentation (as governed by `level`).  If `stream` is
    /// not valid on entry, this operation has no effect.
    bsl::ostream& print(bsl::ostream& stream,
                        int           level = 0,
                        int           spacesPerLevel = 4) const;

#ifndef BDE_OMIT_INTERNAL_DEPRECATED  // pending deprecation

    /// Return the most current BDEX streaming version number supported by
    /// this class.
    ///
    /// @DEPRECATED: Use `maxSupportedBdexVersion(int)` instead.
    static int maxSupportedBdexVersion();

#endif // BDE_OMIT_INTERNAL_DEPRECATED -- pending deprecation
};

// FREE OPERATORS

/// Return `true` if the specified `lhs` and `rhs` index clerks have the same
/// value, and `false` otherwise.  Two `IndexClerk` objects have the same value
/// if they have the same `nextNewIndex()` and would always generate the same
/// sequence of integer indices.
bool operator==(const IndexClerk& lhs, const IndexClerk& rhs);

/// Return `true` if the specified `lhs` and `rhs` index clerks do not have the
/// same value, and `false` otherwise.  Two `IndexClerk` objects do not have
/// the same value if they do not have the same `nextNewIndex()`, or might
/// generate different sequences of integer indices.
bool operator!=(const IndexClerk& lhs, const IndexClerk& rhs);

/// Write the specified `rhs` index clerk to the specified output `stream` in
/// some single-line (human-readable) format, and return a reference to the
/// modifiable `stream`.
bsl::ostream& operator<<(bsl::ostream& stream, const IndexClerk& rhs);

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                            // --------------------
                            // class IndexClerkIter
                            // --------------------

// CREATORS
inline
IndexClerkIter::IndexClerkIter()
: d_index_p(0)
{
}

inline
IndexClerkIter::IndexClerkIter(const int *index)
: d_index_p(index)
{
}

inline
IndexClerkIter::IndexClerkIter(const IndexClerkIter& original)
: d_index_p(original.d_index_p)
{
}

// MANIPULATORS
inline
IndexClerkIter&
IndexClerkIter::operator=(const IndexClerkIter& rhs)
{
    d_index_p = rhs.d_index_p;
    return *this;
}

inline
IndexClerkIter& IndexClerkIter::operator++()
{
    BSLS_ASSERT(0 != d_index_p.base());

    ++d_index_p;
    return *this;
}

inline
IndexClerkIter& IndexClerkIter::operator--()
{
    BSLS_ASSERT(0 != d_index_p.base());

    --d_index_p;
    return *this;
}

// ACCESSORS
inline
int IndexClerkIter::operator*() const
{
#if defined(BSLS_PLATFORM_CMP_GNU) && \
    BSLS_PLATFORM_CMP_VERSION >= 120000 && BSLS_PLATFORM_CMP_VERSION < 150000
    // See implementation notes in the .cpp file for explanation.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    BSLS_ASSERT(0 != d_index_p.base());

    return *d_index_p;
#if defined(BSLS_PLATFORM_CMP_GNU) && \
    BSLS_PLATFORM_CMP_VERSION >= 120000 && BSLS_PLATFORM_CMP_VERSION < 150000
#pragma GCC diagnostic pop
#endif
}

}  // close package namespace

// FREE OPERATORS
inline
bool bdlc::operator==(const IndexClerkIter& lhs, const IndexClerkIter& rhs)
{
    return lhs.d_index_p == rhs.d_index_p;
}

inline
bool bdlc::operator!=(const IndexClerkIter& lhs, const IndexClerkIter& rhs)
{
    return lhs.d_index_p != rhs.d_index_p;
}

namespace bdlc {

                              // ----------------
                              // class IndexClerk
                              // ----------------

// CREATORS
inline
IndexClerk::IndexClerk(bslma::Allocator *basicAllocator)
: d_unusedStack(basicAllocator)
, d_nextNewIndex(0)
{
}

inline
IndexClerk::IndexClerk(const IndexClerk&  original,
                       bslma::Allocator  *basicAllocator)
: d_unusedStack(original.d_unusedStack, basicAllocator)
, d_nextNewIndex(original.d_nextNewIndex)
{
}

inline
IndexClerk::~IndexClerk()
{
    BSLS_ASSERT_SAFE(areInvariantsPreserved(d_unusedStack, d_nextNewIndex));
}

// MANIPULATORS
inline
IndexClerk& IndexClerk::operator=(const IndexClerk& rhs)
{
    d_unusedStack = rhs.d_unusedStack;
    d_nextNewIndex = rhs.d_nextNewIndex;
    return *this;
}

inline
int IndexClerk::getIndex()
{
    if (d_unusedStack.empty()) {
        return d_nextNewIndex++;                                      // RETURN
    }
    else {
        int index = d_unusedStack.back();
        d_unusedStack.pop_back();
        return index;                                                 // RETURN
    }
}

inline
void IndexClerk::putIndex(int index)
{
    BSLS_ASSERT(0 <= index);
    BSLS_ASSERT(     index < d_nextNewIndex);
    BSLS_ASSERT_SAFE(isInUse(index));

    d_unusedStack.push_back(index);
}

inline
void IndexClerk::removeAll()
{
    d_unusedStack.clear();
    d_nextNewIndex = 0;
}

// Note: Order changed from declaration to make use of inlined 'removeAll'.

template <class STREAM>
STREAM& IndexClerk::bdexStreamIn(STREAM& stream, int version)
{
    switch (version) {
      case 1: {
        int nextNewIndex;
        stream.getInt32(nextNewIndex);

        if (!stream || nextNewIndex < 0) {
            stream.invalidate();
            return stream;                                            // RETURN
        }

        bsl::vector<int> unusedStack;
        bslx::InStreamFunctions::bdexStreamIn(stream, unusedStack, version);

        // Stream can be invalidated after streaming in 'd_unusedStack'.

        if (!stream || !areInvariantsPreserved(unusedStack, nextNewIndex)) {
            stream.invalidate();
            return stream;                                            // RETURN
        }

        d_unusedStack  = unusedStack;
        d_nextNewIndex = nextNewIndex;
      } break;
      default: {
        stream.invalidate();
      } break;
    }
    return stream;
}

// ACCESSORS
template <class STREAM>
inline
STREAM& IndexClerk::bdexStreamOut(STREAM& stream, int version) const
{
    if (stream) {
        switch (version) { // switch on the schema version
          case 1: {
            stream.putInt32(d_nextNewIndex);
            bslx::OutStreamFunctions::bdexStreamOut(
                                               stream, d_unusedStack, version);
          } break;
          default: {
            stream.invalidate();  // unrecognized version number
          }
        }
    }
    return stream;
}

inline
int IndexClerk::numCommissionedIndices() const
{
    return d_nextNewIndex - static_cast<int>(d_unusedStack.size());
}

inline
IndexClerkIter IndexClerk::begin() const
{
    return IndexClerkIter(d_unusedStack.begin() + d_unusedStack.size());
}

inline
IndexClerkIter IndexClerk::end() const
{
    return IndexClerkIter(d_unusedStack.begin());
}

inline
int IndexClerk::numDecommissionedIndices() const
{
    return static_cast<int>(d_unusedStack.size());
}

inline
int IndexClerk::nextNewIndex() const
{
    return d_nextNewIndex;
}

inline
int IndexClerk::maxSupportedBdexVersion(int /* versionSelector */)
{
    return 1;
}

#ifndef BDE_OMIT_INTERNAL_DEPRECATED  // pending deprecation

// DEPRECATED METHODS
inline
int IndexClerk::maxSupportedBdexVersion()
{
    return maxSupportedBdexVersion(0);
}

#endif // BDE_OMIT_INTERNAL_DEPRECATED -- pending deprecation

}  // close package namespace

// FREE OPERATORS
inline
bool bdlc::operator==(const IndexClerk& lhs, const IndexClerk& rhs)
{
    return lhs.d_nextNewIndex == rhs.d_nextNewIndex
        && lhs.d_unusedStack  == rhs.d_unusedStack;
}

inline
bool bdlc::operator!=(const IndexClerk& lhs, const IndexClerk& rhs)
{
    return lhs.d_nextNewIndex != rhs.d_nextNewIndex
        || lhs.d_unusedStack  != rhs.d_unusedStack;
}

inline
bsl::ostream& bdlc::operator<<(bsl::ostream& stream, const IndexClerk& rhs)
{
    return rhs.print(stream, 0, -1);
}

}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
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
