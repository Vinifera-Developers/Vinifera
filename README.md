<p align="center" style="font-size:32pt;font-style:bold">
  <img src="https://i.imgur.com/whXZZOp.png" width=750>
</p>
<p align="center" style="font-size:12pt;font-style:bold">
  <b>Vinifera</b> is an open-source community collaboration project extending the Tiberian Sun engine. 
</p>
<p align="center">
  <a href="https://github.com/Vinifera-Developers/Vinifera/releases"><img alt="GitHub Workflow Status (develop)" src="https://img.shields.io/github/downloads/Vinifera-Developers/Vinifera/total?style=flat-square"></a>
  <a href="https://github.com/Vinifera-Developers/Vinifera/actions"><img alt="GitHub Workflow Status (develop)" src="https://img.shields.io/github/actions/workflow/status/Vinifera-Developers/Vinifera/nightly.yml?style=flat-square"></a>
  <a href='https://vinifera.readthedocs.io/en/latest/?badge=latest'><img src='https://readthedocs.org/projects/vinifera/badge/?version=latest&style=flat-square' alt='Documentation Status' /></a>
  <a href="https://www.gnu.org/licenses/gpl-3.0.en.html"><img alt="GitHub" src="https://img.shields.io/github/license/Vinifera-Developers/Vinifera?style=flat-square"></a> 
</p>

