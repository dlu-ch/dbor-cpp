dbor-c++
--------

dbor-c++ is a library implementing a DBOR encoder and decoder for C++:2011, suitable for
real-time embedded systems and use in interrupt handlers. All provided operations are reentrant.

DBOR: <https::github.com/dlu-ch/dbor-spec>.


Build/usage requirements:

 a) C++:2011 compliant (ISO/IEC 14882:2011) compiler (behaviour undefined if not met).

 b) In addition to C++:2011 (behaviour undefined if not met):

    - '#include "path/to/file"' includes the file with (POSIX-style) path <dir>/path/to/file
      for an the first directory path <dir> in the compiler's include search path such than
      <dir>/path/to/file is an existing file. The compilation fails if no such file
      exists.

 c) In addition to C++:2011 (build fails if not met):

    - the size of char is exactly 8 bit

    - a byte as counted by sizeof() is exactly 8 bit (C++:2011 allows anything of at least 8 bit).

    - std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::int8_t, std::int16_t,
      std::int32_t, std::int64_t are defined.

    - std::size_t is std::uint8_t, std::uint16_t, std::uint32_t, or std::uint64_t.

    - float and double are exactly 32 bit and 64 bit, respectively
      with std::numeric_limits<float>::is_iec559 and std::numeric_limits<double>::is_iec559 = true.

 d) In addition to C++:2011 (violates DBOR specification if not met):

    - Each char with a non-negative integral value < 0x80 represents an ASCII character with that
      code which is a member of the basic execution character set.

    - float and double are represented as IEEE-754:2008 binary32 and binary64, respectively.


Implementation properties:

 a) Does not use:

    - Exceptions
    - RTTI
    - Dynamic memory allocation
    - Hosted implementaton of C++ (including STL containers and STL strings)
    - Floating-point operations
    - Global state of any kind (including floating-point state)

 b) Identifiers:

    - All types, global functions, and templates are in the namespace 'dbor'.
    - The names of all preprocessor macros start with 'DBOR_'
    - All included files not defined in the C++:2011 (freestanding implementaton) are included
      like '#include "Dbor/..."', where ... contains only ASCII letters,
      decimal digits, '/', '_', and '.'.


Configuration options:

 - With or without use of 64 bit arithmetics by default


DBOR values and associated set of native data types:

   Value type              Associated set of native data types
   ----------------------  -----------------------------------

   IntegerValue            std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                           std::int8_t, std::int16_t, std::int32_t, std::int64_t

   BinaryRationalValue     float, double

   DecimalRationalValue    (std::int32_t, std::int32_t), (std::int64_t, std::int32_t)

   ByteStringValue         std::uint8_t *

   Utf8StringValue         dbor::String
