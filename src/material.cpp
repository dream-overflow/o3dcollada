/**
 * @file material.cpp
 * @brief Implementation of Material.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include <o3d/engine/material/materialpass.h>
#include "o3d/collada/material.h"

#include <o3d/core/filemanager.h>

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/texture/texturemanager.h>
#include <o3d/engine/material/ambientmaterial.h>
#include <o3d/engine/material/pickingmaterial.h>
#include <o3d/engine/material/lambertmaterial.h>
#include <o3d/engine/material/bumpmaterial.h>
#include <o3d/engine/material/depthpassmaterial.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/object/light.h>
#include <o3d/engine/lodstrategy.h>

#include <dom/domProfile_COMMON.h>
#include <dom/domProfile_GLSL.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CMaterial::CMaterial(
	o3d::Scene *pScene,
	domCOLLADA *pDom,
	ColladaInfo &infos,
	const domInstance_material_Array mat) :
		CBaseObject(pScene,pDom,infos),
		m_materialArray(mat)
{
}

// Destructor
CMaterial::~CMaterial()
{
}

// Define an MaterialProfile for a given material (effect) id.
void CMaterial::getMaterial(o3d::MaterialProfile &profile, UInt32 id) const
{
	O3D_ASSERT(&profile);

	if (id >= static_cast<UInt32>(m_effectList.size()))
		O3D_ERROR(E_IndexOutOfRange("Effect/material index"));

	const Effect &effect = m_effectList[id];

	std::vector<Float> lodLevels;
	lodLevels.push_back(0.f);
	lodLevels.push_back(30.f);

	profile.setNumTechniques(2);
	profile.setLodLevels(lodLevels);
	profile.setLodStrategy(new LodStrategy());
	profile.getTechnique(0).setNumPass(1);
	profile.getTechnique(0).setLodIndex(0);
	profile.getTechnique(1).setNumPass(1);
	profile.getTechnique(1).setLodIndex(1);

	for (Int32 i = 0; i < 2; ++i)
	{
		MaterialPass &materialPass = profile.getTechnique(i).getPass(0);

		materialPass.setAmbient(effect.ambiant);
		materialPass.setDiffuse(effect.diffuse);
		materialPass.setSpecular(effect.specular);
		materialPass.setEmission(effect.emission);
        materialPass.setShine(o3d::max(effect.shininess, 1.f));
		materialPass.setTransparency(effect.transparency);
        materialPass.setAlphaTest(effect.alphaTest);
        materialPass.setAlphaTestFunc(effect.alphaTestFunc, effect.alphaTestRef);
        materialPass.setBlendingFunc(effect.blendFunc);
        //materialPass.setDoubleSide(True);

		if (effect.ambiantMap.map)
		{
			materialPass.setAmbientMap(effect.ambiantMap.map);
			materialPass.setMapFiltering(MaterialPass::AMBIENT_MAP, effect.ambiantMap.filteringMode);
			materialPass.setMapWarp(MaterialPass::AMBIENT_MAP, effect.ambiantMap.warpMode);
			materialPass.setMapAnisotropy(MaterialPass::AMBIENT_MAP, effect.ambiantMap.anisotropy);
		}
		if (effect.diffuseMap.map)
		{
			materialPass.setDiffuseMap(effect.diffuseMap.map);
            materialPass.setMapFiltering(MaterialPass::DIFFUSE_MAP, effect.diffuseMap.filteringMode);
            materialPass.setMapWarp(MaterialPass::DIFFUSE_MAP, effect.diffuseMap.warpMode);
            materialPass.setMapAnisotropy(MaterialPass::DIFFUSE_MAP, effect.diffuseMap.anisotropy);
		}
		if (effect.specularMap.map)
		{
			materialPass.setSpecularMap(effect.specularMap.map);
            materialPass.setMapFiltering(MaterialPass::SPECULAR_MAP, effect.specularMap.filteringMode);
            materialPass.setMapWarp(MaterialPass::SPECULAR_MAP, effect.specularMap.warpMode);
            materialPass.setMapAnisotropy(MaterialPass::SPECULAR_MAP, effect.specularMap.anisotropy);
		}
		if (effect.emissionMap.map)
		{
			materialPass.setEmissionMap(effect.emissionMap.map);
            materialPass.setMapFiltering(MaterialPass::EMISSION_MAP, effect.emissionMap.filteringMode);
            materialPass.setMapWarp(MaterialPass::EMISSION_MAP, effect.emissionMap.warpMode);
            materialPass.setMapAnisotropy(MaterialPass::EMISSION_MAP, effect.emissionMap.anisotropy);
		}
		if (effect.normalMap.map && (i == 0))
		{
			materialPass.setNormalMap(effect.normalMap.map);
            materialPass.setMapFiltering(MaterialPass::NORMAL_MAP, effect.normalMap.filteringMode);
            materialPass.setMapWarp(MaterialPass::NORMAL_MAP, effect.normalMap.warpMode);
            materialPass.setMapAnisotropy(MaterialPass::NORMAL_MAP, effect.normalMap.anisotropy);
		}
		if (effect.bumpMap.map && (i == 0))
		{
			materialPass.setHeightMap(effect.bumpMap.map);
            materialPass.setMapFiltering(MaterialPass::HEIGHT_MAP, effect.bumpMap.filteringMode);
            materialPass.setMapWarp(MaterialPass::HEIGHT_MAP, effect.bumpMap.warpMode);
            materialPass.setMapAnisotropy(MaterialPass::HEIGHT_MAP, effect.bumpMap.anisotropy);
		}

		materialPass.setMaterial(Material::AMBIENT, new o3d::AmbientMaterial(profile.getParent()));
		materialPass.setMaterial(Material::PICKING, new o3d::PickingMaterial(profile.getParent()));

        // first material (nearest) and if bump map, use it
        if (0)//effect.normalMap.map && (i == 0))
		{
            materialPass.setMaterial(Material::LIGHTING, new o3d::BumpMaterial(profile.getParent()));
            materialPass.setMaterial(Material::DEFERRED, new o3d::BumpMaterial(profile.getParent()));
		}
		else
		{
			materialPass.setMaterial(Material::LIGHTING, new o3d::LambertMaterial(profile.getParent()));
            materialPass.setMaterial(Material::DEFERRED, new o3d::LambertMaterial(profile.getParent()));
		}

		materialPass.getMaterial(Material::AMBIENT)->setName(effect.name);
		materialPass.getMaterial(Material::PICKING)->setName(effect.name);
		materialPass.getMaterial(Material::LIGHTING)->setName(effect.name);
        materialPass.getMaterial(Material::DEFERRED)->setName(effect.name);
	}
}

// Import method
Bool CMaterial::import()
{
	for (size_t i = 0; i < m_materialArray.getCount(); ++i)
	{
		domMaterialRef materialRef((domMaterial*)m_materialArray[i]->getTarget().getElement().cast());
		domInstance_effectRef effectRef = materialRef->getInstance_effect();
		m_effectList.push_back(Effect());

		Effect &effect = m_effectList.back();
		setEffect(effect,effectRef);
	}

	return True;
}

// Export method
Bool CMaterial::doExport()
{
	O3D_ASSERT(0);

	return True;
}

void CMaterial::loadSamplers(Effect &effect)
{
	if (effect.ambiantMap.texture.isValid() && !effect.ambiantMap.map)
		effect.ambiantMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.ambiantMap.texture,
			True);

	if (effect.diffuseMap.texture.isValid() && !effect.diffuseMap.map)
		effect.diffuseMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.diffuseMap.texture,
			True);

	if (effect.specularMap.texture.isValid() && !effect.specularMap.map)
		effect.specularMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.specularMap.texture,
			True);

	if (effect.emissionMap.texture.isValid() && !effect.emissionMap.map)
		effect.emissionMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.emissionMap.texture,
			True);

	if (effect.normalMap.texture.isValid() && !effect.normalMap.map)
	{
		/*Image pic(effect.normalMap.texture);
		pic.toNormalMap(1.f,False);
		Texture2D *tex = new Texture2D(m_scene, pic);
		tex->setResourceName(effect.normalMap.texture);
		tex->create(False);
		effect.normalMap.map = tex;
		m_scene->getTextureManager()->addTexture(tex);
		pic.Save("test.png",O3DPicture::PNG);*/
		effect.normalMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.normalMap.texture,
			True);
	}

	if (effect.bumpMap.texture.isValid() && !effect.bumpMap.map)
		effect.bumpMap.map = m_scene->getTextureManager()->addTexture2D(
			effect.bumpMap.texture,
			True);
