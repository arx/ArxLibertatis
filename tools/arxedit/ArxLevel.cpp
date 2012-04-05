/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "arxedit/ArxLevel.h"

// STD
#include <vector>
#include <set>
#include <map>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_stream.hpp>

// Ogre
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreMaterialManager.h>

#include "core/Config.h"

#include "io/Blast.h"
#include "io/resource/PakReader.h"

#include "platform/String.h"

namespace Arx {

Level::Level(Ogre::SceneManager* mSceneManager) 
	: mSceneManager(mSceneManager) {
}

bool Level::load(const fs::path& levelFile) {

	std::string strFile = levelFile.string();
	std::string fsDirSep(1, fs::path::dir_sep);
	std::string resDirSep(1, res::path::dir_sep);

	boost::replace_first(strFile, config.paths.data.string() + fsDirSep, "");
	boost::replace_all(strFile, fsDirSep, resDirSep);
	boost::algorithm::to_lower(strFile);
	mLevelFile = strFile;

	size_t size = 0;
	char * pData = resources->readAlloc(mLevelFile, size);
	if(!pData) {
		return false;
	}

	boost::iostreams::filtering_istream ims;
	ims.push(boost::iostreams::array_source(pData, size));

    Arx::LevelInfo lvl;
    ims.read((char*)&lvl, sizeof(lvl));
    
    int curPos, endPos;
    curPos = ims.tellg();
    ims.seekg (0, std::ios::end);
    endPos = ims.tellg();
    ims.seekg (curPos, std::ios::beg);

    // allocate memory:
    size_t compressedSize = (size_t)(endPos - curPos);
    char* compressedBuffer = new char [compressedSize];

    // read data as a block:
    ims.read(compressedBuffer, compressedSize);

	ims.pop();
	delete pData;
    

	Arx::SceneInfo sceneInfo;
    pData = blastMemAlloc(compressedBuffer, compressedSize, size);
	delete compressedBuffer;
    ims.push(boost::iostreams::array_source(pData, size));
    ims.read((char*)&sceneInfo, sizeof(sceneInfo));
	ims.pop();
    delete pData;	
    

    res::path scenePath = "game";
    scenePath /= res::path::load(safestring(sceneInfo.name));
    scenePath /= "fast.fts";

	pData = resources->readAlloc(scenePath, size);
	if(!pData) {
		return false;
	}

    ims.push(boost::iostreams::array_source(pData, size));

	Arx::SceneHeader sceneHeader;
    ims.read((char*)&sceneHeader, sizeof(sceneHeader));
    ims.seekg(sceneHeader.count * 768, std::ios::cur);

    curPos = ims.tellg();
    ims.seekg (0, std::ios::end);
    endPos = ims.tellg();
    ims.seekg (curPos, std::ios::beg);

    compressedSize = (size_t)(endPos - curPos);
    compressedBuffer = new char [compressedSize];
    ims.read (compressedBuffer,compressedSize);
    if(ims.fail())
        return false;
    ims.pop();
	delete pData;

    pData = blastMemAlloc(compressedBuffer, compressedSize, size);
	delete compressedBuffer;

    ims.push(boost::iostreams::array_source(pData, size));

    Arx::FastSceneHeader fastSceneHeader;
    ims.read((char*)&fastSceneHeader, sizeof(fastSceneHeader));
    
    std::vector<Arx::FastTextureInfo> fastTextureInfos;
	std::map<Arx::s32, Arx::FastTextureInfo*> fastTextureInfosLookup;
    fastTextureInfos.resize(fastSceneHeader.nb_textures);
    for(std::vector<Arx::FastTextureInfo>::iterator it = fastTextureInfos.begin(); it != fastTextureInfos.end(); ++it) {
        ims.read((char*)&(*it), sizeof(*it));
		fastTextureInfosLookup[it->tc] = &*it;
		
		res::path tempPath = res::path::load(safestring(it->fic)).remove_ext();
		bool foundPath = resources->getFile(tempPath.append(".png")) != NULL;
		foundPath = foundPath || resources->getFile(tempPath.set_ext("jpg"));
		foundPath = foundPath || resources->getFile(tempPath.set_ext("jpeg"));
		foundPath = foundPath || resources->getFile(tempPath.set_ext("bmp"));
		foundPath = foundPath || resources->getFile(tempPath.set_ext("tga"));
		strcpy(&it->fic[0], tempPath.string().c_str());
	}
    
    std::vector<Arx::FastPoly> fastPolys;
    fastPolys.resize(fastSceneHeader.nb_polys);
    std::vector<Arx::FastPoly>::iterator itPolys(fastPolys.begin());

    std::vector<Arx::FastSceneInfo> fastSceneInfos;
    fastSceneInfos.resize(fastSceneHeader.sizex * fastSceneHeader.sizez);
    for(std::vector<Arx::FastSceneInfo>::iterator it = fastSceneInfos.begin(); it != fastSceneInfos.end(); ++it) {
        ims.read((char*)&(*it), sizeof(*it));
    
        for(int i = 0; i < it->nbpoly; i++, itPolys++)
            ims.read((char*)&(*itPolys), sizeof(*itPolys));

        ims.seekg(it->nbianchors * sizeof(Arx::u32), std::ios::cur);
    }

    std::vector<Arx::RoomData> rooms;
    rooms.resize(fastSceneHeader.nb_rooms);
    itPolys = fastPolys.begin();
    for(int i = 0; itPolys != fastPolys.end(); ++itPolys, ++i)
    {
		if(itPolys->room != -1 && fastTextureInfosLookup.find(itPolys->tex) != fastTextureInfosLookup.end())
			rooms[itPolys->room-1].materialPolygons[itPolys->tex].push_back(i);
    }

	static int iNbRooms = 0;
	for(std::vector<Arx::RoomData>::iterator itRoom = rooms.begin(); itRoom != rooms.end(); ++itRoom, iNbRooms++) {
		int iNbMaterials = 0;
		for(std::map< Arx::s32, std::vector<Arx::u32> >::iterator itMaterial = itRoom->materialPolygons.begin(); itMaterial != itRoom->materialPolygons.end(); ++itMaterial, ++iNbMaterials) {

			Ogre::ResourceManager::ResourceCreateOrRetrieveResult ret = Ogre::MaterialManager::getSingleton().createOrRetrieve(fastTextureInfosLookup[itMaterial->first]->fic, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			Ogre::MaterialPtr mat = ret.first;

			if(ret.second)
			{
				mat->getTechnique(0)->getPass(0)->createTextureUnitState(fastTextureInfosLookup[itMaterial->first]->fic);
				mat->load();
				mat->setTextureAnisotropy(8);
			}

			Ogre::ManualObject* mo = mSceneManager->createManualObject(std::string("room_") + Ogre::StringConverter::toString(iNbRooms) + std::string("_material_") + Ogre::StringConverter::toString(iNbMaterials));
			mo->setDynamic(false);
			Ogre::uint32 nbVertices = 0;
			mo->begin(mat->getName());
			
			for(std::vector<Arx::u32>::iterator itPoly = itMaterial->second.begin(); itPoly != itMaterial->second.end(); ++itPoly) {
				Arx::FastPoly& poly = fastPolys[*itPoly];
			 
				int nbPoint = poly.type & 64 ? 4 : 3;
				for(int i = 0; i < nbPoint; i++) {
					mo->position(-poly.v[i].ssx + fastSceneHeader.Mscenepos.x, -(poly.v[i].sy - fastSceneHeader.Mscenepos.y), poly.v[i].ssz - fastSceneHeader.Mscenepos.z);
					mo->normal(-poly.nrml[i].x, -poly.nrml[i].y, poly.nrml[i].z);
					mo->textureCoord(poly.v[i].stu, poly.v[i].stv);
				}

				if(nbPoint == 4) {
					mo->triangle(nbVertices+0, nbVertices+1, nbVertices+2);
					mo->triangle(nbVertices+2, nbVertices+1, nbVertices+3);
				} else {
					mo->triangle(nbVertices+0, nbVertices+1, nbVertices+2);
				}

				nbVertices += nbPoint;
			}
			mo->end();
			
			//mo->setCastShadows(true);

			Ogre::SceneNode* roomNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
			roomNode->attachObject(mo);
		}
	}
	
	loadLights();

	return true;
}

bool Level::loadLights() {
	std::string lightFile = mLevelFile.string();

	res::path levelFile = mLevelFile;
	levelFile.set_ext("llf");
	
    size_t size = 0;
	char * pData = resources->readAlloc(levelFile, size);
	if(!pData) {
		return false;
	}

	boost::iostreams::filtering_istream ims;
	ims.push(boost::iostreams::array_source(pData, size));

	int curPos, endPos;
    curPos = ims.tellg();
    ims.seekg (0, std::ios::end);
    endPos = ims.tellg();
    ims.seekg (curPos, std::ios::beg);

    // allocate memory:
    size_t length = (size_t)(endPos - curPos);
    char* buffer = new char [length];

    // read data as a block:
    ims.read (buffer,length);
    if(ims.fail())
        return false;
    ims.pop();
	delete pData;

    size_t uncompressedSize;
    char* dat = blastMemAlloc(buffer, length, uncompressedSize);

    ims.push(boost::iostreams::array_source(dat, uncompressedSize));

	Arx::LevelLightsInfo lli;

	ims.read((char*)&lli, sizeof(lli));

	std::vector<Arx::Light> lights;
	lights.resize(lli.nb_lights);
	static int iNbLights = 0;
	for(std::vector<Arx::Light>::iterator it = lights.begin(); it != lights.end(); ++it, ++iNbLights) {
        ims.read((char*)&(*it), sizeof(*it));

		Ogre::Light* pointLight = mSceneManager->createLight(std::string("pointlight_") + Ogre::StringConverter::toString(iNbLights));
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setPosition(-it->pos.x, -it->pos.y, it->pos.z);
		pointLight->setDiffuseColour(it->rgb.r, it->rgb.g, it->rgb.b);
		//pointLight->setAttenuation(it->fallend, it->intensity, 1, 0);

		Ogre::MovableObject* pLightGizmo = mSceneManager->createEntity(std::string("lightgizmo_") + Ogre::StringConverter::toString(iNbLights), Ogre::SceneManager::PT_SPHERE);
		Ogre::SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
		lightNode->setPosition(-it->pos.x, -it->pos.y, it->pos.z);
		lightNode->scale(0.2f, 0.2f, 0.2f);
		lightNode->attachObject(pLightGizmo);
	}

	return true;
}

}