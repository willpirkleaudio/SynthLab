
# ---------------------------------------------------------------------------------
#
# --- readme.txt
# --- Created by RackAFX(TM)
# --- www.willpirkle.com
#
#
# --- CONTENTS ---
#	- Exporting RackAFX Projects for CMake
#	- CMAKE Installation and Use
#	- Building your ported project
# 	- Modifying your project and CMake files
#
# ---------------------------------------------------------------------------------

# ---------------------------------------------------------------------------------
# --- Exporting RackAFX Projects for CMake
# ---------------------------------------------------------------------------------
This is a raw text file version of documentation available at www.willpirkle.com
Please refer to the page http://www.willpirkle.com/support/exporting-rackafx-projects-with-cmake/
for more information and video tutorials.

This document explains how to export AAX, AU and VST projects with RackAFX v6.9 and above. It includes the details of setting up your target API/SDK, folder naming conventions, and the locations of your finished exported projects. It details the differences between exporting individual projects and universal projects.

Contents:

	Folder Naming Convention

	Exported Project Location Overview

	Individual Exported Project Locations
		- AU
		- AAX
		- VST

		Universal Exported Project Location


# ---------------------------------------------------------------------------------
# --- Folder Naming Convention
# ---------------------------------------------------------------------------------
You will need to navigate either Terminal or the Windows Command Prompt to various locations within your SDKs to run CMake. To keep the folder names simple and easy to use with Terminal/Command Prompt, I use a naming convention with:

- no whitespaces (blanks)
- no numbers
- no special characters


# ---------------------------------------------------------------------------------
# --- Exported Project Location Overview
# ---------------------------------------------------------------------------------
For both the individual and universal API projects, there is only a single outer folder that contains all the RackAFX exported projects along with a single copy of the VSTGUI4 sub-folder. I use the folder name myprojectsù for all of my exports, so I will also use it here as an example. Within your outer myprojects folder, you need to include the VSTGUI4 sub-folder. A typical myprojects folder that contains the VSTGUI4 sub-folder along with three exported projects named Compressor, Reverb, and ParametricEQ would have the following folder hierarchy:

myprojects
   |
   vstgui4
   Compressor
   Reverb
   ParametricEQ

Expanding the ParametricEQ folder reveals the first layer of project files

myprojects
   |
   vstgui4
   Compressor
   Reverb
   ParametricEQ
      |
      cmake
      mac_build
      project_source
      win_build
      CMakeLists.txt

Inside of this folder is the outermost CMakeLists.txt script file. If you ever need to change the location of your projects relative to the SDK, this is the only file you need to edit (see the accompanying Using CMake document). You can take some time and poke around in the various subfolders, but you will generally only need to work inside of the mac_build or win_build folders when creating MacOS or Windows plugins respectively. You can find your original RackAFX project code inside of the folder:

	../project_source -> source -> rafx_source


# ---------------------------------------------------------------------------------
# --- Individual Exported Project Locations
# ---------------------------------------------------------------------------------
There are three supported APIs for RackAFX: AAX, AU and VST and each SDK consists of a series of subfolders with AU being the simplest and VST being the most complex in terms of folder hierarchies.


# ---------------------------------------------------------------------------------
# --- AU
# ---------------------------------------------------------------------------------
The AU SDK is contained within two subfolders named AUPublic and PublicUtitlity. You can get these two folders along with the latest sample code here:

https://developer.apple.com/library/content/samplecode/sc2195/Introduction/Intro.html

or

http://www.willpirkle.com/Downloads/AU_SDK.zip

For the AU SDK, you first need to create an outer container for the two SDK folders. You may name this folder whatever you wish - I use the name AU_SDKù for my folder so we will use it here. For the RackAFX ported projects.

Place your myprojects folder in parallel with the two SDK files and inside of your outer AU_SDK folder like this:

AU_SDK
   |
   AUPublic
   myprojects <--------
   PublicUtility

At this point, you are ready to start exporting AU projects from RackAFX. And, if you decide to use the Universal API paradigm instead, you can re-use the same AU_SDK outer folder without issues.