# Table of Contents
- [Intro](#intro)
- [Community](#community)
- [Downloading Vinifera](#downloading-vinifera)
- [Installing Tiberian Sun](#installing-tiberian-sun)
- [Building Vinifera](#building-vinifera)
- [Developers and Contributors](#developers-and-contributors)
- [Contributing](#contributing)
- [Third Party Libraries](#third-party-libraries)
- [Anti-virus Warning](#anti-virus-warning)
- [Legal](#legal)
- [License](#license)

# Intro
Vinifera is an open source community project which aims to provide new features and bug-fixes fixes for Tiberian Sun.


# Community
You can discuss the development and progress of this project on the **C&C Modding Haven** [Discord server](https://discord.gg/sZeMzz6qVg) at the **`#vinifera-chat`** channel.


# Downloading Vinifera
### Nightly Builds

An automated build of the `develop` branch is uploaded daily.  
These Nightly builds include the latest merged features and updates, but they are **not considered stable** and may contain unfinished or experimental functionality.  
They are intended for active playtesting and for previewing upcoming changes — use them at your own risk.

There are two versions of the Nightly builds available: **Standard** and **Compatibility**.

- The **Standard** build is provided _as-is_, with no modifications for third-party integration.
- The **Compatibility** build includes changes to improve compatibility (where possible) with existing community patches. This version is also used for integration into the **TS Client**.

If you're unsure which version is right for you, feel free to ask a developer on the **Discord** server.

You can find the latest Nightly builds [here](https://nightly.link/Vinifera-Developers/Vinifera/blob/develop/.github/workflows/nightly.yml).  
Alternatively, the latest build from the most recent commit can be found [here](https://nightly.link/Vinifera-Developers/Vinifera/blob/develop/.github/workflows/push.yml).

### Releases

All official release builds are made from the `master` branch.  
Vinifera is currently working toward its first stable release, so no release builds are available yet.  
Once available, they will be uploaded [here](https://github.com/Vinifera-Developers/Vinifera/releases).

### Installing Vinifera

#### With Freeware TS

1. **Download the latest Vinifera nightly build**  
   Make sure to get the one **_not postfixed with `ts_client`_**  
   → https://nightly.link/Vinifera-Developers/Vinifera/workflows/push/develop

2. **Extract all files** from the archive into the **root directory of your Tiberian Sun installation** (where `Game.exe` is located).

3. **Run `LaunchVinifera.exe`** to start the game with Vinifera.

#### With TS Client
Vinifera can be integrated into the latest [TS Client](https://www.moddb.com/mods/tiberian-sun-client).

You can use the `Vinifera Beta` version of the TS Client. To switch to it, go to `Options` → `Updater` tab → `Update channel`, and select `Vinifera Beta`.  
After saving your settings and restarting the client, **force an update** to download the Vinifera build.

Alternatively, if you wish to install it manually, you can follow these steps:
1. **Download the Vinifera nightly build _postfixed with `ts_client`_**  
   → https://nightly.link/Vinifera-Developers/Vinifera/workflows/push/develop

2. **Extract the files** into the **TS Client directory**.

3. **Replace `Game.exe`** with the version patched for Vinifera:
   - Go to the [latest `ts-patches` GitHub release](https://github.com/CnCNet/ts-patches/releases/tag/latest)
   - Download `ts-patches-TSCLIENT-master-#.zip`
   - Extract `GAME.EXE` from the `vinifera` folder inside the archive
   - Replace the existing `Game.exe` in your TS Client directory

4. **Create a file named `VINIFERA.INI`** in the `INI` folder of the TS Client directory, and paste in the following contents:
   ```ini
    [General]
    ProjectName = Tiberian Sun          ; your project name here
    IconFile = Resources\tsicon.ico     ; your project's icon file here.
    CursorFile = Resources\arrow.cur    ; your project's cursor file here.
    SearchPaths = INI,MIX
    ; a blank line at the end is required for the game to parse the INI file correctly
   ``` 

5. **Edit `Resources\ClientDefinitions.ini`**:
   - Change `GameExecutableNames=Game.exe` to `GameExecutableNames=LaunchVinifera.dat`
   - Ensure `ExtraCommandLineParams=` includes `-CD.` **(the dot is required)** before any other flags.

# Installing Tiberian Sun
**NOTE: If you already have Tiberian Sun installed, you can skip this step;**<br><br>
<b><img width="190" src="https://github.com/CnCNet/cncnet-ts-client-package/raw/master/Resources/Default%20Theme/MainMenu/Logo.png" alt="Tiberian Sun"></b><br>
Tiberian Sun was released as freeware by Electronic Arts in 2010 as a part of a promotional build-up to the release of Command & Conquer 4: Tiberian Twilight.

The original links to these downloads on the Electronic Arts servers are no longer active, but an unofficial mirror can be found at the [C&C-Comm Center](https://cnc-comm.com/).<br>
Below are direct links to the released disk images *(English (US) only)*;<br>
**GDI Disk**: [Download](https://cnc-comm.com/tiberian-sun/downloads/the-game/gdi-disc).<br>
**NOD Disk**: [Download](https://cnc-comm.com/tiberian-sun/downloads/the-game/nod-disc).<br>
**Firestorm Disk**: [Download](https://cnc-comm.com/tiberian-sun/downloads/the-game/firestorm-disc).<br>
*<sub>Note: These disk images can be mounted as virtual drives using a variety of free programs.</sub>*

Otherwise, you can also purchase Tiberian Sun as part of the [Command & Conquer The Ultimate Collection](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/) on Steam.

### Updating the game to the latest version
This project currently only supports the latest English (US) version of Tiberian Sun due to technical limitations with patching the original binary.<br>
`GAME.EXE; v2.03[EN]; Monday 5th June, 2000 (21:26:42)`<br>
`MD5: C2C58CBBF83AF0458DC44EF64A3C011F`<br>
You can download the v2.03 patch in English, French, German and Spanish.<br>
**Patch 2.03**: [Download](https://cnc-comm.com/tiberian-sun/downloads/patches/2.03).<br>


# Building Vinifera
**NOTE: This section is only for people who wish to build the source code locally.**

This project uses [CMake](https://cmake.org/) (version 3.17 minimum) for its build system. You can use either CMake via the command line or using the CMake GUI.

The following components are needed to build this project:

- Microsoft Visual Studio 2022 for Windows
- MSVC v143 C++ x86/x64 build tools
- Windows 10 SDK

**PLEASE NOTE:** If you are using the CMake GUI, please make sure to set the output build directory to either outside the source tree or the `./build/` in the source tree root. This directory is ignored for your convenience in the main projects `.gitignore` file.

To run the built version, copy the compiled executables from the build directory into your Tiberian Sun game directory. Then, launch `LaunchVinifera.exe` to start the game with Vinifera enabled.

For more information on using Vinifera, refer to the documentation.  
You’re also welcome to join the **C&C Mod Haven** [Discord server](https://discord.gg/sZeMzz6qVg) and use the **#vinifera-chat** channel to ask questions or get support.



# Developers and Contributors

Vinifera was originally concepted and created by [**CCHyper**](https://github.com/CCHyper) and [**tomsons26**](https://github.com/tomsons26).
- [**ZivDero**](https://github.com/ZivDero) – Project Leader · [Patreon](https://www.patreon.com/c/ZivDero)
- [**Rampastring**](https://github.com/Rampastring) – Project Co-Leader · [Patreon](https://www.patreon.com/c/rampastring) · [Ko-fi](https://ko-fi.com/rampastring)

Special thanks to:

- [**Bittah Commander**](https://github.com/Bittah) – author of the [Dawn of the Tiberian Age](https://www.moddb.com/mods/the-dawn-of-the-tiberium-age) mod, for extensive testing, design discussions, and providing the mod as a major testing platform.  
- [**Crimsonum**](https://github.com/Crimsonum) – author of the [Rubicon](https://www.moddb.com/mods/tiberian-sun-rubicon) mod, for dedicated testing and valuable feedback.  
- [**E1 Elite**](https://github.com/E1Elite) – for consistent testing and thoughtful input on feature behavior and usability.  
- [**OmniBlade**](https://github.com/omniblade) – for early testing and contributions to the codebase; the injector is based on his foundational work.  
- [**MarkJFox**](https://github.com/MarkJFox) – for sidebar graphics and testing support.
- [**Kerbiter**](https://github.com/Metadorius) – for help with setting up and maintaining the project documentation.  

We also appreciate the support and engagement of the [C&C Mod Haven](https://discord.gg/NVuTSsPEqs) and [Dawn of the Tiberium Age](https://discord.gg/6UtC289) communities, as well as all the other testers and community members who provide feedback, report bugs, and help shape the project.

Some of the fixes and features present in Vinifera are based on previous work by the [ts-patches contributors](https://github.com/CnCNet/ts-patches/graphs/contributors).

You can view the full list of contributors [here](https://vinifera.readthedocs.io/en/latest/CREDITS.html).


# Contributing
If you're interested in contributing to this project, basic knowledge of C++ is required. Experience with binary analysis and x86 assembly is highly recommended.

All contributions that align with the project's goals are welcome, as long as they follow our contribution guidelines. We encourage you to join the **Discord** server to discuss these guidelines with the active developers.

Pull requests that fall outside the scope of the project may still receive feedback and suggestions for alternative approaches or directions.


# Third-Party Libraries
Vinifera makes use of third-party libraries to help implement features. Below is a list of libraries used by the project;
 - [LodePNG](https://lodev.org/lodepng/)
 - [Image-Resampler](https://github.com/ramenhut/image-resampler)
 - [XZip](https://www.codeproject.com/Articles/4135/XZip-and-XUnzip-Add-zip-and-or-unzip-to-your-app-w?msg=3792406)


# Anti-virus Warning
Anti-virus software like Windows Defender could mark the binaries built from the DLL configuration in this project as a virus. We would like to assure that this is a false-positive and that these is completely safe to use. If you are still unsure about running these binaries on your system, your are welcome to join our Discord server where one of the developers can explain the process used by this project in detail.


# Legal
This project is an unofficial open-source community collaboration project for preservation, modding and compatibility purposes. EA has not endorsed and does not support this product. Command & Conquer is an Electronic Arts Inc. brand. All Rights Reserved.

No assets, texts, artwork or other media from the original game(s) is included in this repository. We do not condone piracy in any way, shape or form and encourgage users to legally own the original game(s). 

*The video game "Command & Conquer: Tiberian Sun" is copyright © 1999 Westwood Studios. All Rights Reserved.<br>*
*Westwood Studios is a trademark or registered trademark of Electronic Arts in the U.S. and/or other countries. All rights reserved.*


# License
The source code in this repository is licensed under the [GNU General Public License version 3](https://www.gnu.org/licenses/gpl-3.0.html).  
Please refer to the accompanying `LICENSE` file for full details.

This project also incorporates source code released by Electronic Arts Inc. under the same [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html) license.  
These contributions originate from the following official repositories:

- [C&C Remastered Collection](https://github.com/electronicarts/CnC_Remastered_Collection)  
- [C&C Red Alert](https://github.com/electronicarts/CnC_Red_Alert)  
- [C&C Tiberian Dawn](https://github.com/electronicarts/CnC_Tiberian_Dawn)  
- [C&C Renegade](https://github.com/electronicarts/CnC_Renegade)  
- [C&C Generals: Zero Hour](https://github.com/electronicarts/CnC_Generals_Zero_Hour)

They are used in accordance with the original license terms and include additional terms as permitted under [Section 7 of the GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html#section7):

<sub><i>Code within this repository can not be used for commercial or financial software as
dictated by the license released by Electronic Arts Inc. (see ADDITIONAL TERMS - SECTION 7 of LICENSE)</i></sub>

<sub><i>No trademark or publicity rights are granted. This license does NOT give
you any right, title or interest in "Command & Conquer" or any other
Electronic Arts Inc. trademark. You may not distribute any modification of this
program using any Electronic Arts trademark or claim any affiliation or association
with Electronic Arts Inc. or its affiliates or their employees.</i></sub>

<sub><i>Any propagation or conveyance of this program must include this copyright
notice and these terms.</i></sub>

<sub><i>If you convey this program (or any modifications of it) and assume
contractual liability for the program to recipients of it, you agree to
indemnify Electronic Arts for any liability that those contractual
assumptions impose on Electronic Arts.</i></sub>

<sub><i>You may not misrepresent the origins of this program; modified versions of
the program must be marked as such and not identified as the original program.
This disclaimer supplements the one included in the General Public License.</i></sub>

<sub><i>TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, THIS PROGRAM IS
PROVIDED TO YOU "AS IS," WITH ALL FAULTS, WITHOUT WARRANTY OF ANY KIND, AND
YOUR USE IS AT YOUR SOLE RISK. THE ENTIRE RISK OF SATISFACTORY QUALITY AND
PERFORMANCE RESIDES WITH YOU. ELECTRONIC ARTS DISCLAIMS ANY AND ALL EXPRESS,
IMPLIED OR STATUTORY WARRANTIES, INCLUDING IMPLIED WARRANTIES OF
MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A PARTICULAR PURPOSE,
NONINFRINGEMENT OF THIRD PARTY RIGHTS, AND WARRANTIES (IF ANY) ARISING FROM A
COURSE OF DEALING, USAGE, OR TRADE PRACTICE. ELECTRONIC ARTS DOES NOT WARRANT
AGAINST INTERFERENCE WITH YOUR ENJOYMENT OF THE PROGRAM; THAT THE PROGRAM WILL
MEET YOUR REQUIREMENTS; THAT OPERATION OF THE PROGRAM WILL BE UNINTERRUPTED OR
ERROR-FREE, OR THAT THE PROGRAM WILL BE COMPATIBLE WITH THIRD PARTY SOFTWARE
OR THAT ANY ERRORS IN THE PROGRAM WILL BE CORRECTED. NO ORAL OR WRITTEN ADVICE
PROVIDED BY ELECTRONIC ARTS OR ANY AUTHORIZED REPRESENTATIVE SHALL CREATE A
WARRANTY. SOME JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF OR LIMITATIONS ON
IMPLIED WARRANTIES OR THE LIMITATIONS ON THE APPLICABLE STATUTORY RIGHTS OF A
CONSUMER, SO SOME OR ALL OF THE ABOVE EXCLUSIONS AND LIMITATIONS MAY NOT APPLY
TO YOU.</i></sub>
