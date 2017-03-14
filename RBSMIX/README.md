# Lua Redundancy bus mixer script

Download the latest version: https://raw.githubusercontent.com/RealTadango/FrSky/master/RBSMIX/RBSMIX.lua

Installation:
Copy RBSMIX.lua to SCRIPTS\MIXER on the SD card and select the script in the Custom Scripts menu. Discover new sensors to enable the new virtual sensors. 

This scripts creates 8 new sensors from the combined Redundancy bus telemetry values that can be logged and be used in logical switches. The folowing virtual sensors are added:
 - R1FS: This sensor reads 1 when receiver 1 is in a failsave mode. Otherwise it is 0
 - R2FS: This sensor reads 1 when receiver 2 is in a failsave mode. Otherwise it is 0
 - R1FL: This sensor reads 1 when receiver 1 lost a frame. Otherwise it is 0
 - R2FL: This sensor reads 1 when receiver 2 lost a frame. Otherwise it is 0
 - R1LC: This sensor counts how many times R1FL switched from 0 to 1
 - R2LC: This sensor counts how many times R2FL switched from 0 to 1
 - R1HC: This sensor counts how many times R1FS switched from 0 to 1
 - R2HC: This sensor counts how many times R2FS switched from 0 to 1
 
Warning: Not every frameloss can be detected correctly because it is only detected (and counted) when a telemetry frame with the frameloss is received. It is not documentated how this is done in the Redundancy bus. These values can be used to compare setups and receivers as relative values only.
 