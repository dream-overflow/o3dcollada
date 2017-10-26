/**
 * @file camera.h
 * @brief O3DCollada camera import/export.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_CAMERA_H
#define _O3D_COLLADA_CAMERA_H

#include "global.h"

namespace o3d {
namespace collada {

//---------------------------------------------------------------------------------------
//! @class CCamera
//-------------------------------------------------------------------------------------
//! COLLADA camera object. It mean the <camera>
//---------------------------------------------------------------------------------------
class CCamera : public CBaseObject
{
public:

	//! Default ctor.
	CCamera(
		o3d::Scene *pScene,
		domCOLLADA *pDom,
		ColladaInfo &infos,
		const domCameraRef cam);

	//! Destructor
	virtual ~CCamera();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

protected:

	const domCameraRef m_Camera;
};

} // namespace collada
} // namespace o3d

#endif // _O3SCOLLADA_CAMERA_H

