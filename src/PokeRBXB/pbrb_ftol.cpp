/*---------------------------------------------------------------------------
    pbrb_ftol.cpp -- RXDK MSVC x86 float-to-integer conversion helper.

    Based on the proven implementation in Darkone83/DarkDash dd_ftol.cpp.

    Older RXDK CRTs do not provide __ftol2_sse, which the MSVC compiler may
    emit for float->int casts (such as PbrbTextWidth in pbrb_ddf.cpp).

    MSVC x86 name decoration:
      C name:  _ftol2_sse
      COFF:    __ftol2_sse

    The source floating-point value arrives in x87 ST(0). A C cast truncates
    toward zero; x87 defaults to round-to-nearest. Save the caller's control
    word, select truncate, convert a signed 64-bit result to EDX:EAX, and
    restore the control word. FISTP also pops the x87 source value.

    No header or function call is required. Compile this .cpp into PokeRBXB.
    This implementation is strictly for 32-bit x86 / older RXDK builds.
---------------------------------------------------------------------------*/

#if !defined(_M_IX86)
#error pbrb_ftol.cpp requires 32-bit x86 (PokeRBXB RXDK target).
#endif

#pragma warning(push)
#pragma warning(disable: 4731) /* EBP modified in naked function */

extern "C" __declspec(naked) void __cdecl _ftol2_sse(void)
{
    __asm
    {
        push    ebp
        mov     ebp, esp
        sub     esp, 16
        fnstcw  word ptr[ebp - 2]
        movzx   eax, word ptr[ebp - 2]
        or ah, 0Ch
        mov     word ptr[ebp - 4], ax
        fldcw   word ptr[ebp - 4]
        fistp   qword ptr[ebp - 12]
        fldcw   word ptr[ebp - 2]
        mov     eax, dword ptr[ebp - 12]
        mov     edx, dword ptr[ebp - 8]
        mov     esp, ebp
        pop     ebp
        ret
    }
}

#pragma warning(pop)
