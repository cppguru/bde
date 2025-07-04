// bdls_filesystemutil.h                                              -*-C++-*-
#ifndef INCLUDED_BDLS_FILESYSTEMUTIL
#define INCLUDED_BDLS_FILESYSTEMUTIL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide methods for filesystem access with multi-language names.
//
//@CLASSES:
//  bdls::FilesystemUtil: namespace for filesystem access methods
//
//@SEE_ALSO: bdls_pathutil
//
//@DESCRIPTION: This component provides a platform-independent interface to
// filesystem utility methods, supporting multi-language file and path names.
// Each method in the `bdls::FilesystemUtil` namespace is a thin wrapper on top
// of the operating system's own filesystem access functions, providing a
// consistent and unambiguous interface for handling files on all supported
// platforms.
//
// Methods in this component can be used to manipulate files with any name in
// any language on all supported platforms.  To provide such support, the
// following restrictions are applied to file names and patterns passed to
// methods of this component: On Windows, all file names and patterns must be
// passed as UTF-8-encoded strings; file search results will similarly be
// encoded as UTF-8.  On Posix, file names and patterns may be passed in any
// encoding, but all processes accessing a given file must encode its name in
// the same encoding.  On modern Posix installations, this effectively means
// that file names and patterns should be encoded in UTF-8, just as on Windows.
// See the section "Platform-Specific File Name Encoding Caveats" below.
//
///Policies for `open`
///-------------------
// The behavior of the `open` method is governed by three sets of enumerations:
//
///Open/Create Policy: `bdls::FilesystemUtil::FileOpenPolicy`
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// `bdls::FilesystemUtil::FileOpenPolicy` governs whether `open` creates a new
// file or opens an existing one.  The following values are possible:
//
// * `e_OPEN`
//   > Open an existing file.
//
// * `e_CREATE`
//   > Create a new file.
//
// * `e_CREATE_PRIVATE`
//   > Create a new file, with limited permissions where that is supported
//   > (e.g. not necessarily Microsoft Windows).
//
// * `e_OPEN_OR_CREATE`
//   > Open a file if it exists, and create a new file otherwise.
//
///Input/Output Access Policy: `bdls::FilesystemUtil::FileIOPolicy`
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// `bdls::FilesystemUtil::FileIOPolicy` governs what Input/Output operations
// are allowed on a file after it is opened.  The following values are
// possible:
//
// * `e_READ_ONLY`
//   > Allow reading only.
//
// * `e_WRITE_ONLY`
//   > Allow writing only.
//
// * `e_READ_WRITE`
//   > Allow both reading and writing.
//
// * `e_APPEND_ONLY`
//   > Allow appending to end-of-file only.
//
// * `e_READ_APPEND`
//   > Allow both reading and appending to end-of-file.
//
///Truncation Policy: `bdls::FilesystemUtil::FileTruncatePolicy`
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// `bdls::FilesystemUtil::FileTruncatePolicy` governs whether `open` deletes
// the existing contents of a file when it is opened.  The following values are
// possible:
//
// * `e_TRUNCATE`
//   > Delete the file's contents.
//
// * `e_KEEP`
//   > Keep the file's contents.
//
///Starting Points for `seek`
///--------------------------
// The behavior of the `seek` method is governed by an enumeration that
// determines the point from which the seek operation starts:
//
// * `e_SEEK_FROM_BEGINNING`
//   > Seek from the beginning of the file.
//
// * `e_SEEK_FROM_CURRENT`
//   > Seek from the current position in the file.
//
// * `e_SEEK_FROM_END`
//   > Seek from the end of the file.
//
///Platform-Specific File Locking Caveats
///--------------------------------------
// Locking has the following caveats for the following operating systems:
//
// * On Posix, closing a file releases all locks on all file descriptors
//   referring to that file within the current process.  [doc 1] [doc 2]
// * On Posix, the child of a fork does not inherit the locks of the parent
//   process.  [doc 1] [doc 2]
// * On at least some flavors of Unix, you can't lock a file for writing using
//   a file descriptor opened in read-only mode.
//
///Platform-Specific Atomicity Caveats
///-----------------------------------
// The `bdls::FilesystemUtil::read` and `bdls::FilesystemUtil::write` methods
// add no atomicity guarantees for reading and writing to those provided (if
// any) by the underlying platform's methods for reading and writing (see
// `http://lwn.net/articles/180387/`).
//
///Platform-Specific File Name Encoding Caveats
///--------------------------------------------
// File-name encodings have the following caveats for the following operating
// systems:
//
// * On Windows, methods of `bdls::FilesystemUtil` that take a file or
//   directory name or pattern as a `char*` or `bsl::string` type assume that
//   the name is encoded in UTF-8.  The routines attempt to convert the name
//   to a UTF-16 `wchar_t` string via `bdlde::CharConvertUtf16::utf8ToUtf16`,
//   and if the conversion succeeds, call the Windows wide-character `W` APIs
//   with the UTF-16 name.  If the conversion fails, the method fails.
//   Similarly, file searches returning file names call the Windows
//   wide-character `W` APIs and convert the resulting UTF-16 names to UTF-8.
//   - Narrow-character file names in other encodings, containing characters
//     with values in the range 128 - 255, will likely result in files being
//     created with names that appear garbled if the conversion from UTF-8 to
//     UTF-16 happens to succeed.
//   - Neither `utf8ToUtf16` nor the Windows `W` APIs do any normalization of
//     the UTF-16 strings resulting from UTF-8 conversion, and it is therefore
//     possible to have sets of file names that have the same visual
//     representation but are treated as different names by the filesystem.
// * On Posix, a file name or pattern supplied to methods of
//   `bdls::FilesystemUtil` as a `char*` or `bsl::string` type is passed
//   unchanged to the underlying system file APIs.  Because the file names and
//   patterns are passed unchanged, `bdls::FilesystemUtil` methods will work
//   correctly on Posix with any encoding, but will *interoperate* only with
//   processes that use the same encoding as the current process.
// * For compatibility with most modern Posix installs, and consistency with
//   this component's Windows API, best practice is to encode all file names
//   and patterns in UTF-8.
//
///File Truncation Caveats
///-----------------------
// In order to provide consistent behavior across both Posix and Windows
// platforms, when the `open` method is called, file truncation is allowed only
// if the client requests an `openPolicy` containing the word `CREATE` and/or
// an `ioPolicy` containing the word `WRITE`.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: General Usage
/// - - - - - - - - - - - -
// In this example, we start with a (relative) native path to a directory
// containing log files:
// ```
// #ifdef BSLS_PLATFORM_OS_WINDOWS
//   bsl::string logPath = "temp.1\\logs";
// #else
//   bsl::string logPath = "temp.1/logs";
// #endif
// ```
// Suppose that we want to separate files into "old" and "new" subdirectories
// on the basis of modification time.  We will provide paths representing these
// locations, and create the directories if they do not exist:
// ```
// bsl::string oldPath(logPath), newPath(logPath);
// bdls::PathUtil::appendRaw(&oldPath, "old");
// bdls::PathUtil::appendRaw(&newPath, "new");
// int rc = bdls::FilesystemUtil::createDirectories(oldPath, true);
// assert(0 == rc);
// rc = bdls::FilesystemUtil::createDirectories(newPath, true);
// assert(0 == rc);
// ```
// We know that all of our log files match the pattern "*.log", so let's search
// for all such files in the log directory:
// ```
// bdls::PathUtil::appendRaw(&logPath, "*.log");
// bsl::vector<bsl::string> logFiles;
// bdls::FilesystemUtil::findMatchingPaths(&logFiles, logPath.c_str());
// ```
// Now for each of these files, we will get the modification time.  Files that
// are older than 2 days will be moved to "old", and the rest will be moved to
// "new":
// ```
// bdlt::Datetime modTime;
// bsl::string   fileName;
// for (bsl::vector<bsl::string>::iterator it = logFiles.begin();
//                                               it != logFiles.end(); ++it) {
//   assert(0 ==
//              bdls::FilesystemUtil::getLastModificationTime(&modTime, *it));
//   assert(0 == bdls::PathUtil::getLeaf(&fileName, *it));
//   bsl::string *whichDirectory =
//               2 < (bdlt::CurrentTime::utc() - modTime).totalDays()
//               ? &oldPath
//               : &newPath;
//   bdls::PathUtil::appendRaw(whichDirectory, fileName.c_str());
//   assert(0 == bdls::FilesystemUtil::move(it->c_str(),
//                                        whichDirectory->c_str()));
//   bdls::PathUtil::popLeaf(whichDirectory);
// }
// ```
//
///Example 2: Using `bdls::FilesystemUtil::visitPaths`
///- - - - - - - - - - - - - - - - - - - - - - - - - -
// `bdls::FilesystemUtil::visitPaths` enables clients to define a function
// object to operate on file paths that match a specified pattern.  In this
// example, we create a function that can be used to filter out files that have
// a last modified time within a particular time frame.
//
// First we define our filtering function:
// ```
// void getFilesWithinTimeframe(bsl::vector<bsl::string> *vector,
//                              const char               *item,
//                              const bdlt::Datetime&     start,
//                              const bdlt::Datetime&     end)
// {
//     bdlt::Datetime datetime;
//     int ret = bdls::FilesystemUtil::getLastModificationTime(&datetime,
//                                                             item);
//
//     if (ret) {
//         return;                                                   // RETURN
//     }
//
//     if (datetime < start || datetime > end) {
//         return;                                                   // RETURN
//     }
//
//     vector->push_back(item);
// }
// ```
// Then, with the help of `bdls::FilesystemUtil::visitPaths` and
// `bdlf::BindUtil::bind`, we create a function for finding all file paths that
// match a specified pattern and have a last modified time within a specified
// start and end time (both specified as a `bdlt::Datetime`):
// ```
// void findMatchingFilesInTimeframe(bsl::vector<bsl::string> *result,
//                                   const char               *pattern,
//                                   const bdlt::Datetime&     start,
//                                   const bdlt::Datetime&     end)
// {
//     result->clear();
//     bdls::FilesystemUtil::visitPaths(
//                              pattern,
//                              bdlf::BindUtil::bind(&getFilesWithinTimeframe,
//                                                   result,
//                                                   bdlf::PlaceHolders::_1,
//                                                   start,
//                                                   end));
// }
// ```

