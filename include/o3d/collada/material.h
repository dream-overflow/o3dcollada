/**
 * @file material.h
 * @brief Collada material import/export.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_MATERIAL_H
#define _O3D_COLLADA_MATERIAL_H

#include "global.h"
#include <o3d/engine/texture/texture.h>
#include <o3d/engine/material/materialprofile.h>
#include <o3d/engine/texture/texture2d.h>

#include <vector>

namespace o3d {
namespace collada {

//---------------------------------------------------------------------------------------
//! @class CMaterial
//-------------------------------------------------------------------------------------
//! COLLADA material object. It mean the <material>
//---------------------------------------------------------------------------------------
class CMaterial : public CBaseObject
{
public:

	enum EffectType
	{
		DEFAULT,
		LAMBERT,
		CONSTANT,
		PHONG,
		BLINN
	};

	struct Sampler2d
	{
		Sampler2d() :
            warpMode(Texture::REPEAT),//CLAMP),
			filteringMode(Texture::BILINEAR_FILTERING),//TRILINEAR_ANISOTROPIC),
			anisotropy(1.f),//16.f),
            map(nullptr)
		{}

		String texture;
		o3d::Texture::WrapMode warpMode;
		o3d::Texture::FilteringMode filteringMode;
		Float anisotropy;
		o3d::Texture2D *map;
	};

	struct Effect
	{
		Effect() :
			type(DEFAULT),
            ambiant(0.1f,0.1f,0.1f,1.f),
            diffuse(0.f,0.f,0.f,1.f),
            specular(0.f,0.f,0.f,1.f),
            emission(0.f,0.f,0.f,1.f),
            reflective(0.f,0.f,0.f,1.f),
            transparent(0.f,0.f,0.f,1.f),
			shininess(1.f),
			reflectivity(1.f),
			transparency(1.f),
			indexOfRefraction(0.f),
			alphaTest(False),
			alphaTestFunc(COMP_ALWAYS),
			alphaTestRef(0.f),
            blendFunc(Blending::DISABLED)
		{}

		EffectType type;

		String name;
		String sid;

		Color ambiant;
		Color diffuse;
		Color specular;
		Color emission;
		Color reflective;
		Color transparent;

		Float shininess;
		Float reflectivity;
		Float transparency;
		Float indexOfRefraction;

		Sampler2d ambiantMap;
		Sampler2d diffuseMap;
		Sampler2d normalMap;
		Sampler2d bumpMap;
		Sampler2d specularMap;
		Sampler2d emissionMap;
		Sampler2d reflectiveMap;
		Sampler2d transparentMap;

		// TODO pass

		Bool alphaTest;
		Comparison alphaTestFunc;
		Float alphaTestRef;

        Blending::FuncProfile blendFunc;
	};

	//! Default ctor.
	CMaterial(
		o3d::Scene *pScene,
		domCOLLADA *pDom,
		ColladaInfo &infos,
		const domInstance_material_Array mat);

	//! Destructor
	virtual ~CMaterial();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

	//! Define an MaterialProfile for a given material (effect) id.
	void getMaterial(o3d::MaterialProfile &profile, UInt32 id) const;

	//! Get the number of materials.
	UInt32 getNumMaterials() const { return static_cast<UInt32>(m_effectList.size()); }

protected:

	const domInstance_material_Array m_materialArray;

	typedef std::vector<Effect> T_EffectVector;
	typedef T_EffectVector::iterator IT_EffectVector;
	T_EffectVector m_effectList;

	void setEffect(CMaterial::Effect &effect, domInstance_effectRef effectRef);
	void loadSamplers(Effect &effect);
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_MATERIAL_H

