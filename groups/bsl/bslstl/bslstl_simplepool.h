// bslstl_simplepool.h                                                -*-C++-*-
#ifndef INCLUDED_BSLSTL_SIMPLEPOOL
#define INCLUDED_BSLSTL_SIMPLEPOOL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide efficient allocation of memory blocks for a specific type.
//
//@CLASSES:
//  bslstl::SimplePool: memory manager that allocates memory blocks for a type
//
//@SEE_ALSO: bslstl_treenodepool, bdlma_pool
//
//@DESCRIPTION: This component implements a memory pool, `bslstl::SimplePool`,
// that allocates and manages memory blocks of for a parameterized type.  A
// `bslstl::SimplePool` object maintains an internal linked list of
// free memory blocks, and dispenses one block for each `allocate` method
// invocation.  When a memory block is deallocated, it is returned to the free
// list for potential reuse.
//
// Whenever the linked list of free memory blocks is depleted,
// `bslstl::SimplePool` replenishes the list by first allocating a large,
// contiguous "chunk" of memory, then splitting the chunk into multiple memory
// blocks each having the `sizeof` the simple pool's parameterized type.  A
// chunk and its constituent memory blocks can be depicted visually:
// ```
//    +-----+--- memory blocks of uniform size for parameterized type
//    |     |
//  ----- ----- ------------
// |     |     |     ...    |
//  =====^=====^============
//
//  \___________ __________/
//              V
//          a "chunk"
// ```
// This pool implementation is simple because its allocation strategy is not
// configurable.  The size of a chunk starts from 1 memory block, and doubles
// each time a chunk is allocated up to an implementation defined maximum
// number of blocks.
//
///Comparison with `bdema_Pool`
///----------------------------
// There are a few differences between `bslstl::SimplePool` and `bdema_Pool`:
// 1. `bslstl::SimplePool` is parameterized on both allocator and type, which
//    improve performance and memory usage in exchange for increase in code
//    size.
// 2. `bslstl::SimplePool` uses the allocator through the use of
//    `bsl::allocator_traits` (which is generally not relevant to non-container
//    type.
// 3. `bslstl::SimplePool` is less configurable in order to achieve abstraction
//    of allocation and improvement in performance.
//
// Clients are encouraged to use `bdema_Pool` as `bslstl::SimplePool` is
// designed for node-based STL containers, and its pooling behavior may change
// according to the needs of those containers.
//
///Usage
///-----
// This section illustrates intended use for this component.
//
///Example 1: Creating a Node-Based Stack
/// - - - - - - - - - - - - - - - - - - -
// Suppose that we want to implement a stack with a linked list.  It is
// expensive to allocate memory every time a node is inserted.  Therefore, we
// can use `SimplePool` to efficiently manage the memory for the list.
//
// First, we define the class that implements the stack:
// ```
// /// This class defines a node-based stack of integers.
// template <class ALLOCATOR = bsl::allocator<int> >
// class my_Stack {
//
//     // PRIVATE TYPES
//
//     /// This `struct` implements a link data structure containing a
//     /// value and a pointer to the next node.
//     struct Node {
//
//         int   d_value;   // payload value
//         Node *d_next_p;  // pointer to the next node
//     };
//
//     typedef bslstl::SimplePool<Node, ALLOCATOR> Pool;
//         // Alias for memory pool.
//
//   private:
//     // DATA
//     Node *d_head_p;  // pointer to the first node
//     int   d_size;    // size of the stack
//     Pool  d_pool;    // memory manager for the stack
//
//   public:
//     // CREATORS
//
//     /// Create an empty `my_Stack` object.  Optionally specify a
//     /// `basicAllocator` used to supply memory.  If `basicAllocator` is
//     /// 0, the currently installed default allocator is used.
//     my_Stack(const ALLOCATOR& allocator = ALLOCATOR());
//
//     // MANIPULATORS
//
//     /// Insert an element with the specified value to the top of this
//     /// stack.
//     void push(int value);
//
//     /// Remove the top element from this stack.  The behavior is
//     /// undefined unless `1 <= size()`.
//     void pop();
//
//     // ACCESSORS
//
//     /// Return the value of the element on the top of this stack.  The
//     /// behavior is undefined unless `1 <= size()`.
//     int top();
//
//     /// Return the number of elements in this stack.
//     std::size_t size();
// };
// ```
// Now, we define the implementation of the stack.  Notice how
// `bslstl::SimplePool` is used to allocate memory in `push` and deallocate
// memory in `pop`:
// ```
// // CREATORS
// template <class ALLOCATOR>
// my_Stack<ALLOCATOR>::my_Stack(const ALLOCATOR& allocator)
// : d_head_p(0)
// , d_size(0)
// , d_pool(allocator)
// {
// }
//
// // MANIPULATORS
// template <class ALLOCATOR>
// void my_Stack<ALLOCATOR>::push(int value)
// {
//     Node *newNode = d_pool.allocate();
//
//     newNode->d_value = value;
//     newNode->d_next_p = d_head_p;
//     d_head_p = newNode;
//
//     ++d_size;
// }
//
// template <class ALLOCATOR>
// void my_Stack<ALLOCATOR>::pop()
// {
//     BSLS_ASSERT(0 != size());
//
//     Node *n = d_head_p;
//     d_head_p = d_head_p->d_next_p;
//     d_pool.deallocate(n);
//     --d_size;
// }
//
// // ACCESSORS
// template <class ALLOCATOR>
// int my_Stack<ALLOCATOR>::top()
// {
//     BSLS_ASSERT(0 != size());
//
//     return d_head_p->d_value;
// }
//
// template <class ALLOCATOR>
// std::size_t my_Stack<ALLOCATOR>::size()
// {
//     return d_size;
// }
// ```
// Finally, we test our stack by pushing and popping some elements:
// ```
// my_Stack stack;
// stack.push(1);
// stack.push(2);
// stack.push(3);
// stack.push(4);
// stack.push(5);
// assert(5 == stack.size());
//
// assert(5 == stack.top());
// stack.pop();
// assert(4 == stack.top());
// stack.pop();
// assert(3 == stack.top());
// stack.pop();
// assert(2 == stack.top());
// stack.pop();
// assert(1 == stack.top());
// stack.pop();
// assert(0 == stack.size());
// ```

