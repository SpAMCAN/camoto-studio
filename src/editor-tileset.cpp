/**
 * @file   editor-tileset.cpp
 * @brief  Tileset editor.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/bind.hpp>
#include <camoto/gamegraphics.hpp>
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/glcanvas.h>
#include "editor-tileset.hpp"

#ifdef HAVE_GL_GL_H
#include <GL/gl.h>
#else
#ifdef HAVE_OPENGL_GL_H
#include <OpenGL/gl.h>
#endif
#endif

using namespace camoto;
using namespace camoto::gamegraphics;

enum {
	IDC_LAYER = wxID_HIGHEST + 1,
};


class TilesetCanvas: public wxGLCanvas
{
	public:

		TilesetCanvas(wxWindow *parent, TilesetPtr tileset, int *attribList)
			throw () :
				wxGLCanvas(parent, wxID_ANY, wxDefaultPosition,
					wxDefaultSize, 0, wxEmptyString, attribList),
				tileset(tileset)
		{
			this->SetCurrent();
			glClearColor(0.5, 0.5, 0.5, 1);
			glShadeModel(GL_FLAT);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);

			const Tileset::VC_ENTRYPTR& tiles = tileset->getItems();
			this->textureCount = tiles.size();
			this->texture = new GLuint[this->textureCount];
			glGenTextures(this->textureCount, this->texture);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GLushort r[256], g[256], b[256], a[256];;
			if (tileset->getCaps() & Tileset::HasPalette) {
				PaletteTablePtr pal = tileset->getPalette();
				for (int i = 0; i < 256; i++) {
					r[i] = ((*pal)[i].red << 8) | (*pal)[i].red;
					g[i] = ((*pal)[i].green << 8) | (*pal)[i].green;
					b[i] = ((*pal)[i].blue << 8) | (*pal)[i].blue;
					a[i] = (GLushort)-1;
				}
			} else {
				// Default palette
				for (int i = 0; i < 256; i++) {
					r[i] = (i & 4) ? ((i & 8) ? 0xFFFF : 0xAAAA) : ((i & 8) ? 0x5555 : 0x0000);
					g[i] = (i & 2) ? ((i & 8) ? 0xFFFF : 0xAAAA) : ((i & 8) ? 0x5555 : 0x0000);
					b[i] = (i & 1) ? ((i & 8) ? 0xFFFF : 0xAAAA) : ((i & 8) ? 0x5555 : 0x0000);
					a[i] = (GLushort)-1;
				}
			}
			glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
			glPixelMapusv(GL_PIXEL_MAP_I_TO_R, 256, r);
			glPixelMapusv(GL_PIXEL_MAP_I_TO_G, 256, g);
			glPixelMapusv(GL_PIXEL_MAP_I_TO_B, 256, b);
			glPixelMapusv(GL_PIXEL_MAP_I_TO_A, 256, a);

			GLuint *t = this->texture;
			for (Tileset::VC_ENTRYPTR::const_iterator i = tiles.begin();
				i != tiles.end(); i++, t++)
			{
				// Bind each texture in turn to the 2D target
				glBindTexture(GL_TEXTURE_2D, *t);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				// Load an image into the 2D target, which will affect the texture
				// previously bound to it.
				ImagePtr image = tileset->openImage(*i);
				StdImageDataPtr data = image->toStandard();
				unsigned int width, height;
				image->getDimensions(&width, &height);
				if ((width > 512) || (height > 512)) continue; // image too large
				// TODO: If this image has a palette, load it
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data.get());
				if (glGetError()) std::cerr << "[editor-tileset] GL error loading texture into id " << *t << std::endl;
			}
			glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

			this->glReset();
		}

		~TilesetCanvas()
			throw ()
		{
			glDeleteTextures(this->textureCount, this->texture);
			delete[] this->texture;
		}

		void onEraseBG(wxEraseEvent& ev)
		{
			return;
		}

		void onPaint(wxPaintEvent& ev)
		{
			wxPaintDC dummy(this);
			this->redraw();
			return;
		}

		void onResize(wxSizeEvent& ev)
		{
			this->Layout();
			this->glReset();
			return;
		}

		void glReset()
		{
			this->SetCurrent();
			wxSize s = this->GetClientSize();
			glViewport(0, 0, s.x, s.y);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, s.x/2, s.y/2, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			return;
		}

		/// Redraw the document.  Used after toggling layers.
		void redraw()
			throw ()
		{
			this->SetCurrent();
			glClear(GL_COLOR_BUFFER_BIT);

			glEnable(GL_TEXTURE_2D);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

			for (int i = 0; i < this->textureCount; i++) {
				glBindTexture(GL_TEXTURE_2D, this->texture[i]);

				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 0.0);  glVertex2i(i * 16 + 0,  0);
				glTexCoord2d(0.0, 1.0);  glVertex2i(i * 16 + 0,  16);
				glTexCoord2d(1.0, 1.0);  glVertex2i(i * 16 + 16, 16);
				glTexCoord2d(1.0, 0.0);  glVertex2i(i * 16 + 16, 0);
				glEnd();

			}

			glDisable(GL_TEXTURE_2D);

			this->SwapBuffers();
			return;
		}

	protected:
		TilesetPtr tileset;

		int textureCount;
		GLuint *texture;

		DECLARE_EVENT_TABLE();

};

BEGIN_EVENT_TABLE(TilesetCanvas, wxGLCanvas)
	EVT_PAINT(TilesetCanvas::onPaint)
	EVT_ERASE_BACKGROUND(TilesetCanvas::onEraseBG)
	EVT_SIZE(TilesetCanvas::onResize)
END_EVENT_TABLE()

class TilesetDocument: public IDocument
{
	public:
		TilesetDocument(wxWindow *parent, TilesetPtr tileset)
			throw () :
				IDocument(parent, _T("tileset")),
				tileset(tileset)
		{
			int attribList[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 0, 0};
			this->canvas = new TilesetCanvas(this, tileset, attribList);

			wxBoxSizer *s = new wxBoxSizer(wxVERTICAL);
			s->Add(this->canvas, 1, wxEXPAND);
			this->SetSizer(s);
		}

	protected:
		TilesetCanvas *canvas;
		TilesetPtr tileset;

		friend class LayerPanel;

};


TilesetEditor::TilesetEditor(wxWindow *parent)
	throw () :
		parent(parent)
{
}

std::vector<IToolPanel *> TilesetEditor::createToolPanes() const
	throw ()
{
	std::vector<IToolPanel *> panels;
	return panels;
}

IDocument *TilesetEditor::openObject(const wxString& typeMinor,
	camoto::iostream_sptr data, const wxString& filename, SuppMap supp) const
	throw ()
{
	ManagerPtr pManager = getManager();
	TilesetTypePtr pTilesetType;
	if (typeMinor.IsEmpty()) {
		wxMessageDialog dlg(this->parent,
			_T("No file type was specified for this item!"), _T("Open item"),
			wxOK | wxICON_ERROR);
		dlg.ShowModal();
		return NULL;
	} else {
		std::string strType("tls-");
		strType.append(typeMinor.ToUTF8());
		TilesetTypePtr pTestType(pManager->getTilesetTypeByCode(strType));
		if (!pTestType) {
			wxString wxtype(strType.c_str(), wxConvUTF8);
			wxString msg = wxString::Format(_T("Sorry, it is not possible to edit this "
				"tileset as the \"%s\" format is unsupported.  (No handler for \"%s\")"),
				typeMinor.c_str(), wxtype.c_str());
			wxMessageDialog dlg(this->parent, msg, _T("Open item"), wxOK | wxICON_ERROR);
			dlg.ShowModal();
			return NULL;
		}
		pTilesetType = pTestType;
	}

	assert(pTilesetType);
	std::cout << "[editor-tileset] Using handler for " << pTilesetType->getFriendlyName() << "\n";

	// Check to see if the file is actually in this format
	if (pTilesetType->isInstance(data) < TilesetType::PossiblyYes) {
		std::string friendlyType = pTilesetType->getFriendlyName();
		wxString wxtype(friendlyType.c_str(), wxConvUTF8);
		wxString msg = wxString::Format(_T("This file is supposed to be in \"%s\" "
			"format, but it seems this may not be the case.  Would you like to try "
			"opening it anyway?"), wxtype.c_str());
		wxMessageDialog dlg(this->parent, msg, _T("Open item"), wxYES_NO | wxICON_ERROR);
		int r = dlg.ShowModal();
		if (r != wxID_YES) return NULL;
	}

	// Collect any supplemental files supplied
	SuppData suppData;
	SuppMap::iterator s;

	s = supp.find(_T("dict"));
	if (s != supp.end()) suppData[SuppItem::Dictionary].stream = s->second.stream;

	s = supp.find(_T("fat"));
	if (s != supp.end()) suppData[SuppItem::FAT].stream = s->second.stream;

	s = supp.find(_T("pal"));
	if (s != supp.end()) suppData[SuppItem::Palette].stream = s->second.stream;

	// Open the tileset file
	FN_TRUNCATE fnTruncate = boost::bind<void>(truncate, filename.fn_str(), _1);
	TilesetPtr pTileset(pTilesetType->open(data, fnTruncate, suppData));
	assert(pTileset);

	return new TilesetDocument(this->parent, pTileset);
}