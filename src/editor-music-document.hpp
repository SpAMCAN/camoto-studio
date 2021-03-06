/**
 * @file   editor-music-document.hpp
 * @brief  Music IDocument interface.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _EDITOR_MUSIC_DOCUMENT_HPP_
#define _EDITOR_MUSIC_DOCUMENT_HPP_

#include <boost/thread/thread.hpp>
#include <camoto/gamemusic.hpp>
#include "audio.hpp"

class MusicDocument;

#include "audio.hpp"
#include "editor-music.hpp"
#include "editor-music-eventpanel.hpp"

typedef std::vector<camoto::gamemusic::EventPtr> EventVector;

class MusicDocument: public IDocument
{
	public:
		MusicDocument(MusicEditor *editor, camoto::gamemusic::MusicPtr music,
			fn_write fnWriteMusic);

		~MusicDocument();

		virtual void save();

		void onSeekPrev(wxCommandEvent& ev);
		void onPlay(wxCommandEvent& ev);
		void onPause(wxCommandEvent& ev);
		void onSeekNext(wxCommandEvent& ev);
		void onZoomIn(wxCommandEvent& ev);
		void onZoomNormal(wxCommandEvent& ev);
		void onZoomOut(wxCommandEvent& ev);
		void onImport(wxCommandEvent& ev);
		void onExport(wxCommandEvent& ev);
		void onResize(wxSizeEvent& ev);
		void updatePlaybackStatus(wxCommandEvent& ev);

		/// Push the current scroll and zoom settings to each channel and redraw
		void pushViewSettings();

	protected:
		MusicEditor *editor;
		camoto::gamemusic::MusicPtr music;
		fn_write fnWriteMusic;
		MusicStreamPtr musicStream;

		int optimalTicksPerRow; ///< Cache best value for ticksPerRow (for zoom reset)
		unsigned int ticksPerRow;        ///< Current zoom level for all channels

		typedef std::vector<EventPanel *> EventPanelVector;
		EventPanelVector channelPanels;

		wxFont font;         ///< Font to use for event text
		int fontWidth;       ///< Width of each char in pixels
		int fontHeight;      ///< Height of each char/line in pixels
		int halfHeight;      ///< Number of char rows in half a screen (for positioning highlight row)

		wxStaticText *labelPlayback;  ///< Playback position

		/// Song position most recently played through the speakers.
		camoto::gamemusic::Playback::Position lastAudiblePos;

		friend class PlayerThread;
		friend class InstrumentListPanel;
		friend class EventPanel;

		enum {
			IDC_SEEK_PREV = wxID_HIGHEST + 1,
			IDC_PLAY,
			IDC_PAUSE,
			IDC_SEEK_NEXT,
			IDC_IMPORT,
			IDC_EXPORT,
		};
		DECLARE_EVENT_TABLE();
};

#endif // _EDITOR_MUSIC_DOCUMENT_HPP_
