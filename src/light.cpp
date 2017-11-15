/**
 * @file light.cpp
 * @brief Implementation of Light.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/light.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/object/light.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CLight::CLight(
	o3d::Scene *pScene,
	domCOLLADA *pDom,
	ColladaInfo &infos,
	const domLightRef light) :
		CBaseObject(pScene,pDom,infos),
		m_Light(light)
{
}

// Destructor
CLight::~CLight()
{
}

// Import method
Bool CLight::import()
{

	return True;
}

// Export method
Bool CLight::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CLight::toScene()
{

	return True;
}

// Set pre-export values from the scene
Bool CLight::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

