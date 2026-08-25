/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Function for writing PNG files from a graphic surface.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


class FileClass;
class Surface;
class BSurface;
class Buffer;
class PaletteClass;


bool Write_PNG_File(FileClass *name, Surface &pic);
BSurface *Read_PNG_File(FileClass *name, unsigned char *palette = nullptr, void *buff = nullptr, long size = 0);
BSurface *Read_PNG_File(FileClass *name, const Buffer &buff, PaletteClass *palette = nullptr);
