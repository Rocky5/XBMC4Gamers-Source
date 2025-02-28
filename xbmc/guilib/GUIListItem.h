/*!
\file GUIListItem.h
\brief
*/

#ifndef GUILIB_GUILISTITEM_H
#define GUILIB_GUILISTITEM_H

#pragma once

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

#include "utils/StdString.h"

#include <map>
#include <string>

//  Forward
class CGUIListItemLayout;
class CArchive;
class CVariant;

/*!
 \ingroup controls
 \brief
 */
class CGUIListItem
{
public:
  enum GUIIconOverlay { ICON_OVERLAY_NONE = 0,
                        ICON_OVERLAY_RAR,
                        ICON_OVERLAY_ZIP,
                        ICON_OVERLAY_LOCKED,
                        ICON_OVERLAY_HAS_TRAINER,
                        ICON_OVERLAY_TRAINED,
                        ICON_OVERLAY_UNWATCHED,
                        ICON_OVERLAY_WATCHED,
                        ICON_OVERLAY_HD};

  CGUIListItem(void);
  CGUIListItem(const CGUIListItem& item);
  CGUIListItem(const CStdString& strLabel);
  virtual ~CGUIListItem(void);
  virtual CGUIListItem *Clone() const { return new CGUIListItem(*this); };

  const CGUIListItem& operator =(const CGUIListItem& item);

  virtual void SetLabel(const CStdString& strLabel);
  const CStdString& GetLabel() const;

  void SetLabel2(const CStdString& strLabel);
  const CStdString& GetLabel2() const;

  void SetIconImage(const CStdString& strIcon);
  const CStdString& GetIconImage() const;

  void SetThumbnailImage(const CStdString& strThumbnail);
  const CStdString& GetThumbnailImage() const;

  void SetOverlayImage(GUIIconOverlay icon, bool bOnOff=false);
  CStdString GetOverlayImage() const;
  
