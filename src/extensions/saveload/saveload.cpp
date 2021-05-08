/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXT_SAVELOAD.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Handles the saving and loading of extended class data.
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
#include "saveload.h"
#include "wwcrc.h"
#include "vinifera_gitinfo.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <unknwn.h> // for IStream


/**
 *  Constant of the current build version number. This number should be
 *  a sum of all the extended class sizes plus the build date.
 */
unsigned ViniferaSaveGameVersion = 0;


static char LoadedCommitHash[40];
static char DataHeaderString[16];
#define VINIFERA_SAVE_HEADER_NAME "VINIFERA_DATA  "


/**
 *  Saves the header marker for validating data on load.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Save_Header(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Save the header string.
     */
    hr = pStm->Write(VINIFERA_SAVE_HEADER_NAME, 16, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Loads the save data header marker.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Load_Header(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Load the header string.
     */
    hr = pStm->Read(DataHeaderString, 16, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Saves the commit hash for checking on load.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Save_Version_Info(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Save the commit hash.
     */
    hr = pStm->Write(Vinifera_Git_Hash(), 40, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Load the commit hash for version checks.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Load_Version_Info(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Load the commit hash.
     */
    hr = pStm->Read(LoadedCommitHash, 40, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Save all Vinifera data to the file stream.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Put_All(IStream *pStm)
{
    /**
     *  Save the Vinifera data marker which can be used to verify
     *  the state of the data to follow on load.
     */
    DEBUG_INFO("Saving Vinifera header marker\n");
    if (!Vinifera_Save_Header(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save the build version information for load checks.
     */
    DEBUG_INFO("Saving Vinifera version information\n");
    if (!Vinifera_Save_Version_Info(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save class extensions here.
     */
    DEBUG_INFO("Saving extended class data...\n");

    /**
     *  Save global data and values here.
     */
    DEBUG_INFO("Saving global data...\n");

    return true;
}


/**
 *  Load all Vinifera data from the file stream.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Load_All(IStream *pStm)
{
    /**
     *  Load the Vinifera data marker which can be used to verify
     *  the state of the data to follow.
     */
    DEBUG_INFO("Loading Vinifera header marker\n");
    if (!Vinifera_Load_Header(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    if (std::strncmp(VINIFERA_SAVE_HEADER_NAME, DataHeaderString, 16) == 0) {
        DEBUG_WARNING("Invalid header in save file!");
        return false;
    }

    /**
     *  Load the build version information.
     */
    DEBUG_INFO("Loading Vinifera version information\n");
    if (!Vinifera_Load_Version_Info(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    if (std::strncmp(Vinifera_Git_Hash(), LoadedCommitHash, 40) == 0) {
        DEBUG_WARNING("Git has mismatch in save file!");
        //return false;
    }

    /**
     *  Load class extensions here.
     */
    DEBUG_INFO("Loading extended class data...\n");

    /**
     *  Load global data and values here.
     */
    DEBUG_INFO("Loading global data...\n");

    return true;
}