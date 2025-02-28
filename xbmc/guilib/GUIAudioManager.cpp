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

#include "include.h"
#include "GUIAudioManager.h"
#include "Key.h"
#include "AudioContext.h"
#include "GUISound.h"
#include "settings/GUISettings.h"
#include "input/ButtonTranslator.h"
#include "utils/SingleLock.h"
#include "../xbmc/utils/URIUtils.h"
#include "../xbmc/FileSystem/Directory.h"

using namespace std;
using namespace XFILE;

CGUIAudioManager g_audioManager;

CGUIAudioManager::CGUIAudioManager()
{
  m_actionSound=NULL;
  m_bEnabled=true;
}

CGUIAudioManager::~CGUIAudioManager()
{

}

void CGUIAudioManager::Initialize(int iDevice)
{
  CSingleLock lock(m_cs);

  if (iDevice==CAudioContext::DEFAULT_DEVICE)
  {
    bool bAudioOnAllSpeakers=false;
    g_audioContext.SetupSpeakerConfig(2, bAudioOnAllSpeakers);
    g_audioContext.SetActiveDevice(CAudioContext::DIRECTSOUND_DEVICE);
  }
}

void CGUIAudioManager::DeInitialize(int iDevice)
{
  if (!(iDevice == CAudioContext::DIRECTSOUND_DEVICE || iDevice == CAudioContext::DEFAULT_DEVICE)) return;

  CSingleLock lock(m_cs);
  if (m_actionSound) //  Wait for finish when an action sound is playing
    while(m_actionSound->IsPlaying()) {}

  Stop();
}

void CGUIAudioManager::Stop()
{
  CSingleLock lock(m_cs);
  if (m_actionSound)
  {
    delete m_actionSound;
    m_actionSound=NULL;
  }

  for (windowSoundsMap::iterator it=m_windowSounds.begin();it!=m_windowSounds.end();it++)
  {
    CGUISound* sound=it->second;
    if (sound->IsPlaying())
      sound->Stop();

    delete sound;
  }
  m_windowSounds.clear();

  for (pythonSoundsMap::iterator it1=m_pythonSounds.begin();it1!=m_pythonSounds.end();it1++)
  {
    CGUISound* sound=it1->second;
    if (sound->IsPlaying())
      sound->Stop();

    delete sound;
  }
  m_pythonSounds.clear();
}

// \brief Clear any unused audio buffers
void CGUIAudioManager::FreeUnused()
{
  CSingleLock lock(m_cs);

  //  Free the sound from the last action
  if (m_actionSound && !m_actionSound->IsPlaying())
  {
    delete m_actionSound;
    m_actionSound=NULL;
  }

  //  Free sounds from windows
  windowSoundsMap::iterator it=m_windowSounds.begin();
  while (it!=m_windowSounds.end())
  {
    CGUISound* sound=it->second;
    if (!sound->IsPlaying())
    {
      delete sound;
      m_windowSounds.erase(it++);
    }
    else ++it;
  }

  // Free sounds from python
  pythonSoundsMap::iterator it1=m_pythonSounds.begin();
  while (it1!=m_pythonSounds.end())
  {
    CGUISound* sound=it1->second;
    if (!sound->IsPlaying())
    {
      delete sound;
      m_pythonSounds.erase(it1++);
    }
    else ++it1;
  }
}

// \brief Play a sound associated with a CAction
void CGUIAudioManager::PlayActionSound(const CAction& action)
{
  // it's not possible to play gui sounds when passthrough is active
  if (!m_bEnabled || g_audioContext.IsPassthroughActive())
    return;

  CSingleLock lock(m_cs);

  actionSoundMap::iterator it=m_actionSoundMap.find(action.GetID());
  if (it==m_actionSoundMap.end())
    return;

  if (m_actionSound)
  {
    delete m_actionSound;
    m_actionSound=NULL;
  }

  m_actionSound=new CGUISound();
  if (!m_actionSound->Load(URIUtils::AddFileToFolder(m_strMediaDir, it->second)))
  {
    delete m_actionSound;
    m_actionSound=NULL;
    return;
  }

  m_actionSound->Play();
}