# ---------------------------------------------------------------------------------
# --- AAX
# ---------------------------------------------------------------------------------
The AAX SDK is contained within a series of subfolders of the current SDK branch. You must establish an account with Avid to gain access to the SDK and Pro Tools Developer Version and you will need an iLok2 device for storing your Pro Tools activation key.

The AAX SDK architects use numbers when naming the various versions such as AAX_SDK_2p3p0 which reads AAX SDK version 2.3.0ù I prefer to rename the current SDK to AAX_SDK in keeping with folder naming convention discussed in the Intro to CMake document. as well as shortening the name a bit. As with the AU projects, your myprojects subfolder will be located in parallel with the other SDK sub-folders at the outermost level. My folder hierarchy looks like this:

AAX_SDK
   |
   documentation
   ExamplePlugins
   Extensions
   Interfaces
   Libs
   myprojects <--------
   TI
   Utilities

At this point, you are ready to start exporting AAX projects from RackAFX. And, if you decide to use the Universal API paradigm instead, you can re-use the same AAX_SDK outer folder without issues.


# ---------------------------------------------------------------------------------
# --- VST
# ---------------------------------------------------------------------------------
The VST SDK is contained within a series of subfolders of the current SDK branch. The architects name the outermost folder VST_SDK and it contains two inner folders, VST2_SDK and VST3_SDK. In addition, there are two script files, one for Mac and one for Windows. These scripts install the VST2 SDK folders within the VST3 SDK so that you may use the VST2 wrapper that is included with the VST3 SDK.

Important: all RackAFX exported VST projects are VST2 compatible and require the VST2 API files to be setup properly. This is fairly simple:

-> Windows: Navigate to the VST3_SDK folder that contains copy_vst2_to_vst3_sdk.bat, then, double-click on the copy_vst2_to_vst3_sdk.bat file to run the copy mechanism.

-> MacOS: Open a Terminal window and navigate to the VST3_SDK folder that contains the copy_vst2_to_vst3_sdk.sh file. Drag and drop the copy_vst2_to_vst3_sdk.sh file into Terminal. This will run the copy mechanism.

With the VST2 SDK installed, you can now create the myprojects folder for your exported projects. It needs to be located just inside the VST3_SDK subfolder, in parallel with the other SDK files, exactly as with AU and AAX. Your folder hierarchy should look like this:

VST_SDK
   |
   VST3_SDK
      |
      base
      bin
      cmake
      doc
      myprojects <--------
      pluginterfaces
      public.sdk
      vstgui4
      CMakeLists.txt
      index.html

You can see the SDK's CMakeLists.txt file - this is entirely independent of your exported projects and when you run CMake for the exported project, it will not interfere in any way with the SDK's files or CMake scripts. You should note that index.html is the Doxygen documentation link.

At this point, you are ready to start exporting VST projects from RackAFX. And, if you decide to use the Universal API paradigm instead, you can re-use the same VST_SDK outer folder without issues.


# ---------------------------------------------------------------------------------
# --- Universal Exported Project Location
# ---------------------------------------------------------------------------------
Creating a folder hierarchy for the Universal setup is simple: create an outer folder to hold the SDK sub-folders in parallel with each other. I use the folder name ALL_SDK for my Universal projects. Your myprojects sub-folder is located in parallel with all three SDK folders and is not inside any one of them. My universal project folders look like this:

ALL_SDK
  |
   AU_SDK
   AAX_SDK
   myprojects <--------
   VST_SDK

At this point, you are ready to start exporting UNIFIED projects from RackAFX. You can also use the individual projects contained with each SDK without any interference from the other projects. It is also simple to edit the outermost CMakeLists.txt file to convert individual exported projects to universal ones and back again by changing a few lines of script - see the accompanying Using CMake document.


# ---------------------------------------------------------------------------------
# --- CMake Installation and Use
# ---------------------------------------------------------------------------------
This document explains how to install and run CMake on your exported projects that RackAFX has generated for you. It also includes specifics to each API including the location of your final plugin.

