/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended OptionsClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "SDL3/SDL_surface.h"
#include "extension.h"
#include "options.h"
#include "uicontrol.h"


class CCINIClass;


class OptionsClassExtension final : public GlobalExtensionClass<OptionsClass>
{
public:
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

    enum RendererDriverType {
        RENDERER_DRIVER_AUTO = -1,
        RENDERER_DRIVER_DIRECT3D,
        RENDERER_DRIVER_DIRECT3D11,
        RENDERER_DRIVER_DIRECT3D12,
        RENDERER_DRIVER_OPENGL,
        RENDERER_DRIVER_VULKAN
    };

    enum SubtitleModeType {
        SUBTITLE_MODE_NONE,
        SUBTITLE_MODE_ALL,
        SUBTITLE_MODE_SCENARIO,
        SUBTITLE_MODE_SYSTEM
    };

public:
    OptionsClassExtension(const OptionsClass* this_ptr);
    OptionsClassExtension(const NoInitClass& noinit);
    virtual ~OptionsClassExtension();

    /**
     *  OptionsClass extension does not require these to be used, but we
     *  implement them for completeness.
     */
    virtual int Get_Object_Size() const override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual const char* Name() const override { return "Options"; }
    virtual const char* Full_Name() const override { return "Options"; }

    void Load_Settings();
    void Load_Init_Settings();
    void Save_Settings();

    void Set();
    SidebarViewType Get_Sidebar_View_Type() const;

    static RendererDriverType Parse_Renderer_Driver(const char* name);
    static const char* Get_Renderer_Driver_Config_Name(RendererDriverType driver);
    static const char* Get_Renderer_Driver_SDL_Name(RendererDriverType driver);

    static SubtitleModeType Parse_Subtitle_Mode(const char* name);
    static const char* Subtitle_Mode_Config_Name(SubtitleModeType mode);

public:
    /**
     *  Should cameos of defenses (including walls and gates) be sorted to the bottom of the sidebar?
     */
    bool SortDefensesAsLast;

    /**
     *  Are harvesters and MCVs excluded from a band-box selection that includes combat units?
     */
    bool FilterBandBoxSelection;

    /**
     *  User override for the battle sidebar view type. SIDEBAR_COUNT means use UI.INI.
     */
    SidebarViewType SidebarViewTypeOverride;

    /**
     *  Customizable hotkeys for starting a chat.
     */
    int KeyChatToAll1;
    int KeyChatToAll2;
    int KeyChatToAllies;

    /**
     *  Window size override.
     */
    int WindowWidth;
    int WindowHeight;

    /**
     *  Scaling mode.
     */
    SDL_ScaleMode ScaleMode;

    /**
     *  Cursor scale factor.
     */
    int CursorScale;

    /**
     *  Is VSync on?
     */
    bool IsVSync;

    /**
     *  Preferred SDL renderer backend.
     */
    RendererDriverType RendererDriver;

    /**
     *  Which VOX subtitles should be displayed.
     */
    SubtitleModeType SubtitleMode;

    /**
     *  Should building repairs be paused instead of stopped when the player has insufficient funds?
     */
    bool IsPauseRepairs;

    /**
     *  Number of autosaves to make in singleplayer.
     */
    int AutoSaveCount;

    /**
     *  The delay between autosaves in singleplayer in frames.
     */
    int AutoSaveInterval;

    /**
     *  Should skirmish games be auto-saved?
     */
    bool IsAutoSaveInSkirmish;
};
