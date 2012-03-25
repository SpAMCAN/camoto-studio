/**
 * @file   editor-map-tilepanel.hpp
 * @brief  List of available tiles for the map editor.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _EDITOR_MAP_TILEPANEL_HPP_
#define _EDITOR_MAP_TILEPANEL_HPP_

#include "editor.hpp"
#include "editor-map-document.hpp"
#include "editor-tileset-canvas.hpp"

/// Panel window allowing a tile to be selected from a tileset.
class TilePanel: public IToolPanel
{
	public:
		TilePanel(IMainWindow *parent)
			throw ();

		virtual void getPanelInfo(wxString *id, wxString *label) const
			throw ();

		virtual void switchDocument(IDocument *doc)
			throw ();

		virtual void loadSettings(Project *proj)
			throw ();

		virtual void saveSettings(Project *proj) const
			throw ();

	protected:
		MapDocument *doc;
		TilesetCanvas *canvas;

		enum {
			IDC_LAYER = wxID_HIGHEST + 1,
		};

		DECLARE_EVENT_TABLE();
};

#endif // _EDITOR_MAP_TILEPANEL_HPP_