Contents:

	Installing CMake
		- Windows
		- MacOS

	Running CMake
		- Windows
		- MacOS

	Building Your Exported Project
		- Compile Targets
		- Notes on API Specifics
	 	- VST

	Changing from Individual to Universal Builds

	Modifying Your Project: CMake Ramifications


# ---------------------------------------------------------------------------------
# --- Installing CMake
# ---------------------------------------------------------------------------------
In order to generate the RackAFX ported project compiler files, you need to have CMake installed on your OS. For ease, you should add the CMake executable to your default path.

Download the latest version of CMake at http://cmake.org/


-> Windows
During installation, you have the option of adding CMake to your default path list.

Choose YES for this option and you are done.

-> MacOS
Open Terminal and type:

	echo $PATH

which produces a colon : separated list of folders you can run directly with terminal;

/usr/local/bin should be in this list


Next, add a SymLink to the CMake executable and place it in the /usr/local/bin folder in Terminal by running:

	sudo ln -s /Applications/cmake.app/Contents/bin/cmake /usr/local/bin

After that, you are done.


# ---------------------------------------------------------------------------------
# --- Running CMake
# ---------------------------------------------------------------------------------
In each case (Windows vs MacOS), you first navigate to the <OS>_build folder inside of the project folder. For Windows it is named win_build and for MacOS it is named mac_build. You will notice that these folders are empty when you navigate to them. This is correct for a freshly created project.

Running CMake will populate the build folder with the newly created XCode or Visual Studio project. After that you can open the compiler project and  rebuild your solution as usual. To run CMake:

start your engines:

MaxOS: open a Terminal shell
WinOS: open the Command Prompt with admin privileges (aka "Command Prompt (Admin)")

Navigate and find your myprojects folder inside a particular SDK (individual) or in the ALL_SDK folder (universal).

Navigate into your ported project folder and then again into either the win_build or mac_build sub-folders according to your OS. You must run CMake from folder *inside* of the one that contains the outer CMakeLists.txt file. This is a quirk of CMake.

Run CMake as follows:

-> Windows
VS 2015 (for VST3 and AAX) 64-bit:

	cmake -G"Visual Studio 14 2015 Win64" ../

VS 2015 (for VST3 and AAX) 32-bit:

	cmake -G"Visual Studio 14 2015" ../

VS2017 (for VST3 only - currently not supported by AAX as of SDK 2p3p0, but may be
            supported in the future) 64-bit:

	cmake -G"Visual Studio 15 2017 Win64" ../

VS2017 32-bit:

	cmake -G"Visual Studio 15 2017" ../

You can find the CMake generator (G) codes for other Visual Studio compilers here:

https://cmake.org/cmake/help/v3.10/manual/cmake-generators.7.html
NOTE: the VSTGUI SDK requires C++11 so make sure your compiler is 100% C++11 compliant. Many older versions of Visual Studio are NOT 100% compliant so beware.

-> MacOS

	cmake -GXcode ../

There are no version-specific or bit-depth specific Xcode generators.


# ---------------------------------------------------------------------------------
# --- Building Your Exported Project
# ---------------------------------------------------------------------------------
You can find the resulting VS or Xcode project located right inside your win_build or mac_build subfolders respectively. Double-click on the .xcodeproj or .sln file to start your compiler.

Your ported projects will contain multiple targets (XCode) or projects (Visual Studio). These include:

ZERO_CHECK: this will rerun cmake. You can/should execute this after changing something in your CMake files due to adding new files to your project (see below)

ALL_BUILD: is simply a target which builds all projects in the active solution; you will usually just build this target


# ---------------------------------------------------------------------------------
# --- Compile Targets
# ---------------------------------------------------------------------------------
Xcode calls each project a "target" ùwhile Visual Studio uses  "Project"  each of these compiles to produce either your final plugin or a library (or multiple libraries) needed to compile it.

For individual projects, you will only have target(s) associated with one particular API. For universal projects, there will be targets for everything required to build for all APIs.

VST3:
The VST3 compiler project will include targets for two executables: validator and editorhost. These are automatically added by a CMakeLists.txt file that is internal to the VST3 SDK and that we do not want to modify. The validator project is required in the solution. The editorhost is optional and you can safely ignore it. See the VST3 section below for more information about the validator.


