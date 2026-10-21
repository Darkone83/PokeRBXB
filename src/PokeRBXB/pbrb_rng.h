#ifndef PBRB_RNG_H_INCLUDED
#define PBRB_RNG_H_INCLUDED
/* Shared native gameplay generator. The original updates its random bytes
   from the Game Boy divider; native execution cannot reproduce that timing.
   Keep one deterministic 32-bit stream, consume it at gameplay decisions,
   and advance it once per gameplay frame, including menus and dialogue.
   Existing saves retain their 32-bit state; zero is repaired on first use. */
static inline unsigned PbrbRandomNext(unsigned* state) {
    unsigned x = *state; if (!x)x = 0x6d2b79f5u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5; *state = x; return x;
}
#endif
#pragma once
