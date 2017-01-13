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

#include <Windows.h>

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "com_samaust_trackerrs_osvr_json.h"

// Standard includes
#include <iostream>
#include <cmath>
#include <conio.h>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <thread>
#include <memory>

// Intel includes
#include "sp_controller.h"
#include "util_render.h"

#include "TrackerRS.h"

std::unique_ptr<UtilRender> UtilRenderFactory(pxcCHAR *title)
{
	// Implicit move operation into the variable that stores the result.
	return std::make_unique<UtilRender>(title);
}

TrackerRS::TrackerRS(OSVR_PluginRegContext ctx, ScenePerceptionController *scenePerceptionController)
{
	std::cout << "[TrackerRS-OSVR] Initializing Intel R200 camera tracker..." << std::endl;

	m_error_flag = false;
	m_bIsScenePerceptionPaused = true;
	
	m_fSceneQuality = 0.0f;
	m_minAcceptableSceneQuality = 0.25f;

	m_scenePerceptionController = scenePerceptionController;

	m_renderedVolume = UtilRenderFactory(L"Volume");

	m_trackingAccuracy = PXCScenePerception::TrackingAccuracy::HIGH;
	
	m_pSample = NULL;
	m_pSenseManager = m_scenePerceptionController->QuerySenseManager();
	m_pScenePerception = m_scenePerceptionController->QueryScenePerception();

	m_session = PXCSession::CreateInstance();
	m_session->CreateImpl<PXCRotation>(&m_rotation);

	m_pose[0] = 1.0f;
	m_pose[1] = 0;
	m_pose[2] = 0;
	m_pose[3] = 0;

	m_pose[4] = 0;
	m_pose[5] = 1.0f;
	m_pose[6] = 0;
	m_pose[7] = 0;
	
	m_pose[8] = 0;
	m_pose[9] = 0;
	m_pose[10] = 1.0f;
	m_pose[11] = 0;

	// Create the initialization options
	OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

	// configure our tracker
	osvrDeviceTrackerConfigure(opts, &m_tracker);

	// Create the device token with the options
	m_dev.initAsync(ctx, "Tracker", opts);

	// Send JSON descriptor
	m_dev.sendJsonDescriptor(com_samaust_trackerrs_osvr_json);

	// Register update callback
	m_dev.registerUpdateCallback(this);

	std::cout << "[TrackerRS-OSVR] Intel R200 camera tracker initialized" << std::endl;
}

TrackerRS::~TrackerRS(void)
{
	// Release
	//m_scenePerceptionController->Release();
}

OSVR_ReturnCode TrackerRS::update() {
	
	pxcStatus pxcSts = PXC_STATUS_NO_ERROR;
	if (PXC_STATUS_NO_ERROR == (pxcSts = m_pSenseManager->AcquireFrame(true)))
	{
		// Automatically tries to start tracking
		// Look straight ahead when starting the OSVR Server
		if (m_bIsScenePerceptionPaused)
		{
			m_pSample = m_pSenseManager->QuerySample();

			if (m_pSample == NULL || m_pSample->color == NULL || m_pSample->depth == NULL)
				m_error_flag = true;
			else
				m_error_flag = false;

			if (!m_error_flag)
				m_fSceneQuality = m_pScenePerception->CheckSceneQuality(m_pSample);
		}
		else
		{
			m_pSample = m_pSenseManager->QueryScenePerceptionSample();

			if (m_pSample == NULL || m_pSample->color == NULL || m_pSample->depth == NULL)
				m_error_flag = true;
			else
				m_error_flag = false;

			if (!m_error_flag)
			{
				m_pScenePerception->GetCameraPose(m_pose);
				m_trackingAccuracy = m_pScenePerception->QueryTrackingAccuracy();
			}
		}

		if (!m_error_flag)
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				if (GetAsyncKeyState(VK_F12) & 0x8000)
				{
					// Reset
					if (!m_bIsScenePerceptionPaused)
					{
						std::cout << "[TrackerRS-OSVR] Intel R200 camera tracker recentered" << std::endl;

						m_renderedVolume.reset(nullptr);
						m_scenePerceptionController->PauseScenePerception(true);
						m_bIsScenePerceptionPaused = true;
						m_pScenePerception->Reset();
						m_fSceneQuality = 0.0f;

						m_pose[0] = 1.0f;
						m_pose[1] = 0;
						m_pose[2] = 0;
						m_pose[3] = 0;

						m_pose[4] = 0;
						m_pose[5] = 1.0f;
						m_pose[6] = 0;
						m_pose[7] = 0;

						m_pose[8] = 0;
						m_pose[9] = 0;
						m_pose[10] = 1.0f;
						m_pose[11] = 0;
					}
				}
			}

			if (m_bIsScenePerceptionPaused && m_fSceneQuality >= m_minAcceptableSceneQuality)
			{
				m_renderedVolume.reset(new UtilRender(L"Volume"));
				m_scenePerceptionController->PauseScenePerception(false);
				m_bIsScenePerceptionPaused = false;
			}

			m_pSenseManager->ReleaseFrame();

			// Print pose to console
			//std::wcout << L"Camera Orientation: " << m_pose[0] << L" " << m_pose[1] << L" " << m_pose[2] << L" | "
			//	<< m_pose[4] << L" " << m_pose[5] << L" " << m_pose[6] << L" | "
			//	<< m_pose[8] << L" " << m_pose[9] << L" " << m_pose[10] << std::endl;

			// Rotation conversion from matrix to quaternion
			m_rotationMatrix[0][0] = m_pose[0];
			m_rotationMatrix[0][1] = m_pose[1];
			m_rotationMatrix[0][2] = m_pose[2];
		
			m_rotationMatrix[1][0] = m_pose[4];
			m_rotationMatrix[1][1] = m_pose[5];
			m_rotationMatrix[1][2] = m_pose[6];
		
			m_rotationMatrix[2][0] = m_pose[8];
			m_rotationMatrix[2][1] = m_pose[9];
			m_rotationMatrix[2][2] = m_pose[10];

			m_rotation->SetFromRotationMatrix(m_rotationMatrix);
			PXCPoint4DF32 quaternion = m_rotation->QueryQuaternion();		

			// Report pose
			OSVR_Pose3 ovr_pose;
			ovr_pose.translation.data[0] = m_pose[3];
			ovr_pose.translation.data[1] = m_pose[7];
			ovr_pose.translation.data[2] = m_pose[11];
		
			ovr_pose.rotation.data[0] = quaternion.x;
			ovr_pose.rotation.data[1] = quaternion.y;
			ovr_pose.rotation.data[2] = quaternion.z;
			ovr_pose.rotation.data[3] = quaternion.w;

			osvrDeviceTrackerSendPose(m_dev, m_tracker, &ovr_pose, static_cast<OSVR_ChannelCount>(0));
		}
	}

	return OSVR_RETURN_SUCCESS;
}
