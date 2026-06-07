// SPDX-License-Identifier: MIT
//
// Copyright (c) 2023 Antonio Niño Díaz
//
// This file is part of Nitro Engine

#ifndef NE_DISPLAYLIST_H__
#define NE_DISPLAYLIST_H__

/// @file   NEDisplayList.h
/// @brief  Functions to send display lists to the GPU.

/// @defgroup display_list_system Display list handling system
///
/// Functions to send display lists to the GPU.
///
/// @{

/// GFX FIFO commands
typedef enum {
    NOP = 0x00,
    MTX_MODE = 0x10,
    MTX_PUSH = 0x11,
    MTX_POP = 0x12,
    MTX_STORE = 0x13,
    MTX_RESTORE = 0x14,
    MTX_IDENTITY = 0x15,
    MTX_LOAD_4x4 = 0x16,
    MTX_LOAD_4x3 = 0x17,
    MTX_MULT_4x4 = 0x18,
    MTX_MULT_4x3 = 0x19,
    MTX_MULT_3x3 = 0x1A,
    MTX_SCALE = 0x1B,
    MTX_TRANS = 0x1C,
    COLOR = 0x20,
    NORMAL = 0x21,
    TEXCOORD = 0x22,
    VTX_16 = 0x23,
    VTX_10 = 0x24,
    VTX_XY = 0x25,
    VTX_XZ = 0x26,
    VTX_YZ = 0x27,
    VTX_DIFF = 0x28,
    POLYGON_ATTR = 0x29,
    TEXIMAGE_PARAM = 0x2A,
    PLTT_BASE = 0x2B,
    DIF_AMB = 0x30,
    SPE_EMI = 0x31,
    LIGHT_VECTOR = 0x32,
    LIGHT_COLOR = 0x33,
    SHININESS = 0x34,
    BEGIN_VTXS = 0x40,
    END_VTXS = 0x41,
    SWAP_BUFFERS = 0x50,
    VIEWPORT = 0x60,
    BOX_TEST = 0x70,
    POS_TEST = 0x71,
    VEC_TEST = 0x72
} NE_GfxFifoCmd;

/// Possible ways to send display lists to the GPU.
typedef enum {
    NE_DL_CPU,          ///< Send all data to the GPU with CPU copy loop.
    NE_DL_DMA_GFX_FIFO, ///< Default. DMA in GFX FIFO mode (incompatible with safe dual 3D)
    // TODO: Support DMA without GFX FIFO DMA mode, using GFX FIFO IRQ instead.
} NE_DisplayListDrawFunction;

/// Sends a display list to the GPU by using the DMA in GFX FIFO mode.
///
/// Important note: Don't use this function when using safe dual 3D. Check the
/// documentation of NE_DisplayListSetDefaultFunction() for more information.
///
/// @param list Pointer to the display list
void NE_DisplayListDrawDMA_GFX_FIFO(const void *list);

/// Sends a display list to the GPU by using a CPU copy loop.
///
/// @param list Pointer to the display list
void NE_DisplayListDrawCPU(const void *list);

/// Set the default way to send display lists to the GPU.
///
/// Important note: NE_DL_DMA_GFX_FIFO isn't compatible with safe dual 3D mode
/// because it uses DMA in horizontal blanking start mode. There is a hardware
/// bug that makes it unreliable to have both DMA channels active at the same
/// time in HBL start and GFX FIFO mode.
///
/// @param type Copy type to use.
void NE_DisplayListSetDefaultFunction(NE_DisplayListDrawFunction type);

/// Draw a display list using the selected default function.
///
/// This will use the function selected by NE_DisplayListSetDefaultFunction().
///
/// @param list Pointer to the display list
void NE_DisplayListDrawDefault(const void *list);

/// Modify a display list in place using a user-provided function
///
/// @param list Pointer to the display list
/// @param modification Function that can be used to modify the display list
void NE_DisplayListModify(const void *list, void (*modification)(NE_GfxFifoCmd cmd, void *params));

/// @}

#endif // NE_DISPLAYLIST_H__