#include <bdlscm_version.h>

#include <bdls_filesystemutil_unixplatform.h>

#include <bdlt_datetime.h>
#include <bdlt_epochutil.h>

#include <bslmf_assert.h>

#include <bsls_assert.h>

#include <bsl_functional.h>
#include <bsl_optional.h>

#include <bsls_keyword.h>
#include <bsls_libraryfeatures.h>
#include <bsls_platform.h>
#include <bsls_review.h>

#include <bsl_string.h>
#include <bsl_string_view.h>
#include <bsl_vector.h>
#include <bsl_cstddef.h>

#include <string>           // 'std::string', 'std::pmr::string'
#include <vector>

#include <sys/types.h>

namespace BloombergLP {
namespace bdls {

                           // =====================
                           // struct FilesystemUtil
                           // =====================

/// This `struct` provides a namespace for utility functions dealing with
/// platform-independent filesystem access.
struct FilesystemUtil {

    // TYPES
#ifdef BSLS_PLATFORM_OS_WINDOWS
    /// `HANDLE` is a stand-in for the Windows API `HANDLE` type, to allow
    /// us to avoid including `windows.h` in this header.  `HANDLE` should
    /// not be used by client code.
    typedef void *HANDLE;

    /// `FileDescriptor` is an alias for the operating system`s native file
    /// descriptor / file handle type.
    typedef HANDLE FileDescriptor;

    /// `Offset` is an alias for a signed value, representing the offset of
    /// a location within a file.
    typedef __int64 Offset;

    /// maximum representable file offset value
    static const Offset k_OFFSET_MAX = _I64_MAX;

    /// minimum representable file offset value
    static const Offset k_OFFSET_MIN = _I64_MIN;

#elif defined(BSLS_PLATFORM_OS_UNIX)
    /// `FileDescriptor` is an alias for the operating system's native file
    /// descriptor / file handle type.
    typedef int FileDescriptor;

#if defined(BDLS_FILESYSTEMUTIL_UNIXPLATFORM_64_BIT_OFF64)
    /// `Offset` is an alias for a signed value, representing the offset of
    /// a location within a file.
    typedef off64_t Offset;
#else
    /// `Offset` is an alias for a signed value, representing the offset of
    /// a location within a file.
    typedef off_t Offset;
#endif

    /// maximum representable file offset value
    static const Offset k_OFFSET_MAX =  (0x7FFFFFFFFFFFFFFFLL);

    /// minimum representable file offset value
    static const Offset k_OFFSET_MIN = (-0x7FFFFFFFFFFFFFFFLL-1);
#else
# error "'bdls_filesystemutil' does not support this platform."
#endif

    /// Enumeration used to distinguish among different starting points for
    /// a seek operation.
    enum Whence {
        e_SEEK_FROM_BEGINNING = 0,  // Seek from beginning of file.
        e_SEEK_FROM_CURRENT   = 1,  // Seek from current position.
        e_SEEK_FROM_END       = 2   // Seek from end of file.
    };

    enum {
        k_DEFAULT_FILE_GROWTH_INCREMENT = 0x10000  // default block size to
                                                   // grow files by
    };

    enum ErrorType {
        k_ERROR_LOCKING_CONFLICT    =  1,  // value representing a failure to
                                           // obtain a lock on a file

        k_ERROR_LOCKING_INTERRUPTED =  2,  // value representing a failure to
                                           // obtain a lock on a file due to
                                           // interruption by a signal

        k_ERROR_ALREADY_EXISTS      =  3,  // value representing a failure to
                                           // create a directory due to a file
                                           // system entry already existing

        k_ERROR_PATH_NOT_FOUND      =  4,  // value representing a failure to
                                           // create a directory due to one ore
                                           // more components of the path
                                           // either not existing or not being
                                           // a directory

        k_ERROR_PAST_EOF            =  5,  // 'mapChecked' attempted to map
                                           // region past the end of file.

        k_BAD_FILE_DESCRIPTOR       = -1   // value indicating a bad file
                                           // descriptor was supplied
    };

    /// Enumeration used to determine whether 'open' should open an existing
    /// file, or create a new file.
    enum FileOpenPolicy {
        e_OPEN,           // Open a file if it exists, and fail otherwise.

        e_CREATE,         // Create a new file, and fail if the file already
                          // exists.

        e_CREATE_PRIVATE, // Create a new file with access restricted to the
                          // creating userid, where supported, and fail if the
                          // file already exists.

        e_OPEN_OR_CREATE  // Open a file if it exists, and create a new file
                          // otherwise.
    };

