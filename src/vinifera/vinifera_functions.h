/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  General functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


bool Vinifera_Parse_Command_Line(int argc, char *argv[]);
bool Vinifera_Startup();
bool Vinifera_Shutdown();
int Vinifera_Pre_Init_Game(int argc, char *argv[]);
int Vinifera_Post_Init_Game(int argc, char *argv[]);
bool Vinifera_Register_Com_Objects();
