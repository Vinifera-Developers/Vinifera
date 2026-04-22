/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  *
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


class Surface;


bool Scale_Surface_Nearest(Surface *src, Surface *dst);
bool Scale_Surface_Bilinear(Surface *src, Surface *dst);
bool Scale_Surface_Bicubic(Surface *src, Surface *dst);
bool Scale_Surface_Cardinal(Surface *src, Surface *dst);
bool Scale_Surface_Lanczos(Surface *src, Surface *dst);
