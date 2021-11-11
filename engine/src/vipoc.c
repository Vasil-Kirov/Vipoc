#if 0

#include "vipoc.h"



static void
RenderWeirdGradient(render_buffer *Buffer, int XOffset, int YOffset)
{
    
    int64 Pitch = Buffer->Width * Buffer->BytesPerPixel;
    uint8 *Row = Buffer->Memory;
    for(int y = 0; y < Buffer->Height; ++y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int x = 0; x < Buffer->Width; ++x)
        {
            // Memory:   BB GG RR xx
            // Register: xx RR GG BB
            // Green moves from blue by shifting 8
            // Blue binary-or sets the 0s in BB to 1s if they're 1s in blue
            uint8 Blue   = (uint8)(x+XOffset);
            uint8 Green  = (uint8)(y+YOffset);
            *Pixel++ = ((Green << 16) | Blue);
        }
        Row += Pitch;
    }
}


void GameUpdateAndRender(render_buffer *Buffer, int XOffset, int YOffset)
{
    RenderWeirdGradient(Buffer, XOffset, YOffset);
}


#endif