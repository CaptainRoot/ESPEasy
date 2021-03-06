#ifndef GLOBALS_PLUGIN_H
#define GLOBALS_PLUGIN_H

#include <map>
#include <vector>
#include "../DataStructs/ESPEasyLimits.h"


/********************************************************************************************\
   Structures to address the plugins and device configurations.

   A build of ESPeasy may not have all plugins included.
   So there has to be some administration to keep track of what plugin is present 
   and how to address this plugin.
   The data structures containing the available plugins are addressed via a DeviceIndex.

   We have the following one-to-one relations:
   - Plugin_id_to_DeviceIndex  - Map from Plugin ID to Device Index.
   - DeviceIndex_to_Plugin_id  - Vector from DeviceIndex to Plugin ID.
   - Plugin_ptr                - Array of function pointers to call plugins.
   - Device                    - Vector of DeviceStruct containing plugin specific information.
 \*********************************************************************************************/

typedef byte deviceIndex_t;
typedef byte taskIndex_t;
typedef uint8_t pluginID_t;

extern deviceIndex_t INVALID_DEVICE_INDEX;
extern taskIndex_t   INVALID_TASK_INDEX;
extern pluginID_t    INVALID_PLUGIN_ID;

extern int deviceCount;

// Array of function pointers to call plugins.
extern boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);

// Map to match a plugin ID to a "DeviceIndex"
extern std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;

// Vector to match a "DeviceIndex" to a plugin ID.
extern std::vector<pluginID_t> DeviceIndex_to_Plugin_id;

// Vector containing "DeviceIndex" alfabetically sorted.
extern std::vector<deviceIndex_t> DeviceIndex_sorted;


bool validDeviceIndex(deviceIndex_t index);
bool validTaskIndex(taskIndex_t index);
bool validPluginID(pluginID_t pluginID);

// Check if plugin is included in build. 
// N.B. Invalid plugin is also not considered supported.
// This is essentially (validPluginID && validDeviceIndex)
bool supportedPluginID(pluginID_t pluginID);

deviceIndex_t getDeviceIndex_from_TaskIndex(taskIndex_t taskIndex);



/********************************************************************************************\
   Find Device Index given a plugin ID
 \*********************************************************************************************/
deviceIndex_t getDeviceIndex(pluginID_t Number);

String getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex);
String getPluginNameFromPluginID(pluginID_t pluginID);

void sortDeviceIndexArray();



#endif // GLOBALS_PLUGIN_H