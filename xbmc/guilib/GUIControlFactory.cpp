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
#include "GUIControlFactory.h"
#include "LocalizeStrings.h"
#include "GUIButtonControl.h"
#include "GUIRadioButtonControl.h"
#include "GUISpinControl.h"
#include "GUIRSSControl.h"
#include "GUIImage.h"
#include "GUIBorderedImage.h"
#include "GUILabelControl.h"
#include "GUIEditControl.h"
#include "GUIFadeLabelControl.h"
#include "GUICheckMarkControl.h"
#include "GUIToggleButtonControl.h"
#include "GUITextBox.h"
#include "GUIVideoControl.h"
#include "GUIProgressControl.h"
#include "GUISliderControl.h"
#include "GUISelectButtonControl.h"
#include "GUIMoverControl.h"
#include "GUIResizeControl.h"
#include "GUIButtonScroller.h"
#include "GUISpinControlEx.h"
#include "GUIVisualisationControl.h"
#include "GUISettingsSliderControl.h"
#include "GUIMultiImage.h"
#include "GUIControlGroup.h"
#include "GUIControlGroupList.h"
#include "GUIScrollBarControl.h"
#include "GUIListContainer.h"
#include "GUIFixedListContainer.h"
#include "GUIWrappingListContainer.h"
#include "GUIPanelContainer.h"
#include "GUIMultiSelectText.h"
#include "GUIListLabel.h"
#include "GUIListGroup.h"
#include "GUIInfoManager.h"
#include "utils/CharsetConverter.h"
#include "input/ButtonTranslator.h"
#include "XMLUtils.h"
#include "GUIFontManager.h"
#include "GUIColorManager.h"
#include "SkinInfo.h"
#include "settings/Settings.h"

using namespace std;

typedef struct
{
  const char* name;
  CGUIControl::GUICONTROLTYPES type;
} ControlMapping;

static const ControlMapping controls[] =
   {{"button",            CGUIControl::GUICONTROL_BUTTON},
    {"checkmark",         CGUIControl::GUICONTROL_CHECKMARK},
    {"fadelabel",         CGUIControl::GUICONTROL_FADELABEL},
    {"image",             CGUIControl::GUICONTROL_IMAGE},
    {"largeimage",        CGUIControl::GUICONTROL_IMAGE},
    {"image",             CGUIControl::GUICONTROL_BORDEREDIMAGE},
    {"label",             CGUIControl::GUICONTROL_LABEL},
    {"label",             CGUIControl::GUICONTROL_LISTLABEL},
    {"group",             CGUIControl::GUICONTROL_GROUP},
    {"group",             CGUIControl::GUICONTROL_LISTGROUP},
    {"progress",          CGUIControl::GUICONTROL_PROGRESS},
    {"radiobutton",       CGUIControl::GUICONTROL_RADIO},
    {"rss",               CGUIControl::GUICONTROL_RSS},
    {"selectbutton",      CGUIControl::GUICONTROL_SELECTBUTTON},
    {"slider",            CGUIControl::GUICONTROL_SLIDER},
    {"sliderex",          CGUIControl::GUICONTROL_SETTINGS_SLIDER},
    {"spincontrol",       CGUIControl::GUICONTROL_SPIN},
    {"spincontrolex",     CGUIControl::GUICONTROL_SPINEX},
    {"textbox",           CGUIControl::GUICONTROL_TEXTBOX},
    {"togglebutton",      CGUIControl::GUICONTROL_TOGGLEBUTTON},
    {"videowindow",       CGUIControl::GUICONTROL_VIDEO},
    {"mover",             CGUIControl::GUICONTROL_MOVER},
    {"resize",            CGUIControl::GUICONTROL_RESIZE},
    {"buttonscroller",    CGUIControl::GUICONTROL_BUTTONBAR},
    {"edit",              CGUIControl::GUICONTROL_EDIT},
    {"visualisation",     CGUIControl::GUICONTROL_VISUALISATION},
    {"karvisualisation",  CGUIControl::GUICONTROL_VISUALISATION},
    {"multiimage",        CGUIControl::GUICONTROL_MULTI_IMAGE},
    {"grouplist",         CGUIControl::GUICONTROL_GROUPLIST},
    {"scrollbar",         CGUIControl::GUICONTROL_SCROLLBAR},
    {"multiselect",       CGUIControl::GUICONTROL_MULTISELECT},
    {"list",              CGUIControl::GUICONTAINER_LIST},
    {"wraplist",          CGUIControl::GUICONTAINER_WRAPLIST},
    {"fixedlist",         CGUIControl::GUICONTAINER_FIXEDLIST},
    {"panel",             CGUIControl::GUICONTAINER_PANEL}};

CGUIControl::GUICONTROLTYPES CGUIControlFactory::TranslateControlType(const CStdString &type)
{
  for (unsigned int i = 0; i < sizeof(controls) / sizeof(controls[0]); ++i)
    if (0 == type.CompareNoCase(controls[i].name))
      return controls[i].type;
  return CGUIControl::GUICONTROL_UNKNOWN;
}

CStdString CGUIControlFactory::TranslateControlType(CGUIControl::GUICONTROLTYPES type)
{
  for (unsigned int i = 0; i < sizeof(controls) / sizeof(controls[0]); ++i)
    if (type == controls[i].type)
      return controls[i].name;
  return "";
}

CGUIControlFactory::CGUIControlFactory(void)
{}

CGUIControlFactory::~CGUIControlFactory(void)
{}

bool CGUIControlFactory::GetIntRange(const TiXmlNode* pRootNode, const char* strTag, int& iMinValue, int& iMaxValue, int& iIntervalValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag);
  if (!pNode || !pNode->FirstChild()) return false;
  iMinValue = atoi(pNode->FirstChild()->Value());
  const char* maxValue = strchr(pNode->FirstChild()->Value(), ',');
  if (maxValue)
  {
    maxValue++;
    iMaxValue = atoi(maxValue);

    const char* intervalValue = strchr(maxValue, ',');
    if (intervalValue)
    {
      intervalValue++;
      iIntervalValue = atoi(intervalValue);
    }
  }

  return true;
}

bool CGUIControlFactory::GetFloatRange(const TiXmlNode* pRootNode, const char* strTag, float& fMinValue, float& fMaxValue, float& fIntervalValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag);
  if (!pNode || !pNode->FirstChild()) return false;
  fMinValue = (float)atof(pNode->FirstChild()->Value());
  const char* maxValue = strchr(pNode->FirstChild()->Value(), ',');
  if (maxValue)
  {
    maxValue++;
    fMaxValue = (float)atof(maxValue);

    const char* intervalValue = strchr(maxValue, ',');
    if (intervalValue)
    {
      intervalValue++;
      fIntervalValue = (float)atof(intervalValue);
    }
  }

  return true;
}

bool CGUIControlFactory::GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  return g_SkinInfo.ResolveConstant(pNode->FirstChild()->Value(), value);
}

bool CGUIControlFactory::GetUnsigned(const TiXmlNode* pRootNode, const char* strTag, unsigned int &value)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  return g_SkinInfo.ResolveConstant(pNode->FirstChild()->Value(), value);
}

bool CGUIControlFactory::GetDimension(const TiXmlNode *pRootNode, const char* strTag, float &value, float &min)
{
  const TiXmlElement* pNode = pRootNode->FirstChildElement(strTag);
  if (!pNode || !pNode->FirstChild()) return false;
  if (0 == strnicmp("auto", pNode->FirstChild()->Value(), 4))
  { // auto-width - at least min must be set
    g_SkinInfo.ResolveConstant(pNode->Attribute("max"), value);
    g_SkinInfo.ResolveConstant(pNode->Attribute("min"), min);
    if (!min) min = 1;
    return true;
  }
  // else if (0 == strnicmp("auto", pNode->FirstChild()->Value(), 4))
  return g_SkinInfo.ResolveConstant(pNode->FirstChild()->Value(), value);
}

bool CGUIControlFactory::GetPath(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringPath)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode) return false;
  strStringPath = pNode->FirstChild() ? pNode->FirstChild()->Value() : "";
  strStringPath.Replace('/', '\\');
  return true;
}

