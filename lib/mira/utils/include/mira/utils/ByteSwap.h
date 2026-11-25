#pragma once

// Non-Qt cross-platform byte-swap compatibility.
// Maps the Windows byte-swap intrinsics to the common bswap_* names,
// while Unix-like systems use <byteswap.h>.

#ifndef unix
#   define bswap_16 _byteswap_ushort
#   define bswap_32 _byteswap_ulong
#   define bswap_64 _byteswap_uint64
#else
#   include <byteswap.h>
#endif