    /// Enumeration used to distinguish between different sets of actions
    /// permitted on an open file descriptor.
    enum FileIOPolicy {
        e_READ_ONLY,    // Allow reading only.
        e_WRITE_ONLY,   // Allow writing only.
        e_APPEND_ONLY,  // Allow appending to end-of-file only.
        e_READ_WRITE,   // Allow both reading and writing.
        e_READ_APPEND   // Allow both reading and appending to end-of-file.
    };

    /// Enumeration used to distinguish between different ways to handle the
    /// contents, if any, of an existing file immediately upon opening the
    /// file.
    enum FileTruncatePolicy {
        e_TRUNCATE,  // Delete the file's contents on open.
        e_KEEP       // Keep the file's contents.
    };

    // CLASS DATA
    static const FileDescriptor k_INVALID_FD;  // `FileDescriptor` value
                                               // representing no file, used
                                               // as the error return for
                                               // `open`

    // CLASS METHODS

    /// Open the file at the specified `path`, using the specified
    /// `openPolicy` to determine whether to open an existing file or create
    /// a new file, and using the specified `ioPolicy` to determine whether
    /// the file will be opened for reading, writing, or both.  Optionally
    /// specify a `truncatePolicy` to determine whether any contents of the
    /// file will be deleted before `open` returns.  If `truncatePolicy` is
    /// not supplied, the value `e_KEEP` will be used.  Return a valid
    /// `FileDescriptor` for the file on success, or `k_INVALID_FD`
    /// otherwise.  If `openPolicy` is `e_OPEN`, the file will be opened if
    /// it exists, and `open` will fail otherwise.  If `openPolicy` is
    /// `e_CREATE` or `e_CREATE_PRIVATE`, and no file exists at `path`, a
    /// new file will be created, and `open` will fail otherwise.  If
    /// `openPolicy` is `e_CREATE_PRIVATE`, the file will be created with
    /// access restricted to the same userid as the caller in environments
    /// where that is supported (which does not necessarily include Windows)
    /// otherwise the system default access policy is used (e.g. '0777 &
    /// ~umask').  If `openPolicy` is `e_OPEN_OR_CREATE`, the file will be
    /// opened if it exists, and a new file will be created otherwise.  If
    /// `ioPolicy` is `e_READ_ONLY`, the returned `FileDescriptor` will
    /// allow only read operations on the file.  If `ioPolicy` is
    /// `e_WRITE_ONLY` or `e_APPEND_ONLY`, the returned `FileDescriptor`
    /// will allow only write operations on the file.  If `ioPolicy` is
    /// `e_READ_WRITE` or `e_READ_APPEND`, the returned `FileDescriptor`
    /// will allow both read and write operations on the file.
    /// Additionally, if `ioPolicy` is `e_APPEND_ONLY` or `e_READ_APPEND`
    /// all writes will be made to the end of the file ("append mode").  If
    /// `truncatePolicy` is `e_TRUNCATE`, the file will have zero length
    /// when `open` returns.  If `truncatePolicy` is `e_KEEP`, the file will
    /// be opened with its existing contents, if any.  Note that when a file
    /// is opened in `append` mode, all writes will go to the end of the
    /// file, even if there has been seeking on the file descriptor or
    /// another process has changed the length of the file.  Append-mode
    /// writes are not atomic except in limited cases; another thread, or
    /// even another process, operating on the file may cause output not to
    /// be written, unbroken, to the end of the file. (Unix environments
    /// writing to local file systems may promise more.) Note that `open`
    /// will fail to open a file with a `truncatePolicy` of `e_TRUNCATE`
    /// unless at least one of the following policies is specified for
    /// `openPolicy` or `ioPolicy`:
    /// * `e_CREATE`
    /// * `e_CREATE_PRIVATE`
    /// * `e_OPEN_OR_CREATE`
    /// * 'e_WRITE_ONLY
    /// * `e_READ_WRITE`
    /// The parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static FileDescriptor open(const char         *path,
                               FileOpenPolicy      openPolicy,
                               FileIOPolicy        ioPolicy,
                               FileTruncatePolicy  truncatePolicy = e_KEEP);
    template <class STRING_TYPE>
    static FileDescriptor open(const STRING_TYPE& path,
                               FileOpenPolicy     openPolicy,
                               FileIOPolicy       ioPolicy,
                               FileTruncatePolicy truncatePolicy = e_KEEP);


    /// Close the specified `descriptor`.  Return 0 on success and a non-zero
    /// value otherwise.  A return value of `k_BAD_FILE_DESCRIPTOR` indicates
    /// that the supplied `descriptor` is invalid.
    static int close(FileDescriptor descriptor);

    /// Load into the specified 'path' the absolute pathname of the current
    /// working directory.  Return 0 on success and a non-zero value otherwise.
    static int getWorkingDirectory(bsl::string *path);
    static int getWorkingDirectory(std::string *path);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int getWorkingDirectory(std::pmr::string *path);
#endif

    /// Set the working directory of the current process to the specified
    /// `path`.  Return 0 on success and a non-zero value otherwise.  The
    /// parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static int setWorkingDirectory(const char *path);
    template <class STRING_TYPE>
    static int setWorkingDirectory(const STRING_TYPE& path);

    /// Return `true` if there currently exists a file or directory at the
    /// specified `path`, and `false` otherwise.  If `path` is a symlink,
    /// the result of this function is platform dependent. On POSIX/Unix
    /// platforms this method dereferences symlinks, while on Windows it
    /// does not.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    static bool exists(const char *path);
    template <class STRING_TYPE>
    static bool exists(const STRING_TYPE& path);

    /// Return `true` if there currently exists a regular file at the
    /// specified `path`, and `false` otherwise.  If there is a symbolic
    /// link at `path`, follow it only if the optionally specified
    /// `followLinksFlag` is `true` (otherwise, return `false` as the
    /// symbolic link itself is not a regular file irrespective of the file
    /// to which it points).  Platform-specific note: On POSIX, this is a
    /// positive test on the "regular file" mode; on Windows, this is a
    /// negative test on the "directory" attribute, i.e., on Windows,
    /// everything that exists and is not a directory is a regular file.
    /// The parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static bool isRegularFile(const char *path, bool followLinksFlag = false);
    template <class STRING_TYPE>
    static bool isRegularFile(const STRING_TYPE& path,
                              bool               followLinksFlag = false);

    /// Return `true` if there currently exists a directory at the specified
    /// `path`, and `false` otherwise.  If there is a symbolic link at
    /// `path`, follow it only if the optionally specified `followLinksFlag`
    /// is `true` (otherwise return `false`).  Platform-specific note: On
    /// Windows, a "shortcut" is not a symbolic link.  The parameterized
    /// `STRING_TYPE` must be one of `bsl::string`, `std::string`,
    /// `std::pmr::string` (if supported), `bsl::string_view`, or
    /// `bslstl::StringRef`.
    static bool isDirectory(const char *path, bool followLinksFlag = false);
    template <class STRING_TYPE>
    static bool isDirectory(const STRING_TYPE& path,
                            bool               followLinksFlag = false);

    /// Return `true` if there currently exists a symbolic link at the
    /// specified `path`, and `false` otherwise.  Windows directory
    /// junctions are treated as directory symbolic links.  The
    /// parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static bool isSymbolicLink(const char *path);
    template <class STRING_TYPE>
    static bool isSymbolicLink(const STRING_TYPE& path);