// \brief Play a sound associated with a window and its event
// Events: SOUND_INIT, SOUND_DEINIT
void CGUIAudioManager::PlayWindowSound(int id, WINDOW_SOUND event)
{
  // it's not possible to play gui sounds when passthrough is active
  if (!m_bEnabled || g_audioContext.IsPassthroughActive())
    return;

  CSingleLock lock(m_cs);

  windowSoundMap::iterator it=m_windowSoundMap.find(id);
  if (it==m_windowSoundMap.end())
    return;

  CWindowSounds sounds=it->second;
  CStdString strFile;
  switch (event)
  {
  case SOUND_INIT:
    strFile=sounds.strInitFile;
    break;
  case SOUND_DEINIT:
    strFile=sounds.strDeInitFile;
    break;
  }

  if (strFile.IsEmpty())
    return;

  //  One sound buffer for each window
  windowSoundsMap::iterator itsb=m_windowSounds.find(id);
  if (itsb!=m_windowSounds.end())
  {
    CGUISound* sound=itsb->second;
    if (sound->IsPlaying())
      sound->Stop();
    delete sound;
    m_windowSounds.erase(itsb++);
  }

  CGUISound* sound=new CGUISound();
  if (!sound->Load(URIUtils::AddFileToFolder(m_strMediaDir, strFile)))
  {
    delete sound;
    return;
  }

  m_windowSounds.insert(pair<int, CGUISound*>(id, sound));
  sound->Play();
}

// \brief Play a sound given by filename
void CGUIAudioManager::PlayPythonSound(const CStdString& strFileName)
{
  // it's not possible to play gui sounds when passthrough is active
  if (g_audioContext.IsPassthroughActive())
    return;

  CSingleLock lock(m_cs);

  // If we already loaded the sound, just play it
  pythonSoundsMap::iterator itsb=m_pythonSounds.find(strFileName);
  if (itsb!=m_pythonSounds.end())
  {
    CGUISound* sound=itsb->second;
    if (sound->IsPlaying())
      sound->Stop();

    sound->Play();

    return;
  }

  CGUISound* sound=new CGUISound();
  if (!sound->Load(strFileName))
  {
    delete sound;
    return;
  }

  m_pythonSounds.insert(pair<CStdString, CGUISound*>(strFileName, sound));
  sound->Play();
}