#include <bslscm_version.h>

#include <bslma_allocatortraits.h>
#include <bslma_allocatorutil.h>

#include <bslmf_movableref.h>

#include <bsls_alignmentfromtype.h>
#include <bsls_alignmentutil.h>
#include <bsls_assert.h>
#include <bsls_platform.h>

#include <algorithm>  // swap (C++03)
#include <utility>    // swap (C++17)

namespace BloombergLP {
namespace bslstl {

                       // ======================
                       // struct SimplePool_Type
                       // ======================

/// For use only by `bslstl::SimplePool`.  This `struct` provides a
/// namespace for a set of types used to define the base-class of a
/// `SimplePool`.  The parameterized `ALLOCATOR` is bound to
/// `MaxAlignedType` to ensure the allocated memory is maximally aligned.
template <class ALLOCATOR>
struct SimplePool_Type {

    /// Alias for the allocator traits rebound to allocate
    /// `bsls::AlignmentUtil::MaxAlignedType`.
    typedef typename bsl::allocator_traits<ALLOCATOR>::template
            rebind_traits<bsls::AlignmentUtil::MaxAlignedType> AllocatorTraits;

    /// Alias for the allocator type for
    /// `bsls::AlignmentUtil::MaxAlignedType`.
    typedef typename AllocatorTraits::allocator_type AllocatorType;
};

                       // ================
                       // class SimplePool
                       // ================

/// This class provides methods for creating and deleting nodes using the
/// appropriate allocator-traits of the parameterized `ALLOCATOR`.
/// This type is intended to be used as a private base-class for a
/// node-based container, in order to take advantage of the
/// empty-base-class optimization in the case where the base-class has 0
/// size (as may the case if the parameterized `ALLOCATOR` is not a
/// `bslma::Allocator`).
template <class VALUE, class ALLOCATOR>
class SimplePool : public SimplePool_Type<ALLOCATOR>::AllocatorType {

    // PRIVATE TYPES
    typedef SimplePool_Type<ALLOCATOR> Types;

    enum { k_MAX_BLOCKS_PER_CHUNK = 32 };

    /// This `union` implements a link data structure with the size no
    /// smaller than `VALUE` that stores the address of the next link.
    /// It is used to implement the internal linked list of free memory
    /// blocks.
    union Block {

        Block *d_next_p;                    // pointer to the next block

        char   d_size[sizeof(VALUE)];       // make a block has the size of at
                                            // least `VALUE`

        typename bsls::AlignmentFromType<VALUE>::Type d_alignment;
                                            // ensure proper alignment
    };

    /// This `union` prepends to the beginning of each managed block of
    /// allocated memory, implementing a singly-linked list of managed
    /// chunks, and thereby enabling constant-time additions to the list of
    /// chunks.
    union Chunk {

