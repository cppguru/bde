// bdlma_pool.h                                                       -*-C++-*-
#ifndef INCLUDED_BDLMA_POOL
#define INCLUDED_BDLMA_POOL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide efficient allocation of memory blocks of uniform size.
//
//@CLASSES:
//  bdlma::Pool: memory manager that allocates memory blocks of uniform size
//
//@DESCRIPTION: This component implements a memory pool, `bdlma::Pool`, that
// allocates and manages maximally-aligned memory blocks of some uniform size
// specified at construction.  A `bdlma::Pool` object maintains an internal
// linked list of free memory blocks, and dispenses one block for each
// `allocate` method invocation.  When a memory block is deallocated, it is
// returned to the free list for potential reuse.
//
// Whenever the linked list of free memory blocks is depleted, the
// `bdlma::Pool` replenishes the list by first allocating a large, contiguous
// "chunk" of memory, then splitting the chunk into multiple memory blocks.  A
// chunk and its constituent memory blocks can be depicted visually:
// ```
//    +-----+--- memory blocks of uniform size
//    |     |
//  ----- ----- ------------
// |     |     |     ...    |
//  =====^=====^============
//
//  \___________ __________/
//              V
//          a "chunk"
// ```
// Note that the size of the allocated chunk is determined by both the growth
// strategy and maximum blocks per chunk, either of which can be optionally
// specified at construction (see the "Configuration at Construction" section).
//
///Configuration at Construction
///-----------------------------
// When creating a `bdlma::Pool`, clients must specify the specific block size
// managed and dispensed by the pool.  Furthermore, clients can optionally
// configure:
//
// 1. GROWTH STRATEGY -- geometrically growing chunk size starting from 1 (in
//    terms of the number of memory blocks per chunk), or fixed chunk size.  If
//    the growth strategy is not specified, geometric growth is used.
// 2. MAX BLOCKS PER CHUNK -- the maximum number of memory blocks within a
//    chunk.  If the maximum blocks per chunk is not specified, an
//    implementation-defined default value is used.
// 3. BASIC ALLOCATOR -- the allocator used to supply memory to replenish the
//    internal pool.  If not specified, the currently installed default
//    allocator is used (see `bslma_default`).
//
// For example, if geometric growth is used and the maximum blocks per chunk is
// specified as 30, the chunk size grows geometrically, starting from 1, until
// the specified maximum blocks per chunk, as follows:
// ```
// 1, 2, 4, 8, 16, 30, 30, 30 ...
// ```
// If constant growth is used, the chunk size is always the specified maximum
// blocks per chunk (or an implementation-defined value if the maximum blocks
// per chunk is not specified), for example:
// ```
// 30, 30, 30 ...
// ```
// A default-constructed pool has an initial chunk size of 1 (i.e., the number
// of memory blocks of a given size allocated at once to replenish a pool's
// memory), and the pool's chunk size grows geometrically until it reaches an
// implementation-defined maximum, at which it is capped.  Finally, unless
// otherwise specified, all memory comes from the allocator that was the
// currently installed default allocator at the time the `bdlma::Pool` was
// created.
//
///Overloaded Global Operator `new`
///--------------------------------
// This component overloads the global `operator new` to allow convenient
// syntax for the construction of objects using a `bdlma::Pool`.  The `new`
// operator supplied in this component takes a `bdlma::Pool` argument
// indicating the source of the memory.  Consider the following use of standard
// placement `new` syntax (supplied by `bsl_new.h`) along with a `bdlma::Pool`
// to allocate an object of type `T`.  Note that the size of `T` must be the
// same or smaller than the `blockSize` with which the pool is constructed:
// ```
// void f(bdlma::Pool *pool)
// {
//     assert(pool->blockSize() >= sizeof(T));
//
//     T *t = new (pool->allocate()) T(...);
//
//     // ...
// }
// ```
// This usage style is not exception-safe.  If the constructor of `T` throws an
// exception, `pool->deallocate` is never called.
//
// Supplying an overloaded global `operator new`:
// ```
// ::operator new(bsl::size_t size, BloombergLP::bdlma::Pool& pool);
// ```
// allows for the following cleaner usage, which does not require the size
// calculation and guarantees that `pool->deallocate` *is* called in the case
// of an exception:
// ```
// void f(bdlma::Pool *pool)
// {
//     assert(pool->blockSize() >= sizeof(T));
//
//     T *t = new (*pool) T(...);
//
//     // ...
// ```
// Also note that the analogous version of operator `delete` should *not* be
// called directly.  Instead, this component provides a static template member
// function `deleteObject`, parameterized on `TYPE`:
// ```
//     pool->deleteObject(t);
// }
// ```
// The above `deleteObject` call is equivalent to performing the following:
// ```
// t->~TYPE();
// pool->deallocate(t);
// ```
// An overloaded operator `delete` is supplied solely to allow the compiler to
// arrange for it to be called in case of an exception.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Using a `bdlma::Pool` for Efficient Memory Allocation
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// A `bdlma::Pool` can be used by node-based containers (such as lists, trees,
// and hash tables that hold multiple elements of uniform size) for efficient
// memory allocation of new elements.  The following container template class,
// `my_PooledArray`, stores values of (template parameter) `TYPE`
// "out-of-place" as nodes in a `vector` of pointers.  Since the size of each
// node is fixed and known *a priori*, the class uses a `bdlma::Pool` to
// allocate memory for the nodes to improve memory allocation efficiency.  Note
// that for simplicity, we assume that `TYPE` does not require an allocator,
// and that calls to the destructor of `TYPE` can be elided.
//
// First, we define the interface of our `my_PooledArray` template class:
// ```
// // my_poolarray.h
//
// /// This class implements a container that stores values of (template
// /// parameter) `TYPE` out-of-place.  It is assumed that `TYPE` does not
// /// require an allocator, and that calls to the destructor of `TYPE` can
// /// be elided.
// template <class TYPE>
// class my_PooledArray {
//
//     // DATA
//     bsl::vector<TYPE *> d_array_p;  // array of pooled elements
//     bdlma::Pool         d_pool;     // memory manager for array elements
//
//   private:
//     // Not implemented:
//     my_PooledArray(const my_PooledArray&);
//
//   public:
//     // CREATORS
//
//     /// Create a pooled array that stores the `TYPE` element values
//     /// "out-of-place".  Optionally specify a `basicAllocator` used to
//     /// supply memory.  If `basicAllocator` is 0, the currently
//     /// installed default allocator is used.
//     explicit my_PooledArray(bslma::Allocator *basicAllocator = 0);
//
//     /// Destroy this array and all elements held by it.
//     ~my_PooledArray();
//
//     // MANIPULATORS
//
//     /// Append the specified `value` to this array.
//     void append(const TYPE& value);
//
//     /// Remove all elements from this array.
//     void removeAll();
//
//     // ACCESSORS
//
//     /// Return the number of elements in this array.
//     bsl::size_t length() const;
//
//     /// Return a reference providing non-modifiable access to the value
//     /// at the specified `index` in this array.  The behavior is
//     /// undefined unless `0 <= index < length()`.
//     const TYPE& operator[](int index) const;
// };
// ```
// Next, we provide the implementation of the `my_PooledArray` methods that are
// defined `inline`.
//
// Note that in the `removeAll` method, all elements are deallocated by simply
// invoking the pool's `release` method.  This technique implies significant
// performance gain when the array contains many elements:
// ```
// // MANIPULATORS
// template <class TYPE>
// inline
// void my_PooledArray<TYPE>::removeAll()
// {
//     d_array_p.clear();
//     d_pool.release();
// }
//
// // ACCESSORS
// template <class TYPE>
// inline
// bsl::size_t my_PooledArray<TYPE>::length() const
// {
//     return d_array_p.size();
// }
//
// template <class TYPE>
// inline
// const TYPE& my_PooledArray<TYPE>::operator[](int index) const
// {
//     assert(0     <= index);
//     assert(index <  static_cast<int>(length()));
//
//     return *d_array_p[index];
// }
// ```
// Next, we provide the implementation of the `my_PooledArray` methods that are
// defined in the `.cpp` file.
//
// Note that the growth strategy and maximum chunk size of the pool defaults to
// those provided by `bdlma::Pool`:
// ```
// // my_poolarray.cpp
//
// // CREATORS
// template <class TYPE>
// my_PooledArray<TYPE>::my_PooledArray(bslma::Allocator *basicAllocator)
// : d_array_p(basicAllocator)
// , d_pool(sizeof(TYPE), basicAllocator)
// {
// }
// ```
// Since all memory is managed by `d_pool`, we do not have to explicitly invoke
// `deleteObject` to reclaim outstanding memory.  The destructor of the pool
// will automatically deallocate all array elements:
// ```
// template <class TYPE>
// my_PooledArray<TYPE>::~my_PooledArray()
// {
//     // Elements are automatically deallocated when 'd_pool' is destroyed.
// }
// ```
// Finally, note that the overloaded "placement" `new` is used to allocate new
// nodes in the `append` method:
// ```
// // MANIPULATORS
// template <class TYPE>
// void my_PooledArray<TYPE>::append(const TYPE& value)
// {
//     TYPE *tmp = new (d_pool) TYPE(value);
//     d_array_p.push_back(tmp);
// }
// ```