    /// Load into the specified `time` the last modification time of the
    /// file at the specified `path`, as reported by the filesystem.  Return
    /// 0 on success, and a non-zero value otherwise.  The time is reported
    /// in UTC.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    static int getLastModificationTime(bdlt::Datetime *time, const char *path);
    template <class STRING_TYPE>
    static int getLastModificationTime(bdlt::Datetime     *time,
                                       const STRING_TYPE&  path);

    /// Load into the specified `time` the last modification time of the
    /// file with the specified `descriptor`, as reported by the filesystem.
    /// Return 0 on success, and a non-zero value otherwise.  The time is
    /// reported in UTC.
    static int getLastModificationTime(bdlt::Datetime *time,
                                       FileDescriptor  descriptor);

    // TBD: write setModificationTime() when SetFileInformationByHandle()
    // becomes available on our standard Windows platforms.

    /// Create any directories in the specified `path` that do not exist.
    /// If the optionally specified `isLeafDirectoryFlag` is `true`, treat
    /// the final name component in `path` as a directory name, and create
    /// it.  Otherwise, create only the directories leading up to the final
    /// name component.  Return 0 on success, `k_ERROR_PATH_NOT_FOUND` if a
    /// component used as a directory in `path` exists but is not a
    /// directory, and a negative value for any other kind of error.  The
    /// parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static int createDirectories(const char *path,
                                 bool        isLeafDirectoryFlag = false);
    template <class STRING_TYPE>
    static int createDirectories(
                               const STRING_TYPE& path,
                               bool               isLeafDirectoryFlag = false);

    /// Create a private directory with the specified `path`.  Return 0 on
    /// success, `k_ERROR_PATH_NOT_FOUND` if a component used as a directory
    /// in `path` either does not exist or is not a directory,
    /// `k_ERROR_ALREADY_EXISTS` if the file system entry (not necessarily a
    /// directory) with the name `path` already exists, and a negative value
    /// for any other kind of error.  The directory is created with
    /// permissions restricting access, as closely as possible, to the
    /// caller's userid only.  Note that directories created on Microsoft
    /// Windows may receive default, not restricted permissions.  The
    /// parameterized `STRING_TYPE` must be one of `bsl::string`,
    /// `std::string`, `std::pmr::string` (if supported), `bsl::string_view`,
    /// or `bslstl::StringRef`.
    static int createPrivateDirectory(const char *path);
    template <class STRING_TYPE>
    static int createPrivateDirectory(const STRING_TYPE& path);

    /// Load a valid path to the system temporary directory to the specified
    /// `path`.  Return 0 on success, and a non-zero value otherwise.  A
    /// temporary directory is one in which the operating system has permission
    /// to delete its contents, but not necessarily the directory itself, the
    /// next time the computer reboots.
    static int getSystemTemporaryDirectory(bsl::string *path);
    static int getSystemTemporaryDirectory(std::string *path);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int getSystemTemporaryDirectory(std::pmr::string *path);
#endif

    /// Create and open a new file with a name constructed by appending an
    /// automatically-generated suffix to the specified `prefix`, and return
    /// its file descriptor open for reading and writing.  A return value of
    /// `k_INVALID_FD` indicates that no such file could be created; otherwise,
    /// the name of the file created is assigned to the specified `outPath`.
    /// The file is created with permissions restricted, as closely as
    /// possible, to the caller`s userid only.  If the prefix is a relative
    /// path, the file is created relative to the process current directory.
    /// Responsibility for deleting the file is left to the caller.  Note that
    /// on Posix systems, if `outPath` is unlinked immediately, the file will
    /// remain usable until its descriptor is closed.  Note that files created
    /// on Microsoft Windows may receive default, not restricted permissions.
    static FileDescriptor createTemporaryFile(bsl::string             *outPath,
                                              const bsl::string_view&  prefix);
    static FileDescriptor createTemporaryFile(std::string             *outPath,
                                              const bsl::string_view&  prefix);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static FileDescriptor createTemporaryFile(std::pmr::string        *outPath,
                                              const bsl::string_view&  prefix);
#endif

    /// Create a new directory with a name constructed by appending an
    /// automatically-generated suffix to the specified `prefix`.  A non-zero
    /// return value indicates that no such directory could be created;
    /// otherwise the name of the directory created is assigned to the
    /// specified `outPath`.  The directory is created with permissions
    /// restricted, as closely as possible, to the caller only.  If the prefix
    /// is a relative path, the directory is created relative to the process
    /// current directory.  Responsibility for deleting the directory (and any
    /// files subsequently created in it) is left to the caller.
    static int createTemporaryDirectory(bsl::string             *outPath,
                                        const bsl::string_view&  prefix);
    static int createTemporaryDirectory(std::string             *outPath,
                                        const bsl::string_view&  prefix);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int createTemporaryDirectory(std::pmr::string        *outPath,
                                        const bsl::string_view&  prefix);
#endif

    /// Create a new directory with a name constructed by appending an
    /// automatically-generated suffix to the specified `prefix` within the
    /// specified `rootDirectory`.  A non-zero return value indicates that no
    /// such directory could be created; otherwise the name of the directory
    /// created is assigned to the specified `outPath`.  The directory is
    /// created with permissions restricted, as closely as possible, to the
    /// caller only.  If the `rootDirectory` is a relative path, the directory
    /// is created relative to the process current directory.  Responsibility
    /// for deleting the directory (and any files subsequently created in it)
    /// is left to the caller.
    static int createTemporarySubdirectory(
                                        bsl::string             *outPath,
                                        const bsl::string_view&  rootDirectory,
                                        const bsl::string_view&  prefix);
    static int createTemporarySubdirectory(
                                        std::string             *outPath,
                                        const bsl::string_view&  rootDirectory,
                                        const bsl::string_view&  prefix);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int createTemporarySubdirectory(
                                        std::pmr::string        *outPath,
                                        const bsl::string_view&  rootDirectory,
                                        const bsl::string_view&  prefix);
#endif

    /// Construct a file name by appending an automatically-generated suffix to
    /// the specified `prefix`.  The file name constructed is assigned to the
    /// specified `outPath`.  Note that this function is called "unsafe"
    /// because a file with the resulting name may be created by another
    /// program before the caller has opportunity to use the name, which could
    /// be a security vulnerability, and a file with the given name may already
    /// exist where you mean to put it.  Note that the suffix is hashed from
    /// environmental details, including any pre-existing value of `outPath` so
    /// that if a resulting name is unsuitable (e.g. the file exists) this
    /// function may simply be called again, pointing to its previous result,
    /// to get a new, probably different name.
    static void makeUnsafeTemporaryFilename(bsl::string             *outPath,
                                            const bsl::string_view&  prefix);
    static void makeUnsafeTemporaryFilename(std::string             *outPath,
                                            const bsl::string_view&  prefix);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static void makeUnsafeTemporaryFilename(std::pmr::string        *outPath,
                                            const bsl::string_view&  prefix);
#endif

    /// Call the specified `visitor` function object for each path in the
    /// filesystem matching the specified `pattern`.  Return the number of
    /// paths visited on success, and a negative value otherwise.  Note that
    /// if `visitor` deletes files or directories during the search,
    /// `visitor` may subsequently be called with paths which have already
    /// been deleted, so must be prepared for this event.  Also note that
    /// there is no guarantee as to the order in which paths will be
    /// visited.  See `findMatchingPaths` for a discussion of how `pattern`
    /// is interpreted.  Also note that `.` and `..` are never matched by
    /// wild cards.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    ///
    /// IBM-SPECIFIC WARNING: This function is not thread-safe.  The AIX
    /// implementation of the system `glob` function can temporarily change
    /// the working directory of the entire program, causing attempts in
    /// other threads to open files with relative path names to fail.
    static int visitPaths(
                        const char                                   *pattern,
                        const bsl::function<void(const char *path)>&  visitor);
    template <class STRING_TYPE>
    static int visitPaths(
                         const STRING_TYPE&                           pattern,
                         const bsl::function<void(const char *path)>& visitor);

