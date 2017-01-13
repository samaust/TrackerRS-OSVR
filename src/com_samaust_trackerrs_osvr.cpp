// Copyright 2014 Sensics, Inc.
// Copyright 2016 Samuel Austin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Windows.h>

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

#include "TrackerRS.h"

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

// Intel includes
#include "sp_controller.h"

// Globals
WCHAR  arg0[] = L"programName";
WCHAR* argvW[] = { &arg0[0], NULL };
int   argc = (int)(sizeof(argvW) / sizeof(argvW[0])) - 1;

int COLOR_CAPTURE_WIDTH = 320;
int COLOR_CAPTURE_HEIGHT = 240;

int DEPTH_CAPTURE_WIDTH = 320;
int DEPTH_CAPTURE_HEIGHT = 240;
int DEPTH_FRAMERATE = 60;

ScenePerceptionController scenePerceptionController(COLOR_CAPTURE_WIDTH, COLOR_CAPTURE_HEIGHT,
	DEPTH_CAPTURE_WIDTH, DEPTH_CAPTURE_HEIGHT, DEPTH_FRAMERATE);

// Anonymous namespace to avoid symbol collision
namespace {

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
		std::cout << "[TrackerRS-OSVR] Detecting Intel R200 camera..." << std::endl;
        if (!m_found) {
			// Initialize
			if (false == scenePerceptionController.Init(argc, argvW))
				return OSVR_RETURN_FAILURE;

			scenePerceptionController.PauseScenePerception(true);

			std::cout << "[TrackerRS-OSVR] Configuring Intel R200 camera..." << std::endl;

			if (!scenePerceptionController.InitPipeline())
				return OSVR_RETURN_FAILURE;

			std::cout << "[TrackerRS-OSVR] Intel R200 camera configured successfully" << std::endl;

            m_found = true;

            /// Create our device object
            osvr::pluginkit::registerObjectForDeletion(
				ctx, new TrackerRS(ctx, &scenePerceptionController));
        }
        return OSVR_RETURN_SUCCESS;
    }

  private:
    /// @brief Have we found our device yet? (this limits the plugin to one
    /// instance)
    bool m_found;

	
	
};
} // namespace

OSVR_PLUGIN(com_samaust_trackerv2_osvr) {
    osvr::pluginkit::PluginContext context(ctx);

	/// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