#include <bdlscm_version.h>

#include <bdlma_infrequentdeleteblocklist.h>

#include <bslma_allocator.h>
#include <bslma_deleterhelper.h>

#include <bsls_alignmentutil.h>
#include <bsls_assert.h>
#include <bsls_blockgrowth.h>
#include <bsls_types.h>

#include <bsl_cstddef.h>

namespace BloombergLP {
namespace bdlma {

                                // ==========
                                // class Pool
                                // ==========

/// This class implements a memory pool that allocates and manages memory
/// blocks of some uniform size specified at construction.  This memory pool
/// maintains an internal linked list of free memory blocks, and dispenses
/// one block for each `allocate` method invocation.  When a memory block is
/// deallocated, it is returned to the free list for potential reuse.
class Pool {

    // PRIVATE TYPES

    /// This `struct` implements a link data structure that stores the
    /// address of the next link, and is used to implement the internal
    /// linked list of free memory blocks.  Note that this type is
    /// replicated in `bdlma_pool.cpp` to provide access to a compatible
    /// type from static methods defined in `bdlma_pool.cpp`.
    struct Link {

        Link *d_next_p;  // pointer to next link
    };

    // DATA
    bsls::Types::size_type  d_blockSize;          // size (in bytes) of each
                                                  // allocated memory block
                                                  // returned to client