    /// Recursively traverse the directory tree starting at the specified
    /// `root` for files whose leaf names match the specified `pattern`, and
    /// run the specified function `visitor`, passing it the full path
    /// starting with `root` to each pattern matching file.  See
    /// `findMatchingPaths` for a discussion of how `pattern` is
    /// interpreted.  If the specified `sortFlag` is `true`, traverse the
    /// files in the tree in sorted order, sorted by the full path name,
    /// otherwise the order in which the files will be visited is
    /// unspecified.  UTF-8 paths will be sorted by `strcmp`, which sorts by
    /// `char`s, not unicode code points.  Found `.` and `..` directories
    /// are ignored, except that `root` may be `.` or `..`.  Return 0 on
    /// success, and a non-zero value otherwise.  This function will fail if
    /// `root` does not specify a directory, of if `pattern` contains `/` on
    /// Unix or '\' on Windows.  Note that both directories and plain files
    /// whose names match `pattern` will be visited, while other files such
    /// as symlinks will not be visited or followed.  No file or directory
    /// that is not matched will be visited.  All directories are traversed,
    /// regardless of whether they are matched.  If a directory is matched
    /// and `sortFlag` is `true`, it is visited immediately before it is
    /// traversed.  Also note that `root` is never visited, even if it
    /// matches `pattern`.  Also note that no pattern matching is done on
    /// `root` -- if it contains wildcards, they are not interpreted as such
    /// and must exactly match the characters in the name of the directory.
    ///
    /// IBM-SPECIFIC WARNING: This function is not thread-safe.  The AIX
    /// implementation of the system `glob` function can temporarily change
    /// the working directory of the entire program, casuing attempts in
    /// other threads to open files with relative path names to fail.
    static int visitTree(
                const bsl::string_view&                      root,
                const bsl::string_view&                      pattern,
                const bsl::function<void(const char *path)>& visitor,
                bool                                         sortFlag = false);

    /// Load into the specified `result` vector all paths in the filesystem
    /// matching the specified `pattern`.  The '*' character will match any
    /// number of characters in a filename; however, this matching will not
    /// span a directory separator (e.g., "logs/m*.txt" will not match
    /// "logs/march/001.txt").  '?' will match any one character.  '*' and '?'
    /// may be used any number of times in the pattern.  The special
    /// directories "." and ".." will not be matched against any pattern.  Note
    /// that any initial contents of `result` will be erased, and that the
    /// paths in `result` will not be in any particular guaranteed order.
    /// Return the number of paths matched on success, and a negative value
    /// otherwise; if a negative value is returned, the contents of `*result`
    /// are undefined.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    ///
    /// WINDOWS-SPECIFIC NOTE: To support DOS idioms, the OS-provided search
    /// function has behavior that we have chosen not to work around: an
    /// extension consisting of wild-card characters ('?', '*') can match an
    /// extension or *no* extension.  E.g., "file.?" matches "file.z", but not
    /// "file.txt"; however, it also matches "file" (without any extension).
    /// Likewise, "*.*" matches any filename, including filenames having no
    /// extension.  Also, on Windows (but not on Unix) attempting to match a
    /// pattern that is invalid UTF-8 will result in an error.
    ///
    /// IBM-SPECIFIC WARNING: This function is not thread-safe.  The AIX
    /// implementation of the system `glob` function can temporarily change the
    /// working directory of the entire program, casuing attempts in other
    /// threads to open files with relative path names to fail.
    static int findMatchingPaths(bsl::vector<bsl::string> *result,
                                 const char               *pattern);
    template <class STRING_TYPE>
    static int findMatchingPaths(bsl::vector<bsl::string> *result,
                                 const STRING_TYPE&        pattern);
    static int findMatchingPaths(std::vector<std::string> *result,
                                 const char               *pattern);
    template <class STRING_TYPE>
    static int findMatchingPaths(std::vector<std::string> *result,
                                 const STRING_TYPE&        pattern);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int findMatchingPaths(std::pmr::vector<std::pmr::string> *result,
                                 const char                         *pattern);
    template <class STRING_TYPE>
    static int findMatchingPaths(std::pmr::vector<std::pmr::string> *result,
                                 const STRING_TYPE&                  pattern);
#endif

    /// Return the number of bytes available for allocation in the file
    /// system where the file or directory with the specified `path`
    /// resides, or a negative value if an error occurs.  The parameterized
    /// `STRING_TYPE` must be one of `bsl::string`, `std::string`,
    /// `std::pmr::string` (if supported), `bsl::string_view`, or
    /// `bslstl::StringRef`.
    static Offset getAvailableSpace(const char *path);
    template <class STRING_TYPE>
    static Offset getAvailableSpace(const STRING_TYPE& path);

    /// Return the number of bytes available for allocation in the file
    /// system where the file with the specified `descriptor` resides, or a
    /// negative value if an error occurs.
    static Offset getAvailableSpace(FileDescriptor descriptor);

    /// Return the size, in bytes, of the file or directory at the specified
    /// `path`, or a negative value if an error occurs.  Note that the size
    /// of a symbolic link is the size of the file or directory to which it
    /// points.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    static Offset getFileSize(const char *path);
    template <class STRING_TYPE>
    static Offset getFileSize(const STRING_TYPE& path);

    /// Return the size, in bytes, of the file or directory specified by the
    /// specified open `descriptor`, or a negative value if an error occurs.
    /// Note that the size of a symbolic link is the size of the file or
    /// directory to which it points.
    static Offset getFileSize(FileDescriptor descriptor);

    /// Return the file size limit for this process, `k_OFFSET_MAX` if no
    /// limit is set, or a negative value if an error occurs.  Note that if
    /// you are doing any calculations involving the returned value, it is
    /// recommended to check for `k_OFFSET_MAX` specifically to avoid
    /// integer overflow in your calculations.
    static Offset getFileSizeLimit();

    /// Load, into the specified `result`, the target of the symbolic link at
    /// the specified `path`.  Return 0 on success, and a non-zero value
    /// otherwise.  For example, this function will return an error if `path`
    /// does not refer to a symbolic link.  Windows directory junctions are
    /// treated as directory symbolic links.  If `path` is a relative path, it
    /// is evaluated against the current working directory.  The parameterized
    /// `STRING_TYPE` must be one of `bsl::string`, `std::string`,
    /// `std::pmr::string` (if supported), `bsl::string_view`, or
    /// `bslstl::StringRef`.
    static int getSymbolicLinkTarget(bsl::string *result,
                                     const char  *path);
    static int getSymbolicLinkTarget(std::string *result,
                                     const char  *path);
    template <class STRING_TYPE>
    static int getSymbolicLinkTarget(bsl::string        *result,
                                     const STRING_TYPE&  path);
    template <class STRING_TYPE>
    static int getSymbolicLinkTarget(std::string        *result,
                                     const STRING_TYPE&  path);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static int getSymbolicLinkTarget(std::pmr::string *result,
                                     const char       *path);
    template <class STRING_TYPE>
    static int getSymbolicLinkTarget(std::pmr::string   *result,
                                     const STRING_TYPE&  path);
#endif