bool CGUIControlFactory::GetAspectRatio(const TiXmlNode* pRootNode, const char* strTag, CAspectRatio &aspect)
{
  CStdString ratio;
  const TiXmlElement *node = pRootNode->FirstChildElement(strTag);
  if (!node || !node->FirstChild())
    return false;

  ratio = node->FirstChild()->Value();
  if (ratio.CompareNoCase("keep") == 0) aspect.ratio = CAspectRatio::AR_KEEP;
  else if (ratio.CompareNoCase("scale") == 0) aspect.ratio = CAspectRatio::AR_SCALE;
  else if (ratio.CompareNoCase("center") == 0) aspect.ratio = CAspectRatio::AR_CENTER;
  else if (ratio.CompareNoCase("stretch") == 0) aspect.ratio = CAspectRatio::AR_STRETCH;

  const char *attribute = node->Attribute("align");
  if (attribute)
  {
    CStdString align(attribute);
    if (align.CompareNoCase("center") == 0) aspect.align = ASPECT_ALIGN_CENTER | (aspect.align & ASPECT_ALIGNY_MASK);
    else if (align.CompareNoCase("right") == 0) aspect.align = ASPECT_ALIGN_RIGHT | (aspect.align & ASPECT_ALIGNY_MASK);
    else if (align.CompareNoCase("left") == 0) aspect.align = ASPECT_ALIGN_LEFT | (aspect.align & ASPECT_ALIGNY_MASK);
  }
  attribute = node->Attribute("aligny");
  if (attribute)
  {
    CStdString align(attribute);
    if (align.CompareNoCase("center") == 0) aspect.align = ASPECT_ALIGNY_CENTER | (aspect.align & ASPECT_ALIGN_MASK);
    else if (align.CompareNoCase("bottom") == 0) aspect.align = ASPECT_ALIGNY_BOTTOM | (aspect.align & ASPECT_ALIGN_MASK);
    else if (align.CompareNoCase("top") == 0) aspect.align = ASPECT_ALIGNY_TOP | (aspect.align & ASPECT_ALIGN_MASK);
  }
  attribute = node->Attribute("scalediffuse");
  if (attribute)
  {
    CStdString scale(attribute);
    if (scale.CompareNoCase("true") == 0 || scale.CompareNoCase("yes") == 0)
      aspect.scaleDiffuse = true;
    else
      aspect.scaleDiffuse = false;
  }
  return true;
}

bool CGUIControlFactory::GetInfoTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image, CGUIInfoLabel &info, int parentID)
{
  GetTexture(pRootNode, strTag, image);
  image.filename = "";
  GetInfoLabel(pRootNode, strTag, info, parentID);
  return true;
}

bool CGUIControlFactory::GetTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image)
{
  const TiXmlElement* pNode = pRootNode->FirstChildElement(strTag);
  if (!pNode) return false;
  const char *border = pNode->Attribute("border");
  if (border)
    GetRectFromString(border, image.border);
  image.orientation = 0;
  const char *flipX = pNode->Attribute("flipx");
  if (flipX && strcmpi(flipX, "true") == 0) image.orientation = 1;
  const char *flipY = pNode->Attribute("flipy");
  if (flipY && strcmpi(flipY, "true") == 0) image.orientation = 3 - image.orientation;  // either 3 or 2
  image.diffuse = pNode->Attribute("diffuse");
  image.diffuseColor.Parse(pNode->Attribute("colordiffuse"), 0);
  const char *background = pNode->Attribute("background");
  if (background && strnicmp(background, "true", 4) == 0)
    image.useLarge = true;
  image.filename = (pNode->FirstChild() && pNode->FirstChild()->ValueStr() != "-") ? pNode->FirstChild()->Value() : "";
  return true;
}

void CGUIControlFactory::GetRectFromString(const CStdString &string, FRECT &rect)
{
  // format is rect="left,right,top,bottom"
  CStdStringArray strRect;
  StringUtils::SplitString(string, ",", strRect);
  if (strRect.size() == 1)
  {
    g_SkinInfo.ResolveConstant(strRect[0], rect.left);
    rect.top = rect.left;
    rect.right = rect.left;
    rect.bottom = rect.left;
  }
  else if (strRect.size() == 4)
  {
    g_SkinInfo.ResolveConstant(strRect[0], rect.left);
    g_SkinInfo.ResolveConstant(strRect[1], rect.top);
    g_SkinInfo.ResolveConstant(strRect[2], rect.right);
    g_SkinInfo.ResolveConstant(strRect[3], rect.bottom);
  }
}

bool CGUIControlFactory::GetAlignment(const TiXmlNode* pRootNode, const char* strTag, uint32_t& alignment)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag);
  if (!pNode || !pNode->FirstChild()) return false;

  CStdString strAlign = pNode->FirstChild()->Value();
  if (strAlign == "right" || strAlign == "bottom") alignment = XBFONT_RIGHT;
  else if (strAlign == "center") alignment = XBFONT_CENTER_X;
  else if (strAlign == "justify") alignment = XBFONT_JUSTIFIED;
  else alignment = XBFONT_LEFT;
  return true;
}

bool CGUIControlFactory::GetAlignmentY(const TiXmlNode* pRootNode, const char* strTag, uint32_t& alignment)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild())
  {
    return false;
  }

  CStdString strAlign = pNode->FirstChild()->Value();

  alignment = 0;
  if (strAlign == "center")
    alignment = XBFONT_CENTER_Y;
  else if (strAlign == "bottom")
    alignment = XBFONT_BOTTOM;

  return true;
}

bool CGUIControlFactory::GetConditionalVisibility(const TiXmlNode* control, int &condition, CGUIInfoBool &allowHiddenFocus)
{
  const TiXmlElement* node = control->FirstChildElement("visible");
  if (!node) return false;
  vector<CStdString> conditions;
  while (node)
  {
    const char *hidden = node->Attribute("allowhiddenfocus");
    if (hidden)
      allowHiddenFocus.Parse(hidden);
    // add to our condition string
    if (!node->NoChildren())
      conditions.push_back(node->FirstChild()->Value());
    node = node->NextSiblingElement("visible");
  }
  if (!conditions.size())
    return false;
  if (conditions.size() == 1)
    condition = g_infoManager.TranslateString(conditions[0]);
  else
  { // multiple conditions should be anded together
    CStdString conditionString = "[";
    for (unsigned int i = 0; i < conditions.size() - 1; i++)
      conditionString += conditions[i] + "] + [";
    conditionString += conditions[conditions.size() - 1] + "]";
    condition = g_infoManager.TranslateString(conditionString);
  }
  return (condition != 0);
}

bool CGUIControlFactory::GetCondition(const TiXmlNode *control, const char *tag, int &condition)
{
  CStdString condString;
  if (XMLUtils::GetString(control, tag, condString))
  {
    condition = g_infoManager.TranslateString(condString);
    return true;
  }
  return false;
}

bool CGUIControlFactory::GetConditionalVisibility(const TiXmlNode *control, int &condition)
{
  CGUIInfoBool allowHiddenFocus;
  return GetConditionalVisibility(control, condition, allowHiddenFocus);
}

bool CGUIControlFactory::GetAnimations(const TiXmlNode *control, const FRECT &rect, vector<CAnimation> &animations)
{
  const TiXmlElement* node = control->FirstChildElement("animation");
  bool ret = false;
  if (node)
    animations.clear();
  while (node)
  {
    ret = true;
    if (node->FirstChild())
    {
      CAnimation anim;
      anim.Create(node, rect);
      animations.push_back(anim);
      if (strcmpi(node->FirstChild()->Value(), "VisibleChange") == 0)
      { // add the hidden one as well
        TiXmlElement hidden(*node);
        hidden.FirstChild()->SetValue("hidden");
        const char *start = hidden.Attribute("start");
        const char *end = hidden.Attribute("end");
        if (start && end)
        {
          CStdString temp = end;
          hidden.SetAttribute("end", start);
          hidden.SetAttribute("start", temp.c_str());
        }
        else if (start)
          hidden.SetAttribute("end", start);
        else if (end)
          hidden.SetAttribute("start", end);
        CAnimation anim2;
        anim2.Create(&hidden, rect);
        animations.push_back(anim2);
      }
    }
    node = node->NextSiblingElement("animation");
  }
  return ret;
}

