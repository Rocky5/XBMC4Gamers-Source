/*
*      Copyright (C) 2005-2013 Team XBMC
*      http://xbmc.org
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
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#include "programs/GUIWindowPrograms.h"
#include "Util.h"
#include "Shortcut.h"
#include "FileSystem/HDDirectory.h"
#include "GUIPassword.h"
#include "dialogs/GUIDialogTrainerSettings.h"
#include "dialogs/GUIDialogMediaSource.h"
#include "xbox/xbeheader.h"
#include "utils/Trainer.h"
#include "utils/LabelFormatter.h"
#include "Autorun.h"
#include "settings/Profile.h"
#include "GUIWindowManager.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogKeyboard.h"
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
#include "FileSystem/RarManager.h"
#include "FileItem.h"
#include "utils/URIUtils.h"
#include "LocalizeStrings.h"
#include "utils/log.h"
#include "interfaces/Builtins.h"
#include "settings/AdvancedSettings.h"
#include "Application.h"
#include "dialogs/GUIDialogOK.h"
#include "settings/GUISettings.h"
#include "XMLUtils.h"
#include "GUIInfoManager.h"

using namespace XFILE;
using namespace std;

#define CONTROL_BTNVIEWASICONS 2
#define CONTROL_BTNSORTBY      3
#define CONTROL_BTNSORTASC     4
#define CONTROL_LIST          50
#define CONTROL_THUMBS        51
#define CONTROL_LABELFILES    12

CStdString check_xbe_path;
CStdString patched_xbe_path;
CStdString ws_xbe_path;
CStdString up_xbe_path;
CStdString xhd_xbe_path;
CStdString hd_480_xbe_path;
CStdString hd_720_xbe_path;
CStdString xresizer_xbe_path;
CStdString altXBECheck;
int foundExtraXBE = 0;
bool UpdateSynopsisInfo = false;

CGUIWindowPrograms::CGUIWindowPrograms(void)
: CGUIMediaWindow(WINDOW_PROGRAMS, "MyPrograms.xml")
{
	m_thumbLoader.SetObserver(this);
	m_dlgProgress = NULL;
	m_rootDir.AllowNonLocalSources(false); // no nonlocal shares for this window please
}


CGUIWindowPrograms::~CGUIWindowPrograms(void)
{
}

bool CGUIWindowPrograms::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_DEINIT:
			if (m_thumbLoader.IsLoading())
				m_thumbLoader.StopThread();
			m_database.Close();
			break;

		case GUI_MSG_WINDOW_INIT:
			// Disable autoregion if not stock kernels or M8+.
			m_iRegionSet = 0;
			m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
			
			if (m_vecItems->GetPath() == "?" && message.GetStringParam().IsEmpty())
				message.SetStringParam(g_settings.m_defaultProgramSource);

			m_database.Open();
			return CGUIMediaWindow::OnMessage(message);

		case GUI_MSG_CLICKED:
			if (message.GetSenderId() == CONTROL_BTNSORTBY)
			{
				if (CGUIMediaWindow::OnMessage(message))
				{
					LABEL_MASKS labelMasks;
					m_guiState->GetSortMethodLabelMasks(labelMasks);
					CLabelFormatter formatter("", labelMasks.m_strLabel2File);

					for (int i = 0; i < m_vecItems->Size(); ++i)
					{
						CFileItemPtr item = m_vecItems->Get(i);
						if (item->IsShortCut())
							formatter.FormatLabel2(item.get());
					}
					return true;
				}
				return false;
			}

			if (m_viewControl.HasControl(message.GetSenderId()))
			{
				if (message.GetParam1() == ACTION_PLAYER_PLAY)
				{
					OnPlayMedia(m_viewControl.GetSelectedItem());
					return true;
				}
			}
			break;
	}

	return CGUIMediaWindow::OnMessage(message);
}

void CGUIWindowPrograms::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	if (itemNumber < 0 || itemNumber >= m_vecItems->Size())
		return;

	CFileItemPtr item = m_vecItems->Get(itemNumber);
	bool KioskMode = g_infoManager.GetBool(g_infoManager.TranslateString("skin.hassetting(kioskmode)")) == 1;

	if (item)
	{
		if (m_vecItems->IsVirtualDirectoryRoot())
		{
			CGUIDialogContextMenu::GetContextButtons("programs", item, buttons);
		}
		else
		{
			if (item->IsXBE() || item->IsShortCut())
			{
				if (!g_guiSettings.GetBool("mygames.gamesaltsynpsisbutton"))
					buttons.Add(CONTEXT_BUTTON_SYNOPSIS, g_localizeStrings.Get(32019));
				
				CStdString strLaunch = g_localizeStrings.Get(518); // Launch
				if (g_guiSettings.GetBool("myprograms.gameautoregion"))
				{
					int iRegion = GetRegion(itemNumber);
					if (iRegion == VIDEO_NTSCM)
						strLaunch += " (NTSC-M)";
					else if (iRegion == VIDEO_NTSCJ)
						strLaunch += " (NTSC-J)";
					else if (iRegion == VIDEO_PAL50)
						strLaunch += " (PAL)";
					else if (iRegion == VIDEO_PAL60)
						strLaunch += " (PAL-60)";
				}
				buttons.Add(CONTEXT_BUTTON_LAUNCH, strLaunch);

				if (g_guiSettings.GetBool("myprograms.gameautoregion"))
					buttons.Add(CONTEXT_BUTTON_LAUNCH_IN, 519); // launch in video mode

				// Add new menu entry for custom xbe files
				foundExtraXBE = 0, check_xbe_path = "", hd_480_xbe_path = "", hd_720_xbe_path = "", patched_xbe_path = "", ws_xbe_path = "", up_xbe_path = "", xhd_xbe_path = "", xresizer_xbe_path = "";
				URIUtils::GetParentPath(item->GetPath(), check_xbe_path);
				
				if (CFile::Exists(check_xbe_path+"default_orig.xbe")) {   
					// Unpatched Patched XBE
					foundExtraXBE = 1;
					up_xbe_path = (check_xbe_path+"default_orig.xbe");
				}
				if (CFile::Exists(check_xbe_path+"default480p.xbe")) {
					// 720p Patched XBE
					foundExtraXBE = 1;
					hd_480_xbe_path = (check_xbe_path+"default480p.xbe");
				}
				if (CFile::Exists(check_xbe_path+"default720p.xbe")) {
					// 720p Patched XBE
					foundExtraXBE = 1;
					hd_720_xbe_path = (check_xbe_path+"default720p.xbe");
				}
				if (CFile::Exists(check_xbe_path+"default_p.xbe")) {
					// Any Patched XBE
					foundExtraXBE = 1;
					patched_xbe_path = (check_xbe_path+"default_p.xbe");
				}
				if (CFile::Exists(check_xbe_path+"defaultws.xbe")) {
					// Widescreen Patched XBE
					foundExtraXBE = 1;
					ws_xbe_path = (check_xbe_path+"defaultws.xbe");
				}
				if (CFile::Exists(check_xbe_path+"defaultxhd.xbe")) {
					// XHD Patched XBE
					foundExtraXBE = 1;
					xhd_xbe_path = (check_xbe_path+"defaultxhd.xbe");         
				}
				if (CFile::Exists(check_xbe_path+"xresizer.xbe")) {
					// Resize window XBE used by homebrew
					foundExtraXBE = 1;
					xresizer_xbe_path = (check_xbe_path+"xresizer.xbe");
				}
				// Add new list if alternative xbes are found
				if (foundExtraXBE) {
					CStdString strLaunchExtraXBEs = g_localizeStrings.Get(32009);
					altXBECheck = m_database.GetXBEType(item->GetPath());
					if (altXBECheck == "default_orig.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32010) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "default480p.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32011) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "default720p.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32012) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "default_p.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32013) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "defaultws.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32014) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "defaultxhd.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32015) + " " + g_localizeStrings.Get(305);
					if (altXBECheck == "xresizer.xbe")
						strLaunchExtraXBEs = g_localizeStrings.Get(32016) + " " + g_localizeStrings.Get(305);
					
					buttons.Add(CONTEXT_BUTTON_LAUNCH_ALTXBE, strLaunchExtraXBEs);
				}
				
				if (KioskMode && (item->IsXBE() || item->IsShortCut()))
				{
					buttons.Add(CONTEXT_BUTTON_EDIT_REFRESH, g_localizeStrings.Get(32017)); // Refresh synopsis or edit names
				}
				buttons.Add(CONTEXT_BUTTON_TRAINERS, g_localizeStrings.Get(32018)); // Refresh All Synopsis Info
				
				DWORD dwTitleId = CUtil::GetXbeID(item->GetPath());
				CStdString strTitleID, strGameSavepath;
				strTitleID.Format("%08X", dwTitleId);
				URIUtils::AddFileToFolder("E:\\udata\\", strTitleID, strGameSavepath);
				if (CDirectory::Exists(strGameSavepath) && KioskMode)
					buttons.Add(CONTEXT_BUTTON_GAMESAVES, 20322); // Goto GameSaves
				
			}

			// if (KioskMode)
				// buttons.Add(CONTEXT_BUTTON_GOTO_ROOT, 20128); // Go to Root
		}
	}

	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);

	if (item && KioskMode)
		buttons.Add(CONTEXT_BUTTON_SETTINGS, 5); // Settings
}

bool CGUIWindowPrograms::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item = (itemNumber >= 0 && itemNumber < m_vecItems->Size()) ? m_vecItems->Get(itemNumber) : CFileItemPtr();

	if (item && m_vecItems->IsVirtualDirectoryRoot())
	{
		if (CGUIDialogContextMenu::OnContextButton("programs", item, button))
		{
			Update("");
			return true;
		}
	}

	switch (button)
	{
		case CONTEXT_BUTTON_RENAME:
		{
			CStdString strDescription;
			CShortcut cut;
			if (item->IsShortCut())
			{
				cut.Create(item->GetPath());
				strDescription = cut.m_strLabel;
			}
			else
			{
				strDescription = item->GetLabel();
			}

			if (CGUIDialogKeyboard::ShowAndGetInput(strDescription, g_localizeStrings.Get(16008), false))
			{
				if (item->IsShortCut())
				{
					cut.m_strLabel = strDescription;
					cut.Save(item->GetPath());
				}
				else
				{
					if (g_guiSettings.GetBool("mygames.gamesynopsisinfo"))
					{
						CUtil::SetXBEDescription(item->GetPath(), strDescription);
					}
					m_database.SetDescription(item->GetPath(), strDescription);
				}
				Update(m_vecItems->GetPath());
			}
			return true;
		}

		case CONTEXT_BUTTON_TRAINER_OPTIONS:
		{
			DWORD dwTitleId = CUtil::GetXbeID(item->GetPath());
			if (CGUIDialogTrainerSettings::ShowForTitle(dwTitleId, &m_database))
			{
				Update(m_vecItems->GetPath());
			}
			return true;
		}

		case CONTEXT_BUTTON_SCAN_TRAINERS:
		{
			PopulateTrainersList();
			Update(m_vecItems->GetPath());
			return true;
		}

		case CONTEXT_BUTTON_SETTINGS:
			g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYPROGRAMS);
			return true;

		case CONTEXT_BUTTON_GOTO_ROOT:
			Update("");
			return true;

		case CONTEXT_BUTTON_LAUNCH:
			OnClick(itemNumber);
			return true;
		
		case CONTEXT_BUTTON_SYNOPSIS:
		{
			CBuiltins::Execute("ActivateWindow(1101)");
			return true;
		}
		
		case CONTEXT_BUTTON_SYNOPSISUPDATE:
		{
			DWORD dwTitleId = CUtil::GetXbeID(item->GetPath());
			m_database.UpdateProgramInfo(item.get(), dwTitleId);
			Update(m_vecItems->GetPath());
			return true;
		}

		case CONTEXT_BUTTON_SYNOPSISUPDATEALL:
		{
			UpdateSynopsisInfo = true;
			Update(m_vecItems->GetPath());
			return true;
		}

		case CONTEXT_BUTTON_GAMESAVES:
		{
			CStdString strTitleID, strGameSavepath;
			strTitleID.Format("%08X", CUtil::GetXbeID(item->GetPath()));
			URIUtils::AddFileToFolder("E:\\udata\\", strTitleID, strGameSavepath);
			g_windowManager.ActivateWindow(WINDOW_GAMESAVES, strGameSavepath);
			return true;
		}

		case CONTEXT_BUTTON_LAUNCH_IN:
			OnChooseVideoModeAndLaunch(itemNumber);
			return true;

		case CONTEXT_BUTTON_LAUNCH_ALTXBE:
			OnChooseAltXBEAndLaunch(itemNumber);
			return true;

		case CONTEXT_BUTTON_TRAINERS:
			OnChooseTrainersMenu(itemNumber);
			return true;

		case CONTEXT_BUTTON_EDIT_REFRESH:
			OnChooseEditRefreshMenu(itemNumber);
			return true;

		default:
			break;
	}

	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

bool CGUIWindowPrograms::OnChooseEditRefreshMenu(int item)
{
	if (item < 0 || item >= m_vecItems->Size()) return false;

	// Grab the context menu
	CGUIDialogContextMenu* pMenu = (CGUIDialogContextMenu*)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if (!pMenu) return false;

	pMenu->Initialize();

	int btn_1 = -1, btn_2 = -1, btn_3 = -1;

	if (!g_guiSettings.GetBool("mygames.gamesynopsisinfo"))
		btn_1 = pMenu->AddButton(g_localizeStrings.Get(16105)); // rename
	else
		btn_1 = pMenu->AddButton(g_localizeStrings.Get(520)); // edit xbe title
	btn_2 = pMenu->AddButton(g_localizeStrings.Get(32006)); // Refresh Synopsis Info
	btn_3 = pMenu->AddButton(g_localizeStrings.Get(32007)); // Refresh All Synopsis Info
	
	// This will force centre it on the screen
	pMenu->SetPosition(640 - (pMenu->GetWidth() / 2), 360 - (pMenu->GetHeight() / 2));
	pMenu->DoModal();
	int btnid = pMenu->GetButton();

	if (btnid == btn_1 && btn_1 != -1) {
		return OnContextButton(item, CONTEXT_BUTTON_RENAME);
	}
	if (btnid == btn_2 && btn_2 != -1) {
		return OnContextButton(item, CONTEXT_BUTTON_SYNOPSISUPDATE);
	}
	if (btnid == btn_3 && btn_3 != -1) {
		return OnContextButton(item, CONTEXT_BUTTON_SYNOPSISUPDATEALL);
	}

	return true;
}

bool CGUIWindowPrograms::OnChooseTrainersMenu(int item)
{
	if (item < 0 || item >= m_vecItems->Size()) return false;

	// Grab the context menu
	CGUIDialogContextMenu* pMenu = (CGUIDialogContextMenu*)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if (!pMenu) return false;

	pMenu->Initialize();

	int btn_1 = -1, btn_2 = -1;
	
	DWORD dwTitleId = CUtil::GetXbeID(m_vecItems->Get(item)->GetPath());
	if (m_database.ItemHasTrainer(dwTitleId))
		btn_1 = pMenu->AddButton(g_localizeStrings.Get(12015)); // trainer options

	btn_2 = pMenu->AddButton(g_localizeStrings.Get(12012)); // scan trainers

	// This will force centre it on the screen
	pMenu->SetPosition(640 - (pMenu->GetWidth() / 2), 360 - (pMenu->GetHeight() / 2));
	pMenu->DoModal();
	int btnid = pMenu->GetButton();

	if (btnid == btn_1 && btn_1 != -1) {
		return OnContextButton(item, CONTEXT_BUTTON_TRAINER_OPTIONS);
	}    
	if (btnid == btn_2 && btn_2 != -1) {
		return OnContextButton(item, CONTEXT_BUTTON_SCAN_TRAINERS);
	}

	return true;
}


bool CGUIWindowPrograms::OnChooseAltXBEAndLaunch(int item)
{
	if (item < 0 || item >= m_vecItems->Size()) return false;

	// Grab the context menu
	CGUIDialogContextMenu* pMenu = (CGUIDialogContextMenu*)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if (!pMenu) return false;

	pMenu->Initialize();

	struct MenuItem {
		CStdString* path;
		int localizationId;
		int buttonId;
	};

	MenuItem menuItems[] = {
		{&altXBECheck, 192, -1},
		{&up_xbe_path, 32010, -1},
		{&hd_480_xbe_path, 32011, -1},
		{&hd_720_xbe_path, 32012, -1},
		{&patched_xbe_path, 32013, -1},
		{&ws_xbe_path, 32014, -1},
		{&xhd_xbe_path, 32015, -1},
		{&xresizer_xbe_path, 32016, -1}
	};

	int arraySize = sizeof(menuItems) / sizeof(menuItems[0]);

	for (int i = 0; i < arraySize; ++i) {
		if (!menuItems[i].path->IsEmpty()) {
			menuItems[i].buttonId = pMenu->AddButton(g_localizeStrings.Get(menuItems[i].localizationId));
		}
	}

	// This will force centre it on the screen
	pMenu->SetPosition(640 - (pMenu->GetWidth() / 2), 360 - (pMenu->GetHeight() / 2));
	pMenu->DoModal();
	int btnid = pMenu->GetButton();
	CStdString xbe_path = m_vecItems->Get(item)->GetPath();

	for (int i = 0; i < arraySize; ++i) {
		if (btnid == menuItems[i].buttonId) {
			if (btnid == menuItems[0].buttonId) {
				m_database.SetXBEType(xbe_path, "");
			} else {
				m_database.SetXBEType(xbe_path, *menuItems[i].path);
			}
			break;
		}
	}

	return btnid > -1 ? false : true;
}

bool CGUIWindowPrograms::OnChooseVideoModeAndLaunch(int item)
{
	if (item < 0 || item >= m_vecItems->Size()) return false;

	// Grab the context menu
	CGUIDialogContextMenu* pMenu = (CGUIDialogContextMenu*)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
	if (!pMenu) return false;

	pMenu->Initialize();

	struct MenuItem {
		CStdString text;
		int regionValue;
		int buttonId;
	};

	int iRegion = GetRegion(item, true);

	MenuItem menuItems[] = {
		{"PAL", VIDEO_PAL50, -1},
		{"NTSC-M", VIDEO_NTSCM, -1},
		{"NTSC-J", VIDEO_NTSCJ, -1},
		{"PAL-60", VIDEO_PAL60, -1}
	};

	if (iRegion == VIDEO_NTSCM) menuItems[1].text += " (default)";
	if (iRegion == VIDEO_NTSCJ) menuItems[2].text += " (default)";
	if (iRegion == VIDEO_PAL50) menuItems[0].text += " (default)";

	int arraySize = sizeof(menuItems) / sizeof(menuItems[0]);

	for (int i = 0; i < arraySize; ++i) {
		menuItems[i].buttonId = pMenu->AddButton(menuItems[i].text);
	}

	// This will force centre it on the screen
	pMenu->SetPosition(640 - (pMenu->GetWidth() / 2), 360 - (pMenu->GetHeight() / 2));
	pMenu->DoModal();
	int btnid = pMenu->GetButton();

	for (int i = 0; i < arraySize; ++i) {
		if (btnid == menuItems[i].buttonId) {
			m_iRegionSet = menuItems[i].regionValue;
			int regionValue = 0;
			switch (menuItems[i].regionValue) {
				case VIDEO_PAL50: regionValue = 4; break;
				case VIDEO_NTSCM: regionValue = 1; break;
				case VIDEO_NTSCJ: regionValue = 2; break;
				case VIDEO_PAL60: regionValue = 8; break;
				default: break;
			}
			m_database.SetRegion(m_vecItems->Get(item)->GetPath(), regionValue);
			break;
		}
	}

	return btnid > -1 ? OnClick(item) : true;
}

bool CGUIWindowPrograms::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(*m_vecItems);
	return true;
}


bool CGUIWindowPrograms::OnPlayMedia(int iItem)
{
	if (iItem < 0 || iItem >= static_cast<int>(m_vecItems->Size()))
		return false;

	CFileItemPtr pItem = m_vecItems->Get(iItem);

	if (pItem->IsDVD())
		return MEDIA_DETECT::CAutorun::PlayDisc();

	if (pItem->m_bIsFolder)
		return false;

	if (!CFile::Exists(pItem->GetPath()) && g_guiSettings.GetBool("mygames.gamesfasterparsing"))
	{
		CGUIDialogOK *dialog = static_cast<CGUIDialogOK*>(g_windowManager.GetWindow(WINDOW_DIALOG_OK));
		if (dialog)
		{
			dialog->SetHeading(g_localizeStrings.Get(33049));
			dialog->SetLine(0, "");
			dialog->SetLine(1, "No default.xbe found for this item.");
			dialog->SetLine(2, "Disable fast game parsing if this is a folder or you will have to fix this game.");
			dialog->DoModal();
		}
		return false;
	}

	// launch xbe...
	char szPath[1024];
	char szParameters[1024] = {0};

	int iRegion = m_iRegionSet ? m_iRegionSet : GetRegion(iItem);
	DWORD dwTitleId = 0;
	
	if (!pItem->IsOnDVD())
		dwTitleId = m_database.GetTitleId(pItem->GetPath());
	if (!dwTitleId)
		dwTitleId = CUtil::GetXbeID(pItem->GetPath());

	CStdString strTrainer = m_database.GetActiveTrainer(dwTitleId);
	if (!strTrainer.IsEmpty())
	{
		CTrainer trainer;
		if (trainer.Load(strTrainer))
		{
			m_database.GetTrainerOptions(strTrainer, dwTitleId, trainer.GetOptions(), trainer.GetNumberOfOptions());
			CUtil::InstallTrainer(trainer);
		}
	}

	m_database.Close();
	strcpy(szPath, pItem->GetPath().c_str());

	if (pItem->IsShortCut())
	{
		CUtil::RunShortcut(pItem->GetPath().c_str());
		return false;
	}

	CUtil::RunXBE(szPath, szParameters[0] ? szParameters : NULL, F_VIDEO(iRegion));
	return true;
}

int CGUIWindowPrograms::GetRegion(int iItem, bool bReload)
{
	if (!g_guiSettings.GetBool("myprograms.gameautoregion"))
		return 0;

	int iRegion;

	CStdString itemPath = m_vecItems->Get(iItem)->GetPath();

	if (bReload || m_vecItems->Get(iItem)->IsOnDVD())
	{
		CXBE xbe;
		iRegion = xbe.ExtractGameRegion(itemPath);
	}
	else
	{
		m_database.Open();
		iRegion = m_database.GetRegion(itemPath);
		m_database.Close();
	}

	if (iRegion == -1)
	{
		if (g_guiSettings.GetBool("myprograms.gameautoregion"))
		{
			CXBE xbe;
			iRegion = xbe.ExtractGameRegion(itemPath);
			if (iRegion < 1 || iRegion > 7)
				iRegion = 0;

			m_database.SetRegion(itemPath, iRegion);
		}
		else
		{
			iRegion = 0;
		}
	}

	return CXBE::FilterRegion(iRegion, bReload);
}

void CGUIWindowPrograms::PopulateTrainersList()
{
	CDirectory directory;
	CFileItemList trainers;
	CFileItemList archives;
	CFileItemList inArchives;

	std::vector<CStdString> vecTrainerPath;
	m_database.GetAllTrainers(vecTrainerPath);

	CGUIDialogProgress* m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
	m_dlgProgress->SetLine(0, "");
	m_dlgProgress->SetLine(1, 12023);
	m_dlgProgress->SetLine(2, "");
	m_dlgProgress->StartModal();
	m_dlgProgress->SetHeading(12012);
	m_dlgProgress->ShowProgressBar(true);
	m_dlgProgress->Progress();

	bool bBreak = false;
	bool bDatabaseState = m_database.IsOpen();
	if (!bDatabaseState)
		m_database.Open();
	m_database.BeginTransaction();

	// Remove dead items
	for (unsigned int i = 0; i < vecTrainerPath.size(); ++i)
	{
		m_dlgProgress->SetPercentage((int)((float)i / vecTrainerPath.size() * 100.f));
		CStdString strLine;
		strLine.Format("%s %i / %i", g_localizeStrings.Get(12013).c_str(), i + 1, vecTrainerPath.size());
		m_dlgProgress->SetLine(2, strLine);
		m_dlgProgress->Progress();

		if (!CFile::Exists(vecTrainerPath[i]) || vecTrainerPath[i].find(g_guiSettings.GetString("myprograms.trainerpath", false)) == -1)
			m_database.RemoveTrainer(vecTrainerPath[i]);

		if (m_dlgProgress->IsCanceled())
		{
			bBreak = true;
			m_database.RollbackTransaction();
			break;
		}
	}

	if (!bBreak)
	{
		CLog::Log(LOGDEBUG, "trainerpath %s", g_guiSettings.GetString("myprograms.trainerpath", false).c_str());
		directory.GetDirectory(g_guiSettings.GetString("myprograms.trainerpath").c_str(), trainers, ".xbtf|.etm");
		if (g_guiSettings.GetString("myprograms.trainerpath", false).IsEmpty())
		{
			m_database.RollbackTransaction();
			m_dlgProgress->Close();
			return;
		}

		directory.GetDirectory(g_guiSettings.GetString("myprograms.trainerpath").c_str(), archives, ".rar|.zip", false);

		// Process archives
		for (int i = 0; i < archives.Size(); ++i)
		{
			if (stricmp(URIUtils::GetExtension(archives[i]->GetPath()), ".rar") == 0)
			{
				g_RarManager.GetFilesInRar(inArchives, archives[i]->GetPath(), false);
				CHDDirectory dir;
				dir.SetMask(".xbtf|.etm");
				for (int j = 0; j < inArchives.Size(); ++j)
				{
					if (dir.IsAllowed(inArchives[j]->GetPath()))
					{
						CFileItemPtr item(new CFileItem(*inArchives[j]));
						CStdString strPathInArchive = item->GetPath();
						CStdString path;
						URIUtils::CreateArchivePath(path, "rar", archives[i]->GetPath(), strPathInArchive, "");
						item->SetPath(path);
						trainers.Add(item);
					}
				}
			}
			else if (stricmp(URIUtils::GetExtension(archives[i]->GetPath()), ".zip") == 0)
			{
				CStdString strZipPath;
				URIUtils::CreateArchivePath(strZipPath, "zip", archives[i]->GetPath(), "");
				CFileItemList zipTrainers;
				directory.GetDirectory(strZipPath, zipTrainers, ".etm|.xbtf");
				for (int j = 0; j < zipTrainers.Size(); ++j)
				{
					CFileItemPtr item(new CFileItem(*zipTrainers[j]));
					trainers.Add(item);
				}
			}
		}

		if (!m_dlgProgress)
			m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
		m_dlgProgress->SetPercentage(0);
		m_dlgProgress->ShowProgressBar(true);

		CLog::Log(LOGDEBUG, "# trainers %i", trainers.Size());
		m_dlgProgress->SetLine(0, "");

		// Remove folders from list
		for (int j = 0; j < trainers.Size();)
		{
			if (trainers[j]->m_bIsFolder)
				trainers.Remove(j);
			else
				j++;
		}

		for (int i = 0; i < trainers.Size(); ++i)
		{
			CLog::Log(LOGDEBUG, "found trainer %s", trainers[i]->GetPath().c_str());
			m_dlgProgress->SetPercentage((int)((float)(i) / trainers.Size() * 100.f));
			CStdString strLine;
			strLine.Format("%s %i / %i", g_localizeStrings.Get(12013).c_str(), i + 1, trainers.Size());
			m_dlgProgress->SetLine(1, strLine);
			m_dlgProgress->SetLine(2, "");
			m_dlgProgress->Progress();

			if (m_database.HasTrainer(trainers[i]->GetPath()))
				continue;

			CTrainer trainer;
			if (trainer.Load(trainers[i]->GetPath()))
			{
				m_dlgProgress->SetLine(2, trainer.GetName());
				m_dlgProgress->Progress();

				unsigned int iTitle1, iTitle2, iTitle3;
				trainer.GetTitleIds(iTitle1, iTitle2, iTitle3);

				if (iTitle1)
					m_database.AddTrainer(iTitle1, trainers[i]->GetPath());
				if (iTitle2)
					m_database.AddTrainer(iTitle2, trainers[i]->GetPath());
				if (iTitle3)
					m_database.AddTrainer(iTitle3, trainers[i]->GetPath());
			}

			if (m_dlgProgress->IsCanceled())
			{
				m_database.RollbackTransaction();
				break;
			}
		}
	}

	m_database.CommitTransaction();
	m_dlgProgress->Close();

	if (!bDatabaseState)
		m_database.Close();
	else
		Update(m_vecItems->GetPath());
}

bool CGUIWindowPrograms::GetDirectory(const CStdString &strDirectory, CFileItemList &items)
{
	bool bFlattened = false;

	if (!bFlattened && !CGUIMediaWindow::GetDirectory(strDirectory, items))
		return false;

	if (items.IsVirtualDirectoryRoot())
	{
		items.SetLabel("");
		return true;
	}

	m_database.BeginTransaction();
	bool bProgressVisible = false;
	CStdString description, defaultXBE;
	m_dlgProgress->SetPercentage(0);
	m_dlgProgress->ShowProgressBar(true);
	CBuiltins::Execute("Skin.SetString(disablecancel,true)");

	for (int i = 0; i < items.Size(); i++)
	{
		m_dlgProgress->SetPercentage((int)((float)(i) / items.Size() * 100.f));
		CFileItemPtr item = items[i];
		URIUtils::AddFileToFolder(item->GetPath(), "default.xbe", defaultXBE);

		if (!g_guiSettings.GetBool("mygames.gamesfasterparsing") || 
			defaultXBE.find("\\Media\\") != CStdString::npos || 
			defaultXBE.find("\\Movies\\") != CStdString::npos || 
			defaultXBE.find("\\TV Shows\\") != CStdString::npos)
		{
			if (item->m_bIsFolder && !item->IsParentFolder())
			{
				if (CFile::Exists(defaultXBE))
				{
					item->SetPath(defaultXBE);
					item->m_bIsFolder = false;
				}
			}
			else if (item->IsShortCut())
			{
				CShortcut cut;
				if (cut.Create(item->GetPath()))
				{
					CStdString shortcutPath = item->GetPath();
					item->SetPath(cut.m_strPath);
					item->SetThumbnailImage(cut.m_strThumb);

					LABEL_MASKS labelMasks;
					m_guiState->GetSortMethodLabelMasks(labelMasks);
					CLabelFormatter formatter("", labelMasks.m_strLabel2File);
					if (!cut.m_strLabel.IsEmpty())
					{
						item->SetLabel(cut.m_strLabel);
						__stat64 stat;
						if (CFile::Stat(item->GetPath(), &stat) == 0)
							item->m_dwSize = stat.st_size;

						formatter.FormatLabel2(item.get());
						item->SetLabelPreformated(true);
					}
				}
			}
		}
		else
		{
			item->SetPath(defaultXBE);
			item->m_bIsFolder = false;
		}

		if (item->IsXBE())
		{
			DWORD dwTitleID = m_database.GetProgramInfo(item.get());

			if (!dwTitleID)
			{
				if (!bProgressVisible)
				{
					m_dlgProgress->SetHeading(189);
					m_dlgProgress->SetLine(0, "");
					m_dlgProgress->SetLine(1, 20120);
					m_dlgProgress->SetLine(2, "");
					m_dlgProgress->StartModal();
					bProgressVisible = true;
				}

				if (CUtil::GetXBEDescription(item->GetPath(), description) && !item->IsLabelPreformated() && !item->GetLabel().IsEmpty())
				{
					item->SetLabel(description);
					if (bProgressVisible)
					{
						m_dlgProgress->SetLine(2, description);
						m_dlgProgress->Progress();
					}
				}

				dwTitleID = CUtil::GetXbeID(item->GetPath());
				m_database.AddProgramInfo(item.get(), dwTitleID);
			}

			if (UpdateSynopsisInfo)
			{
				dwTitleID = CUtil::GetXbeID(item->GetPath());
				m_dlgProgress->SetHeading(189);
				m_dlgProgress->SetLine(0, "");
				m_dlgProgress->SetLine(1, g_localizeStrings.Get(32008));
				m_dlgProgress->SetLine(2, "");
				m_dlgProgress->StartModal();
				m_dlgProgress->SetLine(2, item->GetLabel());
				m_dlgProgress->Progress();
				bProgressVisible = true;
				m_database.UpdateProgramInfo(item.get(), dwTitleID);
			}
			else
			{
				if (m_database.ItemHasTrainer(dwTitleID))
				{
					if (!m_database.GetActiveTrainer(dwTitleID).IsEmpty())
						item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_TRAINED);
					else
						item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_HAS_TRAINER);
				}
			}
		}
		if (bProgressVisible)
			m_database.GetProgramInfo(item.get());
	}

	m_database.CommitTransaction();

	items.SetThumbnailImage("");
	items.SetCachedProgramThumbs();
	items.SetCachedProgramThumb();
	if (!items.HasThumbnail())
		items.SetUserProgramThumb();

	if (bProgressVisible)
	{
		m_dlgProgress->Close();
		UpdateSynopsisInfo = false;
		CBuiltins::Execute("Skin.SetString(disablecancel,)");
	}

	return true;
}

CStdString CGUIWindowPrograms::GetStartFolder(const CStdString &dir)
{
	SetupShares();
	VECSOURCES shares;
	m_rootDir.GetSources(shares);

	bool bIsSourceName = false;
	int iIndex = CUtil::GetMatchingSource(dir, shares, bIsSourceName);

	if (iIndex > -1)
	{
		if (iIndex < (int)shares.size() && shares[iIndex].m_iHasLock == 2)
		{
			CFileItem item(shares[iIndex]);
			if (!g_passwordManager.IsItemUnlocked(&item, "programs"))
				return "";
		}

		if (bIsSourceName)
			return shares[iIndex].strPath;

		return dir;
	}

	return CGUIMediaWindow::GetStartFolder(dir);
}