/*
	if (effect.type == Constant)
	else if (effect.type == Phong)
	else if (effect.type == Blinn)
	else if (effect.type == Lambert)*/
}

// Set post-import values to the scene
Bool CMaterial::toScene()
{
	// load textures map
	for (size_t i = 0; i < m_effectList.size(); ++i)
		loadSamplers(m_effectList[i]);

	return True;
}

// Set pre-export values from the scene
Bool CMaterial::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

void readFloatOrParamType(
	domCommon_float_or_param_typeRef float_or_param,
	Float &_float)
{
	if (float_or_param->getFloat())
		_float = (Float)float_or_param->getFloat()->getValue();
	else
		_float = 0.f;
}

void readColorOrTextureType(
	domCommon_color_or_texture_type_complexType *color_or_texture,
	Color &_color,
	String &_texture)
{
	if (color_or_texture->getColor())
	{
		const domFx_color_common &color = color_or_texture->getColor()->getValue();
		_color.set((Float)color[0],(Float)color[1],(Float)color[2],(Float)color[3]);
	}
	if (color_or_texture->getTexture())
	{
		_texture = color_or_texture->getTexture()->getTexture();
		_color.set(0.f, 0.f, 0.f, 1.f);
	}
}

void readConstant(CMaterial::Effect &effect, domProfile_COMMON::domTechnique::domConstant *constant)
{
	const domCommon_color_or_texture_typeRef emmisionRef = constant->getEmission();
	if (emmisionRef.cast())
        readColorOrTextureType(emmisionRef,effect.emission,effect.emissionMap.texture);

	const domCommon_color_or_texture_typeRef reflectiveRef = constant->getReflective();
	if (reflectiveRef.cast())
        readColorOrTextureType(reflectiveRef,effect.reflective,effect.reflectiveMap.texture);

	const domCommon_float_or_param_typeRef reflectivityRef = constant->getReflectivity();
	if (reflectivityRef.cast())
        readFloatOrParamType(reflectivityRef,effect.reflectivity);

	const domCommon_transparent_typeRef transparentRef = constant->getTransparent();
	if (transparentRef.cast())
        readColorOrTextureType(transparentRef,effect.transparent,effect.transparentMap.texture);

	const domCommon_float_or_param_typeRef transparencyRef = constant->getTransparency();
	if (transparencyRef.cast())
        readFloatOrParamType(transparencyRef,effect.transparency);

	const domCommon_float_or_param_typeRef indexOfRefractionRef = constant->getIndex_of_refraction();
	if (indexOfRefractionRef.cast())
        readFloatOrParamType(indexOfRefractionRef,effect.indexOfRefraction);
}

