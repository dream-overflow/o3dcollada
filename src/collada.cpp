/**
 * @file collada.cpp
 * @brief Implementation of Collada.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-22
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/collada.h"
#include "o3d/collada/controller.h"
#include "o3d/collada/node.h"

#include <o3d/engine/animation/animation.h>
#include <o3d/engine/animation/animationmanager.h>
#include <o3d/engine/animation/animationplayermanager.h>
#include <o3d/engine/scene/scene.h>
#include <o3d/engine/hierarchy/hierarchytree.h>

using namespace o3d;
using namespace o3d::collada;

Int32 triangulate(DAE *_dae);

// ctor
Collada::Collada() :
	m_scene(NULL),
	m_doc(NULL),
	m_dom(NULL)
{
}

// dtor
Collada::~Collada()
{

}

// Define the o3d scene
void Collada::setScene(o3d::Scene *scene)
{
	m_scene = scene;
	O3D_ASSERT(m_scene);
}

// Run the import processing
Bool Collada::processImport(const String &filename)
{
	m_doc = new DAE();

	String lfilename = filename;
	lfilename.replace('\\','/');

	String lpathname = lfilename;
	Int32 pos = lpathname.reverseFind('/');
	lpathname.truncate(pos+1);

	m_info.setFilePath(lpathname);
	m_info.setFileName(lfilename.sub(pos+1));

	//m_doc->add("simple.dae");
	//m_doc->writeAll();

    m_dom = (domCOLLADA*)m_doc->open(lfilename.toUtf8().getData());
	//return false;

    if (!m_dom)
    {
		deletePtr(m_doc);
		return False;
	}

	m_global = new CGlobal(m_scene, m_dom, m_info);
	m_global->import();
	m_global->toScene();

	triangulate(m_doc);

	// the scene entry
	const domCOLLADA::domSceneRef sceneRef = m_dom->getScene();
	if (sceneRef.cast())
	{
		const domInstanceWithExtraRef instanceVisualScene = sceneRef->getInstance_visual_scene();
		if (instanceVisualScene.cast())
		{
			daeElementRef elementRef = instanceVisualScene->getUrl().getElement();
			domVisual_sceneRef visualSceneRef = (domVisual_scene*)elementRef.cast();

			// get the scene name
			m_scene->setSceneName(visualSceneRef->getName());

			domNode_Array &nodeArray = visualSceneRef->getNode_array();
			for (size_t i = 0; i < nodeArray.getCount(); ++i)
			{
				CNode *pNode = new CNode(
					m_scene,
					m_dom,
					m_info,
					nodeArray.get(i));

				if (pNode->import())
				{
					m_rootNodes.push_back(pNode);
				}
				else
				{
					deletePtr(pNode);
					break;
				}
			}
		}
	}

	// import animations
	// Load all the animation libraries
	for (size_t i = 0; i < m_dom->getLibrary_animations_array().getCount(); ++i)
	{
		domLibrary_animationsRef animationsRef = m_dom->getLibrary_animations_array()[i];
		for (size_t i = 0; i < animationsRef->getAnimation_array().getCount(); i++)
		{
			CAnimation *animation = new CAnimation(m_scene, m_dom, m_info, animationsRef->getAnimation_array()[i]);
			if (!animation->import())
			{
				deletePtr(animation);
				break;
			}
			m_animationList.push_back(animation);
		}
	}

	// clean
	m_doc->close(lfilename.toUtf8().getData());
    m_dom = nullptr;
	deletePtr(m_doc);

	// set imported data to the scene
	for (IT_RootNodeList it = m_rootNodes.begin(); it != m_rootNodes.end(); ++it)
	{
        // root bones are not children of the scene root node
        CNode *cnode = *it;
        if (cnode->isJoin())
            cnode->setParentNode(nullptr);
        else
            cnode->setParentNode(m_scene->getHierarchyTree()->getRootNode());

        if (!cnode->toScene())
			break;
	}

    CNode *rootAnimNode = nullptr;

	// second import pass, mainly used to apply skeleton onto skinning objects
	for (IT_RootNodeList it = m_rootNodes.begin(); it != m_rootNodes.end(); ++it)
	{
		if (!(*it)->postImportPass())
			break;

		if ((*it)->isAnimationRoot())
		{
			rootAnimNode = *it;

			if (rootAnimNode)
			{
				o3d::Animation *animation = new o3d::Animation(m_scene);
				animation->setName(rootAnimNode->getName() + "Anim");
				animation->setResourceName(rootAnimNode->getName() + "Anim.o3dan");
				animation->setFatherNode(rootAnimNode->getAnimationNode());
                animation->setDuration(m_info.getAnimationDuration());

				m_scene->getAnimationManager()->addAnimation(animation);

				o3d::Animatable *pAnimatable = rootAnimNode->getSceneNode();

				o3d::AnimationPlayer *animationPlayer = m_scene->getAnimationPlayerManager()->createAnimationPlayer(animation);
				animationPlayer->setFramePerSec(24.f);
				animationPlayer->setName(rootAnimNode->getName() + "Player");
				animationPlayer->setAnimatable(pAnimatable);
				animationPlayer->setPlayerMode(AnimationPlayer::MODE_LOOP);
				animationPlayer->play();

				m_scene->getAnimationPlayerManager()->add(*animationPlayer);
			}
		}
	}

	// delete temporary imported node hierarchy
	for (IT_RootNodeList it = m_rootNodes.begin(); it != m_rootNodes.end(); ++it)
	{
		deletePtr(*it);
	}

	m_rootNodes.clear();

	// and the global asset
	deletePtr(m_global);

	return True;
}

// Run the export processing
Bool Collada::processExport(const String &filename)
{
	//setScene(o3d::Scene::getCurrentScene());

	return True;
}

// Triangulation
UInt32 getMaxOffset(domInputLocalOffset_Array &input_array)
{
	UInt32 maxOffset = 0;
	for (UInt32 i = 0; i < input_array.getCount(); i++ )
	{
		if (input_array[i]->getOffset() > maxOffset)
			maxOffset = (UInt32)input_array[i]->getOffset();
	}
	return maxOffset;
}

void createTrianglesFromPolylist(domMesh *thisMesh, domPolylist *thisPolylist)
{
	// Create a new <triangles> inside the mesh that has the same material as the <polylist>
	domTriangles *thisTriangles = (domTriangles *)thisMesh->createAndPlace("triangles");
	//thisTriangles->setCount( 0 );
	UInt32 triangles = 0;
	thisTriangles->setMaterial(thisPolylist->getMaterial());
	domP* p_triangles = (domP*)thisTriangles->createAndPlace("p");

	// Give the new <triangles> the same <_dae> and <parameters> as the old <polylist>
	for (Int32 i=0; i<(Int32)(thisPolylist->getInput_array().getCount()); i++)
	{
		thisTriangles->placeElement( thisPolylist->getInput_array()[i]->clone() );
	}

	// Get the number of inputs and primitives for the polygons array
	Int32 numberOfInputs = (Int32)getMaxOffset(thisPolylist->getInput_array()) + 1;
	Int32 numberOfPrimitives = (Int32)(thisPolylist->getVcount()->getValue().getCount());

	UInt32 offset = 0;

	// Triangulate all the primitives, this generates all the triangles in a single <p> element
	for (Int32 j = 0; j < numberOfPrimitives; j++)
	{
		Int32 triangleCount = (Int32)thisPolylist->getVcount()->getValue()[j] -2;
		// Write out the primitives as triangles, just fan using the first element as the base
		int idx = numberOfInputs;
		for (Int32 k = 0; k < triangleCount; k++)
		{
			// First vertex
			for (Int32 l = 0; l < numberOfInputs; l++)
			{
				p_triangles->getValue().append(thisPolylist->getP()->getValue()[offset + l]);
			}
			// Second vertex
			for (Int32 l = 0; l < numberOfInputs; l++)
			{
				p_triangles->getValue().append(thisPolylist->getP()->getValue()[offset + idx + l]);
			}
			// Third vertex
			idx += numberOfInputs;
			for (Int32 l = 0; l < numberOfInputs; l++)
			{
				p_triangles->getValue().append(thisPolylist->getP()->getValue()[offset + idx + l]);
			}
			//thisTriangles->setCount(thisTriangles->getCount()+1);
			triangles++;
		}
		offset += (UInt32)thisPolylist->getVcount()->getValue()[j] * numberOfInputs;
	}
	thisTriangles->setCount( triangles );
}

void createTrianglesFromPolygons(domMesh *thisMesh, domPolygons *thisPolygons)
{
	// Create a new <triangles> inside the mesh that has the same material as the <polygons>
	domTriangles *thisTriangles = (domTriangles *)thisMesh->createAndPlace("triangles");
	thisTriangles->setCount( 0 );
	thisTriangles->setMaterial(thisPolygons->getMaterial());
	domP* p_triangles = (domP*)thisTriangles->createAndPlace("p");

	// Give the new <triangles> the same <_dae> and <parameters> as the old <polygons>
	for (Int32 i=0; i<(Int32)(thisPolygons->getInput_array().getCount()); i++)
	{
		thisTriangles->placeElement( thisPolygons->getInput_array()[i]->clone() );
	}

	// Get the number of inputs and primitives for the polygons array
	Int32 numberOfInputs = (Int32)getMaxOffset(thisPolygons->getInput_array()) +1;
	Int32 numberOfPrimitives = (Int32)(thisPolygons->getP_array().getCount());

	// Triangulate all the primitives, this generates all the triangles in a single <p> element
	for(int j = 0; j < numberOfPrimitives; j++)
	{
		// Check the polygons for consistency (some exported files have the wrong number of indices)
		domP * thisPrimitive = thisPolygons->getP_array()[j];
		Int32 elementCount = (Int32)(thisPrimitive->getValue().getCount());
		if ((elementCount%numberOfInputs) != 0)
		{
			//cerr<<"Primitive "<<j<<" has an element count "<<elementCount<<" not divisible by the number of inputs "<<numberOfInputs<<"\n";
		}
		else
		{
			Int32 triangleCount = (elementCount/numberOfInputs)-2;
			// Write out the primitives as triangles, just fan using the first element as the base
			Int32 idx = numberOfInputs;
			for (Int32 k = 0; k < triangleCount; k++)
			{
				// First vertex
				for (Int32 l = 0; l < numberOfInputs; l++)
				{
					p_triangles->getValue().append(thisPrimitive->getValue()[l]);
				}
				// Second vertex
				for (Int32 l = 0; l < numberOfInputs; l++)
				{
					p_triangles->getValue().append(thisPrimitive->getValue()[idx + l]);
				}
				// Third vertex
				idx += numberOfInputs;
				for (Int32 l = 0; l < numberOfInputs; l++)
				{
					p_triangles->getValue().append(thisPrimitive->getValue()[idx + l]);
				}
				thisTriangles->setCount(thisTriangles->getCount()+1);
			}
		}
	}
}

Int32 triangulate(DAE *_dae)
{
	Int32 error = 0;

	// How many geometry elements are there?
	Int32 geometryElementCount = (Int32)(_dae->getDatabase()->getElementCount(NULL, "geometry" ));

	for (Int32 currentGeometry = 0; currentGeometry < geometryElementCount; currentGeometry++)
	{
		// Find the next geometry element
		domGeometry *thisGeometry;
		error = _dae->getDatabase()->getElement((daeElement**)&thisGeometry,currentGeometry, NULL, "geometry");

		// Get the mesh out of the geometry
		domMesh *thisMesh = thisGeometry->getMesh();

		if (thisMesh == NULL)
			continue;

		// Loop over all the polygon elements
		for (Int32 currentPolygons = 0; currentPolygons < (Int32)(thisMesh->getPolygons_array().getCount()); currentPolygons++)
		{
			// Get the polygons out of the mesh
			// Always get index 0 because every pass through this loop deletes the <polygons> element as it finishes with it
			domPolygons *thisPolygons = thisMesh->getPolygons_array()[currentPolygons];
			createTrianglesFromPolygons( thisMesh, thisPolygons );
		}
		while (thisMesh->getPolygons_array().getCount() > 0)
		{
			domPolygons *thisPolygons = thisMesh->getPolygons_array().get(0);
			// Remove the polygons from the mesh
			thisMesh->removeChildElement(thisPolygons);
		}
		Int32 polylistElementCount = (Int32)(thisMesh->getPolylist_array().getCount());

		for (Int32 currentPolylist = 0; currentPolylist < polylistElementCount; currentPolylist++)
		{
			// Get the polylist out of the mesh
			// Always get index 0 because every pass through this loop deletes the <polygons> element as it finishes with it
			domPolylist *thisPolylist = thisMesh->getPolylist_array()[currentPolylist];
			createTrianglesFromPolylist( thisMesh, thisPolylist );
		}
		while (thisMesh->getPolylist_array().getCount() > 0)
		{
			domPolylist *thisPolylist = thisMesh->getPolylist_array().get(0);
			// Remove the polylist from the mesh
			thisMesh->removeChildElement(thisPolylist);
		}
	}
	return 0;
}

