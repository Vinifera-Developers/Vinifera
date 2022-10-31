/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_UTIL.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         Various utility functions.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "vinifera_util.h"
#include "vinifera_gitinfo.h"
#include "tspp_gitinfo.h"
#include "vinifera_const.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "colorscheme.h"
#include "textprint.h"
#include "dsurface.h"
#include "bsurface.h"
#include "spritecollection.h"
#include "filepng.h"
#include "filepcx.h"
#include "cncnet4_globals.h"
#include "wwfont.h"
#include "msgbox.h"
#include "minidump.h"
#include "winutil.h"
#include "xzip.h"
#include <cstdio>


extern char Execute_Time_Buffer[256];


/**
 *  Returns the Vinifera run config info as string.
 * 
 *  @author: CCHyper
 */
const char *Vinifera_Name_String()
{
    static char _buffer[512] { '\0' };

    if (_buffer[0] == '\0') {

        /**
         *  Append the CnCNet version if enabled.
         */
        char *cncnet_mode = nullptr;
        if (CnCNet4::IsEnabled) {
            cncnet_mode = " (CnCNet4)";
        }

        char *dev_mode = nullptr;
        if (Vinifera_DeveloperMode) {
            dev_mode = " (Dev)";
        }

        if (!dev_mode && !cncnet_mode) {
            std::snprintf(_buffer, sizeof(_buffer), "Vinifera");

        } else {
            std::snprintf(_buffer, sizeof(_buffer), "Vinifera:%s%s",
                cncnet_mode != nullptr ? cncnet_mode : "",
                dev_mode != nullptr ? dev_mode : "");
        }

#if defined(TS_CLIENT)
        std::strcat(_buffer, " (TS-Client)");
#endif
        
    }

    return _buffer;
}


/**
 *  Returns the Vinifera version git info as string.
 * 
 *  The "~" character is added if there are changes made locally before the build was produced.
 * 
 *  @author: CCHyper
 */
const char *Vinifera_Version_Git_String()
{
    static char _buffer[512] { '\0' };

    if (_buffer[0] == '\0') {
        
#ifndef RELEASE
        std::snprintf(_buffer, sizeof(_buffer), "%s %s %s%s %s",
            Vinifera_Git_Branch(), Vinifera_Git_Author(),
            Vinifera_Git_Uncommitted_Changes() ? "~" : "", Vinifera_Git_Hash_Short(), Vinifera_Git_DateTime());
#else
        std::snprintf(_buffer, sizeof(_buffer), "%s%s %s",
            Vinifera_Git_Uncommitted_Changes() ? "~" : "", Vinifera_Git_Hash_Short(), Vinifera_Git_DateTime());
#endif
    }

    return _buffer;
}


/**
 *  Returns the Vinifera version info as string.
 * 
 *  The "~" character is added if there are changes made locally before the build was produced.
 * 
 *  @author: CCHyper
 */
const char *Vinifera_Version_String()
{
    static char _buffer[512] { '\0' };

    if (_buffer[0] == '\0') {

        /**
         *  Append the CnCNet version if enabled.
         */
        char *cncnet_mode = nullptr;
        if (CnCNet4::IsEnabled) {
            cncnet_mode = " (CnCNet4)";
        }
        
#ifndef RELEASE
        std::snprintf(_buffer, sizeof(_buffer), "Vinifera:%s%s - %s %s %s%s %s",
            cncnet_mode != nullptr ? cncnet_mode : "",
            Vinifera_DeveloperMode ? " (Dev)" : "",
            Vinifera_Git_Branch(), Vinifera_Git_Author(),
            Vinifera_Git_Uncommitted_Changes() ? "~" : "", Vinifera_Git_Hash_Short(), Vinifera_Git_DateTime());
#else
        std::snprintf(_buffer, sizeof(_buffer), "Vinifera:%s%s - %s%s %s",
            cncnet_mode != nullptr ? cncnet_mode : "",
            Vinifera_DeveloperMode ? " (Dev)" : "",
            Vinifera_Git_Uncommitted_Changes() ? "~" : "", Vinifera_Git_Hash_Short(), Vinifera_Git_DateTime());
#endif
    }

    return _buffer;
}