    /// Acquire a lock for the file with the specified `descriptor`.  If
    /// `lockWriteFlag` is true, acquire an exclusive write lock; otherwise
    /// acquire a (possibly) shared read lock.  The calling thread will
    /// block until the lock is acquired.  Return 0 on success, and a
    /// non-zero value otherwise.  Note that this operation locks the
    /// indicated file for use by the current *process*, but the behavior is
    /// unspecified (and platform-dependent) when either attempting to lock
    /// `descriptor` multiple times, or attempting to lock another
    /// descriptor referring to the same file, within a single process.
    static int lock(FileDescriptor descriptor, bool lockWriteFlag);

    /// Set the size of the file referred to by the specified `descriptor`
    /// to the specified `size`.  `descriptor` must be open for writing.
    /// After the function call, the position is set to the end of the file.
    /// Return 0 on success and a non-zero value otherwise.  The behavior is
    /// undefined if the file is currently mapped, or if `size` is greater
    /// than the existing size of the file.
    static int truncateFileSize(FileDescriptor descriptor, Offset size);

    /// Acquire a lock for the file with the specified `descriptor` if it is
    /// currently available.  If the specified `lockWriteFlag` is true,
    /// acquire an exclusive write lock unless another process has any type
    /// of lock on the file.  If `lockWriteFlag` is false, acquire a shared
    /// read lock unless a process has a write lock.  This method will not
    /// block.  Return 0 on success, `k_ERROR_LOCKING_CONFLICT` if the
    /// platform reports the lock could not be acquired because another
    /// process holds a conflicting lock, and a negative value for any other
    /// kind of error.  Note that this operation locks the indicated file
    /// for the current *process*, but the behavior is unspecified (and
    /// platform-dependent) when either attempting to lock `descriptor`
    /// multiple times, or attempting to lock another descriptor referring
    /// to the same file, within a single process.
    static int tryLock(FileDescriptor descriptor, bool lockWriteFlag);

    /// Release any lock this process holds on the file with the specified
    /// `descriptor`.  Return 0 on success, and a non-zero value otherwise.
    static int unlock(FileDescriptor descriptor);

    /// Map the region of the specified `size` bytes, starting at the
    /// specified `offset` bytes into the file with the specified
    /// `descriptor` to memory, and load into the specified `address` of the
    /// mapped area.  Return 0 on success, and a non-zero value otherwise.
    /// The access permissions for mapping memory are defined by the
    /// specified `mode`, which may be a combination of
    /// `MemoryUtil::k_ACCESS_READ`, `MemoryUtil::k_ACCESS_WRITE` and
    /// `MemoryUtil::k_ACCESS_EXECUTE`.  Note that on failure, the value of
    /// `address` is undefined.  Also note that mapping will succeed even if
    /// there are fewer than `offset + size` bytes in the specified file,
    /// and an attempt to access the mapped memory beyond the end of the
    /// file will result in undefined behavior (i.e., this function does not
    /// grow the file to guarantee it can accommodate the mapped region).
    /// Also note that mapping past the end of file may return 0, but any
    /// access of the resulting mapped memory may segfault.
    static int map(FileDescriptor   descriptor,
                   void           **address,
                   Offset           offset,
                   bsl::size_t      size,
                   int              mode);

    /// Map the region of the specified `size` bytes, starting at the
    /// specified `offset` bytes into the file with the specified
    /// `descriptor` to memory, and load into the specified `address` of the
    /// mapped area.  Return 0 on success, `k_ERROR_PAST_EOF` if an attempt
    /// is made to map past the end of file, and a non-zero value otherwise.
    /// The access permissions for mapping memory are defined by the
    /// specified `mode`, which may be a combination of
    /// `MemoryUtil::k_ACCESS_READ`, `MemoryUtil::k_ACCESS_WRITE` and
    /// `MemoryUtil::k_ACCESS_EXECUTE`, though on some platforms they must
    /// be a subset of the file permissions.  The behavior is undefined
    /// unless bits in `mode` other than
    /// `MemoryUtil::k_ACCESS_READ_WRITE_EXECUTE` are all clear, unless
    /// `0 <= offset`, and unless `0 < size`, and unless the `offset` is a
    /// multiple of `MemoryUtil::pageSize()`.  Note that on failure, the
    /// value of `address` is undefined.  Also note that the check against
    /// mapping past the end of file and all assertions are done before the
    /// call to map the file.
    static int mapChecked(FileDescriptor   descriptor,
                          void           **address,
                          Offset           offset,
                          bsl::size_t      size,
                          int              mode);

    /// Unmap the memory mapping with the specified base `address` and
    /// specified `size`.  Return 0 on success, and a non-zero value
    /// otherwise.  The behavior is undefined unless this area with
    /// `address` and `size` was previously mapped with a `map` call.
    static int unmap(void *address, bsl::size_t size);

    /// Synchronize the contents of the specified `numBytes` of mapped
    /// memory beginning at the specified `address` with the underlying file
    /// on disk.  If the specified `syncFlag` is true, block until all
    /// writes to nonvolatile media have actually completed, otherwise,
    /// return once they have been scheduled.  Return 0 on success, and a
    /// non-zero value otherwise.  The behavior is undefined unless
    /// `address` is aligned on a page boundary, `numBytes` is a multiple of
    /// `pageSize()`, and `0 <= numBytes`.
    static int sync(char *address, bsl::size_t numBytes, bool syncFlag);

    /// Set the file pointer associated with the specified `descriptor`
    /// (used by calls to the `read` and `write` system calls) according to
    /// the specified `whence` behavior:
    /// ```
    ///  * If 'whence' is e_SEEK_FROM_BEGINNING, set the pointer to
    ///    'offset' bytes from the beginning of the file.
    ///  * If 'whence' is e_SEEK_FROM_CURRENT, advance the pointer by
    ///    'offset' bytes
    ///  * If 'whence' is e_SEEK_FROM_END, set the pointer to 'offset'
    ///    bytes beyond the end of the file.
    /// ```
    /// Return the new location of the file pointer, in bytes from the
    /// beginning of the file, on success; and -1 otherwise.  The effect on
    /// the file pointer is undefined unless the file is on a device capable
    /// of seeking.  Note that `seek` does not change the size of the file
    /// if the pointer advances beyond the end of the file; instead, the
    /// next write at the pointer will increase the file size.
    static Offset seek(FileDescriptor descriptor, Offset offset, int whence);

    /// Read the specified `numBytes` bytes beginning at the file pointer of
    /// the file with the specified `descriptor` into the specified
    /// `buffer`.  Return `numBytes` on success; the number of bytes read
    /// if there were not enough available; or a negative number on some
    /// other error.
    static int read(FileDescriptor descriptor, void *buffer, int numBytes);