# ---------------------------------------------------------------------------------
# --- Notes on API Specifics
# ---------------------------------------------------------------------------------
There are some specifics regarding each API that you need to be aware of. These are properties of the APIs/SDKs themselves, and have nothing to do with the RackAFX projects directly.


# ---------------------------------------------------------------------------------
# --- AAX
# ---------------------------------------------------------------------------------
You must compile the AAX Base Library first. You only need to do this once per SDK release (or if you update your compiler to a newer version).

You can find the XCode and Visual Studio generated library projects in these folders:

	AAX_SDK/Libs/AAXLibrary/WinBuild

	or

	AAX_SDK/Libs/AAXLibrary/MacBuild

Open the project file and compile your project using the same compiler that you will use to compile your project. Anytime you change compilers, you must re-compile the library with your new compiler. The result of the library compilation will be:

-> MacOS
AAX_SDK/Libs/Debug/libAAXLibrary.a
AAX_SDK/Libs/Release/libAAXLibrary.a

-> Windows x64
AAX_SDK/Libs/Debug/AAXLibrary_x64_D.lib
AAX_SDK/Libs/Release/AAXLibrary_x64.lib

Open the compiler project for your newly generated project. You can now re-build the solution to finish the ported project.

Your finished plugin will be located in the following sub-folder:

	AAX_SDK/myprojects/<project name>/mac_build/AAX/<config>

	or

	AAX_SDK/myprojects/<project name>/win_build/AAX/<config>

where <config> is either Debug or Release, depending on how you built the project.

Copy the .aaxplugin into your AAX plugin folder. You must to this manually because in both operating systems, the target folder is admin-protected which prohibits the copy mechanism from being done programmatically during compilation.

MacOS

	/Library/Application Support/Avid/Audio/Plug-Ins


Windows

	C:\Program Files\Common Files\Avid\Audio\Plug-Ins


Now you can run your Pro Tools Developer's Build to test your plugin and generate the Presets for it.


# ---------------------------------------------------------------------------------
# --- AU
# ---------------------------------------------------------------------------------
Open the compiler project for your newly generated project. You can now re-build the solution to finish the ported project.

Your finished plugin will be located in the following sub-folder:

	AU_SDK/myprojects/<project name>/mac_build/AU/<config>

where <config> is either Debug or Release, depending on how you built the project.

For AU, your plugin is AUTOMATICALLY copied to the proper location in your MacOS  device:

	~/Library/Audio/Plug-Ins/Components/<plugin>.component)

run auval from Terminal to validate your plugin prior to use. The RackAFX  ported projects will pass validation by default, however your customizations could create issues, especially if you are still new to plugin development. You received the auval terminal code when you ported the project in RackAFX. If your plugin fails validation, it will be black-listed in Apple Logic and perhaps other clients as well so you should correct any errors before testing.


# ---------------------------------------------------------------------------------
# --- VST
# ---------------------------------------------------------------------------------
Open the compiler project for your newly generated project. You can now re-build the solution to finish the ported project.

Your project will be validated automatically with the validator project that is built-into your compiler project. If your plugin fails validation, the compiler will NOT create the final VST plugin. This is generally good as a plugin that fails validation will likely crash the VST client which may result in it being black-listed from future loads! Fix any validation problems (see the compiler output window).

  Your finished plugins will be located in the following sub-folder:

	VST_SDK/VST3_SDK/myprojects/<project name>/mac_build/VST3/<config>

	or

	VST_SDK/VST3_SDK/myprojects/<project name>/win_build/VST3/<config>

where <config> is either Debug or Release, depending on how you built the project.


-> MacOS
Your VST2 and VST3 plugins are AUTOMATICALLY copied to the proper locations in your MacOS device:

VST2:
	~/Library/Audio/Plug-Ins/VST/<plugin>.vst)

VST3:
	~/Library/Audio/Plug-Ins/VST3/<plugin>.vst3)


-> Windows
You must manually copy the VST plugins to the proper locations - these will vary
depending on your setup.

