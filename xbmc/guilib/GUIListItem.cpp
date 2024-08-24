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
#include "GUIListItem.h"
#include "GUIListItemLayout.h"
#include "utils/Archive.h"
#include "utils/Variant.h"

CGUIListItem::CGUIListItem(const CGUIListItem& item)
{
  m_layout = NULL;
  m_focusedLayout = NULL;
  *this = item;
  SetInvalid();
}

CGUIListItem::CGUIListItem(void)
{
  m_bIsFolder = false;
  m_strLabel2 = "";
  m_strLabel = "";
  m_bSelected = false;
  m_strIcon = "";
  m_strThumbnailImage = "";
  m_overlayIcon = ICON_OVERLAY_NONE;
  m_layout = NULL;
  m_focusedLayout = NULL;
  
  m_strSynopsisAltName = "";
  m_strSynopsisOverview = "";
  m_strSynopsisDeveloper = "";
  m_strSynopsisPublisher = "";
  m_strSynopsisFeaturesGeneral = "";
  m_strSynopsisFeaturesOnline = "";
  m_strSynopsisESRB = "";
  m_strSynopsisESRBDescriptors = "";
  m_strSynopsisGenre = "";
  m_strSynopsisReleaseDate = "";
  m_strSynopsisYear = "";
  m_strSynopsisRating = "";
  m_strSynopsisPlatform = "";
  m_strSynopsisExclusive = "";
  m_strSynopsisTitleID = "";
  m_strSynopsisFanart = "";
  m_strSynopsisResources = "";
  m_strSynopsisPreview = "";
  m_strSynopsisFanart = "";
  m_strSynopsisPlayerCount = "1";
}

CGUIListItem::CGUIListItem(const CStdString& strLabel)
{
  m_bIsFolder = false;
  m_strLabel2 = "";
  m_strLabel = strLabel;
  m_sortLabel = strLabel;
  m_bSelected = false;
  m_strIcon = "";
  m_strThumbnailImage = "";
  m_overlayIcon = ICON_OVERLAY_NONE;
  m_layout = NULL;
  m_focusedLayout = NULL;
  
  m_strSynopsisAltName = "";
  m_strSynopsisOverview = "";
  m_strSynopsisDeveloper = "";
  m_strSynopsisPublisher = "";
  m_strSynopsisFeaturesGeneral = "";
  m_strSynopsisFeaturesOnline = "";
  m_strSynopsisESRB = "";
  m_strSynopsisESRBDescriptors = "";
  m_strSynopsisGenre = "";
  m_strSynopsisReleaseDate = "";
  m_strSynopsisYear = "";
  m_strSynopsisRating = "";
  m_strSynopsisPlatform = "";
  m_strSynopsisExclusive = "";
  m_strSynopsisTitleID = "";
  m_strSynopsisFanart = "";
  m_strSynopsisResources = "";
  m_strSynopsisPreview = "";
  m_strSynopsisFanart = "";
  m_strSynopsisPlayerCount = "1";
}

CGUIListItem::~CGUIListItem(void)
{
  FreeMemory();
}

