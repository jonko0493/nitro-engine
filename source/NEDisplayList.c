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
    case NE_FIFO_NOP:
        return 0;
    case NE_FIFO_MTX_MODE:
        return 1;
    case NE_FIFO_MTX_PUSH:
        return 0;
    case NE_FIFO_MTX_POP:
        return 1;
    case NE_FIFO_MTX_STORE:
        return 1;
    case NE_FIFO_MTX_RESTORE:
        return 1;
    case NE_FIFO_MTX_IDENTITY:
        return 0;
    case NE_FIFO_MTX_LOAD_4x4:
        return 16;
    case NE_FIFO_MTX_LOAD_4x3:
        return 12;
    case NE_FIFO_MTX_MULT_4x4:
        return 16;
    case NE_FIFO_MTX_MULT_4x3:
        return 12;
    case NE_FIFO_MTX_MULT_3x3:
        return 9;
    case NE_FIFO_MTX_SCALE:
        return 3;
    case NE_FIFO_MTX_TRANS:
        return 3;
    case NE_FIFO_COLOR:
        return 1;
    case NE_FIFO_NORMAL:
        return 1;
    case NE_FIFO_TEXCOORD:
        return 1;
    case NE_FIFO_VTX_16:
        return 2;
    case NE_FIFO_VTX_10:
        return 1;
    case NE_FIFO_VTX_XY:
        return 1;
    case NE_FIFO_VTX_XZ:
        return 1;
    case NE_FIFO_VTX_YZ:
        return 1;
    case NE_FIFO_VTX_DIFF:
        return 1;
    case NE_FIFO_POLYGON_ATTR:
        return 1;
    case NE_FIFO_TEXIMAGE_PARAM:
        return 1;
    case NE_FIFO_PLTT_BASE:
        return 1;
    case NE_FIFO_DIF_AMB:
        return 1;
    case NE_FIFO_SPE_EMI:
        return 1;
    case NE_FIFO_LIGHT_VECTOR:
        return 1;
    case NE_FIFO_LIGHT_COLOR:
        return 1;
    case NE_FIFO_SHININESS:
        return 32;
    case NE_FIFO_BEGIN_VTXS:
        return 1;
    case NE_FIFO_END_VTXS:
        return 0;
    case NE_FIFO_SWAP_BUFFERS:
        return 1;
    case NE_FIFO_VIEWPORT:
        return 1;
    case NE_FIFO_BOX_TEST:
        return 3;
    case NE_FIFO_POS_TEST:
        return 2;
    case NE_FIFO_VEC_TEST:
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
    NE_AssertPointer(list, "NULL display list pointer");
    NE_AssertPointer(modification, "NULL modification function");

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