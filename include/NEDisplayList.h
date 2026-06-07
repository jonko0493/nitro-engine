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
    NE_FIFO_NOP = 0x00,
    NE_FIFO_MTX_MODE = 0x10,
    NE_FIFO_MTX_PUSH = 0x11,
    NE_FIFO_MTX_POP = 0x12,
    NE_FIFO_MTX_STORE = 0x13,
    NE_FIFO_MTX_RESTORE = 0x14,
    NE_FIFO_MTX_IDENTITY = 0x15,
    NE_FIFO_MTX_LOAD_4x4 = 0x16,
    NE_FIFO_MTX_LOAD_4x3 = 0x17,
    NE_FIFO_MTX_MULT_4x4 = 0x18,
    NE_FIFO_MTX_MULT_4x3 = 0x19,
    NE_FIFO_MTX_MULT_3x3 = 0x1A,
    NE_FIFO_MTX_SCALE = 0x1B,
    NE_FIFO_MTX_TRANS = 0x1C,
    NE_FIFO_COLOR = 0x20,
    NE_FIFO_NORMAL = 0x21,
    NE_FIFO_TEXCOORD = 0x22,
    NE_FIFO_VTX_16 = 0x23,
    NE_FIFO_VTX_10 = 0x24,
    NE_FIFO_VTX_XY = 0x25,
    NE_FIFO_VTX_XZ = 0x26,
    NE_FIFO_VTX_YZ = 0x27,
    NE_FIFO_VTX_DIFF = 0x28,
    NE_FIFO_POLYGON_ATTR = 0x29,
    NE_FIFO_TEXIMAGE_PARAM = 0x2A,
    NE_FIFO_PLTT_BASE = 0x2B,
    NE_FIFO_DIF_AMB = 0x30,
    NE_FIFO_SPE_EMI = 0x31,
    NE_FIFO_LIGHT_VECTOR = 0x32,
    NE_FIFO_LIGHT_COLOR = 0x33,
    NE_FIFO_SHININESS = 0x34,
    NE_FIFO_BEGIN_VTXS = 0x40,
    NE_FIFO_END_VTXS = 0x41,
    NE_FIFO_SWAP_BUFFERS = 0x50,
    NE_FIFO_VIEWPORT = 0x60,
    NE_FIFO_BOX_TEST = 0x70,
    NE_FIFO_POS_TEST = 0x71,
    NE_FIFO_VEC_TEST = 0x72
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