        struct {
            Chunk                                      *d_next_p;
            typename Types::AllocatorTraits::size_type  d_numBytes;
        } d_info;

        typename bsls::AlignmentFromType<Block>::Type d_alignment;
                          // ensure each block is correctly aligned

        /// Return a pointer to the first block in this chunk.
        Block *firstBlock()
            { return reinterpret_cast<Block *>(this + 1); }
    };

  public:
    // TYPES

    /// Alias for the parameterized type `VALUE`.
    typedef VALUE ValueType;

    /// Alias for the allocator type for a
    /// `bsls::AlignmentUtil::MaxAlignedType`.
    typedef typename Types::AllocatorType AllocatorType;

    /// Alias for the allocator traits for the parameterized
    /// `ALLOCATOR`.
    typedef typename Types::AllocatorTraits AllocatorTraits;

    typedef typename AllocatorTraits::size_type size_type;

  private:
    // DATA
    Chunk *d_chunkList_p;     // linked list of "chunks" of memory

    Block *d_freeList_p;      // linked list of free memory blocks

    int    d_blocksPerChunk;  // current chunk size (in blocks-per-chunk)

  private:
    // NOT IMPLEMENTED
    SimplePool& operator=(bslmf::MovableRef<SimplePool>);
    SimplePool& operator=(const SimplePool&);
    SimplePool(const SimplePool&);

  private:
    // PRIVATE MANIPULATORS

    /// Allocate a chunk of memory having enough usable blocks for the
    /// specified `numBlocks` objects of type `VALUE`, add the chunk to the
    /// chunk list, and return the address of the chunk header.
    Chunk *allocateChunk(std::size_t numBlocks);

    /// Deallocate a chunk of memory at the specified `chunk_p` address.
    /// The behavior is undefined unless `chunk_p` was allocate from this
    /// pool using `allocateChunk` and not yet deallocated.
    void deallocateChunk(Chunk *chunk_p);

    /// Dynamically allocate a new chunk using the pool's underlying growth
    /// strategy, and use the chunk to replenish the free memory list of
    /// this pool.
    void replenish();

  public:
    // CREATORS

    /// Create a memory pool that returns blocks of contiguous memory of the
    /// size of the parameterized `VALUE` using the specified `allocator` to
    /// supply memory.  The chunk size grows starting with at least
    /// `sizeof(VALUE)`, doubling in size up to an implementation defined
    /// maximum number of blocks per chunk.
    explicit SimplePool(const ALLOCATOR& allocator);

    /// Create a memory pool, adopting all outstanding memory allocations
    /// associated with the specified `original` pool, that returns blocks
    /// of contiguous memory of the sizeof the paramterized `VALUE` using
    /// the allocator associated with `original`.  The chunk size is set to
    /// that of `original` and continues to double in size up to an
    /// implementation defined maximum number of blocks per chunk.  Note
    /// that `original` is left in a valid but unspecified state.
    SimplePool(bslmf::MovableRef<SimplePool> original);

    /// Destroy this pool, releasing all associated memory back to the
    /// underlying allocator.
    ~SimplePool();

    // MANIPULATORS

    /// Adopt all outstanding memory allocations associated with the
    /// specified memory `pool`.  The behavior is undefined unless this pool
    /// uses the same allocator as that associated with `pool`.  The
    /// behavior is undefined unless this pool is in the default-constructed
    /// state.
    void adopt(bslmf::MovableRef<SimplePool> pool);

    /// Return a reference providing modifiable access to the rebound
    /// allocator traits for the node-type.  Note that this operation
    /// returns a base-class (`AllocatorType`) reference to this object.
    AllocatorType& allocator();

    /// Return the address of a block of memory of at least the size of
    /// `VALUE`.  Note that the memory is *not* initialized.
    VALUE *allocate();

    /// Relinquish the memory block at the specified `address` back to this
    /// pool object for reuse.  The behavior is undefined unless `address`
    /// is non-zero, was allocated by this pool, and has not already been
    /// deallocated.
    void deallocate(void *address);

    /// Dynamically allocate a new chunk containing the specified
    /// `numBlocks` number of blocks, and add the chunk to the free memory
    /// list of this pool.  The additional memory is added irrespective of
    /// the amount of free memory when called.  The behavior is undefined
    /// unless `0 < numBlocks`.
    void reserve(size_type numBlocks);

    /// Relinquish all memory currently allocated via this pool object.
    void release();

    /// Efficiently exchange the memory blocks of this object with those of
    /// the specified `other` object.  This method provides the no-throw
    /// exception-safety guarantee.  The behavior is undefined unless
    /// `allocator() == other.allocator()`.
    void swap(SimplePool& other);