void readLambert(CMaterial::Effect &effect, domProfile_COMMON::domTechnique::domLambert *lambert)
{
	const domCommon_color_or_texture_typeRef emmisionRef = lambert->getEmission();
	if (emmisionRef.cast())
        readColorOrTextureType(emmisionRef,effect.emission,effect.emissionMap.texture);

	const domCommon_color_or_texture_typeRef ambiantRef = lambert->getAmbient();
	if (ambiantRef.cast())
        readColorOrTextureType(ambiantRef,effect.ambiant,effect.ambiantMap.texture);

	const domCommon_color_or_texture_typeRef diffuseRef = lambert->getDiffuse();
	if (diffuseRef.cast())
        readColorOrTextureType(diffuseRef,effect.diffuse,effect.diffuseMap.texture);

	const domCommon_color_or_texture_typeRef reflectiveRef = lambert->getReflective();
	if (reflectiveRef.cast())
        readColorOrTextureType(reflectiveRef,effect.reflective,effect.reflectiveMap.texture);

	const domCommon_float_or_param_typeRef reflectivityRef = lambert->getReflectivity();
	if (reflectivityRef.cast())
        readFloatOrParamType(reflectivityRef,effect.reflectivity);

	const domCommon_transparent_typeRef transparentRef = lambert->getTransparent();
	if (transparentRef.cast())
        readColorOrTextureType(transparentRef,effect.transparent,effect.transparentMap.texture);

	const domCommon_float_or_param_typeRef transparencyRef = lambert->getTransparency();
	if (transparencyRef.cast())
        readFloatOrParamType(transparencyRef,effect.transparency);

	const domCommon_float_or_param_typeRef indexOfRefractionRef = lambert->getIndex_of_refraction();
	if (indexOfRefractionRef.cast())
        readFloatOrParamType(indexOfRefractionRef,effect.indexOfRefraction);
}

