/**
 * @file global.cpp
 * @brief Implementation of Global.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-28
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/global.h"

#include <o3d/engine/scene/scene.h>

using namespace o3d;
using namespace o3d::collada;

// Find a node using its name
CBaseObject* ColladaInfo::findNodeUsingName(const String &name) const
{
	for (std::vector<CBaseObject*>::const_iterator it = m_nodeList.begin(); it != m_nodeList.end(); ++it)
	{
		if ((*it)->getName() == name)
			return (*it);
	}
	return NULL;
}

// Find a node using its sid
CBaseObject* ColladaInfo::findNodeUsingSid(const String &sid) const
{
	for (std::vector<CBaseObject*>::const_iterator it = m_nodeList.begin(); it != m_nodeList.end(); ++it)
	{
		if ((*it)->getStrId() == sid)
			return (*it);
	}
	return NULL;
}

// Find a node using its id
CBaseObject* ColladaInfo::findNodeUsingId(const String &id) const
{
    if (id.isNull())
        return nullptr;

	for (std::vector<CBaseObject*>::const_iterator it = m_nodeList.begin(); it != m_nodeList.end(); ++it)
	{
		if ((*it)->getId() == id)
			return (*it);
	}
    return nullptr;
}

//! Default ctor
CBaseObject::CBaseObject(o3d::Scene *pScene, domCOLLADA *pDom, ColladaInfo &infos) :
	m_scene(pScene),
	m_dom(*pDom),
	m_infos(infos)
{
}

// Default ctor.
CGlobal::CGlobal(o3d::Scene *pScene, domCOLLADA *pDom, ColladaInfo &infos) :
	CBaseObject(pScene, pDom, infos),
	m_author("Objective-3D"),
	m_comment("Objective-3D (c) http://o3d.dreamoverflow.com"),
	m_upAxis(Y),
	m_unit(0.1f),
	m_unitName("decimetre")
{
	m_created.setCurrent();
	m_modified.setCurrent();
}

// Destructor
CGlobal::~CGlobal()
{
}

// Import method
Bool CGlobal::import()
{
	const domAssetRef asset = m_dom.getAsset();
	if (!asset.cast())
		return True;

	// take only the first contributor
	if (asset->getContributor_array().getCount())
	{
		if (asset->getContributor_array().get(0)->getAuthor().cast())
			m_author = asset->getContributor_array().get(0)->getAuthor()->getValue();

		if (asset->getContributor_array().get(0)->getComments().cast())
			m_comment = asset->getContributor_array().get(0)->getComments()->getValue();

		if (asset->getContributor_array().get(0)->getCopyright().cast())
			m_copyright = asset->getContributor_array().get(0)->getCopyright()->getValue();
	}

	//m_created = asset->getCreated()->getValue(); @TODO
	//m_modified = asset->getModified()->getValue(); @TODO

	if (asset->getRevision().cast())
		m_revision = asset->getRevision()->getValue();

	if (asset->getSubject().cast())
		m_subject = asset->getSubject()->getValue();

	if (asset->getTitle().cast())
		m_title = asset->getTitle()->getValue();

	if (asset->getUnit().cast())
		m_unit = (Float)asset->getUnit()->getMeter();

	if (asset->getUnit().cast())
		m_unitName = asset->getUnit()->getName();

	if (asset->getUp_axis().cast())
		m_infos.setUpAxis((Int32)asset->getUp_axis()->getValue());

	return True;
}

// Export method
Bool CGlobal::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CGlobal::toScene()
{
	m_scene->getSceneInfo().setTitle(m_title);
	m_scene->getSceneInfo().setAuthor(m_author);
	m_scene->getSceneInfo().setComment(m_comment);
	m_scene->getSceneInfo().setCopyright(m_copyright);
	m_scene->getSceneInfo().setSubject(m_subject);
	m_scene->getSceneInfo().setUnit(m_unit);
	m_scene->getSceneInfo().setUnitName(m_unitName);

	if (!m_revision.isEmpty())
		m_scene->getSceneInfo().setRevision(m_revision.toUInt32());

	m_scene->getSceneInfo().setCreated(m_created);
	m_scene->getSceneInfo().setModified(m_modified);

	return True;
}

// Set pre-export values from the scene
Bool CGlobal::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