/**
 *  Returns the TS++ version info as string.
 * 
 *  The "~" character is added if there are changes made locally before the build was produced.
 * 
 *  @author: CCHyper
 */
const char *TSpp_Version_String()
{
    static char _buffer[512] { '\0' };

    if (_buffer[0] == '\0') {
        std::snprintf(_buffer, sizeof(_buffer), "TS++: %s %s %s%s %s",
            TSPP_Git_Branch(), TSPP_Git_Author(),
            TSPP_Git_Uncommitted_Changes() ? "~" : "", TSPP_Git_Hash_Short(), TSPP_Git_DateTime());
    }

    return _buffer;
}


/**
 *  Draws the version info to the input surface.
 * 
 *  @note: This function will draw the text relative to the bottom right of the surface. 
 * 
 *  @author: CCHyper
 */
void Vinifera_Draw_Version_Text(XSurface *surface, bool pre_init)
{
    if (!surface) {
        return;
    }
    
#ifndef RELEASE
    static Point2D warning_pos;
    static Point2D vinifera_pos;
#endif
    static Point2D version_pos;

    Rect surfrect = surface->Get_Rect();

    TextPrintType style = (TPF_RIGHT|TPF_NOSHADOW|TPF_8POINT);
    ColorScheme *color_white = ColorScheme::As_Pointer("White");
    ColorScheme *color_yellow = ColorScheme::As_Pointer("Yellow");

#ifndef RELEASE
#if defined(NIGHTLY)
    ColorType nightly_color = COLOR_WHITE;
    ColorType nightly_back_color = COLOR_BLUE;
#elif defined(PREVIEW)
    ColorType preview_color = COLOR_WHITE;
    ColorType preview_back_color = COLOR_PURPLE;
#else
    ColorType warning_color = COLOR_YELLOW;
    ColorType warning_back_color = COLOR_RED;
#endif
#endif
    ColorType version_color = COLOR_WHITE;
    ColorType back_color = COLOR_BLACK;

    /**
     *  This is just to retrieve the font height, we don't need to know
     *  the width as we print with right alignment.
     */
    WWFontClass *font = Font_Ptr(style);
    font->Set_X_Spacing(2);

    Rect print_rect;
    font->String_Pixel_Rect("X", &print_rect);

    int offset = 3; // Pixels from edge
        
#ifndef RELEASE
    int space = 0; // Line space
    warning_pos.X = surfrect.Width-offset;
    warning_pos.Y = surfrect.Height-offset-(print_rect.Height*1);
    version_pos.X = surfrect.Width-offset;
    version_pos.Y = surfrect.Height-offset-(print_rect.Height*2)-space;
    vinifera_pos.X = surfrect.Width-offset;
    vinifera_pos.Y = surfrect.Height-offset-(print_rect.Height*3)-space;
#else
    version_pos.X = surfrect.Width-offset;
    version_pos.Y = surfrect.Height-offset-(print_rect.Height*1);
#endif

    /**
     *  So, we need to draw the strings slightly differently if this is being drawn
     *  before the games initialisation process has finished. This is because the
     *  ColorSchemes has not be initialised yet.
     */
    if (pre_init) {

#ifndef RELEASE
        /**
         *  Draw the version string.
         */
        Simple_Text_Print(Vinifera_Version_Git_String(), surface, &surfrect, &version_pos, NormalDrawer, version_color, back_color, style);

        /**
         *  Draw the warning string.
         */
    #if defined(NIGHTLY)
        Simple_Text_Print(TXT_VINIFERA_NIGHTLY_BUILD, surface, &surfrect, &warning_pos, NormalDrawer, nightly_color, nightly_back_color, style);
    #elif defined(PREVIEW)
        Simple_Text_Print(TXT_VINIFERA_PREVIEW_BUILD, surface, &surfrect, &warning_pos, NormalDrawer, preview_color, preview_back_color, style);
    #else
        Simple_Text_Print(Vinifera_Git_Uncommitted_Changes() ? TXT_VINIFERA_LOCAL_BUILD : TXT_VINIFERA_UNOFFICIAL_BUILD,
            surface, &surfrect, &warning_pos, NormalDrawer, warning_color, warning_back_color, style);
    #endif

        /**
         *  Draw the vinifera name string.
         */
        Simple_Text_Print(Vinifera_Name_String(), surface, &surfrect, &vinifera_pos, NormalDrawer, version_color, back_color, style);
#else

        /**
         *  Draw the vinifera name string.
         */
        Simple_Text_Print(Vinifera_Version_String(), surface, &surfrect, &version_pos, NormalDrawer, version_color, back_color, style);
#endif

    } else {

#ifndef RELEASE

        /**
         *  Draw the version string.
         */
        Fancy_Text_Print(Vinifera_Version_Git_String(), surface, &surfrect, &version_pos, color_white, back_color, style);

        /**
         *  Draw the warning string.
         */
    #if defined(NIGHTLY)
        Fancy_Text_Print(TXT_VINIFERA_NIGHTLY_BUILD, surface, &surfrect, &warning_pos, color_white, nightly_back_color, style);
    #elif defined(PREVIEW)
        Fancy_Text_Print(TXT_VINIFERA_PREVIEW_BUILD, surface, &surfrect, &warning_pos, color_white, preview_back_color, style);
    #else
        Fancy_Text_Print(Vinifera_Git_Uncommitted_Changes() ? TXT_VINIFERA_LOCAL_BUILD : TXT_VINIFERA_UNOFFICIAL_BUILD,
            surface, &surfrect, &warning_pos, color_yellow, warning_back_color, style);
    #endif

        /**
         *  Draw the vinifera name string.
         */
        Fancy_Text_Print(Vinifera_Name_String(), surface, &surfrect, &vinifera_pos, color_white, back_color, style);
#else

        /**
         *  Draw the version string.
         */
        Fancy_Text_Print(Vinifera_Version_String(), surface, &surfrect, &version_pos, color_white, back_color, style);
#endif

    }
}