    bsls::Types::size_type  d_internalBlockSize;  // actual size of each block
                                                  // maintained on free list
                                                  // (contains overhead for
                                                  // 'Link')

    int                     d_chunkSize;          // current chunk size (in
                                                  // blocks-per-chunk)

    int                     d_maxBlocksPerChunk;  // maximum chunk size (in
                                                  // blocks-per-chunk)

    bsls::BlockGrowth::Strategy
                            d_growthStrategy;     // growth strategy of the
                                                  // chunk size

    Link                   *d_freeList_p;         // linked list of free memory
                                                  // blocks

    InfrequentDeleteBlockList
                            d_blockList;          // memory manager for
                                                  // allocated memory

    char                   *d_begin_p;            // start of a contiguous
                                                  // group of memory blocks

    char                   *d_end_p;              // end of a contiguous group
                                                  // of memory blocks

  private:
    // PRIVATE MANIPULATORS

    /// Dynamically allocate a new chunk using this pool's underlying growth
    /// strategy.
    void replenish();

  private:
    // NOT IMPLEMENTED
    Pool(const Pool&);
    Pool& operator=(const Pool&);

  public:
    // CREATORS

    /// Create a memory pool that returns blocks of contiguous memory of the
    /// specified `blockSize` (in bytes) for each `allocate` method
    /// invocation.  Optionally specify a `growthStrategy` used to control
    /// the growth of internal memory chunks (from which memory blocks are
    /// dispensed).  If `growthStrategy` is not specified, geometric growth
    /// is used.  Optionally specify `maxBlocksPerChunk` as the maximum
    /// chunk size if `growthStrategy` is specified.  If geometric growth is
    /// used, the chunk size grows starting at `blockSize`, doubling in size
    /// until the size is exactly `blockSize * maxBlocksPerChunk`.  If
    /// constant growth is used, the chunk size is always
    /// `blockSize * maxBlocksPerChunk`.  If `maxBlocksPerChunk` is not
    /// specified, an implementation-defined value is used.  Optionally
    /// specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is 0, the currently installed default allocator is
    /// used.  The behavior is undefined unless `1 <= blockSize` and
    /// `1 <= maxBlocksPerChunk`.
    explicit
    Pool(bsls::Types::size_type  blockSize,
         bslma::Allocator       *basicAllocator = 0);
    Pool(bsls::Types::size_type       blockSize,
         bsls::BlockGrowth::Strategy  growthStrategy,
         bslma::Allocator            *basicAllocator = 0);
    Pool(bsls::Types::size_type       blockSize,
         bsls::BlockGrowth::Strategy  growthStrategy,
         int                          maxBlocksPerChunk,
         bslma::Allocator            *basicAllocator = 0);