void readPhong(CMaterial::Effect &effect, domProfile_COMMON::domTechnique::domPhong *phong)
{
	const domCommon_color_or_texture_typeRef emmisionRef = phong->getEmission();
	if (emmisionRef.cast())
        readColorOrTextureType(emmisionRef,effect.emission,effect.emissionMap.texture);

	const domCommon_color_or_texture_typeRef ambiantRef = phong->getAmbient();
	if (ambiantRef.cast())
        readColorOrTextureType(ambiantRef,effect.ambiant,effect.ambiantMap.texture);

	const domCommon_color_or_texture_typeRef diffuseRef = phong->getDiffuse();
	if (diffuseRef.cast())
        readColorOrTextureType(diffuseRef,effect.diffuse,effect.diffuseMap.texture);

	const domCommon_color_or_texture_typeRef specularRef = phong->getSpecular();
	if (specularRef.cast())
        readColorOrTextureType(specularRef,effect.specular,effect.specularMap.texture);

	const domCommon_float_or_param_typeRef shininessRef = phong->getShininess();
	if (shininessRef.cast())
        readFloatOrParamType(shininessRef,effect.shininess);

	const domCommon_color_or_texture_typeRef reflectiveRef = phong->getReflective();
	if (reflectiveRef.cast())
        readColorOrTextureType(reflectiveRef,effect.reflective,effect.reflectiveMap.texture);

	const domCommon_float_or_param_typeRef reflectivityRef = phong->getReflectivity();
	if (reflectivityRef.cast())
        readFloatOrParamType(reflectivityRef,effect.reflectivity);

	const domCommon_transparent_typeRef transparentRef = phong->getTransparent();
	if (transparentRef.cast())
        readColorOrTextureType(transparentRef,effect.transparent,effect.transparentMap.texture);

	const domCommon_float_or_param_typeRef transparencyRef = phong->getTransparency();
	if (transparencyRef.cast())
        readFloatOrParamType(transparencyRef,effect.transparency);

	const domCommon_float_or_param_typeRef indexOfRefractionRef = phong->getIndex_of_refraction();
	if (indexOfRefractionRef.cast())
        readFloatOrParamType(indexOfRefractionRef,effect.indexOfRefraction);
}

void readBlinn(CMaterial::Effect &effect, domProfile_COMMON::domTechnique::domBlinn *blinn)
{
	const domCommon_color_or_texture_typeRef emmisionRef = blinn->getEmission();
	if (emmisionRef.cast())
        readColorOrTextureType(emmisionRef,effect.emission,effect.emissionMap.texture);

	const domCommon_color_or_texture_typeRef ambiantRef = blinn->getAmbient();
	if (ambiantRef.cast())
        readColorOrTextureType(ambiantRef,effect.ambiant,effect.ambiantMap.texture);

	const domCommon_color_or_texture_typeRef diffuseRef = blinn->getDiffuse();
	if (diffuseRef.cast())
        readColorOrTextureType(diffuseRef,effect.diffuse,effect.diffuseMap.texture);

	const domCommon_color_or_texture_typeRef specularRef = blinn->getSpecular();
	if (specularRef.cast())
        readColorOrTextureType(specularRef,effect.specular,effect.specularMap.texture);

	const domCommon_float_or_param_typeRef shininessRef = blinn->getShininess();
	if (shininessRef.cast())
        readFloatOrParamType(shininessRef,effect.shininess);

	const domCommon_color_or_texture_typeRef reflectiveRef = blinn->getReflective();
	if (reflectiveRef.cast())
        readColorOrTextureType(reflectiveRef,effect.reflective,effect.reflectiveMap.texture);

	const domCommon_float_or_param_typeRef reflectivityRef = blinn->getReflectivity();
	if (reflectivityRef.cast())
        readFloatOrParamType(reflectivityRef,effect.reflectivity);

	const domCommon_transparent_typeRef transparentRef = blinn->getTransparent();
	if (transparentRef.cast())
        readColorOrTextureType(transparentRef,effect.transparent,effect.transparentMap.texture);

	const domCommon_float_or_param_typeRef transparencyRef = blinn->getTransparency();
	if (transparencyRef.cast())
        readFloatOrParamType(transparencyRef,effect.transparency);

	const domCommon_float_or_param_typeRef indexOfRefractionRef = blinn->getIndex_of_refraction();
	if (indexOfRefractionRef.cast())
        readFloatOrParamType(indexOfRefractionRef,effect.indexOfRefraction);
}