bool CGUIControlFactory::GetActions(const TiXmlNode* pRootNode, const char* strTag, CGUIAction& action)
{
  action.m_actions.clear();
  const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag);
  while (pElement)
  {
    if (pElement->FirstChild())
    {
      CGUIAction::cond_action_pair pair;
      pair.condition = pElement->Attribute("condition");
      pair.action = pElement->FirstChild()->Value();
      action.m_actions.push_back(pair);
    }
    pElement = pElement->NextSiblingElement(strTag);
  }
  return action.m_actions.size() > 0;
}

bool CGUIControlFactory::GetHitRect(const TiXmlNode *control, CRect &rect)
{
  const TiXmlElement* node = control->FirstChildElement("hitrect");
  if (node)
  {
    if (node->Attribute("x")) g_SkinInfo.ResolveConstant(node->Attribute("x"), rect.x1);
    if (node->Attribute("y")) g_SkinInfo.ResolveConstant(node->Attribute("y"), rect.y1);
    if (node->Attribute("w"))
    {
      g_SkinInfo.ResolveConstant(node->Attribute("w"), rect.x2);
      rect.x2 += rect.x1;
    }
    if (node->Attribute("h"))
    {
      g_SkinInfo.ResolveConstant(node->Attribute("h"), rect.y2);
      rect.y2 += rect.y1;
    }
    return true;
  }
  return false;
}

bool CGUIControlFactory::GetColor(const TiXmlNode *control, const char *strTag, color_t &value)
{
  const TiXmlElement* node = control->FirstChildElement(strTag);
  if (node && node->FirstChild())
  {
    value = g_colorManager.GetColor(node->FirstChild()->Value());
    return true;
  }
  return false;
}

bool CGUIControlFactory::GetInfoColor(const TiXmlNode *control, const char *strTag, CGUIInfoColor &value,int parentID)
{
  const TiXmlElement* node = control->FirstChildElement(strTag);
  if (node && node->FirstChild())
  {
    value.Parse(node->FirstChild()->Value(), parentID);
    return true;
  }
  return false;
}

void CGUIControlFactory::GetInfoLabel(const TiXmlNode *pControlNode, const CStdString &labelTag, CGUIInfoLabel &infoLabel, int parentID)
{
  vector<CGUIInfoLabel> labels;
  GetInfoLabels(pControlNode, labelTag, labels, parentID);
  if (labels.size())
    infoLabel = labels[0];
}

bool CGUIControlFactory::GetInfoLabelFromElement(const TiXmlElement *element, CGUIInfoLabel &infoLabel, int parentID)
{
  if (!element || !element->FirstChild())
    return false;

  CStdString label = element->FirstChild()->Value();
  if (label.IsEmpty() || label == "-")
    return false;

  CStdString fallback = element->Attribute("fallback");
  if (StringUtils::IsNaturalNumber(label))
    label = g_localizeStrings.Get(atoi(label));
  else // we assume the skin xml's aren't encoded as UTF-8
    g_charsetConverter.unknownToUTF8(label);
  if (StringUtils::IsNaturalNumber(fallback))
    fallback = g_localizeStrings.Get(atoi(fallback));
  else
    g_charsetConverter.unknownToUTF8(fallback);
  infoLabel.SetLabel(label, fallback, parentID);
  return true;
}

void CGUIControlFactory::GetInfoLabels(const TiXmlNode *pControlNode, const CStdString &labelTag, vector<CGUIInfoLabel> &infoLabels, int parentID)
{
  // we can have the following infolabels:
  // 1.  <number>1234</number> -> direct number
  // 2.  <label>number</label> -> lookup in localizestrings
  // 3.  <label fallback="blah">$LOCALIZE(blah) $INFO(blah)</label> -> infolabel with given fallback
  // 4.  <info>ListItem.Album</info> (uses <label> as fallback)
  int labelNumber = 0;
  if (XMLUtils::GetInt(pControlNode, "number", labelNumber))
  {
    CStdString label;
    label.Format("%i", labelNumber);
    infoLabels.push_back(CGUIInfoLabel(label));
    return; // done
  }
  const TiXmlElement *labelNode = pControlNode->FirstChildElement(labelTag);
  while (labelNode)
  {
    CGUIInfoLabel label;
    if (GetInfoLabelFromElement(labelNode, label, parentID))
      infoLabels.push_back(label);
    labelNode = labelNode->NextSiblingElement(labelTag);
  }
  const TiXmlNode *infoNode = pControlNode->FirstChild("info");
  if (infoNode)
  { // <info> nodes override <label>'s (backward compatibility)
    CStdString fallback;
    if (infoLabels.size())
      fallback = infoLabels[0].GetLabel(0);
    infoLabels.clear();
    while (infoNode)
    {
      if (infoNode->FirstChild())
      {
        CStdString info;
        info.Format("$INFO[%s]", infoNode->FirstChild()->Value());
        infoLabels.push_back(CGUIInfoLabel(info, fallback, parentID));
      }
      infoNode = infoNode->NextSibling("info");
    }
  }
}

// Convert a string to a GUI label, by translating/parsing the label for localisable strings
CStdString CGUIControlFactory::FilterLabel(const CStdString &label)
{
  CStdString viewLabel = label;
  if (StringUtils::IsNaturalNumber(viewLabel))
    viewLabel = g_localizeStrings.Get(atoi(label));
  else
    g_charsetConverter.unknownToUTF8(viewLabel);
  return viewLabel;
}

bool CGUIControlFactory::GetString(const TiXmlNode* pRootNode, const char *strTag, CStdString &text)
{
  if (!XMLUtils::GetString(pRootNode, strTag, text))
    return false;
  if (text == "-")
    text.Empty();
  if (StringUtils::IsNaturalNumber(text))
    text = g_localizeStrings.Get(atoi(text.c_str()));
  else
    g_charsetConverter.unknownToUTF8(text);
  return true;
}

CStdString CGUIControlFactory::GetType(const TiXmlElement *pControlNode)
{
  CStdString type;
  const char *szType = pControlNode->Attribute("type");
  if (szType)
    type = szType;
  else  // backward compatibility - not desired
    XMLUtils::GetString(pControlNode, "type", type);
  return type;
}