// \brief Load the config file (sounds.xml) for nav sounds
// Can be located in a folder "sounds" in the skin or from a
// subfolder of the folder "sounds" in the root directory of
// xbmc
bool CGUIAudioManager::Load()
{
  CStdString strLabel;
  m_actionSoundMap.clear();
  m_windowSoundMap.clear();

  if (g_guiSettings.GetString("lookandfeel.soundskin")=="OFF")
    return true;

  if (g_guiSettings.GetString("lookandfeel.soundskin")=="SKINDEFAULT")
  {
    m_strMediaDir="special://home/skins/" + g_guiSettings.GetString("lookandfeel.skin") + "/sounds/default";
    if ( ! CDirectory::Exists( m_strMediaDir ) )
    {
      m_strMediaDir = URIUtils::AddFileToFolder("special://xbmc/skins/", g_guiSettings.GetString("lookandfeel.skin"));
      m_strMediaDir = URIUtils::AddFileToFolder(m_strMediaDir, "sounds");
    }
  }
  else
    m_strMediaDir = URIUtils::AddFileToFolder("special://xbmc/skins/", g_guiSettings.GetString("lookandfeel.skin") + "/sounds/" + g_guiSettings.GetString("lookandfeel.soundskin"));

  CStdString strSoundsXml = URIUtils::AddFileToFolder(m_strMediaDir, "sounds.xml");

  //  Load our xml file
  TiXmlDocument xmlDoc;

  CLog::Log(LOGINFO, "Loading %s", strSoundsXml.c_str());

  //  Load the config file
  if (!xmlDoc.LoadFile(strSoundsXml))
  {
    CLog::Log(LOGNOTICE, "%s, Line %d\n%s", strSoundsXml.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
    return false;
  }

  TiXmlElement* pRoot = xmlDoc.RootElement();
  CStdString strValue = pRoot->Value();
  if ( strValue != "sounds")
  {
    CLog::Log(LOGNOTICE, "%s Doesn't contain <sounds>", strSoundsXml.c_str());
    return false;
  }

  //  Load sounds for actions
  TiXmlElement* pActions = pRoot->FirstChildElement("actions");
  if (pActions)
  {
    TiXmlNode* pAction = pActions->FirstChild("action");

    while (pAction)
    {
      TiXmlNode* pIdNode = pAction->FirstChild("name");
      int id = 0;    // action identity
      if (pIdNode && pIdNode->FirstChild())
      {
        CButtonTranslator::TranslateActionString(pIdNode->FirstChild()->Value(), id);
      }

      TiXmlNode* pFileNode = pAction->FirstChild("file");
      CStdString strFile;
      if (pFileNode && pFileNode->FirstChild())
        strFile+=pFileNode->FirstChild()->Value();

      if (id > 0 && !strFile.IsEmpty())
        m_actionSoundMap.insert(pair<int, CStdString>(id, strFile));

      pAction = pAction->NextSibling();
    }
  }

  //  Load window specific sounds
  TiXmlElement* pWindows = pRoot->FirstChildElement("windows");
  if (pWindows)
  {
    TiXmlNode* pWindow = pWindows->FirstChild("window");

    while (pWindow)
    {
      int id = 0;

      TiXmlNode* pIdNode = pWindow->FirstChild("name");
      if (pIdNode)
      {
        if (pIdNode->FirstChild())
          id = CButtonTranslator::TranslateWindow(pIdNode->FirstChild()->Value());
      }

      CWindowSounds sounds;
      LoadWindowSound(pWindow, "activate", sounds.strInitFile);
      LoadWindowSound(pWindow, "deactivate", sounds.strDeInitFile);

      if (id > 0)
        m_windowSoundMap.insert(pair<int, CWindowSounds>(id, sounds));

      pWindow = pWindow->NextSibling();
    }
  }

  return true;
}

// \brief Load a window node of the config file (sounds.xml)
bool CGUIAudioManager::LoadWindowSound(TiXmlNode* pWindowNode, const CStdString& strIdentifier, CStdString& strFile)
{
  if (!pWindowNode)
    return false;

  TiXmlNode* pFileNode = pWindowNode->FirstChild(strIdentifier);
  if (pFileNode && pFileNode->FirstChild())
  {
    strFile = pFileNode->FirstChild()->Value();
    return true;
  }

  return false;
}

// \brief Enable/Disable nav sounds
void CGUIAudioManager::Enable(bool bEnable)
{
  // Enable/Disable has no effect if nav sounds are turned off
  if (g_guiSettings.GetString("lookandfeel.soundskin")=="OFF")
    return;

  m_bEnabled=bEnable;
}

// \brief Sets the volume of all playing sounds
void CGUIAudioManager::SetVolume(int iLevel)
{
  CSingleLock lock(m_cs);

  if (m_actionSound)
    m_actionSound->SetVolume(iLevel);

  windowSoundsMap::iterator it=m_windowSounds.begin();
  while (it!=m_windowSounds.end())
  {
    if (it->second)
      it->second->SetVolume(iLevel);

    ++it;
  }

  pythonSoundsMap::iterator it1=m_pythonSounds.begin();
  while (it1!=m_pythonSounds.end())
  {
    if (it1->second)
      it1->second->SetVolume(iLevel);

    ++it1;
  }
}