    /// Remove the file or directory at the specified `path`.  If the `path`
    /// refers to a directory and the optionally specified `recursiveFlag`
    /// is `true`, recursively remove all files and directories within the
    /// specified directory before removing the directory itself.  Return 0
    /// on success and a non-zero value otherwise.  If `path` refers to a
    /// symbolic link, the symbolic link will be removed, not the target of
    /// the link.  Note that if `path` is a directory, and the directory is
    /// not empty, and recursive is `false`, this method will fail.  Also
    /// note that if the function fails when `recursive` is `true`, it may
    /// or may not have removed *some* files or directories before failing.
    /// Also note that if `remove` is called on "." or "..", it will fail
    /// with no effect.  The parameterized `STRING_TYPE` must be one of
    /// `bsl::string`, `std::string`, `std::pmr::string` (if supported),
    /// `bsl::string_view`, or `bslstl::StringRef`.
    ///
    /// WINDOWS-SPECIFIC WARNING: When `recursiveFlag` is `true`, it is
    /// possible for this function to descend into a directory symbolic link,
    /// resulting in the removal of files or directories that are outside the
    /// subtree of `path`.  This situation occurs only if a directory is
    /// replaced by a symbolic link during the traversal.  Note that when
    /// Developer Mode is not enabled, symbolic links can be created only by
    /// users with administrative privileges.  If Developer Mode is on, using
    /// this function for recursive deletion can result in a vulnerability
    /// similar to
    /// [CVE-2022-21658](https://nvd.nist.gov/vuln/detail/cve-2022-21658).
    static int remove(const char *path, bool recursiveFlag = false);
    template <class STRING_TYPE>
    static int remove(const STRING_TYPE& path, bool recursiveFlag = false);

    /// Remove the file at the specified `path` appended with the specified
    /// `maxSuffix` using a `.` as a separator.  Then move the files with
    /// the suffixes `.1` to `.maxSuffix-1` so they have new suffixes from
    /// `.2` to `.maxSuffix`.  Finally, move `path` to `path` with a `.1`
    /// suffix.  Return 0 on success, and non-zero otherwise.
    static int rollFileChain(const bsl::string_view& path, int maxSuffix);

    /// Move the file or directory at the specified `oldPath` to the specified
    /// `newPath`.  On Windows, this operation occurs in two steps: first, if
    /// there is a regular file or file symbolic link at `newPath`, it is
    /// removed; if there is a directory or directory symbolic link at
    /// `newPath`, this function fails with no effect.  Second, the file or
    /// directory at `oldPath` is actually moved; this operation fails with no
    /// further effect if the source and destination are on different volumes
    /// or if the source does not exist (including in the case where `oldPath`
    /// and `newPath` are the same path).  On Unix, the operation is always
    /// atomic, with the file or directory at `newPath` being replaced by the
    /// item at `oldPath`; the operation fails if the source and destination
    /// are on different filesystems, if the source and destination are
    /// different kinds of item (that is, one is a file and the other is a
    /// directory), or if the destination is a non-empty directory, except that
    /// it succeeds if the source and destination are the same directory; a
    /// symbolic link is considered an existing file even if it is broken or it
    /// points to a directory.  On all platforms, if `oldPath` is a symbolic
    /// link, the link will be moved, not its target.  Return 0 on success, and
    /// a non-zero value otherwise.  The parameterized `OLD_STRING_TYPE` and
    /// `NEW_STRING_TYPE` must be one of `bsl::string`, `std::string`,
    /// `std::pmr::string` (if supported), `bsl::string_view`, or
    /// `bslstl::StringRef`.
    static int move(const char *oldPath, const char *newPath);
    template <class OLD_STRING_TYPE, class NEW_STRING_TYPE>
    static int move(const OLD_STRING_TYPE& oldPath,
                    const NEW_STRING_TYPE& newPath);

    /// Write the specified `numBytes` from the specified `buffer` address
    /// to the file with the specified `descriptor`.  Return `numBytes` on
    /// success; the number of bytes written if space was exhausted; or a
    /// negative value on some other error.
    static int write(FileDescriptor  descriptor,
                     const void     *buffer,
                     int             numBytes);

    /// Grow the file with the specified `descriptor` to the size of at
    /// least the specified `size` bytes.  Return 0 on success, and a
    /// non-zero value otherwise.  If the optionally specified `reserveFlag`
    /// is true, make sure the space on disk is preallocated and not
    /// allocated on demand, preventing a possible out-of-disk-space error
    /// when accessing the data on file systems with sparse file support.
    /// Preallocation is done by writing unspecified data to file in blocks
    /// of the optionally specified `increment` or a default value if
    /// `increment` is zero or unspecified.  Note that if the size of the
    /// file is greater than or equal to `size`, this function has no
    /// effect.  Also note that the contents of the newly grown portion of
    /// the file is undefined.
    static int growFile(
                  FileDescriptor descriptor,
                  Offset         size,
                  bool           reserveFlag = false,
                  bsl::size_t    increment = k_DEFAULT_FILE_GROWTH_INCREMENT);
};

                      // ================================
                      // class FilesystemUtil_CStringUtil
                      // ================================

/// This component-private utility `struct` provides a namespace for the
/// `flatten` overload set intended to be used in concert with an overload
/// set consisting of a function template with a deduced argument and an
/// non-template overload accepting a `const char *`.  The actual
/// implementation of the functionality would be in the `const char *`
/// overload whereas the purpose of the function template is to invoke the
/// `const char *` overload with a null-terminated string.
///
/// The function template achieves null-termination by recursively calling
/// the function and supplying it with the result of `flatten` invoked on
/// the deduced argument.  This `flatten` invocation will call `c_str()` on
/// various supported `string` types, will produce a temporary `bsl::string`
/// for possibly non-null-terminated `bsl::string_view` or `bslstl::StringRef`,
/// and will result in a `BSLMF_ASSERT` for any unsupported type.  Calling the
/// function with the temporary `bsl::string` produced from `bsl::string_view`
/// or `bslstl::StringRef` will result in a second invocation of `flatten`,
/// this time producing `const char *`, and finally calling the function with a
/// null-terminated string.
///
/// Note that the `bslstl::StringRef` overload for `flatten` is provided for
/// backwards compatibility.  Without it, the `bsl::string` and
/// `std::string` overloads would be ambiguous.  In new code, it is
/// preferable to not provide `bslstl::StringRef` overload in a similar
/// facility and require the clients to explicitly state the string type in
/// their code, making a potential allocation obvious.  The same considerations
/// apply to `bsl::string_view`.
struct FilesystemUtil_CStringUtil {

    // CLASS METHODS

    /// Return the specified `cString`.
    static const char *flatten(char *cString);
    static const char *flatten(const char *cString);

    /// Return the result of invoking `c_str()` on the specified `string`.
    static const char *flatten(const bsl::string& string);
    static const char *flatten(const std::string& string);
#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
    static const char *flatten(const std::pmr::string& string);
#endif

    /// Return a temporary `bsl::string` constructed from the specified
    /// `stringView`.
    static bsl::string flatten(const bsl::string_view& stringView);

    /// Return a temporary `bsl::string` constructed from the specified
    /// `stringRef`.
    static bsl::string flatten(const bslstl::StringRef& stringRef);

    /// Produce a compile-time error informing the caller that the
    /// parameterized `TYPE` is not supported as the parameter for the call.
    template <class TYPE>
    static const char *flatten(const TYPE&);
};

// ============================================================================
//                            INLINE DEFINITIONS
// ============================================================================

                      // --------------------
                      // class FilesystemUtil
                      // --------------------

// CLASS METHODS
template <class STRING_TYPE>
inline
FilesystemUtil::FileDescriptor FilesystemUtil::open(
                                             const STRING_TYPE& path,
                                             FileOpenPolicy     openPolicy,
                                             FileIOPolicy       ioPolicy,
                                             FileTruncatePolicy truncatePolicy)
{
    return FilesystemUtil::open(FilesystemUtil_CStringUtil::flatten(path),
                                openPolicy,
                                ioPolicy,
                                truncatePolicy);
}

