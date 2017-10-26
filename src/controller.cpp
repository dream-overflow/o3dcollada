/**
 * @file controller.cpp
 * @brief Implementation of Controller.h
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-05-13
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/material.h"
#include "o3d/collada/controller.h"
#include "o3d/collada/geometry.h"
#include "o3d/collada/node.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/object/skin.h>
#include <o3d/engine/object/skeleton.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CController::CController(
	o3d::Scene *scene,
	domCOLLADA *dom,
	ColladaInfo &infos,
	const domControllerRef ctrl,
	const domBind_materialRef mat) :
		CBaseObject(scene, dom, infos),
		m_controller(ctrl),
		m_Material(mat),
        m_node(nullptr),
        m_geometry(nullptr),
        m_skeleton(nullptr)
{
	domGeometryRef geo = (domGeometry*)ctrl->getSkin()->getSource().getElement().cast();
	m_geometry = new CGeometry(scene, dom, infos, geo, mat);
}

// Destructor
CController::~CController()
{
	deletePtr(m_geometry);
}

// Import method
Bool CController::import()
{
	m_geometry->import();

	// shape matrix
	// matrix
	domSkin::domBind_shape_matrixRef mat = m_controller->getSkin()->getBind_shape_matrix();
	Matrix4 m(
		(Float)mat->getValue().get(0),
		(Float)mat->getValue().get(1),
		(Float)mat->getValue().get(2),
		(Float)mat->getValue().get(3),

		(Float)mat->getValue().get(4),
		(Float)mat->getValue().get(5),
		(Float)mat->getValue().get(6),
		(Float)mat->getValue().get(7),

		(Float)mat->getValue().get(8),
		(Float)mat->getValue().get(9),
		(Float)mat->getValue().get(10),
		(Float)mat->getValue().get(11),

		(Float)mat->getValue().get(12),
		(Float)mat->getValue().get(13),
		(Float)mat->getValue().get(14),
		(Float)mat->getValue().get(15));
	m_shapeMatrix = m;

	// import skin matrices
	domSkin::domJoints *jointsElement = m_controller->getSkin()->getJoints().cast();
    domSource *jointNamesSource = nullptr;
    domSource *invBindMatsSource = nullptr;

	// Scan the <joint> looking for specific <input> semantics and remember where they are
	for (size_t input = 0; input < jointsElement->getInput_array().getCount(); ++input)
	{
		if (strcmp(jointsElement->getInput_array()[input]->getSemantic(),"JOINT") == 0)
		{
			// Found the JOINT input semantic
			jointNamesSource = (domSource *)((daeElement *)jointsElement->getInput_array()[input]->getSource().getElement());
		}
		else if(strcmp(jointsElement->getInput_array()[input]->getSemantic(),"INV_BIND_MATRIX") == 0)
		{
			// Found the INV_BIND_MATRIX semantic
			invBindMatsSource = (domSource *)((daeElement *)jointsElement->getInput_array()[input]->getSource().getElement());
		}
	}

	// Find the inputs we want from <vertex_weights>
	domSkin::domVertex_weights *vertexWeightsElement = m_controller->getSkin()->getVertex_weights();
    domSource *weightsSource = nullptr;

	// Scan the <vertex_weights> looking for specific <input> semantics and remember them
	for (size_t input = 0; input < vertexWeightsElement->getInput_array().getCount(); ++input)
	{
		if (strcmp(vertexWeightsElement->getInput_array()[input]->getSemantic(),"WEIGHT") == 0)
		{
			// Found the WEIGHT semantic
			weightsSource = (domSource *)((daeElement *)vertexWeightsElement->getInput_array()[input]->getSource().getElement());
		}
	}

	// create join and set bones names
	O3D_ASSERT(jointNamesSource->getName_array() || jointNamesSource->getIDREF_array());

	UInt32 jointCount = 0;
	if  (jointNamesSource->getName_array())
	{
		jointCount = (UInt32)jointNamesSource->getName_array()->getCount();
		for (UInt32 n = 0; n < jointCount; ++n)
		{
			m_joinList.push_back(Join());
			Join &join = m_joinList.back();
			join.name = jointNamesSource->getName_array()->getValue()[n];
		}
		m_findJoinByIDRef = False;
	}
	else if (jointNamesSource->getIDREF_array())
	{
		jointCount = (UInt32)jointNamesSource->getIDREF_array()->getCount();
		for (UInt32 n = 0; n < jointCount; ++n)
		{
			m_joinList.push_back(Join());
			Join &join = m_joinList.back();
			join.name = jointNamesSource->getIDREF_array()->getValue()[n].getID();
		}
		m_findJoinByIDRef = True;
	}

	// read inv matrices
    for (UInt32 m = 0; m < invBindMatsSource->getFloat_array()->getCount(); m += 16)
	{
		Matrix4 mat(
			(Float)invBindMatsSource->getFloat_array()->getValue()[m],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+1],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+2],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+3],

			(Float)invBindMatsSource->getFloat_array()->getValue()[m+4],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+5],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+6],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+7],

			(Float)invBindMatsSource->getFloat_array()->getValue()[m+8],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+9],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+10],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+11],

			(Float)invBindMatsSource->getFloat_array()->getValue()[m+12],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+13],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+14],
			(Float)invBindMatsSource->getFloat_array()->getValue()[m+15]);

		m_joinList[m>>4].invMatrix = mat;
	}

	// Allocate space for the joint weights
	UInt32 vertexWeightsCount = (UInt32)vertexWeightsElement->getCount();

	// <vcount> tells how many bones are associated with each vertex, this indicates how many
	// pairs of joint/weight indices to process out of the <v> array for this vertex.
	// get pointers to the vcount and v arrays
	domSkin::domVertex_weights::domVcount *vcountElement = vertexWeightsElement->getVcount();
	domSkin::domVertex_weights::domV *vElement = vertexWeightsElement->getV();
	UInt32 vPos = 0;

	m_influences.resize(vertexWeightsCount);

	// For each vertex in <vcount>
	for (UInt32 vertex = 0; vertex < vertexWeightsCount; ++vertex)
	{
		// Find number of bones (joints/weights) this vertex influences and allocate space to store them
		UInt32 numInfluences = (UInt32)vcountElement->getValue()[vertex];

		// For each bone, copy in the joint number and the actual float value in the weights (indexed by the
		// second value in the <v> array
		for (UInt32 inf = 0; inf < numInfluences; ++inf)
		{
			Influence influence;
			influence.joinId = (UInt32)vElement->getValue()[vPos++];
			influence.weight = (Float)weightsSource->getFloat_array()->getValue()[(size_t)(vElement->getValue()[vPos++])];

			// TODO a way to have more than 4 influences per vertex
			if (inf < 4)
				m_influences[vertex].push_back(influence);
		}

		// remove the lower weights influences
		if (numInfluences > 4)
		{
			// pseudo normalized sum
/*			Float sum = 0.f;

			for (O3D_UINT inf = 0; inf < numInfluences; ++inf)
			{
				sum += m_Influences[vertex][inf].weight;
			}
*/
			// @TODO

			// finally re-normalize the influence onto the vertex
		}
	}

	return True;
}

