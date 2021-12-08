<p align="center" style="font-size:32pt;font-style:bold">
  <img src="https://i.imgur.com/whXZZOp.png" width=750>
</p>
<p align="center" style="font-size:12pt;font-style:bold">
  <b>Vinifera</b> is an open-source community collaboration project extending the Tiberian Sun engine. 
</p>
<p align="center">
  <a href="https://github.com/vinifera-Developers/Vinifera/actions"><img alt="GitHub Workflow Status (develop)" src="https://img.shields.io/github/workflow/status/Vinifera-Developers/Vinifera/nightly/develop?style=flat-square"></a>
  <a href="https://www.gnu.org/licenses/gpl-3.0.en.html"><img alt="GitHub" src="https://img.shields.io/github/license/Vinifera-Developers/Vinifera?style=flat-square"></a>
  <br>
  <b>Latest Nightly Build</b>: <a href="https://nightly.link/Vinifera-Developers/Vinifera/blob/develop/.github/workflows/nightly.yml">Download</a> 
</p>

# Table of Contents
- [Intro](#intro)
- [Community](#community)
- [Contributing](#contributing)
- [Building The Project](#building-the-project)
- [Installing The Game](#installing-the-game)
- [Running The Game](#running-the-game)
- [Third Party Libraries](#third-party-libraries)
- [Anti-virus Warning](#anti-virus-warning)
- [Legal](#legal)
- [License](#license)

# Intro
Vinifera is an open source community project which aims to provide new features and bug-fixes fixes for Tiberian Sun.

# Community
You can discuss the development and progress of this project on the **C&C Modding Haven** [Discord server](<https://discord.gg/sZeMzz6qVg>) at the **#vinifera-chat** channel.

# Contributing
If you are interested in contributing to this project, you will need some knowledge of C++ as a minimum requirement, but it is recommended you have experience with binary analysis and x86 assembly. All contributions towards the projects goals are welcome, provided they follow the contribution guidelines. Please join the Discord server to discuss these guidelines with the active developers. Any pull requests that do not fit within the project guidelines will be recommended to be developed as a downstream project.

# Building The Project
This project uses [CMake](<https://cmake.org/>) (version 3.17 minimum) for its build system. You can use either CMake via the command line or using the CMake GUI.

The following components are needed to build this project:

- Microsoft Visual Studio 2019 for Windows
- MSVC v141 C++ x86/x64 build tools
- Windows 10 SDK

**PLEASE NOTE:** If you are using the CMake GUI, please make sure to set the output build directory to either outside the source tree or the `./build/` in the source tree root. This directory is ignored for your convenience in the main projects `.gitignore` file.

# Installing The Game
**Instructions for downloading and installing the games if you do not already own them:**<br><br>
<b><img width="190" src="https://thecodeconquerors.github.io/img/tibsun_logo.png" alt="Tiberian Sun"></b><br>
Tiberian Sun was released as freeware by Electronic Arts in 2010 as a part of a promotional build-up to the release of Command & Conquer 4: Tiberian Twilight.

The original links to these downloads on the Electronic Arts servers are no longer active, but an unofficial mirror can be found at the [C&C-Comm Center](<https://cnc-comm.com/>).<br>
Below are direct links to the released disk images *(English (US) only)*;<br>
**GDI Disk**: [Download](<https://cnc-comm.com/tiberian-sun/downloads/the-game/gdi-disc>).<br>
**NOD Disk**: [Download](<https://cnc-comm.com/tiberian-sun/downloads/the-game/nod-disc>).<br>
**Firestorm Disk**: [Download](<https://cnc-comm.com/tiberian-sun/downloads/the-game/firestorm-disc>).<br>
*<sub>Note: These disk images can be mounted as virtual drives using a variety of free programs.</sub>*

Otherwise, you can also purchase Tiberian Sun as part of the [Command & Conquer Ultimate Collection](<https://www.origin.com/gbr/en-us/store/command-and-conquer/command-and-conquer-the-ultimate-collection>) on EA's Origin service.

**Updating the game to the latest version:**
This project currently only supports the latest English (US) version of Tiberian Sun due to technical limitations with patching the original binary.<br>
`GAME.EXE; v2.03[EN]; Monday 5th June, 2000 (21:26:42)`<br>
`MD5: C2C58CBBF83AF0458DC44EF64A3C011F`<br>
You can download the v2.03 patch in English, French, German and Spanish.<br>
**Patch 2.03**: [Download](<https://cnc-comm.com/tiberian-sun/downloads/patches/2.03>).<br>

# Running The Game
Copy the built executables from the build directory to the Tiberian Sun directory. Run `LaunchVinifera.exe` to start the game with the Vinifera project applied. For more information on how to use Vinifera, please read the documention or you can join the **C&C Modding Haven** [Discord server](<https://discord.gg/sZeMzz6qVg>) and use the **#vinifera-chat** channel.

# Third-Party Libraries
Vinifera makes use of third-party libraries to help implement features. Below is a list of libraries used by the project;
 - [LodePNG](https://lodev.org/lodepng/)
 - [Image-Resampler](https://github.com/ramenhut/image-resampler)
 - [XZip](https://www.codeproject.com/Articles/4135/XZip-and-XUnzip-Add-zip-and-or-unzip-to-your-app-w?msg=3792406)

# Anti-virus Warning
Anti-virus software like Windows Defender could mark the binaries built from the DLL configuration in this project as a virus. We would like to assure that this is a false-positive and that these is completely safe to use. If you are still unsure about running these binaries on your system, your are welcome to join our Discord server where one of the developers can explain the process used by this project in detail.

# Legal
This project is an unofficial open-source community collaboration project for preservation, modding and compatibility purposes. This project has no direct affiliation with Electronic Arts Inc. and/or Command & Conquer. Command & Conquer is an Electronic Arts Inc. brand. All Rights Reserved.

No assets, texts, artwork or other media from the original game(s) is included in this repository. We do not condone piracy in any way, shape or form and encourgage users to legally own the original game(s). 

*The video game "Command & Conquer: Tiberian Sun" is copyright © 1999 Westwood Studios. All Rights Reserved.<br>*
*Westwood Studios is a trademark or registered trademark of Electronic Arts in the U.S. and/or other countries. All rights reserved.*

# License
The source code provided in this repository is licenced under the [GNU General Public License version 3](<https://www.gnu.org/licenses/gpl-3.0.html>). Please see the accompanying LICENSE file.

Some source code released by Electronic Arts Inc. for the [C&C Remastered Collection](<https://github.com/electronicarts/CnC_Remastered_Collection>)
is used in this project and is licenced under the [GNU General Public License version 3](<https://www.gnu.org/licenses/gpl-3.0.html>).
These source files are marked in the headers for easy identification and are applied
with the following additional terms below, copied from the LICENSE file included in
the C&C Remastered Collection source code repository.

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
