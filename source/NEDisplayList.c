// SPDX-License-Identifier: MIT
//
// Copyright (c) 2023 Antonio Niño Díaz
//
// This file is part of Nitro Engine

#include <nds.h>

#include "NEMain.h"

static int num_params_for_fifo_cmd(NE_GfxFifoCmd cmd)
{
    switch (cmd)
    {
    case NOP:
        return 0;
    case MTX_MODE:
        return 1;
    case MTX_PUSH:
        return 0;
    case MTX_POP:
        return 1;
    case MTX_STORE:
        return 1;
    case MTX_RESTORE:
        return 1;
    case MTX_IDENTITY:
        return 0;
    case MTX_LOAD_4x4:
        return 16;
    case MTX_LOAD_4x3:
        return 12;
    case MTX_MULT_4x4:
        return 16;
    case MTX_MULT_4x3:
        return 12;
    case MTX_MULT_3x3:
        return 9;
    case MTX_SCALE:
        return 3;
    case MTX_TRANS:
        return 3;
    case COLOR:
        return 1;
    case NORMAL:
        return 1;
    case TEXCOORD:
        return 1;
    case VTX_16:
        return 2;
    case VTX_10:
        return 1;
    case VTX_XY:
        return 1;
    case VTX_XZ:
        return 1;
    case VTX_YZ:
        return 1;
    case VTX_DIFF:
        return 1;
    case POLYGON_ATTR:
        return 1;
    case TEXIMAGE_PARAM:
        return 1;
    case PLTT_BASE:
        return 1;
    case DIF_AMB:
        return 1;
    case SPE_EMI:
        return 1;
    case LIGHT_VECTOR:
        return 1;
    case LIGHT_COLOR:
        return 1;
    case SHININESS:
        return 32;
    case BEGIN_VTXS:
        return 1;
    case END_VTXS:
        return 0;
    case SWAP_BUFFERS:
        return 1;
    case VIEWPORT:
        return 1;
    case BOX_TEST:
        return 3;
    case POS_TEST:
        return 2;
    case VEC_TEST:
        return 1;
    }

    return 0;
}

//--------------------------------------------------------------------------

void NE_DisplayListDrawDMA_GFX_FIFO(const void *list)
{
    const uint32_t *p = list;

    NE_AssertPointer(p, "NULL display list pointer");

    uint32_t words = *p++;

    NE_Assert(words > 0, "Empty display list");

    DC_FlushRange(p, words * 4);

    // There is a hardware bug that affects DMA when there are multiple channels
    // active, under certain conditions. Instead of checking for said
    // conditions, simply ensure that there are no DMA channels active.
    while (dmaBusy(0) || dmaBusy(1) || dmaBusy(2) || dmaBusy(3));

#ifdef NE_BLOCKSDS
    dmaSetParams(0, p, (void *)&GFX_FIFO, DMA_FIFO | words);
#else
    DMA_SRC(0) = (uint32_t)p;
    DMA_DEST(0) = (uint32_t)&GFX_FIFO;
    DMA_CR(0) = DMA_FIFO | words;
#endif

    while (dmaBusy(0));
}

void NE_DisplayListDrawCPU(const void *list)
{
    const uint32_t *p = list;

    NE_AssertPointer(p, "NULL display list pointer");

    uint32_t words = *p++;

    NE_Assert(words > 0, "Empty display list");

    while (words--)
        GFX_FIFO = *p++;
}

typedef void (*ne_display_list_draw_fn)(const void *);

static ne_display_list_draw_fn ne_display_list_draw = NE_DisplayListDrawDMA_GFX_FIFO;

void NE_DisplayListSetDefaultFunction(NE_DisplayListDrawFunction type)
{
    if (type == NE_DL_CPU)
    {
        ne_display_list_draw = NE_DisplayListDrawCPU;
    }
    else if (type == NE_DL_DMA_GFX_FIFO)
    {
        ne_display_list_draw = NE_DisplayListDrawDMA_GFX_FIFO;
    }
    else
    {
        NE_Assert(0, "Invalid display list function type");
        ne_display_list_draw = NE_DisplayListDrawDMA_GFX_FIFO;
    }
}

void NE_DisplayListDrawDefault(const void *list)
{
    ne_display_list_draw(list);
}

void NE_DisplayListModify(const void *list, void (*modification)(NE_GfxFifoCmd cmd, void *params))
{
    uint32_t *dl = (uint32_t *)list;
    int size = *dl++;
    for (int i = 0; i < size;)
    {
        NE_GfxFifoCmd dlCmd[4];
        char *dlCmdBuff = (char *)dl;
        for (int j = 0; j < 4; j++)
        {
            dlCmd[j] = (NE_GfxFifoCmd)*dlCmdBuff++;
        }
        dl++;
        i++;
        for (int j = 0; j < 4; j++)
        {
            int dlParamSize = num_params_for_fifo_cmd(dlCmd[j]);
            modification(dlCmd[j], dl);
            dl += dlParamSize;
            i += dlParamSize;
        }
    }
}