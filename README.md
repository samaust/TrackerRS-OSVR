# TrackerRS-OSVR
Tracking plugin for OSVR using RealSense R200 Scene Perception

## Notes

The tracking has some jitter and filtering may be needed to improve the performance.

## Instructions for 64 bit version

Copy TrackerRS-OSVR\build_x64\bin\osvr-plugins-0\Release\com_samaust_trackerrs_osvr.dll to C:\Program Files\OSVR\Runtime\bin\osvr-plugins-0 folder.
Remove other plugins you don't need from C:\Program Files\OSVR\Runtime\bin\osvr-plugins-0.
Edit C:\Program Files\OSVR\Runtime\bin\osvr_server_config.json as necessary.
To autodetect all plugins, rename osvr_server_config.autodetectall.json to osvr_server_config.json
Run osvr_server.exe.

## Shorcuts

Recenter : CTRL + F12

The plugin will only recenter if the scene quality is good enough when the shortcut is pressed. If the plugin does not recenter when pressing the shortcut, try to reorient the camera before trying again.

## How to compile

The provided solution is for Visual Studio 2013 64 bit. You can generate new solutions for other compilers using CMake : 

* Set the source folder to src
* Set the build folder
* Add entry CMAKE_PREFIX_PATH and set it to the OSVR-Core Binary Snapshot folder
* Configure and generate

To compile the provided solution:

* Download and extract an OSVR-Core Binary Snapshot.
* Download and install the RealSense SDK.
* Download and extract boost to a folder.
* Modify the additional include directory project property to point to the OSVR-Core Binary Snapshot folder.
* Modify the additional include directory project property of the build folder.
* Modify the additional include directory project property to point to the boost folder.
* Compile
