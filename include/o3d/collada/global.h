/**
 * @file global.h
 * @brief O3DCollada global parameters.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-04-28
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_GLOBAL_H
#define _O3D_COLLADA_GLOBAL_H

#include <o3d/core/objects.h>

#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/object/geometrydata.h>

#include <dae.h>
#include <dom/domCOLLADA.h>

namespace o3d {
namespace collada {

using namespace ColladaDOM141;

class CBaseObject;

//---------------------------------------------------------------------------------------
//! @class ColladaInfo
//-------------------------------------------------------------------------------------
//! COLLADA object informations.
//---------------------------------------------------------------------------------------
class ColladaInfo
{
public:

	ColladaInfo() :
		m_upAxis(Y),
		m_boundingMode(GeometryData::BOUNDING_AUTO),
		m_AnimDuration(0.f) {}

	//! Get the up axis
	inline UInt32 getUpAxis() const { return m_upAxis; }
	//! Set the up axis
	inline void setUpAxis(UInt32 axis) { m_upAxis = axis; }

	//! Get the file path
	inline String getFilePath() const { return m_filePath; }
	//! Set the file path
	inline void setFilePath(const String &str) { m_filePath = str; }

	//! Get the up axis
	inline String getFileName() const { return m_fileName; }
	//! Get the up axis
	inline void setFileName(const String &str) { m_fileName = str; }

	//! Get the up axis
	inline GeometryData::BoundingMode getBoundingMode() const { return m_boundingMode; }
	//! Get the up axis
	inline void setBoundingMode(GeometryData::BoundingMode mode) { m_boundingMode = mode; }

	//! Add a new imported node
	inline void addNode(CBaseObject *pObject) { m_nodeList.push_back(pObject); }

	//! Find a node using its name
	CBaseObject* findNodeUsingName(const String &name) const;
	//! Find a node using its sid
	CBaseObject* findNodeUsingSid(const String &sid) const;
	//! Find a node using its id
	CBaseObject* findNodeUsingId(const String &id) const;

	//! Get the current imported object name
	inline String getCurrentName() const { return m_currentName; }
	//! Set the current imported object name
	inline void setCurrentName(const String &name) { m_currentName = name; }

	//! Get the animation duration
	inline Float getAnimationDuration() const { return m_AnimDuration; }
	//! Set the animation duration
	inline void maxAnimationDuration(Float f) { m_AnimDuration = max<Float>(m_AnimDuration,f); }

private:

	UInt32 m_upAxis;

	String m_filePath;
	String m_fileName;
	String m_currentName;

	GeometryData::BoundingMode m_boundingMode;

	std::vector<CBaseObject*> m_nodeList;

	Float m_AnimDuration;
};

//---------------------------------------------------------------------------------------
//! @class BaseCollada
//-------------------------------------------------------------------------------------
//! Base COLLADA object interface.
//---------------------------------------------------------------------------------------
class CBaseObject
{
public:

	enum ObjectType
	{
		TYPE_UNDEFINED,
		TYPE_GLOBAL,
		TYPE_BONES,
		TYPE_GEOMETRY,
		TYPE_MATERIAL
	};

	//! Default ctor
	CBaseObject(o3d::Scene *pScene, domCOLLADA *pDom, ColladaInfo &infos);

	//! Dtor
	virtual ~CBaseObject() {}

	//! Import method
	virtual Bool import() = 0;

	//! Export method
	virtual Bool doExport() = 0;

	//! Set post-import values to the scene
	virtual Bool toScene() = 0;

	//! Set pre-export values from the scene
	virtual Bool fromScene() = 0;

	//! Get the id
	inline const String& getId() const { return m_id; }

	//! Get the string id
	inline const String& getStrId() const { return m_StrId; }

	//! Get the string name
	inline const String& getName() const { return m_name; }

protected:

	String m_id;
	String m_name;

	o3d::Scene *m_scene;
	domCOLLADA &m_dom;

	String m_StrId;

	ColladaInfo &m_infos;
};

//---------------------------------------------------------------------------------------
//! @class CGlobal
//-------------------------------------------------------------------------------------
//! COLLADA global parameters. It mean the <asset>
//---------------------------------------------------------------------------------------
class CGlobal : public CBaseObject
{
public:

	//! Default ctor.
	CGlobal(o3d::Scene *scene, domCOLLADA *dom, ColladaInfo &infos);

	//! Destructor.
	virtual ~CGlobal();

	//! Import method.
	virtual Bool import();

	//! Export method.
	virtual Bool doExport();

	//! Set post-import values to the scene.
	virtual Bool toScene();

	//! Set pre-export values from the scene.
	virtual Bool fromScene();

	//! Get the up axis (X,Y or Z).
	inline Int32 getUpAxis() const { return m_upAxis; }

	//! Get the author.
	inline const String& getAuthor() const { return m_author; }

	//! Get the comment.
	inline const String& getComment() const { return m_comment; }

	//! Get the created date.
	inline const Date& getCreated() const { return m_created; }

	//! Get the modified date.
	inline const Date& getModified() const { return m_modified; }

	//! Get the revision name.
	inline const String& getRevisionName() const { return m_revision; }

	//! Get the subject name.
	inline const String& getSubjectName() const { return m_subject; }

	//! Get the title name.
	inline const String& getTitleName() const { return m_title; }

	//! Get the unit.
	inline Float getUnit() const { return m_unit; }

	//! Get the unit name.
	inline const String& getUnitName() const { return m_unitName; }

protected:

	String m_author;
	String m_comment;
	String m_copyright;

	Date m_created;
	Date m_modified;

	String m_revision;
	String m_subject;
	String m_title;

	Int32 m_upAxis;
	Float m_unit;   //! in meters

	String m_unitName;
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_GLOBAL_H

