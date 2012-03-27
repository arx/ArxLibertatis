/*
-----------------------------------------------------------------------------
This source file is supposed to be used with OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2007 Jeroen Dierckx

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
// Includes
#include "ViewportGrid.h"
#include <Ogre/OgreManualObject.h>
#include <Ogre/OgreViewport.h>
#include <Ogre/OgreRenderTarget.h>
#include <Ogre/OgreSceneManager.h>
#include <Ogre/OgreStringConverter.h>
#include <Ogre/OgreMaterialManager.h>
using namespace Ogre;

// Constants
static const String sMatName = "ViewportGrid";


/******************************
* Constructors and destructor *
******************************/

ViewportGrid::ViewportGrid(SceneManager *pSceneMgr, Viewport *pViewport)
	: m_pSceneMgr(pSceneMgr), m_pViewport(pViewport), m_enabled(false)
	, m_pPrevCamera(0), m_prevOrtho(false), m_prevNear(0), m_prevFOVy(0), m_prevAspectRatio(0), m_forceUpdate(true)
	, m_pGrid(0), m_created(false), m_pNode(0)
	, m_colour1(0.7, 0.7, 0.7), m_colour2(0.7, 0.7, 0.7), m_division(10), m_perspSize(100)
	, m_renderScale(true), m_renderMiniAxes(true)
{
	assert(m_pSceneMgr);
	assert(m_pViewport);

	createGrid();
	setRenderLayer(RL_BEHIND);

	// Add this as a render target listener
	m_pViewport->getTarget()->addListener(this);
}

ViewportGrid::~ViewportGrid()
{
	// Remove this as a render target listener
	m_pViewport->getTarget()->removeListener(this);

	destroyGrid();
}


/************************
* Get and set functions *
************************/

/** Sets the colour of the major grid lines (the minor lines are alpha-faded out/in when zooming out/in)
@note The alpha value is automatically set to one
*/
void ViewportGrid::setColour(const ColourValue &colour)
{
	// Force alpha = 1 for the primary colour
	m_colour1 = colour; m_colour1.a = 1.0f;
	m_colour2 = m_colour1;
	forceUpdate();
}

/** Sets in how many lines a grid has to be divided when zoomed in.
Defaults to 10.
*/
void ViewportGrid::setDivision(unsigned int division)
{
	m_division = division;
	forceUpdate();
}

/** Sets the render layer of the grid
@note Ignored in perspective view.
@see Ogre::ViewportGrid::RenderLayer
*/
void ViewportGrid::setRenderLayer(RenderLayer layer)
{
	m_layer = layer;

	switch(m_layer)
	{
	default:
	case RL_BEHIND:
		// Render just before the world geometry
		m_pGrid->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_1 - 1);
		break;

	case RL_INFRONT:
		// Render just before the overlays
		m_pGrid->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
		break;
	}
}

/** Sets the size of the grid in perspective view.
Defaults to 100 units.
@note Ignored in orthographic view.
*/
void ViewportGrid::setPerspectiveSize(Real size)
{
	m_perspSize = size;
	forceUpdate();
}

/** Sets whether to render scaling info in an overlay.
This looks a bit like the typical scaling info on a map.
*/
void ViewportGrid::setRenderScale(bool enabled)
{
	m_renderScale = enabled;
	forceUpdate();
}

/** Sets whether to render mini-axes in an overlay.
*/
void ViewportGrid::setRenderMiniAxes(bool enabled)
{
	m_renderMiniAxes = enabled;
	forceUpdate();
}


/******************
* Other functions *
******************/

bool ViewportGrid::isEnabled() const
{
	return m_enabled;
}

void ViewportGrid::enable()
{
	m_enabled = true;

	if(!m_pGrid->isAttached())
		m_pNode->attachObject(m_pGrid);

	forceUpdate();
}

void ViewportGrid::disable()
{
	m_enabled = false;

	if(m_pGrid->isAttached())
		m_pNode->detachObject(m_pGrid);
}