/**
 *  Write a mini dump file for analysis.
 */
bool Vinifera_Generate_Mini_Dump()
{
#if 0
    MessageBox(MainWindow,
        "A crash dump will now be generated that can be sent to the\n"
        "developers for further analysis.\n\n"
        "Please note: This may take some time depending on the options\n"
        "set by the crash dump generator, please be patient and allow\n"
        "this process to finish. You will be notified when it is complete.\n\n",
        "Crash dump", 
        MB_OK|MB_ICONQUESTION);

    GenerateFullCrashDump = false; // We don't need a full memory dump.
    bool res = Create_Mini_Dump(nullptr, Get_Module_File_Name());

    if (res) {
        char buffer[512];
        std::snprintf(buffer, sizeof(buffer),
            "Crash dump file generated successfully.\n\n"
            "Please make sure you package DEBUG_<date-time>.LOG\n"
            "and EXCEPT_<date-time>.LOG along with this crash dump file!\n\n"
            "Filename:\n\"%s\" \n", MinidumpFilename);
        MessageBox(MainWindow, buffer, "Crash dump", MB_OK);
        return true;
    }

    MessageBox(MainWindow, "Failed to create crash dump!\n\n", "Crash dump", MB_OK|MB_ICONASTERISK);
#endif
    return false;
}


/**
 *  Shows a in-game message box.
 * 
 *  This has been made its own function because we can not allocate on the stack with
 *  our patches, so this handles all that within this function scope.
 * 
 *  @author: CCHyper
 */
