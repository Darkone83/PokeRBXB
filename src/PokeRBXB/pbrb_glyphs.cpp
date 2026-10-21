#include "pbrb_glyphs.h"
#include "pbrb_video.h"
// Vector button glyphs follow DarkDash dd_hints: independent of active DDF font.
static void Stroke(float x1, float y1, float x2, float y2, DWORD c) {
    PbrbPoint a = PbrbPt(x1, y1), b = PbrbPt(x2, y2);
    PbrbLine(a.x, a.y, b.x, b.y, 3.6f * PbrbScale(), c);
}
void PbrbButtonGlyph(float x, float y, char letter) {
    DWORD bg = 0xFFAAAAAA, ink = 0xFFFFFFFF;
    if (letter == 'A')bg = 0xFF3DA94B;
    if (letter == 'B')bg = 0xFFD54848;
    if (letter == 'X')bg = 0xFF3B84D4;
    if (letter == 'Y') { bg = 0xFFE5BD38; ink = 0xFF141821; }
    PbrbPoint p = PbrbPt(x, y);
    PbrbCircle(p.x, p.y, 19.0f * PbrbScale(), 0xFF080C12);
    PbrbCircle(p.x, p.y, 17.0f * PbrbScale(), bg);
    float l = x - 8.0f, t = y - 11.0f;
    switch (letter) {
    case 'A':
        Stroke(l, t + 22, l + 8, t, ink); Stroke(l + 8, t, l + 16, t + 22, ink);
        Stroke(l + 4, t + 14, l + 12, t + 14, ink); break;
    case 'B':
        Stroke(l, t, l, t + 22, ink); Stroke(l, t, l + 13, t, ink);
        Stroke(l, t + 11, l + 13, t + 11, ink); Stroke(l, t + 22, l + 13, t + 22, ink);
        Stroke(l + 13, t, l + 13, t + 11, ink); Stroke(l + 13, t + 11, l + 13, t + 22, ink); break;
    case 'X':Stroke(l, t, l + 16, t + 22, ink); Stroke(l + 16, t, l, t + 22, ink); break;
    case 'Y':Stroke(l, t, l + 8, t + 11, ink); Stroke(l + 16, t, l + 8, t + 11, ink);
        Stroke(l + 8, t + 11, l + 8, t + 22, ink); break;
    }
}