CGUIControl* CGUIControlFactory::Create(int parentID, const FRECT &rect, TiXmlElement* pControlNode, bool insideContainer)
{
  // get the control type
  CStdString strType = GetType(pControlNode);
  CGUIControl::GUICONTROLTYPES type = TranslateControlType(strType);

  int id = 0;
  float posX = 0, posY = 0;
  float width = 0, height = 0;
  float minHeight = 0, minWidth = 0;

  int left = 0, right = 0, up = 0, down = 0, back = 0, next = 0, prev = 0;
  CGUIAction leftActions, rightActions, upActions, downActions, backActions, nextActions, prevActions;

  int pageControl = 0;
  CGUIInfoColor colorDiffuse(0xFFFFFFFF);
  int defaultControl = 0;
  bool  defaultAlways = false;
  CStdString strTmp;
  int singleInfo = 0;
  CStdString strLabel;
  int iUrlSet=0;
  int iToggleSelect;

  float spinWidth = 16;
  float spinHeight = 16;
  float spinPosX = 0, spinPosY = 0;
  float checkWidth = 0, checkHeight = 0;
  CStdString strSubType;
  int iType = SPIN_CONTROL_TYPE_TEXT;
  int iMin = 0;
  int iMax = 100;
  int iInterval = 1;
  float fMin = 0.0f;
  float fMax = 1.0f;
  float fInterval = 0.1f;
  bool bReverse = true;
  bool bReveal = false;
  CTextureInfo textureBackground, textureLeft, textureRight, textureMid, textureOverlay;
  CTextureInfo textureNib, textureNibFocus, textureBar, textureBarFocus;
  CTextureInfo textureLeftFocus, textureRightFocus;
  CTextureInfo textureUp, textureDown;
  CTextureInfo textureUpFocus, textureDownFocus;
  CTextureInfo texture, borderTexture;
  CGUIInfoLabel textureFile;
  CTextureInfo textureCheckMark, textureCheckMarkNF;
  CTextureInfo textureFocus, textureNoFocus;
  CTextureInfo textureAltFocus, textureAltNoFocus;
  CTextureInfo textureRadioOn, textureRadioOff;
  CTextureInfo imageNoFocus, imageFocus;
  CGUIInfoLabel texturePath;
  FRECT borderSize = { 0, 0, 0, 0};

  float sliderWidth = 150, sliderHeight = 16;
  CPoint offset;

  bool bHasPath = false;
  CGUIAction clickActions;
  CGUIAction altclickActions;
  CGUIAction focusActions;
  CGUIAction unfocusActions;
  CGUIAction textChangeActions;
  CStdString strTitle = "";
  CStdString strRSSTags = "";

  DWORD dwBuddyControlID = 0;
  int iNumSlots = 7;
  float buttonGap = 5;
  int iDefaultSlot = 2;
  int iMovementRange = 0;
  bool bHorizontal = false;
  int iAlpha = 0;
  bool bWrapAround = true;
  bool bSmoothScrolling = true;
  CAspectRatio aspect;
#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
  if (insideContainer)  // default for inside containers is keep
    aspect.ratio = CAspectRatio::AR_KEEP;
#endif

  int iVisibleCondition = 0;
  CGUIInfoBool allowHiddenFocus(false);
  int enableCondition = 0;

  vector<CAnimation> animations;

  CGUIControl::GUISCROLLVALUE scrollValue = CGUIControl::FOCUS;
  bool bPulse = true;
  unsigned int timePerImage = 0;
  unsigned int fadeTime = 0;
  unsigned int timeToPauseAtEnd = 0;
  bool randomized = false;
  bool loop = true;
  bool wrapMultiLine = false;
  ORIENTATION orientation = VERTICAL;
  bool showOnePage = true;
  bool scrollOut = true;
  int preloadItems = 0;

  CLabelInfo labelInfo;
  CLabelInfo spinInfo;

  CGUIInfoColor textColor3;
  CGUIInfoColor headlineColor;

  float radioWidth = 0;
  float radioHeight = 0;
  float radioPosX = 0;
  float radioPosY = 0;

  CStdString altLabel;
  CStdString strLabel2;

  int focusPosition = 0;
  int scrollTime = 200;
  bool useControlCoords = false;
  bool renderFocusedLast = false;

  CRect hitRect;
  CPoint camera;
  bool   hasCamera = false;
  bool resetOnLabelChange = true;
  bool bPassword = false;

  /////////////////////////////////////////////////////////////////////////////
  // Read control properties from XML
  //

  if (!pControlNode->Attribute("id", (int*) &id))
    XMLUtils::GetInt(pControlNode, "id", (int&) id);       // backward compatibility - not desired
  // TODO: Perhaps we should check here whether id is valid for focusable controls
  // such as buttons etc.  For labels/fadelabels/images it does not matter

  CStdString pos;
  CStdString perc;
  
  // Define screen size
  float screenWidth = rect.right - rect.left;
  float screenHeight = rect.bottom - rect.top;

  // Calculate left position
  if (GetFloat(pControlNode, "left", posX))
  {
    GetFloat(pControlNode, "left", posX);
    GetFloat(pControlNode, "width", width);
    
    XMLUtils::GetString(pControlNode, "left", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posX = (percValue * screenWidth) / 100.0f;
    }

    // Compatibility with older method
    XMLUtils::GetString(pControlNode, "left", pos);
    if (pos.Right(1) == "r")
    posX = screenWidth - posX;
  }

  // Calculate right position
  if (GetFloat(pControlNode, "right", posX))
  {
    GetFloat(pControlNode, "right", posX);
    GetFloat(pControlNode, "width", width);

    posX = screenWidth - posX - width;
    
    XMLUtils::GetString(pControlNode, "right", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posX = screenWidth - ((percValue * screenWidth) / 100.0f) - width;
    }
  }

  // Calculate top position
  if (GetFloat(pControlNode, "top", posY))
  {
    GetFloat(pControlNode, "top", posY);
    GetFloat(pControlNode, "height", height);
    
    XMLUtils::GetString(pControlNode, "top", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posY = ((percValue * screenHeight) / 100.0f);
    }

    // Compatibility with older method
    XMLUtils::GetString(pControlNode, "top", pos);
    if (pos.Right(1) == "r")
    posY = screenHeight - posY;
  }

  // Calculate bottom position
  if (GetFloat(pControlNode, "bottom", posY))
  {
    GetFloat(pControlNode, "bottom", posY);
    GetFloat(pControlNode, "height", height);

    posY = screenHeight - posY - height;
    
    XMLUtils::GetString(pControlNode, "bottom", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posY = screenHeight - ((percValue * screenHeight) / 100.0f) - height;
    }
  }

  // Calculate center-left position
  if (GetFloat(pControlNode, "centerleft", posX))
  {
    GetFloat(pControlNode, "centerleft", posX);
    GetFloat(pControlNode, "width", width);
    
    posX = posX - (width / 2);

    XMLUtils::GetString(pControlNode, "centerleft", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posX = (percValue * screenWidth) / 100.0f - (width / 2);
    }
  }

  // Calculate center-right position
  if (GetFloat(pControlNode, "centerright", posX))
  {
    GetFloat(pControlNode, "centerright", posX);
    GetFloat(pControlNode, "width", width);
    
    posX = screenWidth - posX - (width / 2);
    
    XMLUtils::GetString(pControlNode, "centerright", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posX = screenWidth - ((percValue * screenWidth) / 100.0f) - (width / 2);
    }
  }

  // Calculate center-top position
  if (GetFloat(pControlNode, "centertop", posY))
  {
    GetFloat(pControlNode, "centertop", posY);
    GetFloat(pControlNode, "height", height);
    
    posY = posY - (height / 2);
    
    XMLUtils::GetString(pControlNode, "centertop", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posY = ((percValue * screenHeight) / 100.0f) - (height / 2);
    }
  }

  // Calculate center-bottom position
  if (GetFloat(pControlNode, "centerbottom", posY))
  {
    GetFloat(pControlNode, "centerbottom", posY);
    GetFloat(pControlNode, "height", height);
    
    posY = screenHeight - posY - (height / 2);
    
    XMLUtils::GetString(pControlNode, "centerbottom", perc);
    if (perc[perc.size() - 1] == '%')
    {
    perc = perc.substr(0, perc.size() - 1);
    float percValue = static_cast<float>(strtod(perc.c_str(), NULL));
    posY = screenHeight - ((percValue * screenHeight) / 100.0f) - (height / 2);
    }
  }

  // Compatibility version for posx
  if (GetFloat(pControlNode, "posx", posX))
  {
    GetFloat(pControlNode, "posx", posX);
    
    // Convert these from relative coords
    XMLUtils::GetString(pControlNode, "posx", pos);
    if (pos.Right(1) == "r")
      posX = screenWidth - posX;
  }

  // Compatibility version for posy
  if (GetFloat(pControlNode, "posy", posY))
  {
    GetFloat(pControlNode, "posy", posY);
    
    // Convert these from relative coords
    XMLUtils::GetString(pControlNode, "posy", pos);
    if (pos.Right(1) == "r")
      posY = screenHeight - posY;
  }  

  GetDimension(pControlNode, "width", width, minWidth);
  GetDimension(pControlNode, "height", height, minHeight);
  GetFloat(pControlNode, "offsetx", offset.x);
  GetFloat(pControlNode, "offsety", offset.y);

  // adjust width and height accordingly for groups.  Groups should
  // take the width/height of the parent (adjusted for positioning)
  // if none is defined.
  if (type == CGUIControl::GUICONTROL_GROUP || type == CGUIControl::GUICONTROL_GROUPLIST)
  {
    if (!width)
      width = max(rect.right - posX, 0.0f);
    if (!height)
      height = max(rect.bottom - posY, 0.0f);
  }

  hitRect.SetRect(posX, posY, posX + width, posY + height);
  GetHitRect(pControlNode, hitRect);

  if (!GetActions(pControlNode, "onup",    upActions))    upActions.SetNavigation(id);
  if (!GetActions(pControlNode, "ondown",  downActions))  downActions.SetNavigation(id);
  if (!GetActions(pControlNode, "onleft",  leftActions))  leftActions.SetNavigation(id);
  if (!GetActions(pControlNode, "onright", rightActions)) rightActions.SetNavigation(id);
  if (!GetActions(pControlNode, "onnext",  nextActions))  nextActions.SetNavigation(id);
  if (!GetActions(pControlNode, "onprev",  prevActions))  prevActions.SetNavigation(id);
  GetActions(pControlNode, "onback",  backActions);

  if (XMLUtils::GetInt(pControlNode, "defaultcontrol", defaultControl))
  {
    const char *always = pControlNode->FirstChildElement("defaultcontrol")->Attribute("always");
    if (always && strnicmp(always, "true", 4) == 0)
      defaultAlways = true;
  }
  XMLUtils::GetInt(pControlNode, "pagecontrol", pageControl);

  GetInfoColor(pControlNode, "colordiffuse", colorDiffuse, parentID);

  GetConditionalVisibility(pControlNode, iVisibleCondition, allowHiddenFocus);
  GetCondition(pControlNode, "enable", enableCondition);

  // note: animrect here uses .right and .bottom as width and height respectively (nonstandard)
  FRECT animRect = { posX, posY, width, height };
  GetAnimations(pControlNode, animRect, animations);

  GetInfoColor(pControlNode, "textcolor", labelInfo.textColor, parentID);
  GetInfoColor(pControlNode, "focusedcolor", labelInfo.focusedColor, parentID);
  GetInfoColor(pControlNode, "disabledcolor", labelInfo.disabledColor, parentID);
  GetInfoColor(pControlNode, "shadowcolor", labelInfo.shadowColor, parentID);
  GetInfoColor(pControlNode, "selectedcolor", labelInfo.selectedColor, parentID);
  GetFloat(pControlNode, "textoffsetx", labelInfo.offsetX);
  GetFloat(pControlNode, "textoffsety", labelInfo.offsetY);
  int angle = 0;  // use the negative angle to compensate for our vertically flipped cartesian plane
  if (XMLUtils::GetInt(pControlNode, "angle", angle)) labelInfo.angle = (float)-angle;
  CStdString strFont;
  if (XMLUtils::GetString(pControlNode, "font", strFont))
    labelInfo.font = g_fontManager.GetFont(strFont);
  GetAlignment(pControlNode, "align", labelInfo.align);
  uint32_t alignY = 0;
  if (GetAlignmentY(pControlNode, "aligny", alignY))
    labelInfo.align |= alignY;
  if (GetFloat(pControlNode, "textwidth", labelInfo.width))
    labelInfo.align |= XBFONT_TRUNCATED;

  GetActions(pControlNode, "onclick", clickActions);
  GetActions(pControlNode, "ontextchange", textChangeActions);
  GetActions(pControlNode, "onfocus", focusActions);
  GetActions(pControlNode, "onunfocus", unfocusActions);
  focusActions.m_sendThreadMessages = unfocusActions.m_sendThreadMessages = true;
  GetActions(pControlNode, "altclick", altclickActions);

  CStdString infoString;
  if (XMLUtils::GetString(pControlNode, "info", infoString))
    singleInfo = g_infoManager.TranslateString(infoString);

  GetTexture(pControlNode, "texturefocus", textureFocus);
  GetTexture(pControlNode, "texturenofocus", textureNoFocus);
  GetTexture(pControlNode, "alttexturefocus", textureAltFocus);
  GetTexture(pControlNode, "alttexturenofocus", textureAltNoFocus);
  CStdString strToggleSelect;
  XMLUtils::GetString(pControlNode, "usealttexture", strToggleSelect);
  XMLUtils::GetString(pControlNode, "selected", strToggleSelect);
  iToggleSelect = g_infoManager.TranslateString(strToggleSelect);

  XMLUtils::GetBoolean(pControlNode, "haspath", bHasPath);

  GetTexture(pControlNode, "textureup", textureUp);
  GetTexture(pControlNode, "texturedown", textureDown);
  GetTexture(pControlNode, "textureupfocus", textureUpFocus);
  GetTexture(pControlNode, "texturedownfocus", textureDownFocus);

  GetTexture(pControlNode, "textureleft", textureLeft);
  GetTexture(pControlNode, "textureright", textureRight);
  GetTexture(pControlNode, "textureleftfocus", textureLeftFocus);
  GetTexture(pControlNode, "texturerightfocus", textureRightFocus);

  GetInfoColor(pControlNode, "spincolor", spinInfo.textColor, parentID);
  if (XMLUtils::GetString(pControlNode, "spinfont", strFont))
    spinInfo.font = g_fontManager.GetFont(strFont);
  if (!spinInfo.font) spinInfo.font = labelInfo.font;

  GetFloat(pControlNode, "spinwidth", spinWidth);
  GetFloat(pControlNode, "spinheight", spinHeight);
  GetFloat(pControlNode, "spinposx", spinPosX);
  GetFloat(pControlNode, "spinposy", spinPosY);
  GetFloat(pControlNode, "spinleft", spinPosX);
  GetFloat(pControlNode, "spintop", spinPosY);

  GetFloat(pControlNode, "markwidth", checkWidth);
  GetFloat(pControlNode, "markheight", checkHeight);
  GetFloat(pControlNode, "sliderwidth", sliderWidth);
  GetFloat(pControlNode, "sliderheight", sliderHeight);
  GetTexture(pControlNode, "texturecheckmark", textureCheckMark);
  GetTexture(pControlNode, "texturecheckmarknofocus", textureCheckMarkNF);
  GetTexture(pControlNode, "textureradiofocus", textureRadioOn);    // backward compatibility
  GetTexture(pControlNode, "textureradionofocus", textureRadioOff);
  GetTexture(pControlNode, "textureradioon", textureRadioOn);
  GetTexture(pControlNode, "textureradiooff", textureRadioOff);

  GetTexture(pControlNode, "texturesliderbackground", textureBackground);
  GetTexture(pControlNode, "texturesliderbar", textureBar);
  GetTexture(pControlNode, "texturesliderbarfocus", textureBarFocus);
  GetTexture(pControlNode, "textureslidernib", textureNib);
  GetTexture(pControlNode, "textureslidernibfocus", textureNibFocus);

  XMLUtils::GetString(pControlNode, "title", strTitle);
  XMLUtils::GetString(pControlNode, "tagset", strRSSTags);
  GetInfoColor(pControlNode, "headlinecolor", headlineColor, parentID);
  GetInfoColor(pControlNode, "titlecolor", textColor3, parentID);

  if (XMLUtils::GetString(pControlNode, "subtype", strSubType))
  {
    strSubType.ToLower();

    if ( strSubType == "int")
      iType = SPIN_CONTROL_TYPE_INT;
    else if ( strSubType == "page")
      iType = SPIN_CONTROL_TYPE_PAGE;
    else if ( strSubType == "float")
      iType = SPIN_CONTROL_TYPE_FLOAT;
    else
      iType = SPIN_CONTROL_TYPE_TEXT;
  }

  if (!GetIntRange(pControlNode, "range", iMin, iMax, iInterval))
  {
    GetFloatRange(pControlNode, "range", fMin, fMax, fInterval);
  }

  XMLUtils::GetBoolean(pControlNode, "reverse", bReverse);
  XMLUtils::GetBoolean(pControlNode, "reveal", bReveal);

  GetTexture(pControlNode, "texturebg", textureBackground);
  GetTexture(pControlNode, "lefttexture", textureLeft);
  GetTexture(pControlNode, "midtexture", textureMid);
  GetTexture(pControlNode, "righttexture", textureRight);
  GetTexture(pControlNode, "overlaytexture", textureOverlay);

  // the <texture> tag can be overridden by the <info> tag
  GetInfoTexture(pControlNode, "texture", texture, textureFile, parentID);

  GetTexture(pControlNode, "bordertexture", borderTexture);

  GetTexture(pControlNode, "imagefolder", imageNoFocus);
  GetTexture(pControlNode, "imagefolderfocus", imageFocus);

  // fade label can have a whole bunch, but most just have one
  vector<CGUIInfoLabel> infoLabels;
  GetInfoLabels(pControlNode, "label", infoLabels, parentID);

  GetString(pControlNode, "label", strLabel);
  GetString(pControlNode, "altlabel", altLabel);
  GetString(pControlNode, "label2", strLabel2);

  XMLUtils::GetBoolean(pControlNode, "wrapmultiline", wrapMultiLine);
  XMLUtils::GetInt(pControlNode,"urlset",iUrlSet);

  // stuff for button scroller
  if ( XMLUtils::GetString(pControlNode, "orientation", strTmp) )
  {
    if (strTmp.ToLower() == "horizontal")
    {
      bHorizontal = true;
      orientation = HORIZONTAL;
    }
  }
  GetFloat(pControlNode, "buttongap", buttonGap);
  GetFloat(pControlNode, "itemgap", buttonGap);
  XMLUtils::GetInt(pControlNode, "numbuttons", iNumSlots);
  XMLUtils::GetInt(pControlNode, "movement", iMovementRange);
  XMLUtils::GetInt(pControlNode, "defaultbutton", iDefaultSlot);
  XMLUtils::GetInt(pControlNode, "alpha", iAlpha);
  XMLUtils::GetBoolean(pControlNode, "wraparound", bWrapAround);
  XMLUtils::GetBoolean(pControlNode, "smoothscrolling", bSmoothScrolling);
  GetAspectRatio(pControlNode, "aspectratio", aspect);
  
  bool alwaysScroll;
  if (XMLUtils::GetBoolean(pControlNode, "scroll", alwaysScroll))
    scrollValue = alwaysScroll ? CGUIControl::ALWAYS : CGUIControl::NEVER;
  
  XMLUtils::GetBoolean(pControlNode,"pulseonselect", bPulse);

  GetInfoTexture(pControlNode, "imagepath", texture, texturePath, parentID);

  GetUnsigned(pControlNode,"timeperimage", timePerImage);
  GetUnsigned(pControlNode,"fadetime", fadeTime);
  GetUnsigned(pControlNode,"pauseatend", timeToPauseAtEnd);
  XMLUtils::GetBoolean(pControlNode, "randomize", randomized);
  XMLUtils::GetBoolean(pControlNode, "loop", loop);
  XMLUtils::GetBoolean(pControlNode, "scrollout", scrollOut);

  GetFloat(pControlNode, "radiowidth", radioWidth);
  GetFloat(pControlNode, "radioheight", radioHeight);
  GetFloat(pControlNode, "radioposx", radioPosX);
  GetFloat(pControlNode, "radioposy", radioPosY);
  GetFloat(pControlNode, "radioleft", radioPosX);
  GetFloat(pControlNode, "radiotop", radioPosY);
  CStdString borderStr;
  if (XMLUtils::GetString(pControlNode, "bordersize", borderStr))
    GetRectFromString(borderStr, borderSize);

  XMLUtils::GetBoolean(pControlNode, "showonepage", showOnePage);
  XMLUtils::GetInt(pControlNode, "focusposition", focusPosition);
  XMLUtils::GetInt(pControlNode, "scrolltime", scrollTime);
  XMLUtils::GetInt(pControlNode, "preloaditems", preloadItems, 0, 2);

  XMLUtils::GetBoolean(pControlNode, "usecontrolcoords", useControlCoords);
  XMLUtils::GetBoolean(pControlNode, "renderfocusedlast", renderFocusedLast);
  XMLUtils::GetBoolean(pControlNode, "resetonlabelchange", resetOnLabelChange);

  XMLUtils::GetBoolean(pControlNode, "password", bPassword);

  // view type
  VIEW_TYPE viewType = VIEW_TYPE_NONE;
  CStdString viewLabel;
  if (type == CGUIControl::GUICONTAINER_PANEL)
  {
    viewType = VIEW_TYPE_ICON;
    viewLabel = g_localizeStrings.Get(536);
  }
  else if (type == CGUIControl::GUICONTAINER_LIST)
  {
    viewType = VIEW_TYPE_LIST;
    viewLabel = g_localizeStrings.Get(535);
  }
  else
  {
    viewType = VIEW_TYPE_WRAP;
    viewLabel = g_localizeStrings.Get(541);
  }
  TiXmlElement *itemElement = pControlNode->FirstChildElement("viewtype");
  if (itemElement && itemElement->FirstChild())
  {
    CStdString type = itemElement->FirstChild()->Value();
    if (type == "list")
      viewType = VIEW_TYPE_LIST;
    else if (type == "icon")
      viewType = VIEW_TYPE_ICON;
    else if (type == "biglist")
      viewType = VIEW_TYPE_BIG_LIST;
    else if (type == "bigicon")
      viewType = VIEW_TYPE_BIG_ICON;
    else if (type == "wide")
      viewType = VIEW_TYPE_WIDE;
    else if (type == "bigwide")
      viewType = VIEW_TYPE_BIG_WIDE;
    else if (type == "wrap")
      viewType = VIEW_TYPE_WRAP;
    else if (type == "bigwrap")
      viewType = VIEW_TYPE_BIG_WRAP;
    const char *label = itemElement->Attribute("label");
    if (label)
      viewLabel = CGUIInfoLabel::GetLabel(FilterLabel(label));
  }

  TiXmlElement *cam = pControlNode->FirstChildElement("camera");
  if (cam)
  {
    hasCamera = true;
    g_SkinInfo.ResolveConstant(cam->Attribute("x"), camera.x);
    g_SkinInfo.ResolveConstant(cam->Attribute("y"), camera.y);
  }

  XMLUtils::GetInt(pControlNode, "scrollspeed", labelInfo.scrollSpeed);
  spinInfo.scrollSpeed = labelInfo.scrollSpeed;

  GetString(pControlNode, "scrollsuffix", labelInfo.scrollSuffix);
  spinInfo.scrollSuffix = labelInfo.scrollSuffix;

  /////////////////////////////////////////////////////////////////////////////
  // Instantiate a new control using the properties gathered above
  //

  CGUIControl *control = NULL;
  if (type == CGUIControl::GUICONTROL_GROUP)
  {
    if (insideContainer)
    {
      control = new CGUIListGroup(parentID, id, posX, posY, width, height);
    }
    else
    {
      control = new CGUIControlGroup(
        parentID, id, posX, posY, width, height);
      ((CGUIControlGroup *)control)->SetDefaultControl(defaultControl, defaultAlways);
      ((CGUIControlGroup *)control)->SetRenderFocusedLast(renderFocusedLast);
    }
  }
  else if (type == CGUIControl::GUICONTROL_GROUPLIST)
  {
    control = new CGUIControlGroupList(
      parentID, id, posX, posY, width, height, buttonGap, pageControl, orientation, useControlCoords, labelInfo.align, scrollTime);
    ((CGUIControlGroup *)control)->SetRenderFocusedLast(renderFocusedLast);
	((CGUIControlGroupList *)control)->SetMinSize(minWidth, minHeight);
  }
  else if (type == CGUIControl::GUICONTROL_LABEL)
  {
    const CGUIInfoLabel &content = (infoLabels.size()) ? infoLabels[0] : CGUIInfoLabel("");
    if (insideContainer)
    { // inside lists we use CGUIListLabel
      control = new CGUIListLabel(parentID, id, posX, posY, width, height, labelInfo, content, scrollValue);
    }
    else
    {
      control = new CGUILabelControl(
        parentID, id, posX, posY, width, height,
        labelInfo, wrapMultiLine, bHasPath);
      ((CGUILabelControl *)control)->SetInfo(content);
      ((CGUILabelControl *)control)->SetWidthControl(minWidth, (scrollValue == CGUIControl::ALWAYS) ? true : false);
    }
  }
  else if (type == CGUIControl::GUICONTROL_EDIT)
  {
    control = new CGUIEditControl(
      parentID, id, posX, posY, width, height, textureFocus, textureNoFocus,
      labelInfo, strLabel);

    CGUIInfoLabel hint_text;
    GetInfoLabel(pControlNode, "hinttext", hint_text, parentID);
    ((CGUIEditControl *) control)->SetHint(hint_text);

    if (bPassword)
      ((CGUIEditControl *) control)->SetInputType(CGUIEditControl::INPUT_TYPE_PASSWORD, 0);
    ((CGUIEditControl *) control)->SetTextChangeActions(textChangeActions);          
  }
  else if (type == CGUIControl::GUICONTROL_VIDEO)
  {
    control = new CGUIVideoControl(
      parentID, id, posX, posY, width, height);
  }
  else if (type == CGUIControl::GUICONTROL_FADELABEL)
  {
    control = new CGUIFadeLabelControl(
      parentID, id, posX, posY, width, height,
      labelInfo, scrollOut, timeToPauseAtEnd, resetOnLabelChange);

    ((CGUIFadeLabelControl *)control)->SetInfo(infoLabels);
  }
  else if (type == CGUIControl::GUICONTROL_RSS)
  {
    control = new CGUIRSSControl(
      parentID, id, posX, posY, width, height,
      labelInfo, textColor3, headlineColor, strRSSTags);

    std::map<int,CSettings::RssSet>::iterator iter=g_settings.m_mapRssUrls.find(iUrlSet);
    if (iter != g_settings.m_mapRssUrls.end())
    {
      ((CGUIRSSControl *)control)->SetUrls(iter->second.url,iter->second.rtl);
      ((CGUIRSSControl *)control)->SetIntervals(iter->second.interval);
    }
    else
      CLog::Log(LOGERROR,"invalid rss url set referenced in skin");
  }
  else if (type == CGUIControl::GUICONTROL_BUTTON)
  {
    control = new CGUIButtonControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus,
      labelInfo, wrapMultiLine);

    ((CGUIButtonControl *)control)->SetLabel(strLabel);
    ((CGUIButtonControl *)control)->SetLabel2(strLabel2);
	((CGUIButtonControl *)control)->SetMinWidth(minWidth);
    ((CGUIButtonControl *)control)->SetClickActions(clickActions);
    ((CGUIButtonControl *)control)->SetFocusActions(focusActions);
    ((CGUIButtonControl *)control)->SetUnFocusActions(unfocusActions);
  }
  else if (type == CGUIControl::GUICONTROL_TOGGLEBUTTON)
  {
    control = new CGUIToggleButtonControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus,
      textureAltFocus, textureAltNoFocus, labelInfo);

    ((CGUIToggleButtonControl *)control)->SetLabel(strLabel);
    ((CGUIToggleButtonControl *)control)->SetAltLabel(altLabel);
    ((CGUIToggleButtonControl *)control)->SetClickActions(clickActions);
    ((CGUIToggleButtonControl *)control)->SetAltClickActions(altclickActions);
    ((CGUIToggleButtonControl *)control)->SetFocusActions(focusActions);
    ((CGUIToggleButtonControl *)control)->SetUnFocusActions(unfocusActions);
    ((CGUIToggleButtonControl *)control)->SetToggleSelect(iToggleSelect);
  }
  else if (type == CGUIControl::GUICONTROL_CHECKMARK)
  {
    control = new CGUICheckMarkControl(
      parentID, id, posX, posY, width, height,
      textureCheckMark, textureCheckMarkNF,
      checkWidth, checkHeight, labelInfo);

    ((CGUICheckMarkControl *)control)->SetLabel(strLabel);
  }
  else if (type == CGUIControl::GUICONTROL_RADIO)
  {
    control = new CGUIRadioButtonControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus,
      labelInfo,
      textureRadioOn, textureRadioOff);

    ((CGUIRadioButtonControl *)control)->SetLabel(strLabel);
    ((CGUIRadioButtonControl *)control)->SetRadioDimensions(radioPosX, radioPosY, radioWidth, radioHeight);
    ((CGUIRadioButtonControl *)control)->SetToggleSelect(iToggleSelect);
    ((CGUIRadioButtonControl *)control)->SetClickActions(clickActions);
    ((CGUIRadioButtonControl *)control)->SetFocusActions(focusActions);
    ((CGUIRadioButtonControl *)control)->SetUnFocusActions(unfocusActions);
  }
  else if (type == CGUIControl::GUICONTROL_MULTISELECT)
  {
    CGUIInfoLabel label;
    if (infoLabels.size())
      label = infoLabels[0];
    control = new CGUIMultiSelectTextControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus, labelInfo, label);
  }
  else if (type == CGUIControl::GUICONTROL_SPIN)
  {
    control = new CGUISpinControl(
      parentID, id, posX, posY, width, height,
      textureUp, textureDown, textureUpFocus, textureDownFocus,
      labelInfo, iType);

    ((CGUISpinControl *)control)->SetReverse(bReverse);

    if (iType == SPIN_CONTROL_TYPE_INT)
    {
      ((CGUISpinControl *)control)->SetRange(iMin, iMax);
    }
    else if (iType == SPIN_CONTROL_TYPE_PAGE)
    {
      ((CGUISpinControl *)control)->SetRange(iMin, iMax);
      ((CGUISpinControl *)control)->SetShowRange(true);
      ((CGUISpinControl *)control)->SetReverse(false);
      ((CGUISpinControl *)control)->SetShowOnePage(showOnePage);
    }
    else if (iType == SPIN_CONTROL_TYPE_FLOAT)
    {
      ((CGUISpinControl *)control)->SetFloatRange(fMin, fMax);
      ((CGUISpinControl *)control)->SetFloatInterval(fInterval);
    }
  }
  else if (type == CGUIControl::GUICONTROL_SLIDER)
  {
    control = new CGUISliderControl(
      parentID, id, posX, posY, width, height,
      textureBar, textureNib, textureNibFocus, SPIN_CONTROL_TYPE_TEXT);

    ((CGUISliderControl *)control)->SetInfo(singleInfo);
  }
  else if (type == CGUIControl::GUICONTROL_SETTINGS_SLIDER)
  {
    labelInfo.align |= XBFONT_CENTER_Y;    // always center text vertically
    control = new CGUISettingsSliderControl(
      parentID, id, posX, posY, width, height, sliderWidth, sliderHeight, textureFocus, textureNoFocus,
      textureBar, textureNib, textureNibFocus, labelInfo, SPIN_CONTROL_TYPE_TEXT);

    ((CGUISettingsSliderControl *)control)->SetText(strLabel);
    ((CGUISettingsSliderControl *)control)->SetInfo(singleInfo);
  }
  else if (type == CGUIControl::GUICONTROL_SCROLLBAR)
  {
    control = new CGUIScrollBar(
      parentID, id, posX, posY, width, height,
      textureBackground, textureBar, textureBarFocus, textureNib, textureNibFocus, orientation, showOnePage);
  }
  else if (type == CGUIControl::GUICONTROL_PROGRESS)
  {
    control = new CGUIProgressControl(
      parentID, id, posX, posY, width, height,
      textureBackground, textureLeft, textureMid, textureRight,
      textureOverlay, bReveal);
    ((CGUIProgressControl *)control)->SetInfo(singleInfo);
  }
  else if (type == CGUIControl::GUICONTROL_IMAGE)
  {
    if (strType == "largeimage")
      texture.useLarge = true;

    // use a bordered texture if we have <bordersize> or <bordertexture> specified.
    if (borderTexture.filename.IsEmpty() && borderStr.IsEmpty())
      control = new CGUIImage(
        parentID, id, posX, posY, width, height, texture);
    else
      control = new CGUIBorderedImage(
        parentID, id, posX, posY, width, height, texture, borderTexture, borderSize);
#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
    if (insideContainer && textureFile.IsConstant())
      aspect.ratio = CAspectRatio::AR_STRETCH;
#endif
    ((CGUIImage *)control)->SetInfo(textureFile);
    ((CGUIImage *)control)->SetAspectRatio(aspect);
    ((CGUIImage *)control)->SetCrossFade(fadeTime);
  }
  else if (type == CGUIControl::GUICONTROL_MULTI_IMAGE)
  {
    control = new CGUIMultiImage(
      parentID, id, posX, posY, width, height, texture, timePerImage, fadeTime, randomized, loop, timeToPauseAtEnd);
    ((CGUIMultiImage *)control)->SetInfo(texturePath);
    ((CGUIMultiImage *)control)->SetAspectRatio(aspect);
  }
  else if (type == CGUIControl::GUICONTAINER_LIST)
  {
    control = new CGUIListContainer(parentID, id, posX, posY, width, height, orientation, scrollTime, preloadItems);
    ((CGUIListContainer *)control)->LoadLayout(pControlNode);
    ((CGUIListContainer *)control)->LoadContent(pControlNode);
    ((CGUIListContainer *)control)->SetType(viewType, viewLabel);
    ((CGUIListContainer *)control)->SetPageControl(pageControl);
    ((CGUIListContainer *)control)->SetRenderOffset(offset);
  }
  else if (type == CGUIControl::GUICONTAINER_WRAPLIST)
  {
    control = new CGUIWrappingListContainer(parentID, id, posX, posY, width, height, orientation, scrollTime, preloadItems, focusPosition);
    ((CGUIWrappingListContainer *)control)->LoadLayout(pControlNode);
    ((CGUIWrappingListContainer *)control)->LoadContent(pControlNode);
    ((CGUIWrappingListContainer *)control)->SetType(viewType, viewLabel);
    ((CGUIWrappingListContainer *)control)->SetPageControl(pageControl);
    ((CGUIWrappingListContainer *)control)->SetRenderOffset(offset);
  }
  else if (type == CGUIControl::GUICONTAINER_FIXEDLIST)
  {
    control = new CGUIFixedListContainer(parentID, id, posX, posY, width, height, orientation, scrollTime, preloadItems, focusPosition, iMovementRange);
    ((CGUIFixedListContainer *)control)->LoadLayout(pControlNode);
    ((CGUIFixedListContainer *)control)->LoadContent(pControlNode);
    ((CGUIFixedListContainer *)control)->SetType(viewType, viewLabel);
    ((CGUIFixedListContainer *)control)->SetPageControl(pageControl);
    ((CGUIFixedListContainer *)control)->SetRenderOffset(offset);
  }
  else if (type == CGUIControl::GUICONTAINER_PANEL)
  {
    control = new CGUIPanelContainer(parentID, id, posX, posY, width, height, orientation, scrollTime, preloadItems);
    ((CGUIPanelContainer *)control)->LoadLayout(pControlNode);
    ((CGUIPanelContainer *)control)->LoadContent(pControlNode);
    ((CGUIPanelContainer *)control)->SetType(viewType, viewLabel);
    ((CGUIPanelContainer *)control)->SetPageControl(pageControl);
    ((CGUIPanelContainer *)control)->SetRenderOffset(offset);
  }
  else if (type == CGUIControl::GUICONTROL_TEXTBOX)
  {
    control = new CGUITextBox(
      parentID, id, posX, posY, width, height,
      labelInfo, scrollTime);

    ((CGUITextBox *)control)->SetPageControl(pageControl);
    if (infoLabels.size())
      ((CGUITextBox *)control)->SetInfo(infoLabels[0]);
    ((CGUITextBox *)control)->SetAutoScrolling(pControlNode);
  }
  else if (type == CGUIControl::GUICONTROL_SELECTBUTTON)
  {
    control = new CGUISelectButtonControl(
      parentID, id, posX, posY,
      width, height, textureFocus, textureNoFocus,
      labelInfo,
      textureBackground, textureLeft, textureLeftFocus, textureRight, textureRightFocus);

    ((CGUISelectButtonControl *)control)->SetLabel(strLabel);
  }
  else if (type == CGUIControl::GUICONTROL_MOVER)
  {
    control = new CGUIMoverControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus);
  }
  else if (type == CGUIControl::GUICONTROL_RESIZE)
  {
    control = new CGUIResizeControl(
      parentID, id, posX, posY, width, height,
      textureFocus, textureNoFocus);
  }
  else if (type == CGUIControl::GUICONTROL_BUTTONBAR)
  {
    control = new CGUIButtonScroller(
      parentID, id, posX, posY, width, height, buttonGap, iNumSlots, iDefaultSlot,
      iMovementRange, bHorizontal, iAlpha, bWrapAround, bSmoothScrolling,
      textureFocus, textureNoFocus, labelInfo);
    ((CGUIButtonScroller *)control)->LoadButtons(pControlNode);
  }
  else if (type == CGUIControl::GUICONTROL_SPINEX)
  {
    control = new CGUISpinControlEx(
      parentID, id, posX, posY, width, height, spinWidth, spinHeight,
      labelInfo, textureFocus, textureNoFocus, textureUp, textureDown, textureUpFocus, textureDownFocus,
      labelInfo, iType);

    ((CGUISpinControlEx *)control)->SetSpinPosition(spinPosX);
    ((CGUISpinControlEx *)control)->SetText(strLabel);
    ((CGUISpinControlEx *)control)->SetReverse(bReverse);
  }
  else if (type == CGUIControl::GUICONTROL_VISUALISATION)
  {
    control = new CGUIVisualisationControl(parentID, id, posX, posY, width, height);
  }

  // things that apply to all controls
  if (control)
  {
    control->SetHitRect(hitRect);
    control->SetVisibleCondition(iVisibleCondition, allowHiddenFocus);
    control->SetEnableCondition(enableCondition);
    control->SetAnimations(animations);
    control->SetColorDiffuse(colorDiffuse);
    control->SetNavigationActions(upActions, downActions, leftActions, rightActions, backActions);
    control->SetPulseOnSelect(bPulse);
    if (hasCamera)
      control->SetCamera(camera);
  }
  return control;
}

