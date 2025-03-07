// bslma_destructionutil.h                                            -*-C++-*-
#ifndef INCLUDED_BSLMA_DESTRUCTIONUTIL
#define INCLUDED_BSLMA_DESTRUCTIONUTIL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide routines that destroy objects efficiently.
//
//@CLASSES:
//  bslma::DestructionUtil: namespace for routines that destroy objects
//
//@SEE_ALSO: bslma_constructionutil
//
//@DESCRIPTION: This component provides utilities to destroy scalars with a
// uniform interface, but select a different implementation according to the
// traits possessed by the underlying type.
//
// The trait under consideration by this component is:
// ```
// Trait                             Note
// -------------------------------   -------------------------------------
// bslmf::IsBitwiseCopyable          Expressed in English as "TYPE has the
//                                   bit-wise copyable trait", or "TYPE is
//                                   bit-wise copyable", this trait also
//                                   implies that destructor calls can be
//                                   elided with no effect on observable
//                                   behavior.
//
// ```
//
///Usage
///-----
// In this section we show intended use of this component.  Note that this
// component is for use by the `bslstl` package.  Other clients should use the
// STL algorithms (in header `<algorithm>` and `<memory>`).
//
///Example 1: Destroy `int` and an Integer Wrapper
///- - - - - - - - - - - - - - - - - - - - - - - -
// In this example, we will use `bslma::DestructionUtil` to destroy both a
// scalar integer and a `MyInteger` type object.  Calling the `destroy` method
// on a scalar integer is a no-op while calling the `destroy` method on an
// object of `MyInteger` class invokes the destructor of the object.
//
// First, we define a `MyInteger` class that represents an integer value:
// ```
// /// This class represents an integer value.
// class MyInteger {
//
//     // DATA
//     int d_intValue;  // integer value
//
//   public:
//     // CREATORS
//
//     /// Create a `MyInteger` object having integer value `0`.
//     MyInteger();
//
//     /// Create a `MyInteger` object having the specified `value`.
//     explicit MyInteger(int value);
//
//     /// Destroy this object.
//     ~MyInteger();
//
//     // ACCESSORS
//     int getValue() const;
// };
// ```
// Then, we create an object, `myInteger`, of type `MyInteger`:
// ```
// bsls::ObjectBuffer<MyInteger> buffer;
// MyInteger *myInteger = &buffer.object();
// new (myInteger) MyInteger(1);
// ```
// Notice that we use an `ObjectBuffer` to allow us to safely invoke the
// destructor explicitly.
//
// Now, we define a primitive integer:
// ```
// int scalarInteger = 2;
// ```
// Finally, we use the uniform `bslma::DestructionUtil::destroy`
// method to destroy both `myInteger` and `scalarInteger`:
// ```
// bslma::DestructionUtil::destroy(myInteger);
// bslma::DestructionUtil::destroy(&scalarInteger);
// ```

#include <bslscm_version.h>

#include <bslmf_integralconstant.h>
#include <bslmf_isbitwisecopyable.h>
#include <bslmf_removecv.h>

#include <bsls_assert.h>
#include <bsls_platform.h>

#include <stddef.h>  // 'size_t'
#include <string.h>  // 'memset'

namespace BloombergLP {

namespace bslma {

                           // ======================
                           // struct DestructionUtil
                           // ======================

/// This `struct` provides a namespace for a suite of utility functions that
/// destroy elements of the parameterized type `TYPE`.  Depending on the
/// traits of `TYPE`, the destructor may be invoked or not (i.e., optimized
/// away as a no-op).
struct DestructionUtil {

  private:
    // PRIVATE CLASS METHODS

    /// Destroy the object of the parameterized `TYPE` at the specified
    /// `address` if the second argument is of type `bsl::false_type`, and
    /// do nothing otherwise.  This method is a no-op if the second argument
    /// is of type `bsl::true_type`, indicating that the object at `address`
    /// is bit-wise copyable.  Note that the second argument is for overload
    /// resolution only and its value is ignored.
    template <class TYPE>
    static void destroy(TYPE *address, bsl::true_type);
    template <class TYPE>
    static void destroy(const TYPE *address, bsl::true_type);
    template <class TYPE>
    static void destroy(TYPE *address, bsl::false_type);

    /// Write the specified `numBytes` bytes of arbitary values at the
    /// specified `address`.  Note that this function is deliberately out
    /// of line to avoid compiler warnings, most importantly a spurious
    /// gcc-11 warning https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101854.
    static void scribbleOverMemory(void *address, size_t numBytes);

  public:
    // CLASS METHODS

    /// Destroy the specified `object` of the parameterized `TYPE`, as if by
    /// calling the `TYPE` destructor, but do not deallocate the memory
    /// occupied by `object`.  Note that the destructor may deallocate other
    /// memory owned by `object`.  Also note that this function is a no-op
    /// if the `TYPE` has the trivial destructor trait.
    template <class TYPE>
    static void destroy(TYPE *object);

};

// ============================================================================
//                  INLINE AND TEMPLATE FUNCTION DEFINITIONS
// ============================================================================

                          // ----------------------
                          // struct DestructionUtil
                          // ----------------------

// PRIVATE CLASS METHODS
template <class TYPE>
inline
void DestructionUtil::destroy(TYPE *address, bsl::true_type)
{
#ifdef BSLS_ASSERT_SAFE_IS_ACTIVE
    scribbleOverMemory(address, sizeof(TYPE));
#endif
    (void)address;
}

template <class TYPE>
inline
void DestructionUtil::destroy(const TYPE *address, bsl::true_type)
{
    // No-op.

    (void) address;
}

#ifdef BSLS_PLATFORM_CMP_MSVC
#pragma warning( push )           //  For some reason, VC2008 does not detect
#pragma warning( disable : 4100 ) //  that 'address' is used.
#endif

template <class TYPE>
inline
void DestructionUtil::destroy(TYPE *address, bsl::false_type)
{
#ifndef BSLS_PLATFORM_CMP_SUN
    address->~TYPE();
#else
    // Workaround for a bug in Sun's CC whereby destructors cannot be called on
    // 'const' objects of polymorphic types.

    typedef bsl::remove_cv<TYPE>::type NoCvType;
    const_cast<NoCvType *>(address)->~NoCvType();
#endif
}

#ifdef BSLS_PLATFORM_CMP_MSVC
#pragma warning( pop )
#endif

// CLASS METHODS
template <class TYPE>
inline
void DestructionUtil::destroy(TYPE *object)
{
    BSLS_ASSERT_SAFE(object);

    destroy(object, typename bslmf::IsBitwiseCopyable<TYPE>::type());
}

}  // close package namespace
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