VST3:
There are generally accepted "default" locations for 32 and 64 bit builds.  These are located in admin-protected folders and can't be copied programmatically:

32-bit
	C:\Program Files (x86)\Common Files\VST3

64-bit
	C:\Program Files\Common Files\VST3

You should copy your .vst3 plugin to the appropriate folder as well as any other
DAW-specific VST3 folder that exist on your system.

VST2:
There is no default location for VST2 plugins.

You should copy your .vst3 plugin into your VST2 folder(s) for your DAW(s). THEN, rename the suffix from ".vst3" to ".dll" to convert it from a VST3 to a VST2 plugin.


# ---------------------------------------------------------------------------------
# --- Changing from Individual to Universal Exports
# ---------------------------------------------------------------------------------
When you export a project from RackAFX, you have the option of creating individual exports, or the universal export pattern. But, you actually receive the same files for each option - only the outer most CMakeLists.txt file is different. If you open this file and check out the very top most portion, you will find the BOOLEAN variables that turn on and off the various API projects as well as the default locations for the SDKs when you are using the Universal API Export.

The individual projects may be enabled/disabled with the code below. If you enable the UNIVERSAL_SDK_BUILD, then all API projects will be generated regardless of the settings below it.

# --- Universal Build Flag
set(UNIVERSAL_SDK_BUILD TRUE)

To enable/disable individual projects when NOT using the universal build, alter the code below it:

# --- Individual project builds
set(AAX_SDK_BUILD TRUE)
set(AU_SDK_BUILD FALSE)
set(VST_SDK_BUILD FALSE)

Here, I've set CMake to generate only the AAX project.


# ---------------------------------------------------------------------------------
# --- Modifying your Project: CMake Ramifications
# ---------------------------------------------------------------------------------
Ordinarily, you will export your projects from RackAFX once they are finally completed, or you will generate a series of ports up until the completion point. At this point, you will usually just build the plugin, test it and release it.

However you may decide to alter your existing exported project after the fact, and in this case, you will need to alter your CMake files accordingly if you plan on updating the project when a future version of a given SDK is released.

-> Adding/Removing/Relocating Files
If you add, remove, or relocate project files you will need to update the CMakeLists.txt file for each API that you are supporting or compiling against. Navigate into the appropriate folder within your project:

../myprojects/<project name>/project_source/cmake

Inside of this folder are a set of sub-folders, one for each API and each containing the CMakeLists.txt file for that specific API.

../myprojects/<project name>/project_source/cmake/aax_cmake

../myprojects/<project name>/project_source/cmake/au_cmake

../myprojects/<project name>/project_source/cmake/vst_cmake

Your project source files are listed at the very top and are identical for each API's CMakeLists.txt file. NOTE: yes, there are ways to combine the portions of these files but in general I've found it safer to simply keep each file independent. Add, remove or relocate by altering the lines of text after the set(rafx_<api>_sources) chunk. ${RAFX_SOURCE_ROOT} is your rafx_source folder that contains the files you originally wrote in RackAFX:

set(rafx_aax_sources
	${RAFX_SOURCE_ROOT}/DelayLine.h
	${RAFX_SOURCE_ROOT}/DynamicsProcessor.h
	${RAFX_SOURCE_ROOT}/GUIViewAttributes.h
	etc. . .

Save the files and re-run CMake. You may need to delete the existing files from your mac_build or win_build folders first if CMake complains that something has changed in the cache file. After re-running CMake, your project will reflect the changes properly.


# ---------------------------------------------------------------------------------
# --- Modifying your Project: Deep Changes
# ---------------------------------------------------------------------------------
If you decide to add 3rd party components, libraries, or other stuff to your project which requires that you alter the project settings (in Xcode these are in the target's Build Settings while in Visual Studio they are the Project's Properties), then you have a bit of work to do to alter the CMake files to use the new compiler settings. This may be simple or it may be very complicated depending on the changes you have made. There is no way to tell in advance what these changes may be, so the only help I can give you is to use the information at cmake.org as well as other sites like stackoverflow.com which has scores of help topics related to CMake compiler settings.
