/**
 * @file geometry.h
 * @brief O3DCollada geometry importer/exporter.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_GEOMETRY_H
#define _O3D_COLLADA_GEOMETRY_H

#include "global.h"
#include <dom/domElements.h>
#include <o3d/engine/hierarchy/node.h>

namespace o3d {
namespace collada {

class MeshData;
class CMaterial;

//---------------------------------------------------------------------------------------
//! @class CGeometry
//-------------------------------------------------------------------------------------
//! COLLADA geometry object. It mean the <geometry>
//---------------------------------------------------------------------------------------
class CGeometry : public CBaseObject
{
public:

	//! Default ctor.
	CGeometry(
		o3d::Scene *scene,
		domCOLLADA *dom,
		ColladaInfo &infos,
		const domGeometryRef geo,
		const domBind_materialRef mat);

	//! Destructor.
	virtual ~CGeometry();

	//! Import method.
	virtual Bool import();

	//! Export method.
	virtual Bool doExport();

	//! Set post-import values to the scene.
	virtual Bool toScene();

	//! Set pre-export values from the scene.
	virtual Bool fromScene();

	//! Set the scene node.
	inline void setNode(o3d::Node *pNode) { m_node = pNode; }

	//! Set smart array for skinning and weighting.
	inline void setSkinning(SmartArrayFloat skinning, SmartArrayFloat weighting, const Matrix4 &shapeMatrix)
	{
		m_skinning = skinning;
		m_weighting = weighting;
		m_shapeMatrix = shapeMatrix;
		m_asSkinning = True;
	}

	//! Get the lookup table.
	inline std::vector<std::vector<UInt32> >& getLookup() { return m_lookupTable; }

	//! Get the number of vertices after they are duplicated.
	inline UInt32 getNumVerticesDup() const { return m_vertices.getSize()/3; }

protected:

	friend class FaceList;

	Bool m_asSkinning;

	o3d::Node *m_node;

	const domGeometryRef m_geometry;
	const domBind_materialRef m_material;

	CMaterial m_CMaterial;

	ArrayFloat m_vertices;
	ArrayFloat m_normals;
	ArrayFloat m_texCoords;

	Matrix4 m_shapeMatrix;

	SmartArrayFloat m_skinning;
	SmartArrayFloat m_weighting;

	std::vector<std::vector<UInt32> > m_lookupTable;

	class FaceList
	{
	public:

		ArrayUInt32 faces;
		String material;

		void triangulate();
		void exploid();
	};

	std::vector<FaceList> m_facesList;

	void buildLines(const domLines_Array &LinesArray);
	void buildLineStrip(const domLinestrips_Array &lineStripArray);
	void buildTriangles(const domTriangles_Array &triangleArray);
	void buildTriangleStrip(const domTristrips_Array &triangleArray);
	void buildPolygonList(const domPolylist_Array &polysArray);

	UInt32 getTriIndexList(UInt32 *pIndices, domTriangles *pTris, UInt32 triNum);
	UInt32 countPotentialTris(domPolygons *pPolygons);
	UInt32 countPotentialTris(domPolylist *pPolylist);
	UInt32 getMaxOffsetFromInputs(domInputLocalOffset_Array &inputs);

	class Offsets
	{
	public:

		Offsets(domInputLocalOffset_Array &inputs)
		{
			maxOffset = 0;
			positionOffset = -1;
			normalOffset = -1;
			texture1Offset = -1;
            positionFloats = nullptr;
            normalFloats = nullptr;
            texture1Floats = nullptr;
			positionStride = 3;
			normalStride = 3;
			texture1Stride = 2;
			positionNum = 0;
			setInputs(inputs);
		};

		Int32 maxOffset;

		Int32 positionOffset;
		Int32 normalOffset;
		Int32 texture1Offset;
		Int32 positionStride;
		Int32 normalStride;
		Int32 texture1Stride;
		Int32 positionNum;

		domListOfFloats *positionFloats;
		domListOfFloats *normalFloats;
		domListOfFloats *texture1Floats;

	private:

		void setInputs(domInputLocalOffset_Array &inputs);
	};

	UInt32 setVertexData(Offsets& offset, const domListOfUInts &values, UInt32 i);
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_GEOMETRY_H