int Vinifera_Do_WWMessageBox(const char *msg, const char *btn1, const char *btn2, const char *btn3)
{
    return WWMessageBox().Process(msg, 0, btn1, btn2, btn3);
}


/**
 *  Shows a in-game warning message box only if developer mode is active.
 * 
 *  This has been made its own function because we can not allocate on the stack with
 *  our patches, so this handles all that within this function scope.
 * 
 *  @author: CCHyper
 */
void Vinifera_DeveloperMode_Warning_WWMessageBox(const char *msg, ...)
{
    if (Vinifera_DeveloperMode) {

        char msg_buff[512];
        std:snprintf(msg_buff, sizeof(msg_buff), "WARNING!\n%s", msg);
        
        char buffer[512];	    // Working staging buffer.
        va_list	arg;		    // Argument list var.

        va_start(arg, msg);
        vsnprintf(buffer, sizeof(buffer), msg_buff, arg);
        va_end(arg);

        WWMessageBox().Process(buffer, 0, "OK");
    }
}


/**
 *  Build the window title name.
 * 
 *  @author: CCHyper
 */
const char *Vinifera_Get_Window_Title(DWORD dwPid)
{
    static char _window_name[512];

    if (_window_name[0] != '\0') {
        return _window_name;
    }

    char title_buff[32];
    if (Vinifera_ProjectName[0] != '\0') {
        std::strncpy(title_buff, Vinifera_ProjectName, sizeof(title_buff));
    } else {
        std::strncpy(title_buff, Text_String(TXT_SHORT_TITLE), sizeof(title_buff));
    }
    title_buff[sizeof(title_buff)-1] = '\0';

#ifndef NDEBUG
    std::snprintf(_window_name, sizeof(_window_name),
        "%s (PID:%d) - [Vinifera (Dev)] (%s %s%s %s)",
        title_buff,
        dwPid,
        Vinifera_Git_Branch(),
        Vinifera_Git_Uncommitted_Changes() ? "~" : "",
        Vinifera_Git_Hash_Short(),
        Vinifera_Git_DateTime());
#else
    if (Vinifera_DeveloperMode) {
        std::snprintf(_window_name, sizeof(_window_name),
            "%s (PID:%d) (Developer Mode)", title_buff, dwPid);

#if defined(TS_CLIENT)
        std::strcat(_window_name, " (TS-Client)");
#endif

    } else {
        std::snprintf(_window_name, sizeof(_window_name),
            "%s", title_buff);
    }
#endif

    return _window_name;
}


/**
 *  Creates a zip file is the specified files.
 * 
 *  @note: If the zip file already exists, it will be updated.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Create_Zip(const char *filename, DynamicVectorClass<const char *> &filelist, const char *path)
{
    char buffer[PATH_MAX];
    
    if (path) {
        std::snprintf(buffer, sizeof(buffer), "%s\\%s", path, filename);
    } else {
        std::snprintf(buffer, sizeof(buffer), ".\\%s", filename);
    }

    HZIP hZip = CreateZip((void *)buffer, 0, ZIP_FILENAME);
    if (!hZip) {
        DEBUG_ERROR("Failed to create zip archive \"%s\"!\n", filename);
        return false;
    }

    /**
     *  
     */
    for (int i = 0; i < filelist.Count(); ++i) {
        if (path) {
            std::snprintf(buffer, sizeof(buffer), "%s\\%s", path, filelist[i]);
        } else {
            std::snprintf(buffer, sizeof(buffer), ".\\%s", filelist[i]);
        }
        ZRESULT zresult = ZipAdd(hZip, filelist[i], buffer, 0, ZIP_FILENAME);
        if (zresult != ZR_OK) {
            DEBUG_ERROR("Failed to add file \"%s\" to zip archive \"%s\"!\n", buffer, filename);
            return false;
        }
    }
    
    DEBUG_INFO("Zip archive \"%s\" created sucessfully.\n", filename);

    return CloseZip(hZip) == ZR_OK;
}


