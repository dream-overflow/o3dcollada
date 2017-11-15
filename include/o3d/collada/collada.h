/**
 * @file collada.h
 * @brief O3DCollada importer/exporter.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-22
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_H
#define _O3D_COLLADA_H

#include <o3d/engine/hierarchy/node.h>

#include <dae.h>
#include <dom/domCOLLADA.h>

#include "global.h"
#include "material.h"
#include "geometry.h"
#include "animation.h"

namespace o3d {
namespace collada {

class CNode;

//---------------------------------------------------------------------------------------
//! @class Collada
//-------------------------------------------------------------------------------------
//! Import/export COLLADA format to/from objective-3d native scene.
//---------------------------------------------------------------------------------------
class Collada
{
public:

	//! Default ctor.
	Collada();

	//! Destructor.
	virtual ~Collada();

	//! Define the O3D scene.
	void setScene(o3d::Scene *scene);

	//! Run the import processing.
	Bool processImport(const String &filename);

	//! Run the export processing.
	Bool processExport(const String &filename);

protected:

	o3d::Scene *m_scene;

	DAE *m_doc;
	domCOLLADA *m_dom;

	CGlobal *m_global;
	ColladaInfo m_info;

	typedef std::list<CNode*> T_RootNodeList;
	typedef T_RootNodeList::iterator IT_RootNodeList;
	T_RootNodeList m_rootNodes;

	typedef std::list<CAnimation*> T_AnimationList;
	typedef T_AnimationList::iterator IT_AnimationList;
	T_AnimationList m_animationList;
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_H