// Export method
Bool CController::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CController::toScene()
{
	UInt32 nbrVertices = m_influences.size();

	// create influences arrays
	SmartArrayFloat weighting(m_geometry->getNumVerticesDup()<<2);
	SmartArrayFloat bonesId(m_geometry->getNumVerticesDup()<<2);

	for (UInt32 i = 0; i < nbrVertices; ++i)
	{
		std::vector<Influence> &infs = m_influences[i];
		O3D_ASSERT(infs.size() < 5);

		std::vector<UInt32> &id = m_geometry->getLookup()[i];
		for (UInt32 j = 0; j < id.size(); ++j)
		{
			UInt32 i4 = id[j]<<2;

			switch (infs.size())
			{
				case 0:
					weighting[i4+0] = 0.f;
					weighting[i4+1] = 0.f;
					weighting[i4+2] = 0.f;
					weighting[i4+3] = 0.f;

					bonesId[i4+0] = -1.f;
					bonesId[i4+1] = -1.f;
					bonesId[i4+2] = -1.f;
					bonesId[i4+3] = -1.f;
					break;

				case 1:
					weighting[i4+0] = infs[0].weight;
					weighting[i4+1] = 0.f;
					weighting[i4+2] = 0.f;
					weighting[i4+3] = 0.f;

					bonesId[i4+0] = (Float)infs[0].joinId;
					bonesId[i4+1] = -1.f;
					bonesId[i4+2] = -1.f;
					bonesId[i4+3] = -1.f;
					break;

				case 2:
					weighting[i4+0] = infs[0].weight;
					weighting[i4+1] = infs[1].weight;
					weighting[i4+2] = 0.f;
					weighting[i4+3] = 0.f;

					bonesId[i4+0] = (Float)infs[0].joinId;
					bonesId[i4+1] = (Float)infs[1].joinId;
					bonesId[i4+2] = -1.f;
					bonesId[i4+3] = -1.f;
					break;

				case 3:
					weighting[i4+0] = infs[0].weight;
					weighting[i4+1] = infs[1].weight;
					weighting[i4+2] = infs[2].weight;
					weighting[i4+3] = 0.f;

					bonesId[i4+0] = (Float)infs[0].joinId;
					bonesId[i4+1] = (Float)infs[1].joinId;
					bonesId[i4+2] = (Float)infs[2].joinId;
					bonesId[i4+3] = -1.f;
					break;

				case 4:
					weighting[i4+0] = infs[0].weight;
					weighting[i4+1] = infs[1].weight;
					weighting[i4+2] = infs[2].weight;
					weighting[i4+3] = infs[3].weight;

					bonesId[i4+0] = (Float)infs[0].joinId;
					bonesId[i4+1] = (Float)infs[1].joinId;
					bonesId[i4+2] = (Float)infs[2].joinId;
					bonesId[i4+3] = (Float)infs[3].joinId;
					break;

				default:
					break;
			}
		}
	}

	m_geometry->setSkinning(bonesId, weighting, m_shapeMatrix);
	m_geometry->setNode(m_node);
	m_geometry->toScene();

	// set bones
    Skinning *skinning = (Skinning*)m_node->getSonList().front();
	skinning->setNumBones(m_joinList.size());

    // take the skinning skeleton for usage in postImportPass
    m_skeleton = skinning->getSkeleton();

	return True;
}

