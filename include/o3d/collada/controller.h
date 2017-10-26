/**
 * @file controller.h
 * @brief O3DCollada controller (skinning) importer/exporter.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_CONTROLLER_H
#define _O3D_COLLADA_CONTROLLER_H

#include "global.h"
#include <dom/domElements.h>
#include <o3d/engine/hierarchy/node.h>
#include <o3d/core/stringlist.h>

namespace o3d {

class Skeleton;

namespace collada {

class CGeometry;

//---------------------------------------------------------------------------------------
//! @class CController
//-------------------------------------------------------------------------------------
//! COLLADA controller skin object. It mean the <controller>
//---------------------------------------------------------------------------------------
class CController : public CBaseObject
{
public:

	//! Default ctor.
	CController(
		o3d::Scene *pScene,
		domCOLLADA *pDom,
		ColladaInfo &infos,
		const domControllerRef ctrl,
		const domBind_materialRef mat);

	//! Destructor
	virtual ~CController();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

	//! Apply skeleton to skinning
	Bool postImportPass();

	//! Set the scene node
	inline void setNode(Node *pNode) { m_node = pNode; }

	//! Set the skeleton node id
    inline void addSkeletonNode(const String &id) { m_skeletonId.push_back(id); }

    //! Get the skeleton object
    inline Skeleton* getSkeleton() { return m_skeleton; }

protected:

	const domControllerRef m_controller;
	const domBind_materialRef m_Material;

	Int32 m_upAxis;
	String m_filePath;

	o3d::Node *m_node;

	CGeometry *m_geometry;

    T_StringList m_skeletonId;
    Skeleton *m_skeleton;

	Matrix4 m_shapeMatrix;

	struct Join
	{
		Matrix4 invMatrix;
		String name;
	};

	typedef std::vector<Join> T_JoinList;
	typedef T_JoinList::iterator IT_JoinList;

	T_JoinList m_joinList;

	Bool m_findJoinByIDRef;  //!< true mean idRef joins, else name (or sid) joins

	struct Influence
	{
		UInt32 joinId;
		Float weight;
	};

	std::vector<std::vector<Influence> > m_influences; //!< influence on each vertex
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_CONTROLLER_H