    /// Efficiently exchange the memory blocks of this object with those of
    /// the specified `other` object.  This method provides the no-throw
    /// exception-safety guarantee.  The behavior is undefined unless
    /// `allocator() == other.allocator()`.
    void quickSwapRetainAllocators(SimplePool& other);

    /// Efficiently exchange the memory blocks and the allocator of this
    /// object with those of the specified `other` object.  This method
    /// provides the no-throw exception-safety guarantee.
    void quickSwapExchangeAllocators(SimplePool& other);

    // ACCESSORS

    /// Return a reference providing non-modifiable access to the rebound
    /// allocator traits for the node-type.  Note that this operation
    /// returns a base-class (`AllocatorType`) reference to this object.
    const AllocatorType& allocator() const;

    /// Return `true` if this object holds free (currently unused) blocks,
    /// and `false` otherwise.
    bool hasFreeBlocks() const;
};

// ============================================================================
//                  TEMPLATE AND INLINE FUNCTION DEFINITIONS
// ============================================================================

// PRIVATE MANIPULATORS
template <class VALUE, class ALLOCATOR>
typename SimplePool<VALUE, ALLOCATOR>::Chunk *
SimplePool<VALUE, ALLOCATOR>::allocateChunk(std::size_t numBlocks)
{
    std::size_t       numBytes  = sizeof(Chunk) + sizeof(Block) * numBlocks;
    const std::size_t alignment = bsls::AlignmentFromType<Chunk>::VALUE;

    Chunk *chunkPtr =
        static_cast<Chunk *>(bslma::AllocatorUtil::allocateBytes(allocator(),
                                                                 numBytes,
                                                                 alignment));

    BSLS_ASSERT_SAFE(0 ==
             reinterpret_cast<bsls::Types::UintPtr>(chunkPtr) % alignment);

    chunkPtr->d_info.d_next_p   = d_chunkList_p;
    chunkPtr->d_info.d_numBytes = numBytes;
    d_chunkList_p               = chunkPtr;

    return chunkPtr;
}

template <class VALUE, class ALLOCATOR>
inline void
SimplePool<VALUE, ALLOCATOR>::deallocateChunk(Chunk *chunk_p)
{
    std::size_t       numBytes  = chunk_p->d_info.d_numBytes;
    const std::size_t alignment = bsls::AlignmentFromType<Chunk>::VALUE;

    bslma::AllocatorUtil::deallocateBytes(allocator(), chunk_p,
                                          numBytes, alignment);
}

template <class VALUE, class ALLOCATOR>
inline
void SimplePool<VALUE, ALLOCATOR>::replenish()
{
    reserve(d_blocksPerChunk);

    if (d_blocksPerChunk < k_MAX_BLOCKS_PER_CHUNK) {
        d_blocksPerChunk *= 2;
    }
}

// CREATORS
template <class VALUE, class ALLOCATOR>
inline
SimplePool<VALUE, ALLOCATOR>::SimplePool(const ALLOCATOR& allocator)
: AllocatorType(allocator)
, d_chunkList_p(0)
, d_freeList_p(0)
, d_blocksPerChunk(1)
{
}

template <class VALUE, class ALLOCATOR>
inline
SimplePool<VALUE, ALLOCATOR>::SimplePool(
                                        bslmf::MovableRef<SimplePool> original)
: AllocatorType(bslmf::MovableRefUtil::access(original).allocator())
, d_chunkList_p(bslmf::MovableRefUtil::access(original).d_chunkList_p)
, d_freeList_p(bslmf::MovableRefUtil::access(original).d_freeList_p)
, d_blocksPerChunk(bslmf::MovableRefUtil::access(original).d_blocksPerChunk)
{
    SimplePool& lvalue = original;
    lvalue.d_chunkList_p = 0;
    lvalue.d_freeList_p = 0;
    lvalue.d_blocksPerChunk = 1;
}

template <class VALUE, class ALLOCATOR>
inline
SimplePool<VALUE, ALLOCATOR>::~SimplePool()
{
    release();
}

// MANIPULATORS
template <class VALUE, class ALLOCATOR>
inline
void
SimplePool<VALUE, ALLOCATOR>::adopt(bslmf::MovableRef<SimplePool> pool)
{
    BSLS_ASSERT_SAFE(0 == d_chunkList_p);
    BSLS_ASSERT_SAFE(0 == d_freeList_p);
    BSLS_ASSERT_SAFE(allocator()
                           == bslmf::MovableRefUtil::access(pool).allocator());

    SimplePool& lvalue = pool;
    d_chunkList_p = lvalue.d_chunkList_p;
    d_freeList_p = lvalue.d_freeList_p;
    d_blocksPerChunk = lvalue.d_blocksPerChunk;

    lvalue.d_chunkList_p = 0;
    lvalue.d_freeList_p = 0;
    lvalue.d_blocksPerChunk = 1;
}

template <class VALUE, class ALLOCATOR>
inline
typename SimplePool<VALUE, ALLOCATOR>::AllocatorType&
SimplePool<VALUE, ALLOCATOR>::allocator()
{
    return *this;
}

template <class VALUE, class ALLOCATOR>
inline
VALUE *SimplePool<VALUE, ALLOCATOR>::allocate()
{
    if (!d_freeList_p) {
        replenish();
    }
    VALUE *block = reinterpret_cast<VALUE *>(d_freeList_p);
    d_freeList_p = d_freeList_p->d_next_p;
    return block;
}

template <class VALUE, class ALLOCATOR>
inline
void SimplePool<VALUE, ALLOCATOR>::deallocate(void *address)
{
    BSLS_ASSERT_SAFE(address);

    reinterpret_cast<Block *>(address)->d_next_p = d_freeList_p;
    d_freeList_p = reinterpret_cast<Block *>(address);
}

template <class VALUE, class ALLOCATOR>
inline
void SimplePool<VALUE, ALLOCATOR>::swap(SimplePool<VALUE, ALLOCATOR>& other)
{
    BSLS_ASSERT_SAFE(allocator() == other.allocator());

    std::swap(d_blocksPerChunk, other.d_blocksPerChunk);
    std::swap(d_freeList_p, other.d_freeList_p);
    std::swap(d_chunkList_p, other.d_chunkList_p);
}

template <class VALUE, class ALLOCATOR>
inline
void SimplePool<VALUE, ALLOCATOR>::quickSwapRetainAllocators(
                                           SimplePool<VALUE, ALLOCATOR>& other)
{
    swap(other);
}

template <class VALUE, class ALLOCATOR>
inline
void SimplePool<VALUE, ALLOCATOR>::quickSwapExchangeAllocators(
                                           SimplePool<VALUE, ALLOCATOR>& other)
{
    // We don't know which operation (copy/move assignment or swap) this
    // function is being called for, but if any of the propagation traits are
    // true, then the allocator must support assignment, so we turn propagation
    // on for all of them.
    typedef bsl::allocator_traits<ALLOCATOR> AllocTraits;
    typedef bsl::integral_constant<
        bool,
        AllocTraits::propagate_on_container_copy_assignment::value ||
        AllocTraits::propagate_on_container_move_assignment::value ||
        AllocTraits::propagate_on_container_swap::value> Propagate;

    using std::swap;
    using BloombergLP::bslma::AllocatorUtil;
    AllocatorUtil::swap(&this->allocator(), &other.allocator(), Propagate());
    swap(d_blocksPerChunk,  other.d_blocksPerChunk);
    swap(d_freeList_p,      other.d_freeList_p);
    swap(d_chunkList_p,     other.d_chunkList_p);
}

template <class VALUE, class ALLOCATOR>
void SimplePool<VALUE, ALLOCATOR>::reserve(size_type numBlocks)
{
    BSLS_ASSERT(0 < numBlocks);

    Block *begin = allocateChunk(numBlocks)->firstBlock();
    Block *end   = begin + numBlocks - 1;  // last block, NOT past-the-end

    // The last block is deliberately excluded from this loop.
    for (Block *p = begin; p < end; ++p) {
        p->d_next_p = p + 1;
    }
    end->d_next_p = d_freeList_p;  // Handle the last block here
    d_freeList_p  = begin;
}

template <class VALUE, class ALLOCATOR>
void SimplePool<VALUE, ALLOCATOR>::release()
{
    while (d_chunkList_p) {
        Chunk *lastChunk = d_chunkList_p;
        d_chunkList_p    = d_chunkList_p->d_info.d_next_p;
        deallocateChunk(lastChunk);
    }

    d_freeList_p = 0;
}

// ACCESSORS
template <class VALUE, class ALLOCATOR>
inline
const typename SimplePool<VALUE, ALLOCATOR>::AllocatorType&
SimplePool<VALUE, ALLOCATOR>::allocator() const
{
    return *this;
}

template <class VALUE, class ALLOCATOR>
inline
bool SimplePool<VALUE, ALLOCATOR>::hasFreeBlocks() const
{
    return d_freeList_p;
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
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
