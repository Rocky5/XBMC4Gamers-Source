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
#include "GUIStaticItem.h"
#include "XMLUtils.h"
#include "GUIControlFactory.h"
#include "utils/StringUtils.h"
#include "GUIInfoManager.h"

using namespace std;

CGUIStaticItem::CGUIStaticItem(const TiXmlElement *item, int parentID) : CFileItem()
{
  assert(item);

  // check whether we're using the more verbose method...
  const TiXmlNode *click = item->FirstChild("onclick");
  if (click && click->FirstChild())
  {
    CGUIInfoLabel label, label2, SortLabel, thumb, icon;
    CGUIControlFactory::GetInfoLabel(item, "label", label, parentID);
    CGUIControlFactory::GetInfoLabel(item, "label2", label2, parentID);
    CGUIControlFactory::GetInfoLabel(item, "thumb", thumb, parentID);
    CGUIControlFactory::GetInfoLabel(item, "icon", icon, parentID);
	
    CGUIControlFactory::GetInfoLabel(item, "name", label, parentID);
    CGUIControlFactory::GetInfoLabel(item, "synopsis", label2, parentID);
    CGUIControlFactory::GetInfoLabel(item, "fanart", thumb, parentID);
    CGUIControlFactory::GetInfoLabel(item, "poster", icon, parentID);
	
    const char *id = item->Attribute("id");
    int visibleCondition = 0;
    CGUIControlFactory::GetConditionalVisibility(item, visibleCondition);
    CGUIControlFactory::GetActions(item, "onclick", m_clickActions);
    SetLabel(label.GetLabel(parentID));
    SetLabel2(label2.GetLabel(parentID));
    SetSortLabel(SortLabel.GetLabel(parentID));
    SetThumbnailImage(thumb.GetLabel(parentID, true));
    SetIconImage(icon.GetLabel(parentID, true));
    if (!label.IsConstant())  m_info.push_back(make_pair(label, "label"));
    if (!label2.IsConstant()) m_info.push_back(make_pair(label2, "label2"));
    if (!thumb.IsConstant())  m_info.push_back(make_pair(thumb, "thumb"));
    if (!icon.IsConstant())   m_info.push_back(make_pair(icon, "icon"));
	
    if (!label.IsConstant())  m_info.push_back(make_pair(label, "name"));
    if (!label2.IsConstant()) m_info.push_back(make_pair(label2, "synopsis"));
    if (!thumb.IsConstant())  m_info.push_back(make_pair(thumb, "fanart"));
    if (!icon.IsConstant())   m_info.push_back(make_pair(icon, "poster"));
	
    m_iprogramCount = id ? atoi(id) : 0;
    m_idepth = visibleCondition;
    // add any properties
    const TiXmlElement *property = item->FirstChildElement("property");
    while (property)
    {
      CStdString name = property->Attribute("name");
      CGUIInfoLabel prop;
      if (!name.IsEmpty() && CGUIControlFactory::GetInfoLabelFromElement(property, prop, parentID))
      {
        SetProperty(name, prop.GetLabel(parentID, true));
        if (!prop.IsConstant())
          m_info.push_back(make_pair(prop, name));
      }
      property = property->NextSiblingElement("property");
    }
  }
  else
  {
    CStdString label, label2, SortLabel, thumb, icon;
    label  = item->Attribute("label");  label  = CGUIControlFactory::FilterLabel(label);
    label2 = item->Attribute("label2"); label2 = CGUIControlFactory::FilterLabel(label2);
    thumb  = item->Attribute("thumb");  thumb  = CGUIControlFactory::FilterLabel(thumb);
    icon   = item->Attribute("icon");   icon   = CGUIControlFactory::FilterLabel(icon);
	
    label  = item->Attribute("name");  label  = CGUIControlFactory::FilterLabel(label);
    label2 = item->Attribute("synopsis"); label2 = CGUIControlFactory::FilterLabel(label2);
    thumb  = item->Attribute("fanart");  thumb  = CGUIControlFactory::FilterLabel(thumb);
    icon   = item->Attribute("poster");   icon   = CGUIControlFactory::FilterLabel(icon);
	
    const char *id = item->Attribute("id");
    SetLabel(CGUIInfoLabel::GetLabel(label, parentID));
    SetPath(item->FirstChild()->Value());
    SetLabel2(CGUIInfoLabel::GetLabel(label2, parentID));
    SetSortLabel(CGUIInfoLabel::GetLabel(SortLabel, parentID));
    SetThumbnailImage(CGUIInfoLabel::GetLabel(thumb, parentID, true));
    SetIconImage(CGUIInfoLabel::GetLabel(icon, parentID, true));
    m_iprogramCount = id ? atoi(id) : 0;
    m_idepth = 0;  // no visibility condition
  }
}

void CGUIStaticItem::UpdateProperties(int contextWindow)
{
  for (InfoVector::const_iterator i = m_info.begin(); i != m_info.end(); i++)
  {
    const CGUIInfoLabel &info = i->first;
    const CStdString &name = i->second;
    bool preferTexture = strnicmp("label", name.c_str(), 5) != 0;
    CStdString value(info.GetLabel(contextWindow, preferTexture));
    if (name.Equals("label"))
      SetLabel(value);
    else if (name.Equals("label2"))
      SetLabel2(value);
    else if (name.Equals("thumb"))
      SetThumbnailImage(value);
    else if (name.Equals("icon"))
      SetIconImage(value);
  
    if (name.Equals("name"))
      SetLabel(value);
    else if (name.Equals("synopsis"))
      SetLabel2(value);
    else if (name.Equals("fanart"))
      SetThumbnailImage(value);
    else if (name.Equals("poster"))
      SetIconImage(value);
    else
      SetProperty(name, value);
  }
}