void CGUIListItem::SetLabel(const CStdString& strLabel)
{
  if (m_strLabel == strLabel)
    return;
  m_strLabel = strLabel;
  if (m_sortLabel.IsEmpty())
    m_sortLabel = strLabel;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabel() const
{
  return m_strLabel;
}

// Synopsis code

void CGUIListItem::SetLabelSynopsis_AltName(const CStdString& strSynopsisAltName)
{
  if (m_strSynopsisAltName == strSynopsisAltName)
    return;
  m_strSynopsisAltName = strSynopsisAltName;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_AltName() const
{
  return m_strSynopsisAltName;
}

void CGUIListItem::SetLabelSynopsis_Overview(const CStdString& strSynopsisOverview)
{
  if (m_strSynopsisOverview == strSynopsisOverview)
    return;
  m_strSynopsisOverview = strSynopsisOverview;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Overview() const
{
  return m_strSynopsisOverview;
}

void CGUIListItem::SetLabelSynopsis_Developer(const CStdString& strSynopsisDeveloper)
{
  if (m_strSynopsisDeveloper == strSynopsisDeveloper)
    return;
  m_strSynopsisDeveloper = strSynopsisDeveloper;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Developer() const
{
  return m_strSynopsisDeveloper;
}

void CGUIListItem::SetLabelSynopsis_Publisher(const CStdString& strSynopsisPublisher)
{
  if (m_strSynopsisPublisher == strSynopsisPublisher)
    return;
  m_strSynopsisPublisher = strSynopsisPublisher;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Publisher() const
{
  return m_strSynopsisPublisher;
}

void CGUIListItem::SetLabelSynopsis_FeaturesGeneral(const CStdString& strSynopsisFeaturesGeneral)
{
  if (m_strSynopsisFeaturesGeneral == strSynopsisFeaturesGeneral)
    return;
  m_strSynopsisFeaturesGeneral = strSynopsisFeaturesGeneral;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_FeaturesGeneral() const
{
  return m_strSynopsisFeaturesGeneral;
}

void CGUIListItem::SetLabelSynopsis_FeaturesOnline(const CStdString& strSynopsisFeaturesOnline)
{
  if (m_strSynopsisFeaturesOnline == strSynopsisFeaturesOnline)
    return;
  m_strSynopsisFeaturesOnline = strSynopsisFeaturesOnline;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_FeaturesOnline() const
{
  return m_strSynopsisFeaturesOnline;
}

void CGUIListItem::SetLabelSynopsis_ESRB(const CStdString& strSynopsisESRB)
{
  if (m_strSynopsisESRB == strSynopsisESRB)
    return;
  m_strSynopsisESRB = strSynopsisESRB;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_ESRB() const
{
  return m_strSynopsisESRB;
}

void CGUIListItem::SetLabelSynopsis_ESRBDescriptors(const CStdString& strSynopsisESRBDescriptors)
{
  if (m_strSynopsisESRBDescriptors == strSynopsisESRBDescriptors)
    return;
  m_strSynopsisESRBDescriptors = strSynopsisESRBDescriptors;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_ESRBDescriptors() const
{
  return m_strSynopsisESRBDescriptors;
}

void CGUIListItem::SetLabelSynopsis_Genre(const CStdString& strSynopsisGenre)
{
  if (m_strSynopsisGenre == strSynopsisGenre)
    return;
  m_strSynopsisGenre = strSynopsisGenre;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Genre() const
{
  return m_strSynopsisGenre;
}

void CGUIListItem::SetLabelSynopsis_ReleaseDate(const CStdString& strSynopsisReleaseDate)
{
  if (m_strSynopsisReleaseDate == strSynopsisReleaseDate)
    return;
  m_strSynopsisReleaseDate = strSynopsisReleaseDate;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_ReleaseDate() const
{
  return m_strSynopsisReleaseDate;
}

void CGUIListItem::SetLabelSynopsis_Year(const CStdString& strSynopsisYear)
{
  if (m_strSynopsisYear == strSynopsisYear)
    return;
  m_strSynopsisYear = strSynopsisYear;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Year() const
{
  return m_strSynopsisYear;
}

void CGUIListItem::SetLabelSynopsis_Rating(const CStdString& strSynopsisRating)
{
  if (m_strSynopsisRating == strSynopsisRating)
    return;
  m_strSynopsisRating = strSynopsisRating;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Rating() const
{
  return m_strSynopsisRating;
}

void CGUIListItem::SetLabelSynopsis_Platform(const CStdString& strSynopsisPlatform)
{
  if (m_strSynopsisPlatform == strSynopsisPlatform)
    return;
  m_strSynopsisPlatform = strSynopsisPlatform;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Platform() const
{
  return m_strSynopsisPlatform;
}

void CGUIListItem::SetLabelSynopsis_Exclusive(const CStdString& strSynopsisExclusive)
{
  if (m_strSynopsisExclusive == strSynopsisExclusive)
    return;
  m_strSynopsisExclusive = strSynopsisExclusive;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Exclusive() const
{
  return m_strSynopsisExclusive;
}

void CGUIListItem::SetLabelSynopsis_TitleID(const CStdString& strSynopsisTitleID)
{
  if (m_strSynopsisTitleID == strSynopsisTitleID)
    return;
  m_strSynopsisTitleID = strSynopsisTitleID;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_TitleID() const
{
  return m_strSynopsisTitleID;
}

void CGUIListItem::SetLabelSynopsis_Fanart(const CStdString& strSynopsisFanart)
{
  if (m_strSynopsisFanart == strSynopsisFanart)
    return;
  m_strSynopsisFanart = strSynopsisFanart;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Fanart() const
{
  return m_strSynopsisFanart;
}

void CGUIListItem::SetLabelSynopsis_Resources(const CStdString& strSynopsisResources)
{
  if (m_strSynopsisResources == strSynopsisResources)
    return;
  m_strSynopsisResources = strSynopsisResources;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_Resources() const
{
  return m_strSynopsisResources;
}

void CGUIListItem::SetSynopsis_Preview(const CStdString& strSynopsisPreview)
{
  if (m_strSynopsisPreview == strSynopsisPreview)
    return;
  m_strSynopsisPreview = strSynopsisPreview;
  SetInvalid();
}

const CStdString& CGUIListItem::GetSynopsis_Preview() const
{
  return m_strSynopsisPreview;
}

void CGUIListItem::SetSynopsis_Fanart(const CStdString& strSynopsisFanart)
{
  if (m_strSynopsisFanart == strSynopsisFanart)
    return;
  m_strSynopsisFanart = strSynopsisFanart;
  SetInvalid();
}

const CStdString& CGUIListItem::GetSynopsis_Fanart() const
{
  return m_strSynopsisFanart;
}

void CGUIListItem::SetLabelSynopsis_PlayerCount(const CStdString& strSynopsisPlayerCount)
{
  if (m_strSynopsisPlayerCount == strSynopsisPlayerCount)
    return;
  m_strSynopsisPlayerCount = strSynopsisPlayerCount;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabelSynopsis_PlayerCount() const
{
  return m_strSynopsisPlayerCount;
}

void CGUIListItem::SetLabel2(const CStdString& strLabel2)
{
  if (m_strLabel2 == strLabel2)
    return;
  m_strLabel2 = strLabel2;
  SetInvalid();
}

const CStdString& CGUIListItem::GetLabel2() const
{
  return m_strLabel2;
}

void CGUIListItem::SetSortLabel(const CStdString &label)
{
  m_sortLabel = label;
  // no need to invalidate - this is never shown in the UI
}

const CStdString& CGUIListItem::GetSortLabel() const
{
  return m_sortLabel;
}

void CGUIListItem::SetThumbnailImage(const CStdString& strThumbnail)
{
  if (m_strThumbnailImage == strThumbnail)
    return;
  m_strThumbnailImage = strThumbnail;
  SetInvalid();
}

const CStdString& CGUIListItem::GetThumbnailImage() const
{
  return m_strThumbnailImage;
}

void CGUIListItem::SetIconImage(const CStdString& strIcon)
{
  if (m_strIcon == strIcon)
    return;
  m_strIcon = strIcon;
  SetInvalid();
}

const CStdString& CGUIListItem::GetIconImage() const
{
  return m_strIcon;
}

void CGUIListItem::SetOverlayImage(GUIIconOverlay icon, bool bOnOff)
{
  GUIIconOverlay newIcon = (bOnOff) ? GUIIconOverlay((int)(icon)+1) : icon;
  if (m_overlayIcon == newIcon)
    return;
  m_overlayIcon = newIcon;
  SetInvalid();
}

CStdString CGUIListItem::GetOverlayImage() const
{
  switch (m_overlayIcon)
  {
  case ICON_OVERLAY_RAR:
    return "OverlayRAR.png";
  case ICON_OVERLAY_ZIP:
    return "OverlayZIP.png";
  case ICON_OVERLAY_TRAINED:
    return "OverlayTrained.png";
  case ICON_OVERLAY_HAS_TRAINER:
    return "OverlayHasTrainer.png";
  case ICON_OVERLAY_LOCKED:
    return "OverlayLocked.png";
  case ICON_OVERLAY_UNWATCHED:
    return "OverlayUnwatched.png";
  case ICON_OVERLAY_WATCHED:
    return "OverlayWatched.png";
  case ICON_OVERLAY_HD:
    return "OverlayHD.png";
  default:
    return "";
  }
}

void CGUIListItem::Select(bool bOnOff)
{
  m_bSelected = bOnOff;
}

bool CGUIListItem::HasIcon() const
{
  return (m_strIcon.size() != 0);
}


bool CGUIListItem::HasThumbnail() const
{
  return (m_strThumbnailImage.size() != 0);
}

bool CGUIListItem::HasOverlay() const
{
  return (m_overlayIcon != CGUIListItem::ICON_OVERLAY_NONE);
}

bool CGUIListItem::IsSelected() const
{
  return m_bSelected;
}

const CGUIListItem& CGUIListItem::operator =(const CGUIListItem& item)
{
  if (&item == this) return * this;
  m_strLabel2 = item.m_strLabel2;
  m_strLabel = item.m_strLabel;
  m_sortLabel = item.m_sortLabel;
  
  m_strSynopsisAltName = item.m_strSynopsisAltName;
  m_strSynopsisOverview = item.m_strSynopsisOverview;
  m_strSynopsisDeveloper = item.m_strSynopsisDeveloper;
  m_strSynopsisPublisher = item.m_strSynopsisPublisher;
  m_strSynopsisFeaturesGeneral = item.m_strSynopsisFeaturesGeneral;
  m_strSynopsisFeaturesOnline = item.m_strSynopsisFeaturesOnline;
  m_strSynopsisESRB = item.m_strSynopsisESRB;
  m_strSynopsisESRBDescriptors = item.m_strSynopsisESRBDescriptors;
  m_strSynopsisGenre = item.m_strSynopsisGenre;
  m_strSynopsisReleaseDate = item.m_strSynopsisReleaseDate;
  m_strSynopsisYear = item.m_strSynopsisYear;
  m_strSynopsisRating = item.m_strSynopsisRating;
  m_strSynopsisPlatform = item.m_strSynopsisPlatform;
  m_strSynopsisExclusive = item.m_strSynopsisExclusive;
  m_strSynopsisTitleID = item.m_strSynopsisTitleID;
  m_strSynopsisFanart = item.m_strSynopsisFanart;
  m_strSynopsisResources = item.m_strSynopsisResources;
  m_strSynopsisPreview = item.m_strSynopsisPreview;
  m_strSynopsisFanart = item.m_strSynopsisFanart;
  m_strSynopsisPlayerCount = item.m_strSynopsisPlayerCount;
  
  FreeMemory();
  m_bSelected = item.m_bSelected;
  m_strIcon = item.m_strIcon;
  m_strThumbnailImage = item.m_strThumbnailImage;
  m_overlayIcon = item.m_overlayIcon;
  m_bIsFolder = item.m_bIsFolder;
  m_mapProperties = item.m_mapProperties;
  SetInvalid();
  return *this;
}

void CGUIListItem::Archive(CArchive &ar)
{
  if (ar.IsStoring())
  {
    ar << m_bIsFolder;
    ar << m_strLabel;
    ar << m_strLabel2;
    ar << m_sortLabel;
	
    ar << m_strSynopsisAltName;
    ar << m_strSynopsisOverview;
    ar << m_strSynopsisDeveloper;
    ar << m_strSynopsisPublisher;
    ar << m_strSynopsisFeaturesGeneral;
    ar << m_strSynopsisFeaturesOnline;
    ar << m_strSynopsisESRB;
    ar << m_strSynopsisESRBDescriptors;
    ar << m_strSynopsisGenre;
    ar << m_strSynopsisReleaseDate;
    ar << m_strSynopsisYear;
    ar << m_strSynopsisRating;
    ar << m_strSynopsisPlatform;
    ar << m_strSynopsisExclusive;
    ar << m_strSynopsisTitleID;
    ar << m_strSynopsisFanart;
    ar << m_strSynopsisResources;
    ar << m_strSynopsisPreview;
    ar << m_strSynopsisFanart;
    ar << m_strSynopsisPlayerCount;
	
    ar << m_strThumbnailImage;
    ar << m_strIcon;
    ar << m_bSelected;
    ar << m_overlayIcon;
    ar << m_mapProperties.size();
    for (std::map<CStdString, CStdString, icompare>::const_iterator it = m_mapProperties.begin(); it != m_mapProperties.end(); it++)
    {
      ar << it->first;
      ar << it->second;
    }
  }
  else
  {
    ar >> m_bIsFolder;
    ar >> m_strLabel;
    ar >> m_strLabel2;
    ar >> m_sortLabel;
	
    ar >> m_strSynopsisAltName;
    ar >> m_strSynopsisOverview;
    ar >> m_strSynopsisDeveloper;
    ar >> m_strSynopsisPublisher;
    ar >> m_strSynopsisFeaturesGeneral;
    ar >> m_strSynopsisFeaturesOnline;
    ar >> m_strSynopsisESRB;
    ar >> m_strSynopsisESRBDescriptors;
    ar >> m_strSynopsisGenre;
    ar >> m_strSynopsisReleaseDate;
    ar >> m_strSynopsisYear;
    ar >> m_strSynopsisRating;
    ar >> m_strSynopsisPlatform;
    ar >> m_strSynopsisExclusive;
    ar >> m_strSynopsisTitleID;
    ar >> m_strSynopsisFanart;
    ar >> m_strSynopsisResources;
    ar >> m_strSynopsisPreview;
    ar >> m_strSynopsisFanart;
    ar >> m_strSynopsisPlayerCount;
	
    ar >> m_strThumbnailImage;
    ar >> m_strIcon;
    ar >> m_bSelected;
    ar >> (int&)m_overlayIcon;
    int mapSize;
    ar >> mapSize;
    for (int i = 0; i < mapSize; i++)
    {
      CStdString key, value;
      ar >> key;
      ar >> value;
      SetProperty(key, value);
    }
  }
}
void CGUIListItem::Serialize(CVariant &value)
{
  value["isFolder"] = m_bIsFolder;
  value["strLabel"] = m_strLabel;
  value["strLabel2"] = m_strLabel2;
  
  value["strSynopsisAltName"] = m_strSynopsisAltName;
  value["strSynopsisOverview"] = m_strSynopsisOverview;
  value["strSynopsisDeveloper"] = m_strSynopsisDeveloper;
  value["strSynopsisPublisher"] = m_strSynopsisPublisher;
  value["strSynopsisFeaturesGeneral"] = m_strSynopsisFeaturesGeneral;
  value["strSynopsisFeaturesOnline"] = m_strSynopsisFeaturesOnline;
  value["strSynopsisESRB"] = m_strSynopsisESRB;
  value["strSynopsisESRBDescriptors"] = m_strSynopsisESRBDescriptors;
  value["strSynopsisGenre"] = m_strSynopsisGenre;
  value["strSynopsisReleaseDate"] = m_strSynopsisReleaseDate;
  value["strSynopsisYear"] = m_strSynopsisYear;
  value["strSynopsisRating"] = m_strSynopsisRating;
  value["strSynopsisPlatform"] = m_strSynopsisPlatform;
  value["strSynopsisExclusive"] = m_strSynopsisExclusive;
  value["strSynopsisTitleID"] = m_strSynopsisTitleID;
  value["strSynopsisFanart"] = m_strSynopsisFanart;
  value["strSynopsisResources"] = m_strSynopsisResources;
  value["strSynopsisPreview"] = m_strSynopsisPreview;
  value["strSynopsisFanart"] = m_strSynopsisFanart;
  value["strSynopsisPlayerCount"] = m_strSynopsisPlayerCount;
  
  value["sortLabel"] = CStdString(m_sortLabel);
  value["strThumbnailImage"] = m_strThumbnailImage;
  value["strIcon"] = m_strIcon;
  value["selected"] = m_bSelected;

  for (std::map<CStdString, CStdString, icompare>::const_iterator it = m_mapProperties.begin(); it != m_mapProperties.end(); it++)
  {
    value["properties"][it->first] = it->second;
  }
}

void CGUIListItem::FreeIcons()
{
  FreeMemory();
  m_strThumbnailImage = "";
  m_strIcon = "";
  SetInvalid();
}

void CGUIListItem::FreeMemory(bool immediately)
{
  if (m_layout)
  {
    m_layout->FreeResources(immediately);
    delete m_layout;
    m_layout = NULL;
  }
  if (m_focusedLayout)
  {
    m_focusedLayout->FreeResources(immediately);
    delete m_focusedLayout;
    m_focusedLayout = NULL;
  }
}

void CGUIListItem::SetLayout(CGUIListItemLayout *layout)
{
  if (m_layout)
    delete m_layout;
  m_layout = layout;
}

CGUIListItemLayout *CGUIListItem::GetLayout()
{
  return m_layout;
}

void CGUIListItem::SetFocusedLayout(CGUIListItemLayout *layout)
{
  if (m_focusedLayout)
    delete m_focusedLayout;
  m_focusedLayout = layout;
}

CGUIListItemLayout *CGUIListItem::GetFocusedLayout()
{
  return m_focusedLayout;
}

void CGUIListItem::SetInvalid()
{
  if (m_layout) m_layout->SetInvalid();
  if (m_focusedLayout) m_focusedLayout->SetInvalid();
}

void CGUIListItem::SetProperty(const CStdString &strKey, const char *strValue)
{
  m_mapProperties[strKey] = strValue;
}

void CGUIListItem::SetProperty(const CStdString &strKey, const CStdString &strValue)
{
  m_mapProperties[strKey] = strValue;
}

CStdString CGUIListItem::GetProperty(const CStdString &strKey) const
{
  PropertyMap::const_iterator iter = m_mapProperties.find(strKey);
  if (iter == m_mapProperties.end())
    return "";

  return iter->second;
}

bool CGUIListItem::HasProperty(const CStdString &strKey) const
{
  PropertyMap::const_iterator iter = m_mapProperties.find(strKey);
  if (iter == m_mapProperties.end())
    return false;

  return true;
}

void CGUIListItem::ClearProperty(const CStdString &strKey)
{
  PropertyMap::iterator iter = m_mapProperties.find(strKey);
  if (iter != m_mapProperties.end())
    m_mapProperties.erase(iter);
}

void CGUIListItem::ClearProperties()
{
  m_mapProperties.clear();
}

void CGUIListItem::SetProperty(const CStdString &strKey, int nVal)
{
  CStdString strVal;
  strVal.Format("%d",nVal);
  SetProperty(strKey, strVal);
}

void CGUIListItem::IncrementProperty(const CStdString &strKey, int nVal)
{
  int i = GetPropertyInt(strKey);
  i += nVal;
  SetProperty(strKey, i);
}

void CGUIListItem::SetProperty(const CStdString &strKey, bool bVal)
{
  SetProperty(strKey, bVal?"1":"0");
}

void CGUIListItem::SetProperty(const CStdString &strKey, double dVal)
{
  CStdString strVal;
  strVal.Format("%f",dVal);
  SetProperty(strKey, strVal);
}

void CGUIListItem::IncrementProperty(const CStdString &strKey, double dVal)
{
  double d = GetPropertyDouble(strKey);
  d += dVal;
  SetProperty(strKey, d);
}

bool CGUIListItem::GetPropertyBOOL(const CStdString &strKey) const
{
  return GetProperty(strKey) == "1";
}

int CGUIListItem::GetPropertyInt(const CStdString &strKey) const
{
  return atoi(GetProperty(strKey).c_str()) ;
}

double CGUIListItem::GetPropertyDouble(const CStdString &strKey) const
{
  return atof(GetProperty(strKey).c_str()) ;
}

