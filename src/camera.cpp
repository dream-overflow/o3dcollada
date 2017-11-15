/**
 * @file camera.cpp
 * @brief Implementation of Camera.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/camera.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/object/camera.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CCamera::CCamera(
	o3d::Scene *pScene,
	domCOLLADA *pDom,
	ColladaInfo &infos,
	const domCameraRef cam) :
		CBaseObject(pScene,pDom,infos),
		m_Camera(cam)
{
}

// Destructor
CCamera::~CCamera()
{
}

// Import method
Bool CCamera::import()
{

	return True;
}

// Export method
Bool CCamera::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CCamera::toScene()
{

	return True;
}

// Set pre-export values from the scene
Bool CCamera::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