  void SetFanartImage(const CStdString& strLabel);
  const CStdString& GetFanartImage() const;
  // Synopsis Text
  void SetLabelSynopsis_AltName(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_AltName() const;
  
  void SetLabelSynopsis_Overview(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Overview() const;
  
  void SetLabelSynopsis_Developer(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Developer() const;
  
  void SetLabelSynopsis_Publisher(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Publisher() const;
  
  void SetLabelSynopsis_FeaturesGeneral(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_FeaturesGeneral() const;
  
  void SetLabelSynopsis_FeaturesOnline(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_FeaturesOnline() const;
  
  void SetLabelSynopsis_ESRB(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_ESRB() const;
  
  void SetLabelSynopsis_ESRBDescriptors(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_ESRBDescriptors() const;
  
  void SetLabelSynopsis_Genre(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Genre() const;
  
  void SetLabelSynopsis_ReleaseDate(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_ReleaseDate() const;
  
  void SetLabelSynopsis_Year(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Year() const;
  
  void SetLabelSynopsis_Rating(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Rating() const;
  
  void SetLabelSynopsis_Platform(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Platform() const;
  
  void SetLabelSynopsis_Exclusive(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Exclusive() const;
  
  void SetLabelSynopsis_TitleID(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_TitleID() const;
  
  void SetLabelSynopsis_Fanart(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Fanart() const;
  
  void SetLabelSynopsis_Resources(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_Resources() const;
  
  void SetSynopsis_Preview(const CStdString& strLabel);
  const CStdString& GetSynopsis_Preview() const;
  
  void SetSynopsis_Screenshot(const CStdString& strLabel);
  const CStdString& GetSynopsis_Screenshot() const;
  
  void SetSynopsis_Fanart(const CStdString& strLabel);
  const CStdString& GetSynopsis_Fanart() const;
  
  void SetLabelSynopsis_PlayerCount(const CStdString& strLabel);
  const CStdString& GetLabelSynopsis_PlayerCount() const;
  
  void SetLabelLastPlayed(const CStdString& strLabel);
  const CStdString& GetLabelLastPlayed() const;

  void SetSortLabel(const CStdString &label);
  const CStdString &GetSortLabel() const;

  void Select(bool bOnOff);
  bool IsSelected() const;

  bool HasIcon() const;
  bool HasThumbnail() const;
  bool HasFanart() const;
  bool HasSynopsis_Fanart() const;
  bool HasOverlay() const;
  virtual bool IsFileItem() const { return false; };

  void SetLayout(CGUIListItemLayout *layout);
  CGUIListItemLayout *GetLayout();

  void SetFocusedLayout(CGUIListItemLayout *layout);
  CGUIListItemLayout *GetFocusedLayout();

  void FreeIcons();
  void FreeMemory(bool immediately = false);
  void SetInvalid();

  bool m_bIsFolder;     ///< is item a folder or a file

  void SetProperty(const CStdString &strKey, const char *strValue);
  void SetProperty(const CStdString &strKey, const CStdString &strValue);
  void SetProperty(const CStdString &strKey, int nVal);
  void SetProperty(const CStdString &strKey, bool bVal);
  void SetProperty(const CStdString &strKey, double dVal);

  void IncrementProperty(const CStdString &strKey, int nVal);
  void IncrementProperty(const CStdString &strKey, double dVal);

  void ClearProperties();

  void Archive(CArchive& ar);
  void Serialize(CVariant& value);

  bool       HasProperty(const CStdString &strKey) const;
  bool       HasProperties() const { return m_mapProperties.size() > 0; };
  void       ClearProperty(const CStdString &strKey);

  CStdString GetProperty(const CStdString &strKey) const;
  bool       GetPropertyBOOL(const CStdString &strKey) const;
  int        GetPropertyInt(const CStdString &strKey) const;
  double     GetPropertyDouble(const CStdString &strKey) const;

 /*! \brief Set the current item number within it's container
   Our container classes will set this member with the items position
   in the container starting at 1.
   \param position Position of the item in the container starting at 1.
   */
  void SetCurrentItem(unsigned int position);
  /*! \brief Get the current item number within it's container
   Retrieve the items position in a container, this is useful to show
   for example numbering in front of entities in an arbitrary list of entities,
   like songs of a playlist.
   */
  unsigned int GetCurrentItem() const;
  
protected:
  CStdString m_strLabel2;     // text of column2
  CStdString m_strThumbnailImage; // filename of thumbnail
  CStdString m_strIcon;      // filename of icon
  GUIIconOverlay m_overlayIcon; // type of overlay icon
  
  // Synopsis Text
  CStdString m_strSynopsisAltName;
  CStdString m_strSynopsisOverview;
  CStdString m_strSynopsisDeveloper;
  CStdString m_strSynopsisPublisher;
  CStdString m_strSynopsisFeaturesGeneral;
  CStdString m_strSynopsisFeaturesOnline;
  CStdString m_strSynopsisESRB;
  CStdString m_strSynopsisESRBDescriptors;
  CStdString m_strSynopsisGenre;
  CStdString m_strSynopsisReleaseDate;
  CStdString m_strSynopsisYear;
  CStdString m_strSynopsisRating;
  CStdString m_strSynopsisPlatform;
  CStdString m_strSynopsisExclusive;
  CStdString m_strSynopsisTitleID;
  CStdString m_strSynopsisFanart;
  CStdString m_strSynopsisResources;
  CStdString m_strSynopsisPreview;
  CStdString m_strSynopsisScreenshot;
  CStdString m_strSynopsisPlayerCount;
  CStdString m_strGameLastPlayed;

  CGUIListItemLayout *m_layout;
  CGUIListItemLayout *m_focusedLayout;
  bool m_bSelected;     // item is selected or not
  unsigned int m_currentItem; // current item number within container (starting at 1)

  struct icompare
  {
    bool operator()(const CStdString &s1, const CStdString &s2) const
    {
      return s1.CompareNoCase(s2) < 0;
    }
  };

  typedef std::map<CStdString, CStdString, icompare> PropertyMap;
  PropertyMap m_mapProperties;
private:
  CStdString m_sortLabel;     // text for sorting
  CStdString m_strLabel;      // text of column1
};
#endif
