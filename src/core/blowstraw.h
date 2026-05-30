/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Blowfish driven straw.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) Electronic Arts
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "blowfish.h"
#include "straw.h"


class BlowStraw : public Straw
{
	public:
		typedef enum CryptControl {
			ENCRYPT,
			DECRYPT
		} CryptControl;

	public:
		BlowStraw(CryptControl control) : BF(nullptr), Counter(0), Control(control) {}
		virtual ~BlowStraw() { delete BF; BF = nullptr; }

		virtual int Get(void * source, int slen) override;

		void Key(void const * key, int length);

	protected:
		BlowfishEngine * BF;

	private:
		char Buffer[8];
		int Counter;
		CryptControl Control;

	private:
		BlowStraw(BlowStraw &) = delete;
		BlowStraw & operator = (const BlowStraw &) = delete;
};