void ViewportGrid::toggle()
{
	setEnabled(!m_enabled);
}

void ViewportGrid::setEnabled(bool enabled)
{
	if(enabled)
		enable();
	else
		disable();
}


/***********************
* RenderTargetListener *
***********************/

void ViewportGrid::preViewportUpdate(const RenderTargetViewportEvent &evt)
{
	if(evt.source != m_pViewport) return;

	m_pGrid->setVisible(true);

	if(m_enabled)
		update();
}

void ViewportGrid::postViewportUpdate(const RenderTargetViewportEvent &evt)
{
	if(evt.source != m_pViewport) return;

	m_pGrid->setVisible(false);
}

/****************************
* Other protected functions *
****************************/

void ViewportGrid::createGrid()
{
	String name = m_pViewport->getTarget()->getName() + "::";
	name += StringConverter::toString(m_pViewport->getZOrder()) + "::ViewportGrid";

	// Create the manual object
	m_pGrid = m_pSceneMgr->createManualObject(name);
	m_pGrid->setDynamic(true);

	// Create the scene node (not attached yet)
	m_pNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(name);
	m_enabled = false;

	// Make sure the material is created
	//! @todo Should we destroy the material somewhere?
	MaterialManager &matMgr = MaterialManager::getSingleton();
	if(!matMgr.resourceExists(sMatName))
	{
		MaterialPtr pMaterial = matMgr.create(sMatName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		pMaterial->setLightingEnabled(false);
		pMaterial->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	}
}

void ViewportGrid::destroyGrid()
{
	// Destroy the manual object
	m_pSceneMgr->destroyManualObject(m_pGrid);
	m_pGrid = 0;

	// Destroy the scene node
	m_pSceneMgr->destroySceneNode(m_pNode->getName());
	m_pNode = 0;
}

void ViewportGrid::update()
{
	if(!m_enabled) return;

	Camera *pCamera = m_pViewport->getCamera();
	if(!pCamera) return;

	// Check if an update is necessary
	if(!checkUpdate() && !m_forceUpdate)
		return;

	if(pCamera->getProjectionType() == PT_ORTHOGRAPHIC)
		updateOrtho();
	else
		updatePersp();

	m_forceUpdate = false;
}

void ViewportGrid::updateOrtho()
{
	// Screen dimensions
	int width = m_pViewport->getActualWidth();
	int height = m_pViewport->getActualHeight();

	// Camera information
	Camera *pCamera = m_pViewport->getCamera();
	const Vector3 &camPos = pCamera->getPosition();
	Vector3 camDir = pCamera->getDirection();
	Vector3 camUp = pCamera->getUp();
	Vector3 camRight = pCamera->getRight();

	// Translation in grid space
	Real dx = camPos.dotProduct(camRight);
	Real dy = camPos.dotProduct(camUp);

	// Frustum dimensions
	// Note: Tan calculates the opposite side of a _right_ triangle given its angle, so we make sure it is one, and double the result
	Real worldWidth = 2 * Math::Tan(pCamera->getFOVy() / 2) * pCamera->getAspectRatio() * pCamera->getNearClipDistance();
	Real worldHeight = worldWidth / pCamera->getAspectRatio();
	Real worldLeft = dx - worldWidth / 2;
	Real worldRight = dx + worldWidth / 2;
	Real worldBottom = dy - worldHeight / 2;
	Real worldTop = dy + worldHeight / 2;

	// Conversion values (note: same as working with the height values)
	Real worldToScreen = width / worldWidth;
	Real screenToWorld = worldWidth / width;

	//! @todo Treshold should be dependent on window width/height (min? max?) so there are no more then m_division full alpha-lines
	static const int treshold = 10; // Treshhold in pixels

	// Calculate the spacing multiplier
	Real mult = 0;
	int exp = 0;
	Real temp = worldToScreen; // 1 world unit
	if(worldToScreen < treshold)
	{
		while(temp < treshold)
		{
			++exp;
			temp *= treshold;
		}

		mult = Math::Pow(m_division, exp);
	}
	else
	{
		while(temp > m_division * treshold)
		{
			++exp;
			temp /= treshold;
		}

		mult = Math::Pow(1.0f / m_division, exp);
	}

	// Interpolate alpha for (multiplied) spacing between treshold and m_division * treshold
	m_colour2.a = worldToScreen * mult / (m_division * treshold - treshold);
	if(m_colour2.a > 1.0f) m_colour2.a = 1.0f;

	// Calculate the horizontal zero-axis color
	Real camRightX = Math::Abs(camRight.x);
	Real camRightY = Math::Abs(camRight.y);
	Real camRightZ = Math::Abs(camRight.z);
	const ColourValue &horAxisColor = Math::RealEqual(camRightX, 1.0f) ? ColourValue::Red
		: Math::RealEqual(camRightY, 1.0f) ? ColourValue::Green
		: Math::RealEqual(camRightZ, 1.0f) ? ColourValue::Blue : m_colour1;

	// Calculate the vertical zero-axis color
	Real camUpX = Math::Abs(camUp.x);
	Real camUpY = Math::Abs(camUp.y);
	Real camUpZ = Math::Abs(camUp.z);
	const ColourValue &vertAxisColor = Math::RealEqual(camUpX, 1.0f) ? ColourValue::Red
		: Math::RealEqual(camUpY, 1.0f) ? ColourValue::Green
		: Math::RealEqual(camUpZ, 1.0f) ? ColourValue::Blue : m_colour1;

	// The number of lines
	int numLinesWidth = (int) (worldWidth / mult) + 1;
	int numLinesHeight = (int) (worldHeight / mult) + 1;

	// Start creating or updating the grid
	m_pGrid->estimateVertexCount(2 * numLinesWidth + 2 * numLinesHeight);
	if(m_created)
		m_pGrid->beginUpdate(0);
	else
	{
		m_pGrid->begin(sMatName, Ogre::RenderOperation::OT_LINE_LIST);
		m_created = true;
	}

	// Vertical lines
	Real startX = mult * (int) (worldLeft / mult);
	Real x = startX;
	while(x <= worldRight)
	{
		// Get the right color for this line
		int multX = (x == 0) ? x : (x < 0) ? (int) (x / mult - 0.5f) : (int) (x / mult + 0.5f);
		const ColourValue &colour = (multX == 0) ? vertAxisColor : (multX % (int) m_division) ? m_colour2 : m_colour1;

		// Add the line
		m_pGrid->position(x, worldBottom, 0);
		m_pGrid->colour(colour);
		m_pGrid->position(x, worldTop, 0);
		m_pGrid->colour(colour);

		x += mult;
	}

	// Horizontal lines
	Real startY = mult * (int) (worldBottom / mult);
	Real y = startY;
	while(y <= worldTop)
	{
		// Get the right color for this line
		int multY = (y == 0) ? y : (y < 0) ? (int) (y / mult - 0.5f) : (int) (y / mult + 0.5f);
		const ColourValue &colour = (multY == 0) ? horAxisColor : (multY % (int) m_division) ? m_colour2 : m_colour1;

		// Add the line
		m_pGrid->position(worldLeft, y, 0);
		m_pGrid->colour(colour);
		m_pGrid->position(worldRight, y, 0);
		m_pGrid->colour(colour);

		y += mult;
	}

	m_pGrid->end();

	m_pNode->setOrientation(pCamera->getOrientation());
}

void ViewportGrid::updatePersp()
{
	//! @todo Calculate the spacing multiplier
	Real mult = 200;

	//! @todo Interpolate alpha
	m_colour2.a = 0.5f;
	//if(m_colour2.a > 1.0f) m_colour2.a = 1.0f;

	// Calculate the vertical zero-axis color
	const ColourValue &xAxisColor = ColourValue::Blue;

	// Calculate the horizontal zero-axis color
	const ColourValue &zAxisColor = ColourValue::Red;
	
	// The number of lines
	int numLines = (int) (m_perspSize / mult) + 1;

	// Start creating or updating the grid
	m_pGrid->estimateVertexCount(4 * numLines);
	if(m_created)
		m_pGrid->beginUpdate(0);
	else
	{
		m_pGrid->begin(sMatName, Ogre::RenderOperation::OT_LINE_LIST);
		m_created = true;
	}

	// X-axis
	Real start = mult * (int) (-m_perspSize / 2 / mult);
	Real x = start;
	while(x <= m_perspSize / 2)
	{
		// Get the right color for this line
		int multX = (x == 0) ? x : (x < 0) ? (int) (x / mult - 0.5f) : (int) (x / mult + 0.5f);
		const ColourValue &colour = (multX == 0) ? xAxisColor : (multX % (int) m_division) ? m_colour2 : m_colour1;

		// Add the line
		m_pGrid->position(x, 0, -m_perspSize / 2);
		m_pGrid->colour(colour);
		m_pGrid->position(x, 0, m_perspSize / 2);
		m_pGrid->colour(colour);

		x += mult;
	}

	// Z-axis
	Real z = start;
	while(z <= m_perspSize / 2)
	{
		// Get the right color for this line
		int multZ = (z == 0) ? z : (z < 0) ? (int) (z / mult - 0.5f) : (int) (z / mult + 0.5f);
		const ColourValue &colour = (multZ == 0) ? zAxisColor : (multZ % (int) m_division) ? m_colour2 : m_colour1;

		// Add the line
		m_pGrid->position(-m_perspSize / 2, 0, z);
		m_pGrid->colour(colour);
		m_pGrid->position(m_perspSize / 2, 0, z);
		m_pGrid->colour(colour);

		z += mult;
	}

	m_pGrid->end();

	// Normal orientation, grid in the X-Z plane
	m_pNode->resetOrientation();
}

/* Checks if an update is necessary */
bool ViewportGrid::checkUpdate()
{
	bool update = false;

	Camera *pCamera = m_pViewport->getCamera();
	if(!pCamera) return false;

	if(pCamera != m_pPrevCamera)
	{
		m_pPrevCamera = pCamera;
		update = true;
	}

	bool ortho = (pCamera->getProjectionType() == PT_ORTHOGRAPHIC);
	if(ortho != m_prevOrtho)
	{
		m_prevOrtho = ortho;
		update = true;

		// Set correct material properties
		MaterialPtr pMaterial = MaterialManager::getSingleton().getByName(sMatName);
		if(!pMaterial.isNull())
		{
			pMaterial->setDepthWriteEnabled(!ortho);
			pMaterial->setDepthCheckEnabled(!ortho);
		}
	}

	return update || ortho ? checkUpdateOrtho() : checkUpdatePersp();
}

bool ViewportGrid::checkUpdateOrtho()
{
	bool update = false;

	Camera *pCamera = m_pViewport->getCamera();
	if(!pCamera) return false;

	if(pCamera->getPosition() != m_prevCamPos)
	{
		m_prevCamPos = pCamera->getPosition();
		update = true;
	}

	if(pCamera->getNearClipDistance() != m_prevNear)
	{
		m_prevNear = pCamera->getNearClipDistance();
		update = true;
	}

	if(pCamera->getFOVy() != m_prevFOVy)
	{
		m_prevFOVy = pCamera->getFOVy();
		update = true;
	}

	if(pCamera->getAspectRatio() != m_prevAspectRatio)
	{
		m_prevAspectRatio = pCamera->getAspectRatio();
		update = true;
	}

	return update;
}

bool ViewportGrid::checkUpdatePersp()
{
	//! @todo Add a check if grid line splitting/merging is implemented
	return false;
}
