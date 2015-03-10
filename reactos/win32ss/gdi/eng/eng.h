#pragma once

extern const BYTE gajRop2ToRop3[16];

#define MIX_TO_ROP4(mix) \
    (((ULONG)gajRop2ToRop3[((mix) - 1) & 0xF]) | \
     ((ULONG)gajRop2ToRop3[(((mix) >> 8) - 1) & 0xF] << 8))

VOID
NTAPI
EngAcquireSemaphoreShared(
    IN HSEMAPHORE hsem);

BOOL
APIENTRY
IntEngMaskBlt(
    _Inout_ SURFOBJ *psoDest,
    _In_ SURFOBJ *psoMask,
    _In_ CLIPOBJ *pco,
    _In_ XLATEOBJ *pxloDest,
    _In_ XLATEOBJ *pxloSource,
    _In_ RECTL *prclDest,
    _In_ POINTL *pptlMask,
    _In_ BRUSHOBJ *pbo,
    _In_ POINTL *pptlBrushOrg);

VOID FASTCALL
IntEngWindowChanged(
    _In_    PWND Window,
    _In_    FLONG flChanged);

VOID FASTCALL IntGdiAcquireSemaphore ( HSEMAPHORE hsem );
VOID FASTCALL IntGdiReleaseSemaphore ( HSEMAPHORE hsem );
ULONGLONG APIENTRY EngGetTickCount(VOID);

VOID DecompressBitmap(SIZEL Size, BYTE *CompressedBits, BYTE *UncompressedBits, LONG Delta, ULONG iFormat);

HANDLE
APIENTRY
EngSecureMemForRead(PVOID Address, ULONG Length);