template <class STRING_TYPE>
inline
int FilesystemUtil::setWorkingDirectory(const STRING_TYPE& path)
{
    return FilesystemUtil::setWorkingDirectory(
        FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
bool FilesystemUtil::exists(const STRING_TYPE& path)
{
    return FilesystemUtil::exists(FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
bool FilesystemUtil::isRegularFile(const STRING_TYPE& path,
                                   bool               followLinksFlag)
{
    return FilesystemUtil::isRegularFile(
        FilesystemUtil_CStringUtil::flatten(path), followLinksFlag);
}

template <class STRING_TYPE>
inline
bool FilesystemUtil::isDirectory(const STRING_TYPE& path,
                                 bool               followLinksFlag)
{
    return FilesystemUtil::isDirectory(
        FilesystemUtil_CStringUtil::flatten(path), followLinksFlag);
}

template <class STRING_TYPE>
inline
bool FilesystemUtil::isSymbolicLink(const STRING_TYPE& path)
{
    return FilesystemUtil::isSymbolicLink(
        FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::getLastModificationTime(bdlt::Datetime     *time,
                                            const STRING_TYPE&  path)
{
    return FilesystemUtil::getLastModificationTime(
        time, FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::createDirectories(
                               const STRING_TYPE& path,
                               bool               isLeafDirectoryFlag)
{
    return FilesystemUtil::createDirectories(
        FilesystemUtil_CStringUtil::flatten(path), isLeafDirectoryFlag);
}

template <class STRING_TYPE>
inline
int FilesystemUtil::createPrivateDirectory(const STRING_TYPE& path)
{
    return FilesystemUtil::createPrivateDirectory(
        FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::visitPaths(
                          const STRING_TYPE&                           pattern,
                          const bsl::function<void(const char *path)>& visitor)
{
    return FilesystemUtil::visitPaths(
        FilesystemUtil_CStringUtil::flatten(pattern), visitor);
}

template <class STRING_TYPE>
inline
int FilesystemUtil::findMatchingPaths(bsl::vector<bsl::string> *result,
                                      const STRING_TYPE&        pattern)
{
    return FilesystemUtil::findMatchingPaths(
        result, FilesystemUtil_CStringUtil::flatten(pattern));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::findMatchingPaths(std::vector<std::string> *result,
                                      const STRING_TYPE&        pattern)
{
    return FilesystemUtil::findMatchingPaths(
        result, FilesystemUtil_CStringUtil::flatten(pattern));
}

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
template <class STRING_TYPE>
inline
int FilesystemUtil::findMatchingPaths(
                                   std::pmr::vector<std::pmr::string> *result,
                                   const STRING_TYPE&                  pattern)
{
    return FilesystemUtil::findMatchingPaths(
        result, FilesystemUtil_CStringUtil::flatten(pattern));
}
#endif

template <class STRING_TYPE>
inline
FilesystemUtil::Offset FilesystemUtil::getAvailableSpace(
                                                       const STRING_TYPE& path)
{
    return FilesystemUtil::getAvailableSpace(
        FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
FilesystemUtil::Offset FilesystemUtil::getFileSize(const STRING_TYPE& path)
{
    return FilesystemUtil::getFileSize(
        FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::getSymbolicLinkTarget(bsl::string        *result,
                                          const STRING_TYPE&  path)
{
    return FilesystemUtil::getSymbolicLinkTarget(
        result, FilesystemUtil_CStringUtil::flatten(path));
}

template <class STRING_TYPE>
inline
int FilesystemUtil::getSymbolicLinkTarget(std::string        *result,
                                          const STRING_TYPE&  path)
{
    return FilesystemUtil::getSymbolicLinkTarget(
        result, FilesystemUtil_CStringUtil::flatten(path));
}

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
template <class STRING_TYPE>
inline
int FilesystemUtil::getSymbolicLinkTarget(std::pmr::string   *result,
                                          const STRING_TYPE&  path)
{
    return FilesystemUtil::getSymbolicLinkTarget(
        result, FilesystemUtil_CStringUtil::flatten(path));
}
#endif

template <class STRING_TYPE>
inline
int FilesystemUtil::remove(const STRING_TYPE& path, bool recursiveFlag)
{
    return FilesystemUtil::remove(FilesystemUtil_CStringUtil::flatten(path),
                                  recursiveFlag);
}

template <class OLD_STRING_TYPE, class NEW_STRING_TYPE>
inline
int FilesystemUtil::move(const OLD_STRING_TYPE& oldPath,
                         const NEW_STRING_TYPE& newPath)
{
    return FilesystemUtil::move(FilesystemUtil_CStringUtil::flatten(oldPath),
                                FilesystemUtil_CStringUtil::flatten(newPath));
}




                      // --------------------------------
                      // class FilesystemUtil_CStringUtil
                      // --------------------------------

// CLASS METHODS
inline
const char *FilesystemUtil_CStringUtil::flatten(char *cString)
{
    return cString;
}

inline
const char *FilesystemUtil_CStringUtil::flatten(const char *cString)
{
    return cString;
}

inline
const char *FilesystemUtil_CStringUtil::flatten(const bsl::string& string)
{
    return string.c_str();
}

inline
const char *FilesystemUtil_CStringUtil::flatten(const std::string& string)
{
    return string.c_str();
}

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_PMR_STRING
inline
const char *FilesystemUtil_CStringUtil::flatten(const std::pmr::string& string)
{
    return string.c_str();
}
#endif

inline
bsl::string FilesystemUtil_CStringUtil::flatten(
                                            const bsl::string_view& stringView)
{
    bsl::string ret(stringView);
    return ret;
}

inline
bsl::string FilesystemUtil_CStringUtil::flatten(
                                            const bslstl::StringRef& stringRef)
{
    return stringRef;
}

template <class TYPE>
inline
const char *FilesystemUtil_CStringUtil::flatten(const TYPE&)
{
    BSLMF_ASSERT(("Unsupported parameter type." && !sizeof(TYPE)));
    return 0;
}

// FREE OPERATORS

/// Output the specified `value` to the specified `stream`, in
/// human-readable form.  If `value` is not a valid `Whence` value, report
/// that it is invalid and output it as an integer.
bsl::ostream& operator<<(bsl::ostream& stream, FilesystemUtil::Whence value);

/// Output the specified `value` to the specified `stream`, in
/// human-readable form.  If `value` is not a valid `ErrorType` value,
/// report that it is invalid and output it as an integer.
bsl::ostream& operator<<(bsl::ostream&                      stream,
                         FilesystemUtil::ErrorType          value);

/// Output the specified `value` to the specified `stream`, in
/// human-readable form.  If `value` is not a valid `FileOpenPolicy` value,
/// report that it is invalid and output it as an integer.
bsl::ostream& operator<<(bsl::ostream&                      stream,
                         FilesystemUtil::FileOpenPolicy     value);

/// Output the specified `value` to the specified `stream`, in
/// human-readable form.  If `value` is not a valid `FileIOPolicy` value,
/// report that it is invalid and output it as an integer.
bsl::ostream& operator<<(bsl::ostream&                      stream,
                         FilesystemUtil::FileIOPolicy       value);

/// Output the specified `value` to the specified `stream`, in
/// human-readable form.  If `value` is not a valid `FileTruncatePolicy`
/// value, report that it is invalid and output it as an integer.
bsl::ostream& operator<<(bsl::ostream&                      stream,
                         FilesystemUtil::FileTruncatePolicy value);

}  // close package namespace
}  // close enterprise namespace

#endif

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