// Apply skeleton to skinning
Bool CController::postImportPass()
{
    Skinning *skinning = (Skinning*)m_node->getSonList().front();
    Bool root = False; // false until root bones is set

    if (!m_findJoinByIDRef)
	{
		for (size_t i = 0; i < m_joinList.size(); ++i)
		{
			CNode *cNode = (CNode*)m_infos.findNodeUsingName(m_joinList[i].name);
			
			// try with id (apparently there is a metaphysic behavior on some dae files
			if (!cNode)
				cNode = (CNode*)m_infos.findNodeUsingSid(m_joinList[i].name);

			if (cNode)
			{
                Bones *bones = (Bones*)cNode->getSceneNode();
                bones->setSkeleton(m_skeleton);

                // the root node of the skeleton have no parent node
                if (!root)
                {
                    BaseNode *rootBones = bones;
                    while (rootBones->getNode())
                    {
                        rootBones = rootBones->getNode();
                    }

                    if (rootBones->getType() != ENGINE_BONES)
                        O3D_ERROR(E_InvalidParameter("Root must be a Bones"));

                    // TODO does we use this as start ? and when there is more than one ?
                    if (!m_skeletonId.empty())
                        rootBones = static_cast<CNode*>(m_infos.findNodeUsingId(m_skeletonId.front()))->getSceneNode();

                    m_skeleton->setRoot(static_cast<Bones*>(rootBones));
                    root = True;
                }

                skinning->setBone(i, bones);
                skinning->setRefMatrix(i,m_joinList[i].invMatrix.invert());
			}
			else
				O3D_ERROR(E_InvalidParameter(String("Unable to find the bones ") + m_joinList[i].name));
		}
	}
	else
	{
		for (size_t i = 0; i < m_joinList.size(); ++i)
		{
			CNode *cNode = (CNode*)m_infos.findNodeUsingId(m_joinList[i].name);
			if (cNode)
			{
                Bones *bones = (Bones*)cNode->getSceneNode();
                bones->setSkeleton(m_skeleton);

                // the root node of the skeleton have no parent node
                if (!root)
                {
                    BaseNode *rootBones = bones;
                    while (rootBones->getNode())
                    {
                        rootBones = rootBones->getNode();
                    }

                    if (rootBones->getType() != ENGINE_BONES)
                        O3D_ERROR(E_InvalidParameter("Root must be a Bones"));

                    if (!m_skeletonId.empty())
                        rootBones = static_cast<CNode*>(m_infos.findNodeUsingId(m_skeletonId.front()))->getSceneNode();

                    m_skeleton->setRoot(static_cast<Bones*>(rootBones));
                    root = True;
                }

                skinning->setBone(i, bones);
				skinning->setRefMatrix(i,m_joinList[i].invMatrix.invert());
			}
			else
				O3D_ERROR(E_InvalidParameter(String("Unable to find the bones ") + m_joinList[i].name));
		}
	}

	skinning->initialize();

	return True;
}

// Set pre-export values from the scene
Bool CController::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