    /// Destroy this pool, releasing all associated memory back to the
    /// underlying allocator.
    ~Pool();

    // MANIPULATORS

    /// Return the address of a contiguous block of maximally-aligned memory
    /// having the fixed block size specified at construction.
    void *allocate();

    /// Relinquish the memory block at the specified `address` back to this
    /// pool object for reuse.  The behavior is undefined unless `address` is
    /// non-zero, was allocated by this pool, and has not already been
    /// deallocated.
    void deallocate(void *address);

    /// Destroy the specified `object` based on its dynamic type and then
    /// use this pool to deallocate its memory footprint.  This method has
    /// no effect if `object` is 0.  The behavior is undefined unless
    /// `object`, when cast appropriately to `void *`, was allocated using
    /// this pool and has not already been deallocated.  Note that
    /// `dynamic_cast<void *>(object)` is applied if `TYPE` is polymorphic,
    /// and `static_cast<void *>(object)` is applied otherwise.
    template <class TYPE>
    void deleteObject(const TYPE *object);

    /// Destroy the specified `object` and then use this pool to deallocate its
    /// memory footprint.  This method has no effect if `object` is 0.  The
    /// behavior is undefined unless `object` is **not** a secondary base class
    /// pointer (i.e., the address is (numerically) the same as when it was
    /// originally dispensed by this pool), was allocated using this pool, and
    /// has not already been deallocated.
    template <class TYPE>
    void deleteObjectRaw(const TYPE *object);

    /// Relinquish all memory currently allocated via this pool object.
    void release();

    /// Reserve memory from this pool to satisfy memory requests for at least
    /// the specified `numBlocks` before the pool replenishes.  The behavior is
    /// undefined unless `0 <= numBlocks`.
    void reserveCapacity(int numBlocks);

    // ACCESSORS

    /// Return the size (in bytes) of the memory blocks allocated from this
    /// pool object.  Note that all blocks dispensed by this pool have the same
    /// size.
    bsls::Types::size_type blockSize() const;

                                  // Aspects

    /// Return the allocator used by this object to allocate memory.  Note that
    /// this allocator can not be used to deallocate memory allocated through
    /// this pool.
    bslma::Allocator *allocator() const;
};

}  // close package namespace
}  // close enterprise namespace

// Note that the `new` and `delete` operators are declared outside the
// `BloombergLP` namespace so that they do not hide the standard placement
// `new` and `delete` operators (i.e.,
// `void *operator new(bsl::size_t, void *)` and
// `void operator delete(void *)`).
//
// Also note that only the scalar versions of operators `new` and `delete` are
// provided, because overloading `new` (and `delete`) with their array versions
// would cause dangerous ambiguity.  Consider what would have happened had we
// overloaded the array version of `operator new`:
// ```
//  void *operator new[](bsl::size_t size, BloombergLP::bdlma::Pool& pool);
// ```
// A user of `bdlma::Pool` may expect to be able to use array `operator new` as
// follows:
// ```
//   new (*pool) my_Type[...];
// ```
// The problem is that this expression returns an array that cannot be safely
// deallocated.  On the one hand, there is no syntax in C++ to invoke an
// overloaded `operator delete`; on the other hand, the pointer returned by
// `operator new` cannot be passed to the `deallocate` method directly because
// the pointer is different from the one returned by the `allocate` method.
// The compiler offsets the value of this pointer by a header, which is used to
// maintain the number of objects in the array (so that `operator delete` can
// destroy the right number of objects).

