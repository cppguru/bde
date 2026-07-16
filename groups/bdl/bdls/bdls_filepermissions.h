// bdls_filepermissions.h                                             -*-C++-*-
#ifndef INCLUDED_BDLS_FILEPERMISSIONS
#define INCLUDED_BDLS_FILEPERMISSIONS

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Enumerate the file-system permission bits used across `bdl`.
//
//@CLASSES:
//  bdls::FilePermissions: namespace for a `std::filesystem::perms`-style enum
//
//@SEE_ALSO: bdls_filesystemutil, balb_pipecontrolchannel
//
//@DESCRIPTION: This component provides a `struct`, `bdls::FilePermissions`,
// that scopes a bit-mask `enum`, `Enum`, modelled on `std::filesystem::perms`
// [https://en.cppreference.com/w/cpp/filesystem/perms].  The enumerators
// describe the standard Unix file-system permission bits (owner, group, and
// others read/write/execute triples, plus the "set uid", "set gid", and
// sticky bits) with numeric values that match the traditional Unix octal
// representation.  This component also provides two static predicates for
// validating an `int` bit mask against the set of defined bits.
//
// This component is intended to be used by other components that accept a
// permission bit mask as an `int` argument.  Taking an `int` (rather than
// the enumeration type) lets callers pass either a familiar octal literal
// (for example `0666`) or a bitwise-OR of `bdls::FilePermissions` enumerators.
// The consuming component is expected to validate its argument by calling
// one of the `isValid*` predicates.
//
///Enumerators
///-----------
// The `Enum` values are:
// ```
//  Enumerator          Value  Meaning
//  ----------------- --------  ----------------------------------------------
//  k_NONE                  0  No permissions.
//  k_OWNER_READ         0400  Owner has read permission.
//  k_OWNER_WRITE        0200  Owner has write permission.
//  k_OWNER_EXEC         0100  Owner has execute permission.
//  k_OWNER_ALL          0700  Owner has read, write, and execute permission.
//  k_GROUP_READ         0040  Group has read permission.
//  k_GROUP_WRITE        0020  Group has write permission.
//  k_GROUP_EXEC         0010  Group has execute permission.
//  k_GROUP_ALL          0070  Group has read, write, and execute permission.
//  k_OTHERS_READ        0004  Others have read permission.
//  k_OTHERS_WRITE       0002  Others have write permission.
//  k_OTHERS_EXEC        0001  Others have execute permission.
//  k_OTHERS_ALL         0007  Others have read, write, and execute perm.
//  k_ALL                0777  Owner, group, and others all have full perms.
//  k_SET_UID           04000  Set-user-ID on execution.
//  k_SET_GID           02000  Set-group-ID on execution.
//  k_STICKY_BIT        01000  Sticky bit.
//  k_MASK              07777  Union of every defined bit.
// ```
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Composing and Validating Permissions
///- - - - - - - - - - - - - - - - - - - - - - - -
// A component (for instance, `balb::PipeControlChannel`) may accept an `int`
// permission bit mask.  A caller can build such a mask by OR-ing together
// enumerators of `bdls::FilePermissions`.  Suppose we want a file readable
// and writable by the owner and readable by the owning group:
// ```
//  int perms = bdls::FilePermissions::k_OWNER_READ  |
//              bdls::FilePermissions::k_OWNER_WRITE |
//              bdls::FilePermissions::k_GROUP_READ;
//  assert(0640 == perms);
// ```
// The consuming component may validate the mask before using it.  To reject
// a mask that contains any bit outside the traditional 9-bit rwx set (that
// is, to reject `k_SET_UID`, `k_SET_GID`, and `k_STICKY_BIT`), use
// `isValidBaseBits`:
// ```
//  assert( bdls::FilePermissions::isValidBaseBits(0640));
//  assert(!bdls::FilePermissions::isValidBaseBits(0640 |
//                                bdls::FilePermissions::k_SET_UID));
// ```
// To accept any combination of defined bits, use `isValid`:
// ```
//  assert( bdls::FilePermissions::isValid(0640 |
//                                bdls::FilePermissions::k_SET_UID));
//  assert(!bdls::FilePermissions::isValid(1 << 15));
// ```

#include <bdlscm_version.h>

#include <bsl_iosfwd.h>

namespace BloombergLP {
namespace bdls {

