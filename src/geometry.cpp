/**
 * @file geometry.cpp
 * @brief Implementation of Geometry.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/material.h"
#include "o3d/collada/geometry.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/object/mesh.h>
#include <o3d/engine/object/meshdatamanager.h>
#include <o3d/engine/object/skin.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/material/materialpass.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CGeometry::CGeometry(
	o3d::Scene *scene,
	domCOLLADA *dom,
	ColladaInfo &infos,
	const domGeometryRef geo,
	const domBind_materialRef mat) :
		CBaseObject(scene,dom,infos),
		m_asSkinning(False),
        m_node(nullptr),
		m_geometry(geo),
		m_material(mat),
		m_CMaterial(scene,dom,infos,mat->getTechnique_common()->getInstance_material_array())
{
}

// Destructor
CGeometry::~CGeometry()
{
}

void CGeometry::Offsets::setInputs(domInputLocalOffset_Array &inputs)
{
	// inputs with offsets
	for (UInt32 i = 0; i < inputs.getCount(); i++)
	{
		Int32 thisoffset = (Int32)inputs[i]->getOffset();

		if (maxOffset < thisoffset)
			maxOffset++;

		domSource * source = (domSource*)inputs[i]->getSource().getElement().cast();

		if (strcmp("VERTEX", inputs[i]->getSemantic()) == 0)
		{
			positionOffset = thisoffset;
		}
		else if(strcmp("NORMAL", inputs[i]->getSemantic()) == 0)
		{
			normalStride = (Int32)source->getTechnique_common()->getAccessor()->getStride();
			normalOffset = thisoffset;
			normalFloats = &source->getFloat_array()->getValue();
		}
		else if((texture1Offset == -1) && ((strcmp("TEXCOORD", inputs[i]->getSemantic()) == 0) ||
				(strcmp("UV", inputs[i]->getSemantic()) == 0)))
		{
			texture1Stride = (Int32)source->getTechnique_common()->getAccessor()->getStride();
			texture1Offset = thisoffset;
			texture1Floats = &source->getFloat_array()->getValue();
		}
	}
	maxOffset++;

	// inputs without offsets in vertices
	domMesh *mesh = (domMesh*)inputs[0]->getParentElement()->getParentElement();
	domVertices *vertices = mesh->getVertices();
	domInputLocal_Array &vertices_inputs = vertices->getInput_array();
	for (UInt32 i = 0; i < vertices_inputs.getCount(); i++)
	{
		domSource *source = (domSource*)vertices_inputs[i]->getSource().getElement().cast();
		if (strcmp("POSITION", vertices_inputs[i]->getSemantic()) == 0)
		{
			positionStride = (Int32)source->getTechnique_common()->getAccessor()->getStride();
			positionFloats = &source->getFloat_array()->getValue();
			positionNum = (Int32)source->getFloat_array()->getCount() / positionStride;
		}
		else if(strcmp("NORMAL", vertices_inputs[i]->getSemantic()) == 0)
		{
			normalFloats = &source->getFloat_array()->getValue();
			normalOffset = positionOffset;
			normalStride = (Int32)source->getTechnique_common()->getAccessor()->getStride();
		}
		else if((strcmp("TEXCOORD", vertices_inputs[i]->getSemantic()) == 0) ||
				(strcmp("UV", vertices_inputs[i]->getSemantic()) == 0))
		{
			texture1Floats = &source->getFloat_array()->getValue();
			texture1Offset = positionOffset;
			texture1Stride = (Int32)source->getTechnique_common()->getAccessor()->getStride();
		}
	}
}

// Import method
Bool CGeometry::import()
{
	m_StrId = m_geometry->getId() ? m_geometry->getId() : "";
	m_name = m_geometry->getName();

	if (m_geometry->getSpline().cast())
	{
		O3D_ERROR(E_InvalidFormat("Unsupported spline feature"));
		return False;
	}

	if (m_geometry->getMesh().cast())
	{
		// firstly process vertices
		domMesh *mesh = m_geometry->getMesh().cast();

		// triangles
		if (mesh->getTriangles_array().getCount())
		{
			const domTriangles_Array &triangleArray = mesh->getTriangles_array();
			buildTriangles(triangleArray);
		}
		// or triangle strip
		else if (mesh->getTristrips_array().getCount())
		{
			const domTristrips_Array &triangleArray = mesh->getTristrips_array();
			buildTriangleStrip(triangleArray);
		}
		// or triangle fan
		else if (mesh->getTrifans_array().getCount())
		{
			O3D_ERROR(E_InvalidFormat("Unsupported triangle fan feature"));
			return False;
		}
		// or lines
		else if (mesh->getLines_array().getCount())
		{
			const domLines_Array &LinesArray = mesh->getLines_array();
			buildLines(LinesArray);
		}
		// or lines loop
		else if (mesh->getLinestrips_array().getCount())
		{
			const domLinestrips_Array &lineStripArray = mesh->getLinestrips_array();
			buildLineStrip(lineStripArray);
		}
		// or polygon
		else if (mesh->getPolygons_array().getCount())
		{
			O3D_ERROR(E_InvalidFormat("Unsupported polygon feature"));
			return False;
		}
		// or polygon list
		else if (mesh->getPolylist_array().getCount())
		{
			const domPolylist_Array &polysArray = mesh->getPolylist_array();
			buildPolygonList(polysArray);
		}
	}

	m_CMaterial.import();

	return True;
}

// Export method
Bool CGeometry::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CGeometry::toScene()
{
	m_infos.setCurrentName(m_name);

    o3d::MeshData *meshData = nullptr;

	// exists ?
	if (m_scene->getMeshDataManager()->isMeshData(m_name + ".o3dms"))
	{
		meshData = m_scene->getMeshDataManager()->addMeshData(m_name + ".o3dms");
	}
	// create
	else
	{
		meshData = new o3d::MeshData(m_scene);
		meshData->setName(m_name);
		meshData->setResourceName(m_name + ".o3dms");

		m_scene->getMeshDataManager()->addMeshData(meshData);

        O3D_LOG(Logger::INFO, String("Found geometry: ") + m_name);

		meshData->setGeometry(new o3d::GeometryData(meshData));
        //meshData->getGeometry()->setInterleave(True);

		// vertices
		// transform by shape matrix
		if (m_asSkinning)
		{
			for (Int32 i = 0; i < m_vertices.getSize(); i += 3)
			{
				Vector3 vec(&m_vertices[i]);
				vec = m_shapeMatrix * vec;

				m_vertices[i] = vec[X];
				m_vertices[i+1] = vec[Y];
				m_vertices[i+2] = vec[Z];
			}
		}
		meshData->getGeometry()->createElement(
				V_VERTICES_ARRAY,
				SmartArrayFloat(m_vertices.getData(),m_vertices.getSize()));

		// normals
        if (m_normals.isValid())
		{
			meshData->getGeometry()->createElement(
					V_NORMALS_ARRAY,
					SmartArrayFloat(m_normals.getData(),m_normals.getSize()));
		}

		// texture coordinates
        if (m_texCoords.isValid())
		{
			meshData->getGeometry()->createElement(
					V_TEXCOORDS_2D_1_ARRAY,
					SmartArrayFloat(m_texCoords.getData(),m_texCoords.getSize()));
		}

		// texture coordinates
        if (m_skinning.isValid() && m_asSkinning)
		{
			meshData->getGeometry()->createElement(V_SKINNING_ARRAY, m_skinning);
		}

		// texture coordinates
        if (m_weighting.isValid() && m_asSkinning)
		{
			meshData->getGeometry()->createElement(V_WEIGHTING_ARRAY, m_weighting);
		}

		for (size_t i = 0; i < m_facesList.size(); ++i)
		{
            FaceArray *faceArray = nullptr;
			ArrayUInt32 &faces = m_facesList[i].faces;

			if (m_vertices.getSize()/3 < 65536)
			{
                faceArray = new FaceArrayUInt16(meshData->getScene()->getContext(), P_TRIANGLES);//,m_infos.getFaceArrayMem());

				if (faces.getSize() > 0)
				{
					SmartArrayUInt16 facesData(faces.getSize());
					for (Int32 j = 0; j < faces.getSize(); ++j)
					{
						facesData[j] = (UInt16)faces[j];
					}

					reinterpret_cast<FaceArrayUInt16*>(faceArray)->setFaces(facesData);
				}
				else
					reinterpret_cast<FaceArrayUInt16*>(faceArray)->setFaces(
							SmartArrayUInt16());
			}
			else
			{
                faceArray = new FaceArrayUInt32(meshData->getScene()->getContext(), P_TRIANGLES);//,m_infos.getFaceArrayMem());
				if (faces.getSize() > 0)
					reinterpret_cast<FaceArrayUInt32*>(faceArray)->setFaces(
							SmartArrayUInt32(faces.getData(), faces.getSize()));
				else
					reinterpret_cast<FaceArrayUInt32*>(faceArray)->setFaces(
							SmartArrayUInt32());
			}

			meshData->getGeometry()->addFaceArray(i, faceArray);
		}

		meshData->computeBounding(m_infos.getBoundingMode());
        meshData->createGeometry();
	}

	// material
	m_CMaterial.toScene();

	// skinning or mesh
	if (m_asSkinning)
	{
		o3d::Skinning *skinning = new Skinning(m_node);
		skinning->setName(m_name);
		skinning->setMeshData(meshData);
		skinning->setBoundingAutoRegen(True);

		UInt32 numProfiles = m_CMaterial.getNumMaterials();
		skinning->setNumMaterialProfiles(numProfiles);

		for (UInt32 i = 0; i < numProfiles; ++i)
			m_CMaterial.getMaterial(skinning->getMaterialProfile(i), i);

		skinning->initMaterialProfiles();

		m_node->addSonLast(skinning);
	}
	else
	{
		o3d::Mesh *mesh = new Mesh(m_node);
		mesh->setName(m_name);
		mesh->setMeshData(meshData);

		UInt32 numProfiles = m_CMaterial.getNumMaterials();
		mesh->setNumMaterialProfiles(numProfiles);

		for (UInt32 i = 0; i < numProfiles; ++i)
			m_CMaterial.getMaterial(mesh->getMaterialProfile(i), i);

		mesh->initMaterialProfiles();

		m_node->addSonLast(mesh);
	}

	return True;
}

// Set pre-export values from the scene
Bool CGeometry::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

void CGeometry::buildLines(const domLines_Array &LinesArray)
{
	O3D_ASSERT(0);
}

void CGeometry::buildLineStrip(const domLinestrips_Array &lineStripArray)
{
	O3D_ASSERT(0);
}

void CGeometry::buildTriangles(const domTriangles_Array &triangleArray)
{
	for (size_t i = 0; i < triangleArray.getCount(); ++i)
	{
		String matName = triangleArray[i]->getMaterial();
		domInputLocalOffset_Array &inputs = triangleArray[i]->getInput_array();
		Offsets offsets(inputs);

		m_lookupTable.resize(offsets.positionNum);

		UInt32 nbrTriangles = (UInt32)triangleArray[i]->getCount();
		if (nbrTriangles == 0)
			continue;

		m_facesList.push_back(FaceList());
		FaceList &faceList = m_facesList.back();

		// set index, they all have the same index since we process deindexer conditioner
		const domListOfUInts &P = triangleArray[i]->getP()->getValue();

		for (UInt32 ivertex = 0; ivertex < nbrTriangles * 3; ++ivertex)
		{
			faceList.faces.push(setVertexData(offsets, P, ivertex));
		}
	}
}

void CGeometry::buildTriangleStrip(const domTristrips_Array &triangleArray)
{
	O3D_ASSERT(0);
}

void CGeometry::buildPolygonList(const domPolylist_Array &polysArray)
{
	for (size_t i = 0; i < polysArray.getCount(); ++i)
	{
		String matName = polysArray[i]->getMaterial();
		domInputLocalOffset_Array &inputs = polysArray[i]->getInput_array();
		Offsets offsets(inputs);

		m_lookupTable.resize(offsets.positionNum);

		UInt32 nbrPolys = (UInt32)polysArray[i]->getCount();
		m_facesList.push_back(FaceList());
		FaceList &faceList = m_facesList.back();

		// set index, they all have the same index since we process deindexer conditioner
		const domListOfUInts &P = polysArray[i]->getP()->getValue();
		const domListOfUInts &Vcount = polysArray[i]->getVcount()->getValue();

		UInt32 a,b,c,count;
		UInt32 v = 0;

		for (UInt32 iface = 0; iface < nbrPolys; ++iface)
		{
			count = (UInt32)Vcount[iface] - 2;

			a = v;

			for (UInt32 ivertex = 0; ivertex < count; ++ivertex)
			{
				b = v+ivertex+1;
				c = v+ivertex+2;

				faceList.faces.push(setVertexData(offsets, P, a));
				faceList.faces.push(setVertexData(offsets, P, b));
				faceList.faces.push(setVertexData(offsets, P, c));
			}

			v += count + 2;
		}
	}
}

UInt32 CGeometry::getTriIndexList(UInt32 *indices, domTriangles *tris, UInt32 triNum)
{
	// All the triangles are in a single P now
	// The number of indices in a triangle is the number of inputs times 3
	UInt32 idxCount = 3 * ((UInt32)tris->getInput_array().getCount());
	// The first index of this triangle is triNum * the number of indices per triangle
	UInt32 triStart = triNum * idxCount;
	// Since this is a triangle, we just do a straight copy of the indices into the output array
	for (UInt32 i = 0; i < idxCount; i++)
	{
		indices[i] = (UInt32)tris->getP()->getValue()[triStart+i];
	}
	return idxCount;
}

UInt32 CGeometry::countPotentialTris(domPolygons *polygons)
{
	UInt32 numTris = 0;
	UInt32 numInputs = (UInt32)polygons->getInput_array().getCount();
	UInt32 count = (UInt32)polygons->getP_array().getCount();
	for (UInt32 i = 0; i < count; i++)
	{
		// number of indexes / number of inputs - 2
		numTris += (UInt32)(polygons->getP_array()[i]->getValue().getCount()/numInputs) - 2;
	}
	return numTris;
}

UInt32 CGeometry::countPotentialTris(domPolylist *pPolylist)
{
	UInt32 numTris = 0;
	UInt32 count = (UInt32)pPolylist->getVcount()->getValue().getCount();
	for (UInt32 i = 0; i < count; i++)
	{
		// number of indexes / number of inputs - 2
		numTris += (UInt32)pPolylist->getVcount()->getValue()[i] - 2;
	}
	return numTris;
}

UInt32 CGeometry::getMaxOffsetFromInputs(domInputLocalOffset_Array &inputs)
{
	UInt32 maxoffset = 0;
	UInt32 count = (UInt32)inputs.getCount();
	for (UInt32 i = 0; i < count; i++)
	{
		UInt32 thisoffset  = (UInt32)inputs[i]->getOffset();
		if (maxoffset < thisoffset) maxoffset++;
	}
	return maxoffset + 1;
}

UInt32 CGeometry::setVertexData(Offsets &offset, const domListOfUInts &values, UInt32 i)
{
	Float vertex[3];
	Float normal[3];
	Float texCoord[2];

	UInt32 count = m_vertices.getSize() / 3;
	UInt32 i2, i3;

	if (offset.positionOffset != -1)
	{
		i3 = (UInt32)values[i*offset.maxOffset + offset.positionOffset] * offset.positionStride;
		vertex[0] = (Float)(*offset.positionFloats)[(size_t)i3+0];
		vertex[1] = (Float)(*offset.positionFloats)[(size_t)i3+1];
		vertex[2] = (Float)(*offset.positionFloats)[(size_t)i3+2];

		if (m_infos.getUpAxis() == X)
		{
			Float tmp;
			tmp = vertex[Y];
			vertex[Y] = vertex[X];
			vertex[X] = tmp;
		}
		else if (m_infos.getUpAxis() == Z)
		{
			Float tmp;
			tmp = vertex[Z];
			vertex[Z] = -vertex[Y];
			vertex[Y] = tmp;
		}
	}

	if (offset.normalOffset != -1)
	{
		i3 = (UInt32)values[i*offset.maxOffset + offset.normalOffset] * offset.normalStride;
		normal[0] = (Float)(*offset.normalFloats)[(size_t)i3+0];
		normal[1] = (Float)(*offset.normalFloats)[(size_t)i3+1];
		normal[2] = (Float)(*offset.normalFloats)[(size_t)i3+2];

 		if (m_infos.getUpAxis() == X)
		{
			Float tmp;
			tmp = normal[Y];
			normal[Y] = normal[X];
			normal[X] = tmp;
		}
		else if (m_infos.getUpAxis() == Z)
		{
			Float tmp;
			tmp = normal[Z];
			normal[Z] = -normal[Y];
			normal[Y] = tmp;
		}
	}

	if (offset.texture1Offset != -1)
	{
		i2 = (UInt32)values[i*offset.maxOffset + offset.texture1Offset] * offset.texture1Stride;
		texCoord[0] = (Float)(*offset.texture1Floats)[(size_t)i2+0];
		texCoord[1] = (Float)(*offset.texture1Floats)[(size_t)i2+1];

		if (m_infos.getUpAxis() == X)
		{
			texCoord[0] = 1.f - texCoord[0];
		}
		else if (m_infos.getUpAxis() == Y)
		{
			texCoord[1] = 1.f - texCoord[1];
		}
		else if (m_infos.getUpAxis() == Z)
		{
			texCoord[1] = 1.f - texCoord[1];
		}
	}

	// is existing vertex
	if ((offset.positionOffset != -1) &&
		(offset.normalOffset != -1) &&
		(offset.texture1Offset != -1))
	{
		for (UInt32 k = 0; k < count; ++k)
		{
			i2 = k << 1;
			i3 = k * 3;

			if ((m_vertices[i3+0] == vertex[0]) &&
				(m_vertices[i3+1] == vertex[1]) &&
				(m_vertices[i3+2] == vertex[2]) &&
				(m_normals[i3+0] == normal[0]) &&
				(m_normals[i3+1] == normal[1]) &&
				(m_normals[i3+2] == normal[2]) &&
				(m_texCoords[i2+0] == texCoord[0]) &&
				(m_texCoords[i2+1] == texCoord[1]))
			{
				// commented, to avoid duplication of indices, but maybe some vertices will
				// miss skinning, so check before add, or uncomment it and let the duplication (no danger only slower).
//				Int32 index = (UInt32)values[i*offset.maxOffset + offset.positionOffset];
//				m_lookupTable[index].push_back(k);

				return k;
			}
		}

		// not exist so add it
		m_vertices.pushArray(vertex,3);
		m_normals.pushArray(normal,3);
		m_texCoords.pushArray(texCoord,2);
	}
	else if ((offset.positionOffset != -1) &&
			 (offset.normalOffset != -1) &&
			 (offset.texture1Offset == -1))
	{
		for (UInt32 k = 0; k < count; ++k)
		{
			i2 = k << 1;
			i3 = k * 3;

			if ((m_vertices[i3+0] == vertex[0]) &&
				(m_vertices[i3+1] == vertex[1]) &&
				(m_vertices[i3+2] == vertex[2]) &&
				(m_normals[i3+0] == normal[0]) &&
				(m_normals[i3+1] == normal[1]) &&
				(m_normals[i3+2] == normal[2]))
			{
				Int32 index = (UInt32)values[i*offset.maxOffset + offset.positionOffset];
				m_lookupTable[index].push_back(k);

				return k;
			}
		}

		// not exist so add it
		m_vertices.pushArray(vertex,3);
		m_normals.pushArray(normal,3);
	}
	else if ((offset.positionOffset != -1) &&
			 (offset.normalOffset == -1) &&
			 (offset.texture1Offset != -1))
	{
		for (UInt32 k = 0; k < count; ++k)
		{
			i2 = k << 1;
			i3 = k * 3;

			if ((m_vertices[i3+0] == vertex[0]) &&
				(m_vertices[i3+1] == vertex[1]) &&
				(m_vertices[i3+2] == vertex[2]) &&
				(m_texCoords[i2+0] == texCoord[0]) &&
				(m_texCoords[i2+1] == texCoord[1]))
			{
				Int32 index = (UInt32)values[i*offset.maxOffset + offset.positionOffset];
				m_lookupTable[index].push_back(k);

				return k;
			}
		}

		// not exist so add it
		m_vertices.pushArray(vertex,3);
		m_texCoords.pushArray(texCoord,2);
	}
	else if ((offset.positionOffset != -1) &&
			 (offset.normalOffset == -1) &&
			 (offset.texture1Offset == -1))
	{
		for (UInt32 k = 0; k < count; ++k)
		{
			i3 = k * 3;

			if ((m_vertices[i3+0] == vertex[0]) &&
				(m_vertices[i3+1] == vertex[1]) &&
				(m_vertices[i3+2] == vertex[2]))
			{
				Int32 index = (UInt32)values[i*offset.maxOffset + offset.positionOffset];
				m_lookupTable[index].push_back(k);

				return k;
			}
		}

		// not exist so add it
		m_vertices.pushArray(vertex,3);
	}

	Int32 index = (UInt32)values[i*offset.maxOffset + offset.positionOffset];
	count = (m_vertices.getSize()-3) / 3;

	m_lookupTable[index].push_back(count);
	return count;
}

