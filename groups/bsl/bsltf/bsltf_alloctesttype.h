// bsltf_alloctesttype.h                                              -*-C++-*-
#ifndef INCLUDED_BSLTF_ALLOCTESTTYPE
#define INCLUDED_BSLTF_ALLOCTESTTYPE

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a class for testing that allocates with `bsl::allocator`.
//
//@CLASSES:
//   bsltf::AllocTestType: allocating test class
//
//@SEE_ALSO: bsltf_templatetestfacility
//
//@DESCRIPTION: This component provides a single, unconstrained
// (value-semantic) attribute class, `AllocTestType`, that uses a
// `bsl::allocator<>` to supply memory and defines the type trait
// `bslma::UsesBslmaAllocator`.  Furthermore, this class is not
// bitwise-moveable, and will assert on destruction if it has been moved.  This
// class is primarily provided to facilitate testing of templates by defining a
// simple type representative of user-defined types having an allocator.
//
///Salient Attributes
///------------------
//
// | Name  | Type  | Default  |
// | ----- | ----- | -------- |
// | data  | int   | 0        |
//
// * `data`: representation of the object's value
//
///Non-salient Attributes
///----------------------
//
// | Name       | Type              | Default            |
// | ---------- | ----------------- | ------------------ |
// | allocator  | bsl::allocator<>  | bsl::allocator<>{} |
//
// * `allocator`: allocator used by the object
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Printing the Supported Traits
/// - - - - - - - - - - - - - - - - - - - -
// Suppose we wanted to print the supported traits of this test type.
//
// First, we create a function template `printTypeTraits` with a parameterized
// `TYPE`:
// ```
// /// Prints the traits of the parameterized `TYPE` to the console.
// template <class TYPE>
// void printTypeTraits()
// {
//     if (bslma::UsesBslmaAllocator<TYPE>::value) {
//         printf("Type defines bslma::UsesBslmaAllocator.\n");
//     }
//     else {
//         printf(
//             "Type does not define bslma::UsesBslmaAllocator.\n");
//     }
//
//     if (bslmf::IsBitwiseMoveable<TYPE>::value) {
//         printf("Type defines bslmf::IsBitwiseMoveable.\n");
//     }
//     else {
//         printf("Type does not define bslmf::IsBitwiseMoveable.\n");
//     }
// }
// ```
// Now, we invoke the `printTypeTraits` function template using
// `AllocTestType` as the parameterized `TYPE`:
// ```
// printTypeTraits<AllocTestType>();
// ```
// Finally, we observe the console output:
// ```
// Type defines bslma::UsesBslmaAllocator.
// Type does not define bslmf::IsBitwiseMoveable.
// ```

#include <bslscm_version.h>

#include <bsla_maybeunused.h>

#include <bslma_allocator.h>
#include <bslma_bslallocator.h>

namespace BloombergLP {

namespace bsltf {

                        // ===================
                        // class AllocTestType
                        // ===================

/// This unconstrained (value-semantic) attribute class that uses a
/// `bsl::allocator<>` to supply memory and defines the type trait
/// `bslma::UsesBslmaAllocator`.  This class is primarily provided
/// to facilitate testing of templates by defining a simple type
/// representative of user-defined types having an allocator.  See the
/// Attributes section under @DESCRIPTION in the component-level
/// documentation for information on the class attributes.
class AllocTestType {

    // DATA
    bsl::allocator<int> d_allocator;  // allocator used to supply memory
    int                *d_data_p;     // pointer to the data value

    BSLA_MAYBE_UNUSED AllocTestType *d_self_p;  // pointer to self (to verify
                                                // this object is not bit-wise
                                                // moved)

  public:
    // TYPES
    typedef bsl::allocator<int> allocator_type;

    // CREATORS

    /// Create a `AllocTestType` object having the (default) attribute
    /// values:
    /// ```
    /// data() == 0
    /// ```
    /// Optionally specify a `allocator` used to supply memory.  If
    /// `allocator` is 0, the currently installed default allocator is
    /// used.
    AllocTestType();
    explicit AllocTestType(const allocator_type& allocator);

    /// Create a `AllocTestType` object having the specified `data`
    /// attribute value.  Optionally specify a `allocator` used to supply
    /// memory (e.g., the address of a `bslma::Allocator` object).  If
    /// `allocator` is not specified, the currently installed default
    /// allocator is used.
    explicit AllocTestType(int                   data,
                           const allocator_type& allocator = allocator_type());

    /// Create a `AllocTestType` object having the same value as the
    /// specified `original` object.  Optionally specify a `allocator` used
    /// to supply memory (e.g., the address of a `bslma::Allocator` object).
    /// If `allocator` is not specified, the currently installed default
    /// allocator is used.
    AllocTestType(const AllocTestType&  original,
                  const allocator_type& allocator = allocator_type());

    /// Destroy this object.
    ~AllocTestType();

    // MANIPULATORS

    /// Assign to this object the value of the specified `rhs` object, and
    /// return a reference providing modifiable access to this object.
    AllocTestType& operator=(const AllocTestType& rhs);

    /// Set the `data` attribute of this object to the specified `value`.
    void setData(int value);

    // ACCESSORS

    /// Return the value of the `data` attribute of this object.
    int data() const;

                                  // Aspects

    /// Return `get_allocator().mechanism()`.
    bslma::Allocator *allocator() const;

    /// Return the allocator used by this object to supply memory.
    allocator_type get_allocator() const;
};

// FREE OPERATORS

/// Return `true` if the specified `lhs` and `rhs` objects have the same
/// value, and `false` otherwise.  Two `AllocTestType` objects have the same
/// if their `data` attributes are the same.
bool operator==(const AllocTestType& lhs, const AllocTestType& rhs);

/// Return `true` if the specified `lhs` and `rhs` objects do not have the
/// same value, and `false` otherwise.  Two `AllocTestType` objects do not
/// have the same value if their `data` attributes are not the same.
bool operator!=(const AllocTestType& lhs, const AllocTestType& rhs);

// ============================================================================
//                  INLINE AND TEMPLATE FUNCTION IMPLEMENTATIONS
// ============================================================================

                        // -------------------
                        // class AllocTestType
                        // -------------------

// MANIPULATORS
inline
void AllocTestType::setData(int value)
{
    *d_data_p = value;
}

// ACCESSORS
inline
int AllocTestType::data() const
{
    return *d_data_p;
}

                                  // Aspects

inline
bslma::Allocator *AllocTestType::allocator() const
{
    return d_allocator.mechanism();
}

inline
AllocTestType::allocator_type AllocTestType::get_allocator() const
{
    return d_allocator;
}

}  // close package namespace

// FREE OPERATORS
inline
bool bsltf::operator==(const AllocTestType& lhs, const AllocTestType& rhs)
{
    return lhs.data() == rhs.data();
}

inline
bool bsltf::operator!=(const AllocTestType& lhs, const AllocTestType& rhs)
{
    return lhs.data() != rhs.data();
}

}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
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