void CGUIControlFactory::ScaleElement(TiXmlElement *element, RESOLUTION fileRes, RESOLUTION destRes)
{
  if (element->FirstChild())
  {
    const char *value = element->FirstChild()->Value();
    if (value)
    {
      float v = (float)atof(value);
      CStdString name = element->Value();
      if (name == "posx" ||
          name == "left" ||
          name == "right" ||
          name == "centerleft" ||
          name == "centerright" ||
          name == "width" ||
          name == "textoffsetx" ||
          name == "textwidth" ||
          name == "spinwidth" ||
          name == "spinposx" ||
          name == "spinleft" ||
          name == "markwidth" ||
          name == "sliderwidth" ||
          name == "radiowidth" ||
          name == "radioposx" ||
          name == "radioleft")
      {
        // scale
        v *= (float)g_settings.m_ResInfo[destRes].iWidth / g_settings.m_ResInfo[fileRes].iWidth;
        CStdString floatValue;
        floatValue.Format("%f", v);
        element->FirstChild()->SetValue(floatValue);
      }
      else if (name == "posy" ||
          name == "top" ||
          name == "bottom" ||
          name == "centerleft" ||
          name == "centerright" ||
          name == "height" ||
          name == "textoffsety" ||
          name == "spinheight" ||
          name == "spinposy" ||
          name == "spintop" ||
          name == "markheight" ||
          name == "sliderheight" ||
          name == "buttongap" ||  // should really depend on orientation
          name == "radioheight" ||
          name == "radioposy" ||
          name == "radiotop")
      {
        // scale
        v *= (float)g_settings.m_ResInfo[destRes].iHeight / g_settings.m_ResInfo[fileRes].iHeight;
        CStdString floatValue;
        floatValue.Format("%f", v);
        element->FirstChild()->SetValue(floatValue);
      }
    }
  }
}