                          // ======================
                          // struct FilePermissions
                          // ======================

/// This `struct` provides a namespace for a `std::filesystem::perms`-style
/// enumeration of file-system permission bits, and a pair of predicates
/// for validating an `int` bit mask against that enumeration.
struct FilePermissions {

    // TYPES

    /// File-system permission bits, in the conventional Unix octal layout.
    /// Values may be combined with the bitwise-OR operator.  See the
    /// component-level documentation for the meaning of each enumerator.
    enum Enum {
        k_NONE         = 0,

        k_OWNER_READ   = 0400,
        k_OWNER_WRITE  = 0200,
        k_OWNER_EXEC   = 0100,
        k_OWNER_ALL    = k_OWNER_READ  | k_OWNER_WRITE  | k_OWNER_EXEC,

        k_GROUP_READ   = 0040,
        k_GROUP_WRITE  = 0020,
        k_GROUP_EXEC   = 0010,
        k_GROUP_ALL    = k_GROUP_READ  | k_GROUP_WRITE  | k_GROUP_EXEC,

        k_OTHERS_READ  = 0004,
        k_OTHERS_WRITE = 0002,
        k_OTHERS_EXEC  = 0001,
        k_OTHERS_ALL   = k_OTHERS_READ | k_OTHERS_WRITE | k_OTHERS_EXEC,

        k_ALL          = k_OWNER_ALL   | k_GROUP_ALL    | k_OTHERS_ALL,

        k_SET_UID      = 04000,
        k_SET_GID      = 02000,
        k_STICKY_BIT   = 01000,

        k_MASK         = k_ALL | k_SET_UID | k_SET_GID | k_STICKY_BIT
    };

    // CLASS METHODS

    /// Return `true` if the specified `permissions` bit mask consists only
    /// of the "base" nine rwx bits (owner/group/others read/write/execute,
    /// i.e., bits in `k_ALL`), and `false` otherwise.  Note that this
    /// predicate is stricter than `isValid`: `k_SET_UID`, `k_SET_GID`, and
    /// `k_STICKY_BIT` are rejected.
    static bool isValidBaseBits(int permissions);

    /// Return `true` if the specified `permissions` bit mask consists only
    /// of bits defined in `Enum` (i.e., bits in `k_MASK`), and `false`
    /// otherwise.  A negative value always returns `false`.
    static bool isValid(int permissions);

    /// Write the octal numeric representation of the specified permission bit
    /// mask `value` to the specified output `stream`, and return a reference
    /// to `stream`.  Optionally specify an initial indentation `level`, whose
    /// absolute value is incremented recursively for nested objects.  If
    /// `level` is specified, optionally specify `spacesPerLevel`, whose
    /// absolute value indicates the number of spaces per indentation level for
    /// this and all of its nested objects.  If `level` is negative, suppress
    /// indentation of the first line.  If `spacesPerLevel` is negative, format
    /// the entire output on one line, suppressing all but the initial
    /// indentation (as governed by `level`).  The `value` is written in octal
    /// notation *without* the customary leading `0` prefix (for example, the
    /// mask `0640` is written as `640`).
    static bsl::ostream& print(bsl::ostream&         stream,
                               FilePermissions::Enum value,
                               int                   level          = 0,
                               int                   spacesPerLevel = 4);
};

// FREE OPERATORS

/// Write the octal numeric representation of the specified permission bit mask
/// `value` to the specified output `stream` in a single-line format, and
/// return a reference to `stream`.  The `value` is written in octal notation
/// *without* the customary leading `0` prefix (for example, the mask `0640` is
/// written as `640`).  Note that this method has the same behavior as
/// ```
/// bdls::FilePermissions::print(stream, value, 0, -1);
/// ```
bsl::ostream& operator<<(bsl::ostream&         stream,
                         FilePermissions::Enum value);

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                          // ----------------------
                          // struct FilePermissions
                          // ----------------------

// CLASS METHODS
inline
bool FilePermissions::isValidBaseBits(int permissions)
{
    return 0 <= permissions
        && 0 == (permissions & ~static_cast<int>(k_ALL));
}

inline
bool FilePermissions::isValid(int permissions)
{
    return 0 <= permissions
        && 0 == (permissions & ~static_cast<int>(k_MASK));
}

}  // close package namespace

// FREE OPERATORS
inline
bsl::ostream& bdls::operator<<(bsl::ostream&         stream,
                               FilePermissions::Enum value)
{
    return FilePermissions::print(stream, value, 0, -1);
}

}  // close enterprise namespace

#endif

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
