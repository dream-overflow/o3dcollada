/**
 * @file light.h
 * @brief Collada light importer/expoter.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_LIGHT_H
#define _O3D_COLLADA_LIGHT_H

#include "global.h"

namespace o3d {
namespace collada {

//---------------------------------------------------------------------------------------
//! @class CLight
//-------------------------------------------------------------------------------------
//! COLLADA light object. It mean the <light>
//---------------------------------------------------------------------------------------
class CLight : public CBaseObject
{
public:

	//! Default ctor.
	CLight(
		o3d::Scene *pScene,
		domCOLLADA *pDom,
		ColladaInfo &infos,
		const domLightRef light);

	//! Destructor
	virtual ~CLight();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

protected:

	const domLightRef m_Light;
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_LIGHT_H

