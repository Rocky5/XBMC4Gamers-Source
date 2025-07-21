/*
 *      Copyright (C) 2005-2013 Team XBMC
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "utils/log.h"
#include "GUIWindowGameSaves.h"
#include "Util.h"
#include "FileSystem/ZipManager.h"
#include "dialogs/GUIDialogFileBrowser.h"
#include "windows/GUIWindowFileManager.h"
#include "GUIPassword.h"
#include <fstream>
//#include "Utils/HTTP.h"  // For Download Function
#include "storage/MediaManager.h"
#include "utils/LabelFormatter.h"
#include "music/tags/MusicInfoTag.h"// todo - program tags
#include "GUIWindowManager.h"
#include "dialogs/GUIDialogProgress.h"
#include "dialogs/GUIDialogYesNo.h"
#include "FileSystem/Directory.h"
#include "FileItem.h"
#include "utils/URIUtils.h"
#include "LocalizeStrings.h"
#include "utils/CharsetConverter.h"

using namespace XFILE;

#define CONTROL_BTNVIEWASICONS     2
#define CONTROL_BTNSORTBY          3
#define CONTROL_BTNSORTASC         4
#define CONTROL_LIST              50
#define CONTROL_THUMBS            51
#define CONTROL_LABELFILES        12

CGUIWindowGameSaves::CGUIWindowGameSaves()
: CGUIMediaWindow(WINDOW_GAMESAVES, "MyGameSaves.xml")
{

}

CGUIWindowGameSaves::~CGUIWindowGameSaves()
{
}

void CGUIWindowGameSaves::GoParentFolder() {
	if (m_history.GetParentPath() == m_vecItems->GetPath())
		m_history.RemoveParentPath();
	CStdString strParent = m_history.RemoveParentPath();
	CStdString strOldPath(m_vecItems->GetPath());
	CStdString strPath(m_strParentPath);
	VECSOURCES shares;
	bool bIsSourceName = false;

	SetupShares();
	m_rootDir.GetSources(shares);

	int iIndex = CUtil::GetMatchingSource(strPath, shares, bIsSourceName);

	if (iIndex > -1) {
		if (strPath.size() == 2)
			if (strPath[1] == ':')
				URIUtils::AddSlashAtEnd(strPath);
		Update(strPath);
	} else {
		Update(strParent);
	}
	if (!g_guiSettings.GetBool("filelists.fulldirectoryhistory"))
		m_history.RemoveSelectedItem(strOldPath); //Delete current path
}

bool CGUIWindowGameSaves::OnClick(int iItem) {
	if (!m_vecItems->Get(iItem)->m_bIsFolder) {
		OnPopupMenu(iItem);
		return true;
	}

	return CGUIMediaWindow::OnClick(iItem);
}

bool CGUIWindowGameSaves::OnMessage(CGUIMessage & message) {
	switch (message.GetMessage()) {
	case GUI_MSG_CLICKED: {
		if (message.GetSenderId() == CONTROL_BTNSORTBY) {
			if (CGUIMediaWindow::OnMessage(message)) {
				LABEL_MASKS labelMasks;
				m_guiState->GetSortMethodLabelMasks(labelMasks);
				CLabelFormatter formatter("", labelMasks.m_strLabel2File);
				for (int i = 0; i < m_vecItems->Size(); ++i) {
					CFileItemPtr item = m_vecItems->Get(i);
					formatter.FormatLabel2(item.get());
				}
				return true;
			} else
				return false;
		}
	}
	case GUI_MSG_WINDOW_INIT: {
		CStdString strDestination = message.GetStringParam();
		// if (m_vecItems->GetPath() == "?")
			// m_vecItems->SetPath("E:\\udata");
		m_rootDir.SetMask("/");
		VECSOURCES shares;
		bool bIsSourceName = false;
		SetupShares();
		m_rootDir.GetSources(shares);
		int iIndex = CUtil::GetMatchingSource(strDestination, shares, bIsSourceName);
		// if bIsSourceName == True,   chances are its "Local GameSaves" or something else :D)
		if (iIndex > -1) {
			bool bDoStuff = true;
			if (shares[iIndex].m_iHasLock == 2) {
				CFileItem item(shares[iIndex]);
				if (!g_passwordManager.IsItemUnlocked( & item, "programs")) {
					m_vecItems->SetPath(""); // no u don't
					bDoStuff = false;
					CLog::Log(LOGINFO, "  Failure! Failed to unlock destination path: %s", strDestination.c_str());
				}
			}
			if (bIsSourceName) {
				m_vecItems->SetPath(shares[iIndex].strPath);
			} else if (CDirectory::Exists(strDestination)) {
				m_vecItems->SetPath(strDestination);
			}
		}
		return CGUIMediaWindow::OnMessage(message);
	}
	break;
	}
	return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowGameSaves::OnPlayMedia(int iItem) {
	CFileItemPtr pItem = m_vecItems->Get(iItem);
	CStdString strPath = pItem->GetPath();
	return true;
}

bool CGUIWindowGameSaves::GetDirectory(const CStdString & strDirectory, CFileItemList & items) {
	// Retrieve directory entries from the given directory.
	if (!m_rootDir.GetDirectory(strDirectory, items, false))
		return false;
	CLog::Log(LOGDEBUG, "CGUIWindowGameSaves::GetDirectory (%s) - Total Items: %d", strDirectory.c_str(), items.Size());
	// Set the parent path, if available.
	CStdString strParentPath;
	if (!URIUtils::GetParentPath(strDirectory, strParentPath))
		m_strParentPath = "";
	else
		m_strParentPath = strParentPath;
	CFile newfile;
	DWORD dwTick = timeGetTime();
	bool bProgressVisible = false;
	CGUIDialogProgress * m_dlgProgress = (CGUIDialogProgress * ) g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
	LABEL_MASKS labelMasks;
	m_guiState->GetSortMethodLabelMasks(labelMasks);
	CLabelFormatter formatter("", labelMasks.m_strLabel2File);
	// Process each item found in the directory.
	for (int i = 0; i < items.Size(); i++) {
		CFileItemPtr item = items[i];
		if (!bProgressVisible && timeGetTime() - dwTick > 1500 && m_dlgProgress) {
			m_dlgProgress->SetHeading(189);
			m_dlgProgress->SetLine(0, 20120);
			m_dlgProgress->StartModal();
			bProgressVisible = true;
		}
		// Remove any non-folder item.
		if (!item->m_bIsFolder) {
			items.Remove(i);
			i--;
			continue;
		}
		// Process valid game save folders.
		if (item->m_bIsFolder && !item->IsParentFolder() && !item->m_bIsShareOrDrive) {
			if (bProgressVisible) {
				m_dlgProgress->SetLine(2, item->GetLabel());
				m_dlgProgress->Progress();
			}
			// Build paths for the metadata files.
			CStdString titlemetaXBX, savemetaXBX;
			URIUtils::AddFileToFolder(item->GetPath(), "titlemeta.xbx", titlemetaXBX);
			URIUtils::AddFileToFolder(item->GetPath(), "savemeta.xbx", savemetaXBX);
			// 1 = titlemeta.xbx 2 = savemeta.xbx.
			int domode = 0;
			CStdString metadataPath;
			if (CFile::Exists(titlemetaXBX)) {
				domode = 1;
				metadataPath = titlemetaXBX;
			} else if (CFile::Exists(savemetaXBX)) {
				domode = 2;
				metadataPath = savemetaXBX;
			}
			// Only display saves that have a metadata file, I did toy with Unknown Save (TITLEID) but a just remove them instead.
			if (domode == 0) {
				items.Remove(i);
				i--;
				continue;
			}
			if (newfile.Open(metadataPath)) {
				// Read the file into a raw byte buffer.
				size_t byteLength = static_cast < size_t > (newfile.GetLength());
				std::vector < char > rawBuffer(byteLength + 1);
				newfile.Read( & rawBuffer[0], byteLength);
				rawBuffer[byteLength] = '\0';
				// This is where XBMC was having issues and not displaying names and folders all being wack!
				bool isUtf16 = false;
				bool isUtf8BOM = false;
				if (byteLength >= 2) {
					unsigned char b0 = (unsigned char) rawBuffer[0];
					unsigned char b1 = (unsigned char) rawBuffer[1];
					if ((b0 == 0xFF && b1 == 0xFE) || (b0 == 0xFE && b1 == 0xFF))
						isUtf16 = true;
					if (byteLength >= 3 && (unsigned char) rawBuffer[0] == 0xEF && (unsigned char) rawBuffer[1] == 0xBB && (unsigned char) rawBuffer[2] == 0xBF) {
						isUtf8BOM = true;
					}
				}
				CStdString strDescription;
				if (isUtf16) {
					size_t wcharLength = byteLength / sizeof(WCHAR);
					std::vector < WCHAR > wbuffer(wcharLength + 1);
					memcpy( & wbuffer[0], & rawBuffer[0], byteLength);
					wbuffer[wcharLength] = L'\0';
					g_charsetConverter.wToUTF8( & wbuffer[0], strDescription);
					if (!strDescription.IsEmpty() && strDescription[0] == 0xFEFF)
						strDescription = strDescription.Mid(1);
				} else {
					const char * textStart = & rawBuffer[0];
					if (isUtf8BOM)
						textStart += 3;
					strDescription = textStart;
				}
				CLog::Log(LOGDEBUG, "Raw Metadata Read: %s", strDescription.c_str());
				int poss = (domode == 1) ? strDescription.find("TitleName=") : strDescription.find("Name=");
				if (poss != -1) {
					int offset = (domode == 1) ? 10 : 5;
					int pose = strDescription.find("\n", poss + offset);
					if (pose != -1)
						strDescription = strDescription.Mid(poss + offset, pose - poss - offset);
					else
						strDescription = strDescription.Mid(poss + offset);
				} else {
					strDescription = "Unknown Save";
					CStdString folderName;
					CUtil::GetDirectoryName(item->GetPath(), folderName);
					strDescription += " (" + folderName + ")";
				}
				strDescription = CUtil::MakeLegalFileName(strDescription, LEGAL_NONE);
				CLog::Log(LOGDEBUG, "Extracted Name: %s", strDescription.c_str());
				newfile.Close();
				if (domode == 1) {
					// For titlemeta.xbx, check subfolders for savemeta.xbx to decide if the folder should be flattened.
					CFileItemList items2;
					m_rootDir.GetDirectory(item->GetPath(), items2, false);
					bool hasSubFolder = false;
					for (int j = 0; j < items2.Size(); ++j) {
						if (items2[j]->m_bIsFolder) {
							CStdString strPath;
							URIUtils::AddFileToFolder(items2[j]->GetPath(), "savemeta.xbx", strPath);
							if (CFile::Exists(strPath)) {
								hasSubFolder = true;
								break;
							}
						}
					}
					if (!hasSubFolder) {
						item->m_bIsFolder = false;
						item->SetPath(titlemetaXBX);
					}
				} else {
					// For savemeta.xbx, treat this item as a file.
					item->m_bIsFolder = false;
					item->SetPath(savemetaXBX);
				}
				// Set the extracted title as the label and update related properties.
				item->GetMusicInfoTag()->SetTitle(item->GetLabel());
				item->SetLabel(strDescription);
				item->SetIconImage("defaultProgram.png");
				formatter.FormatLabel2(item.get());
				item->SetLabelPreformated(true);
			} else {
				// If the metadata file cannot be opened, remove the save from the list.
				items.Remove(i);
				i--;
				continue;
			}
		}
	}
	items.SetGameSavesThumbs();
	if (bProgressVisible)
		m_dlgProgress->Close();
	return true;
}

void CGUIWindowGameSaves::GetContextButtons(int itemNumber, CContextButtons & buttons) {
	if (itemNumber < 0 || itemNumber >= m_vecItems->Size() || m_vecItems->Get(itemNumber)->m_bIsShareOrDrive)
		return;
	buttons.Add(CONTEXT_BUTTON_COPY, 115);
	buttons.Add(CONTEXT_BUTTON_MOVE, 116);
	buttons.Add(CONTEXT_BUTTON_DELETE, 117);
}

bool CGUIWindowGameSaves::OnContextButton(int itemNumber, CONTEXT_BUTTON button) {
	// Validate item number
	if (itemNumber < 0 || itemNumber >= m_vecItems->Size())
		return false;
	// Set up local memory shares
	VECSOURCES localMemShares;
	if (CDirectory::Exists("E:\\udata"))
	{
		CMediaSource shareE;
		shareE.strName = "Local E: Game Saves";
		shareE.strPath = "E:\\udata";
		shareE.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
		localMemShares.push_back(shareE);
	}
	// Additional source: F:\udata
	if (CDirectory::Exists("F:\\udata"))
	{
		CMediaSource shareF;
		shareF.strName = "Local F: Game Saves";
		shareF.strPath = "F:\\udata";
		shareF.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
		localMemShares.push_back(shareF);
	}
	// Additional source: G:\udata
	if (CDirectory::Exists("G:\\udata"))
	{
		CMediaSource shareG;
		shareG.strName = "Local G: Game Saves";
		shareG.strPath = "G:\\udata";
		shareG.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
		localMemShares.push_back(shareG);
	}
	g_mediaManager.GetLocalDrives(localMemShares);
	CVirtualDirectory dir;
	dir.SetSources(localMemShares);
	dir.GetSources(localMemShares);
	CFileItem item( * m_vecItems->Get(itemNumber));
	CStdString strFileName = URIUtils::GetFileName(item.GetPath());
	switch (button) {
	case CONTEXT_BUTTON_COPY: {
		CStdString value;
		if (!CGUIDialogFileBrowser::ShowAndGetDirectory(localMemShares, g_localizeStrings.Get(20328), value, true) ||
			!CGUIDialogYesNo::ShowAndGetInput(120, 123, 20022, 20022)) // Confirmation check
			return true;
		CStdString path;
		CStdString folderName;
		CUtil::GetDirectoryName(item.GetPath(), folderName);
		CStdString targetPath = value;
		if (targetPath.Right(1) != "\\")
			targetPath += "\\";
		targetPath += folderName;
		if (strFileName.Equals("savemeta.xbx") || strFileName.Equals("titlemeta.xbx")) {
			ProcessMetaFile(item, targetPath, path);
		} else {
			URIUtils::AddFileToFolder(targetPath, URIUtils::GetFileName(item.GetPath()), path);
		}
		item.Select(true);
		CLog::Log(LOGDEBUG, "GSM: Copy of folder confirmed for folder %s", item.GetPath().c_str());
		CGUIWindowFileManager::CopyItem( & item, path, true);
		return true;
	}
	case CONTEXT_BUTTON_DELETE: {
		CLog::Log(LOGDEBUG, "GSM: Deletion of folder confirmed for folder %s", item.GetPath().c_str());
		if (strFileName.Equals("savemeta.xbx") || strFileName.Equals("titlemeta.xbx")) {
			CStdString itemPath;
			URIUtils::GetDirectory(m_vecItems->Get(itemNumber)->GetPath(), itemPath);
			item.SetPath(itemPath);
			item.m_bIsFolder = true;
		}
		item.Select(true);
		if (CGUIWindowFileManager::DeleteItem( & item)) {
			CFile::Delete(item.GetThumbnailImage());
			Update(m_vecItems->GetPath());
		}
		return true;
	}
	case CONTEXT_BUTTON_MOVE: {
		CStdString value;
		if (!CGUIDialogFileBrowser::ShowAndGetDirectory(localMemShares, g_localizeStrings.Get(20328), value, true) ||
			!CGUIDialogYesNo::ShowAndGetInput(121, 124, 20022, 20022))
			return true;
		CStdString path;
		CStdString folderName;
		CUtil::GetDirectoryName(item.GetPath(), folderName);
		CStdString targetPath = value;
		if (targetPath.Right(1) != "\\")
			targetPath += "\\";
		targetPath += folderName;
		if (strFileName.Equals("savemeta.xbx") || strFileName.Equals("titlemeta.xbx")) {
			ProcessMetaFile(item, targetPath, path);
		} else {
			URIUtils::AddFileToFolder(targetPath, URIUtils::GetFileName(item.GetPath()), path);
		}
		item.Select(true);
		CLog::Log(LOGDEBUG, "GSM: Move of folder confirmed for folder %s to %s", item.GetPath().c_str(), targetPath.c_str());
		CGUIWindowFileManager::MoveItem(&item, path, true);
		CDirectory::Remove(item.GetPath());
		Update(m_vecItems->GetPath());
		return true;
	}
	default:
		break;
	}
	return false;
}

void CGUIWindowGameSaves::ProcessMetaFile(CFileItem & item,
	const CStdString & value, CStdString & path) {
	CStdString itemPath;
	URIUtils::GetParentPath(item.GetPath(), itemPath);
	item.SetPath(itemPath);
	item.m_bIsFolder = true;
	CFileItemList items2;
	CDirectory::GetDirectory(itemPath, items2);
	CStdString strParentName = URIUtils::GetFileName(itemPath);
	URIUtils::AddFileToFolder(value, strParentName, path);
	for (int j = 0; j < items2.Size(); ++j) {
		if (!items2[j]->m_bIsFolder) {
			CStdString strFileName = URIUtils::GetFileName(items2[j]->GetPath());
			CStdString strDest;
			URIUtils::AddFileToFolder(path, strFileName, strDest);
			CFile::Cache(items2[j]->GetPath(), strDest);
		}
	}
	URIUtils::AddFileToFolder(path, URIUtils::GetFileName(item.GetPath()), path);
}