/**
 *  Collects the debug files from this session and creates a zip file.
 * 
 *  @note: If the zip file already exists, it will be updated.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Collect_Debug_Files()
{
#if 0
    char buffer[PATH_MAX];

    char debug_buffer[PATH_MAX];
    char except_buffer[PATH_MAX];
    char stack_buffer[PATH_MAX];
    char crashdump_buffer[PATH_MAX];

    RawFileClass tmpfile;
    DynamicVectorClass<const char *> files;

    std::snprintf(debug_buffer, sizeof(debug_buffer), "%s\\DEBUG_%s.LOG", Vinifera_DebugDirectory, Execute_Time_Buffer);
    tmpfile.Set_Name(debug_buffer);
    if (tmpfile.Is_Available()) {
        std::snprintf(buffer, sizeof(buffer), "DEBUG_%s.LOG", Execute_Time_Buffer);
        files.Add(strdup(buffer));
    }

    std::snprintf(except_buffer, sizeof(except_buffer), "%s\\EXCEPT_%s.TXT", Vinifera_DebugDirectory, Execute_Time_Buffer);
    tmpfile.Set_Name(except_buffer);
    if (tmpfile.Is_Available()) {
        std::snprintf(buffer, sizeof(buffer), "EXCEPT_%s.TXT", Execute_Time_Buffer);
        files.Add(strdup(buffer));
    }

    std::snprintf(stack_buffer, sizeof(stack_buffer), "%s\\STACK_%s.LOG", Vinifera_DebugDirectory, Execute_Time_Buffer);
    tmpfile.Set_Name(stack_buffer);
    if (tmpfile.Is_Available()) {
        std::snprintf(buffer, sizeof(buffer), "STACK_%s.LOG", Execute_Time_Buffer);
        files.Add(strdup(buffer));
    }

    const char *module_name = strupr((char *)Get_Module_File_Name());
    std::snprintf(crashdump_buffer, sizeof(crashdump_buffer), "%s\\CRASHDUMP_%s_%s.DMP", Vinifera_DebugDirectory, module_name, Execute_Time_Buffer);
    tmpfile.Set_Name(crashdump_buffer);
    if (tmpfile.Is_Available()) {
        std::snprintf(buffer, sizeof(buffer), "CRASHDUMP_%s_%s.DMP", module_name, Execute_Time_Buffer);
        files.Add(strdup(buffer));
    }

    std::snprintf(buffer, sizeof(buffer), "%s_%s.ZIP", "DEBUG", Execute_Time_Buffer);
    bool result = Vinifera_Create_Zip(buffer, files, Vinifera_DebugDirectory);

    /**
     *  Cleanup files.
     */
    if (result) {
        RawFileClass(debug_buffer).Delete();
        RawFileClass(except_buffer).Delete();
        RawFileClass(stack_buffer).Delete();
        RawFileClass(crashdump_buffer).Delete();
    }

    /**
     *  Cleanup memory.
     */
    for (int i = 0; i < files.Count(); ++i) {
        std::free((void *)files[i]);
    }
    files.Delete_All();

    return result;
#endif
    return true;
}


/**
 *  Fetch string from the program resources.
 */
