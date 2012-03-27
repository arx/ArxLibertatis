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
#ifndef OGRE_VIEWPORTGRID_H
#define OGRE_VIEWPORTGRID_H

// Includes
#include <Ogre/OgreRenderTargetlistener.h>
#include <Ogre/OgreColourValue.h>
#include <Ogre/OgreMath.h>
#include <Ogre/OgreVector3.h>

namespace Ogre
{
	/** Use this class to render a dynamically adjusting grid inside an Ogre viewport.
	@todo
	Make the grid work with an arbitrary rotated orthogonal camera (e.g. for working in object space mode).
	Or should the grid be rendered in object space too then?
	*/
	class ViewportGrid: public RenderTargetListener
	{
	public:
		//! The render layer of the grid in orthogonal view
		enum RenderLayer
		{
			RL_BEHIND, //!< Behind all objects
			RL_INFRONT //!< In front of all objects
		};

	protected:
		// Member variables
		SceneManager *m_pSceneMgr;
		Viewport *m_pViewport;
		bool m_enabled;
		RenderLayer m_layer;

		Camera *m_pPrevCamera;
		bool m_prevOrtho;
		Vector3 m_prevCamPos;
		Real m_prevNear;
		Radian m_prevFOVy;
		Real m_prevAspectRatio;
		bool m_forceUpdate;

		ManualObject *m_pGrid;
		bool m_created;
		SceneNode *m_pNode;

		ColourValue m_colour1, m_colour2;
		unsigned int m_division;
		Real m_perspSize;
		bool m_renderScale, m_renderMiniAxes;

	public:
		// Constructors and destructor
		ViewportGrid(SceneManager *pSceneMgr, Viewport *pViewport);
		virtual ~ViewportGrid();

		//! Grid colour
		const ColourValue &getColour() const { return m_colour1; }
		void setColour(const ColourValue &colour);

		//! Grid division (the number of new lines that are created when zooming in).
		unsigned int getDivision() const { return m_division; }
		void setDivision(unsigned int division);

		//! Grid render layer (behind of in front of the objects).
		RenderLayer getRenderLayer() const { return m_layer; }
		void setRenderLayer(RenderLayer layer);

		//! Size of the grid in perspective view
		Real getPerspectiveSize() const { return m_perspSize; }
		void setPerspectiveSize(Real size);

		//! Render scaling info? Defaults to true.
		//! @todo Implement this
		bool getRenderScale() const { return m_renderScale; }
		void setRenderScale(bool enabled = true);

		//! Render mini axes? Defaults to true.
		//! @todo Implement this
		bool getRenderMiniAxes() const { return m_renderMiniAxes; }
		void setRenderMiniAxes(bool enabled = true);

		// Enable / disable
		bool isEnabled() const;
		void enable();
		void disable();
		void toggle();
		void setEnabled(bool enabled);

		// Other functions
		void forceUpdate() { m_forceUpdate = true; }

	protected:
		// RenderTargetListener
		void preViewportUpdate(const RenderTargetViewportEvent &evt);
		void postViewportUpdate(const RenderTargetViewportEvent &evt);

		// Other protected functions
		void createGrid();
		void destroyGrid();

		void update();
		void updateOrtho();
		void updatePersp();

		bool checkUpdate();
		bool checkUpdateOrtho();
		bool checkUpdatePersp();
	};
}

#endif // OGRE_VIEWPORTGRID_H