void retrieveTexture(
		std::map<String,domCommon_newparam_type*> &newParam,
        const String &sid,
        CMaterial::Sampler2d &texture,
		const String &basePath)
{
    String surface_SID = newParam[sid]->getSampler2D()->getSource()->getValue();

	if (!newParam[surface_SID]->getSurface()->getFx_surface_init_common())
		return;

	xsIDREF& idRef = newParam[surface_SID]->getSurface()->
		getFx_surface_init_common()->getInit_from_array()[0]->getValue();

	idRef.resolveElement();
	domImage* image_element = (domImage*)(domElement*)idRef.getElement();

	if (image_element && image_element->getInit_from().cast())
	{
        texture.texture = cdom::uriToNativePath(image_element->getInit_from()->getValue().str().c_str()).c_str();
        texture.texture.replace('\\','/');
	}

	// make absolute path file name
    if (FileManager::instance()->isRelativePath(texture.texture))
	{
        texture.texture = basePath + '/' + texture.texture;
        FileManager::adaptPath(texture.texture);
	}

    O3D_MESSAGE(String("Found texture: ") + texture.texture);
}

void CMaterial::setEffect(CMaterial::Effect &effect, domInstance_effectRef instanceEffectRef)
{
	domEffectRef effectRef((domEffect*)instanceEffectRef->getUrl().getElement().cast());

	effect.name = effectRef->getName() ? effectRef->getName() : "";
	effect.sid = effectRef->getId() ? effectRef->getId() : "";

	const domFx_profile_abstract_Array &profileArray = effectRef->getFx_profile_abstract_array();
	//const domInstance_effect::domSetparam_Array &setParamArray = effectRef->getSetparam_array();

	// for each profile
	for (size_t profileIndex = 0; profileIndex < profileArray.getCount(); ++profileIndex)
	{
		String typeName = profileArray[profileIndex]->getTypeName();

		if (typeName == "profile_COMMON")
		{
			// Found the common profile and get the technique from it
			domProfile_COMMON *common = (domProfile_COMMON*)(domFx_profile_abstract*)profileArray[profileIndex];

			domProfile_COMMON::domTechnique *technique = common->getTechnique();
            if (technique == nullptr)
				continue;

			domProfile_COMMON::domTechnique::domConstant *constant = technique->getConstant();
			if (constant)
			{
                readConstant(effect, constant);
				effect.type = CONSTANT;
			}
			domProfile_COMMON::domTechnique::domLambert *lambert = technique->getLambert();
			if (lambert)
			{
                readLambert(effect, lambert);
				effect.type = LAMBERT;
			}
			domProfile_COMMON::domTechnique::domPhong *phong = technique->getPhong();
			if (phong)
			{
                readPhong(effect, phong);
				effect.type = PHONG;
			}
			domProfile_COMMON::domTechnique::domBlinn *blinn = technique->getBlinn();
			if(blinn)
			{
                readBlinn(effect, blinn);
				effect.type = BLINN;
			}

			domCommon_newparam_type_Array newparam_array = common->getNewparam_array();
            std::map<String,domCommon_newparam_type*> newParams;
			for (size_t i = 0; i < newparam_array.getCount(); ++i)
			{
                newParams[newparam_array[i]->getSid()] = newparam_array[i];
			}

			// retrieve textures's file name
			if (effect.ambiantMap.texture.length())
                retrieveTexture(newParams, effect.ambiantMap.texture, effect.ambiantMap, m_infos.getFilePath());

			if (effect.diffuseMap.texture.length())
                retrieveTexture(newParams, effect.diffuseMap.texture, effect.diffuseMap,m_infos.getFilePath());

			if (effect.specularMap.texture.length())
                retrieveTexture(newParams, effect.specularMap.texture, effect.specularMap, m_infos.getFilePath());

			if (effect.emissionMap.texture.length())
                retrieveTexture(newParams, effect.emissionMap.texture, effect.emissionMap, m_infos.getFilePath());

			if (effect.reflectiveMap.texture.length())
                retrieveTexture(newParams, effect.reflectiveMap.texture, effect.reflectiveMap, m_infos.getFilePath());

			if (effect.transparentMap.texture.length())
                retrieveTexture(newParams, effect.transparentMap.texture, effect.transparentMap, m_infos.getFilePath());

            if (effect.bumpMap.texture.length())
                retrieveTexture(newParams, effect.bumpMap.texture, effect.bumpMap, m_infos.getFilePath());

			// extra, MAYA give us the normal map here...
			domExtra_Array extraArray = technique->getExtra_array();
			if (extraArray.getCount() >= 1)
			{
				daeElement *techniqueElt = extraArray[0]->getChild("technique");
				if (techniqueElt)
				{
					daeElement *elementBump = techniqueElt->getChild("bump");
					if (elementBump)
					{
						daeElement *textureElement = elementBump->getChild("texture");
						if (textureElement)
						{
                            String sid = textureElement->getAttribute("texture").c_str();
                            retrieveTexture(newParams, sid, effect.normalMap, m_infos.getFilePath());
						}
					}
				}
			}
		}
		else if (typeName == "profile_GLSL")
		{
			// Found the common profile and get the technique from it
			domProfile_GLSL *glsl = (domProfile_GLSL*)(domFx_profile_abstract*)profileArray[profileIndex];

			if (glsl->getTechnique_array().getCount() > 0)
			{
				domProfile_GLSL::domTechnique *technique = glsl->getTechnique_array()[0];
                if (technique == nullptr)
					continue;

				// TODO multi pass
				for (size_t passIndex = 0; passIndex < technique->getPass_array().getCount(); ++passIndex)
				{
					domProfile_GLSL::domTechnique::domPass *pass = technique->getPass_array()[passIndex];

                    if (pass == nullptr)
						continue;

					domGl_pipeline_settings::domAlpha_func *alphaFunc = pass->getGl_pipeline_settings_array()[0]->getAlpha_func();
					if (alphaFunc)
					{
						effect.alphaTest = True;
						effect.alphaTestFunc = (Comparison)alphaFunc->getFunc()->getValue();
						effect.alphaTestRef = (Float)alphaFunc->getValue()->getValue();
					}

					domGl_pipeline_settings::domBlend_func *blendFunc = pass->getGl_pipeline_settings_array()[0]->getBlend_func();
					if (blendFunc)
					{
                        // SRC_A__ONE_MINUS_SRC_A
						if ((blendFunc->getSrc()->getValue() == GL_BLEND_TYPE_SRC_ALPHA) &&
							(blendFunc->getDest()->getValue() == GL_BLEND_TYPE_ONE_MINUS_SRC_ALPHA))
                            effect.blendFunc = Blending::SRC_A__ONE_MINUS_SRC_A;
                        // SRC_A__ONE
						else if ((blendFunc->getSrc()->getValue() == GL_BLEND_TYPE_SRC_ALPHA) &&
								(blendFunc->getDest()->getValue() == GL_BLEND_TYPE_ONE))
                                effect.blendFunc = Blending::SRC_A__ONE;
                        // SRC_A__ZERO
						else if ((blendFunc->getSrc()->getValue() == GL_BLEND_TYPE_SRC_ALPHA) &&
								(blendFunc->getDest()->getValue() == GL_BLEND_TYPE_ZERO))
                                effect.blendFunc = Blending::SRC_A__ZERO;
                        // ONE__ONE
						else if ((blendFunc->getSrc()->getValue() == GL_BLEND_TYPE_ONE) &&
								(blendFunc->getDest()->getValue() == GL_BLEND_TYPE_ONE))
                                effect.blendFunc = Blending::ONE__ONE;
                        // DST_COL__SRC_COL
						else if ((blendFunc->getSrc()->getValue() == GL_BLEND_TYPE_DEST_COLOR) &&
								(blendFunc->getDest()->getValue() == GL_BLEND_TYPE_SRC_COLOR))
                                effect.blendFunc = Blending::DST_COL__SRC_COL;
                        // DISABLED
						else
                            effect.blendFunc =Blending::DISABLED;
					}

					// TODO blend_equation
					// TODO blend_equation_separate
					// TODO cull_face
					// TODO fog_mode
					// TODO front_face
					// TODO depth_range
				}
			}
		}
		else
		{
			O3D_WARNING(String("Unsupported material profile type ") + typeName);
		}
	}
}

