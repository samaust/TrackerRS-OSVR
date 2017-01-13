// Copyright 2016 Samuel Austin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "sp_controller.h"
#include "util_render.h"
#include "utilities\pxcrotation.h"

class TrackerRS
{
	public:
		TrackerRS(OSVR_PluginRegContext ctx, ScenePerceptionController *scenePerceptionController);
		~TrackerRS();
		
		OSVR_ReturnCode update();

	private:
		osvr::pluginkit::DeviceToken m_dev;
		OSVR_TrackerDeviceInterface m_tracker;

		bool m_error_flag;
		bool m_bIsScenePerceptionPaused;

		float m_fSceneQuality;
		float m_minAcceptableSceneQuality;

		ScenePerceptionController *m_scenePerceptionController;
		std::unique_ptr<UtilRender> m_renderedVolume;
		PXCScenePerception::TrackingAccuracy m_trackingAccuracy;
		PXCCapture::Sample* m_pSample;
		PXCSenseManager *m_pSenseManager;
		PXCScenePerception *m_pScenePerception;
		PXCSession* m_session;
		PXCRotation* m_rotation;
		pxcF32 m_rotationMatrix[3][3];

		// m_pose: Array of 12 pxcF32 to store camera pose in
		// row - major order.Camera pose is specified in a 3 by 4 matrix
		// [R | T] = [Rotation Matrix | Translation Vector]
		//	  where R = [r11 r12 r13]
		//		  [r21 r22 r23]
		//		  [r31 r32 r33]
		//	  T = [tx  ty  tz]
		//		  Pose Array Layout = [r11 r12 r13 tx r21 r22 r23 ty r31 r32 r33 tz]
		//		  Translation vector is in meters.
		float m_pose[12];
};