#ifndef NDEBUG
const char *Vinifera_Fetch_String(HMODULE handle, ULONG id, const char *file, int line)
#else
const char *Vinifera_Fetch_String(HMODULE handle, ULONG id)
#endif
{
    static struct StringCache
    {
        StringCache() : ID(-1), Index(-1), Buffer() {}
        ~StringCache() {}

        int ID;
        int Index;
        char Buffer[2048];

    } _string_cache[256];

    static int _used = 0;

    static const char _null = '\0';
    //static char _buffer[2048];
    //char _buff[2048];

    //DEBUG_INFO("Fetch_String(enter)\n");

    if (handle == nullptr || id == -1) {
        return _null;
    }

    int next_index = ++_used;

    for (int i = 0; i < ARRAY_SIZE(_string_cache); ++i) {
        StringCache &s = _string_cache[i];
        if (s.ID == id) {
            s.Index = next_index;
            return s.Buffer;
        }
    }

    int free_index = -1;

    int last_index = -1;
    for (int i = 0; i < ARRAY_SIZE(_string_cache); ++i) {

        StringCache &s = _string_cache[i];
        if (free_index == -1 || last_index > s.Index) {

            free_index = i;
            last_index = s.Index;

            if (s.Index == -1 || s.ID == -1) {
                break;
            }
        }
    }

    StringCache &free_entry = _string_cache[free_index];
    free_entry.ID = id;
    free_entry.Index = free_index;

    DWORD rc = LoadString(handle, id, free_entry.Buffer, sizeof(free_entry.Buffer));
    //DWORD rc = Load_String_Ex(handle, id, free_entry.Buffer, sizeof(free_entry.Buffer), ResourceLang);
    if (!rc) {
        DEBUG_ERROR("Fetch_String() - LoadString failed. Error! %s.\n", Last_System_Error_As_String());
        return _null;
    }

    //std::strncpy(_buffer, _buff, sizeof(_buffer));
    //_buffer[sizeof(_buffer)-1] = '\0';
    free_entry.Buffer[sizeof(free_entry.Buffer)-1] = '\0';

    //DEBUG_INFO("Fetch_String() - Returning '%s'.\n", free_entry.Buffer);

    return free_entry.Buffer;
}


/**
 *  Fetch a resource.
 */
#ifndef NDEBUG
HGLOBAL Vinifera_Fetch_Resource(HMODULE handle, const char *id, const char *type, const char *file, int line)
#else
HGLOBAL Vinifera_Fetch_Resource(HMODULE handle, const char *id, const char *type)
#endif
{
    //DEBUGINFO("Fetch_Resource(enter)\n");
    
    //HRSRC res = FindResourceEx(handle, id, type, ResourceLang);
    HRSRC res = FindResource(handle, id, type);
    if (res == nullptr) {
        DEBUG_ERROR("Fetch_Resource() - FindResource failed. Error! %s.\n", Last_System_Error_As_String());
        return nullptr;
    }

    HGLOBAL res_handle = LoadResource(handle, res);
    if (res_handle == nullptr) {
        DEBUG_ERROR("Fetch_Resource() - LoadResource failed. Error! %s.\n", Last_System_Error_As_String());
        return nullptr;
    }

    /**
     *  Note from MSDN for LockResource().
     *    LockResource does not actually lock memory, it is just used to obtain
     *    a pointer to the memory containing the resource data. The name of the
     *    function comes from versions prior to Windows XP, when it was used to
     *    lock a global memory block allocated by LoadResource.
     */
    void *res_data = LockResource(res_handle);
    if (res_data == nullptr) {
        DEBUG_ERROR("Fetch_Resource() - LockResource failed. Error! %s.\n", Last_System_Error_As_String());
        return nullptr;
    }

    //DEBUGINFO("Fetch_Resource() - Resource loaded sucessfully.\n");

    return res_data;
}


/**
 *  Fetch a image surface from the specified filename if it exists.
 *  
 *  @return      NULL if the image file was not found.
 * 
 *  @warning     The input filename must not contain an extension!
 * 
 *  @author: CCHyper
 */
BSurface *Vinifera_Get_Image_Surface(const char *filename)
{
    BSurface *surface = nullptr;
    CCFileClass file;

    Wstring fname = filename;
    fname.To_Upper();

    Wstring png_fname = fname;
    png_fname += ".PNG";

    file.Set_Name(png_fname.Peek_Buffer());

    surface = Read_PNG_File(&file);
    if (surface) {
        return surface;
    }

    surface = Get_BMP_Image_Surface(fname.Peek_Buffer());
    if (surface) {
        return surface;
    }

    surface = Get_PCX_Image_Surface(fname.Peek_Buffer());
    if (surface) {
        return surface;
    }

    return nullptr;
}