// FREE OPERATORS

/// Return a block of memory of the specified `size` (in bytes) allocated
/// from the specified `pool`.  The behavior is undefined unless `size` is
/// the same or smaller than the `blockSize` with which `pool` was
/// constructed.  Note that an object may allocate additional memory
/// internally, requiring the allocator to be passed in as a constructor
/// argument:
/// ```
/// my_Type *newMyType(bdlma::Pool *pool, bslma::Allocator *basicAllocator)
/// {
///     return new (*pool) my_Type(..., basicAllocator);
/// }
/// ```
/// Also note that the analogous version of `operator delete` should not be
/// called directly.  Instead, this component provides a static template
/// member function, `deleteObject`, parameterized by `TYPE`:
/// ```
/// void deleteMyType(my_Type *t, bdlma::Pool *pool)
/// {
///     pool->deleteObject(t);
/// }
/// ```
/// `deleteObject` performs the following:
/// ```
/// t->~my_Type();
/// pool->deallocate(t);
/// ```
void *operator new(bsl::size_t size, BloombergLP::bdlma::Pool& pool);

/// Use the specified `pool` to deallocate the memory at the specified
/// `address`.  The behavior is undefined unless `address` is non-zero, was
/// allocated using `pool`, and has not already been deallocated.  Note that
/// this operator is supplied solely to allow the compiler to arrange for it
/// to be called in the case of an exception.
void operator delete(void *address, BloombergLP::bdlma::Pool& pool);

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

namespace BloombergLP {
namespace bdlma {

                                // ----------
                                // class Pool
                                // ----------

// MANIPULATORS
inline
void *Pool::allocate()
{
    if (d_begin_p == d_end_p) {
        if (d_freeList_p) {
            Link *p      = d_freeList_p;
            d_freeList_p = p->d_next_p;
            return p;                                                 // RETURN
        }

        replenish();
    }

    char *p = d_begin_p;
    d_begin_p += d_internalBlockSize;
    return p;
}

inline
void Pool::deallocate(void *address)
{
    BSLS_ASSERT_SAFE(address);

    static_cast<Link *>(address)->d_next_p = d_freeList_p;
    d_freeList_p = static_cast<Link *>(address);
}

template <class TYPE>
inline
void Pool::deleteObject(const TYPE *object)
{
    bslma::DeleterHelper::deleteObject(object, this);
}

template <class TYPE>
inline
void Pool::deleteObjectRaw(const TYPE *object)
{
    bslma::DeleterHelper::deleteObjectRaw(object, this);
}

inline
void Pool::release()
{
    d_blockList.release();
    d_freeList_p = 0;
    d_begin_p = 0;
    d_end_p = 0;
}

// ACCESSORS
inline
bsls::Types::size_type Pool::blockSize() const
{
    return d_blockSize;
}

// Aspects

inline
bslma::Allocator *Pool::allocator() const
{
    return d_blockList.allocator();
}

}  // close package namespace
}  // close enterprise namespace

// FREE OPERATORS
inline
void *operator new(bsl::size_t size, BloombergLP::bdlma::Pool& pool)
{
    using namespace BloombergLP;

    BSLS_ASSERT_SAFE(size <= pool.blockSize() &&
        bsls::AlignmentUtil::calculateAlignmentFromSize(size)
         <= bsls::AlignmentUtil::calculateAlignmentFromSize(pool.blockSize()));

    static_cast<void>(size);  // suppress "unused parameter" warnings
    return pool.allocate();
}

inline
void operator delete(void *address, BloombergLP::bdlma::Pool& pool)
{
    BSLS_ASSERT_SAFE(address);

    pool.deallocate(address);
}

#endif

// ----------------------------------------------------------------------------
// Copyright 2016 Bloomberg Finance L.P.
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
