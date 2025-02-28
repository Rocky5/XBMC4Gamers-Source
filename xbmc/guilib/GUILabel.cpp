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

#define NOMINMAX
#include "GUILabel.h"
#include "SkinInfo.h"
#include "utils/CharsetConverter.h"
#include <limits>


CGUILabel::CGUILabel(float posX, float posY, float width, float height, const CLabelInfo& labelInfo, CGUILabel::OVER_FLOW overflow)
    : m_textLayout(labelInfo.font, overflow == OVER_FLOW_WRAP, height)
    , m_scrollInfo(50, 0, labelInfo.scrollSpeed, labelInfo.scrollSuffix)
    , m_maxRect(posX, posY, posX + width, posY + height)
{
  m_selected = false;
  m_overflowType = overflow;
  m_scrolling = (overflow == OVER_FLOW_SCROLL);
  m_label = labelInfo;
  m_invalid = true;
}

CGUILabel::~CGUILabel(void)
{
}

void CGUILabel::SetScrolling(bool scrolling)
{
  m_scrolling = scrolling;
  if (!m_scrolling)
    m_scrollInfo.Reset();
}

void CGUILabel::SetColor(CGUILabel::COLOR color)
{
  m_color = color;
}

color_t CGUILabel::GetColor() const
{
  switch (m_color)
  {
    case COLOR_SELECTED:
      return m_label.selectedColor;
    case COLOR_DISABLED:
      return m_label.disabledColor;
    case COLOR_FOCUSED:
      return m_label.focusedColor ? m_label.focusedColor : m_label.textColor;
    default:
      break;
  }
  return m_label.textColor;
}

void CGUILabel::Render()
{
  color_t color = GetColor();
  bool renderSolid = (m_color == COLOR_DISABLED);
  bool overFlows = (m_renderRect.Width() + 0.5f < m_textLayout.GetTextWidth()); // 0.5f to deal with floating point rounding issues

  // compatibility for old skins like pm3 where text overflowed even if a width set,
  // and with no width set, the right alignment was based on calculated length.
  if (g_SkinInfo.GetLegacy() && overFlows && !m_renderRect.Width()) {
    overFlows = false;
    m_renderRect.x2 = m_renderRect.x1 + m_textLayout.GetTextWidth();
    if (m_label.align & XBFONT_RIGHT) {
      m_renderRect.x1 -= m_textLayout.GetTextWidth();
      m_renderRect.x2 -= m_textLayout.GetTextWidth();
    }
  }

  if (overFlows && m_scrolling && !renderSolid)
    m_textLayout.RenderScrolling(m_renderRect.x1, m_renderRect.y1, m_label.angle, color, m_label.shadowColor, 0, m_renderRect.Width(), m_scrollInfo);
  else
  {
    float posX = m_renderRect.x1;
    float posY = m_renderRect.y1;
    uint32_t align = 0;
    if (!overFlows)
    { // hack for right and centered multiline text, as GUITextLayout::Render() treats posX as the right hand
      // or center edge of the text (see GUIFontTTF::DrawTextInternal), and this has already been taken care of
      // in UpdateRenderRect(), but we wish to still pass the horizontal alignment info through (so that multiline text
      // is aligned correctly), so we must undo the UpdateRenderRect() changes for horizontal alignment.
      if (m_label.align & XBFONT_RIGHT)
        posX += m_renderRect.Width();
      else if (m_label.align & XBFONT_CENTER_X)
        posX += m_renderRect.Width() * 0.5f;
      if (m_label.align & XBFONT_CENTER_Y) // need to pass a centered Y so that <angle> will rotate around the correct point.
        posY += m_renderRect.Height() * 0.5f;
      align = m_label.align;
    }
    else
      align |= XBFONT_TRUNCATED;
    m_textLayout.Render(posX, posY, m_label.angle, color, m_label.shadowColor, align, m_renderRect.Width(), renderSolid);
  }
}

void CGUILabel::SetInvalid()
{
  m_invalid = true;
}

void CGUILabel::UpdateColors()
{
  m_label.UpdateColors();
}

void CGUILabel::SetMaxRect(float x, float y, float w, float h)
{
  m_maxRect.SetRect(x, y, x + w, y + h);
  UpdateRenderRect();
}

void CGUILabel::SetAlign(uint32_t align)
{
  m_label.align = align;
  UpdateRenderRect();
}

void CGUILabel::SetText(const CStdString &label)
{
  if (m_textLayout.Update(label, m_maxRect.Width(), m_invalid))
  { // needed an update - reset scrolling and update our text layout
    m_scrollInfo.Reset();
    UpdateRenderRect();
    m_invalid = false;
  }
}

void CGUILabel::SetTextW(const CStdStringW &label)
{
  m_textLayout.SetText(label);
  m_scrollInfo.Reset();
  UpdateRenderRect();
  m_invalid = false;
}

void CGUILabel::UpdateRenderRect()
{
  // recalculate our text layout
  float width, height;
  m_textLayout.GetTextExtent(width, height);
  width = std::min(width, GetMaxWidth());
  if (m_label.align & XBFONT_CENTER_Y)
    m_renderRect.y1 = m_maxRect.y1 + (m_maxRect.Height() - height) * 0.5f;
  else
    m_renderRect.y1 = m_maxRect.y1 + m_label.offsetY;
  if (m_label.align & XBFONT_RIGHT)
    m_renderRect.x1 = m_maxRect.x2 - width - m_label.offsetX;
  else if (m_label.align & XBFONT_CENTER_X)
    m_renderRect.x1 = m_maxRect.x1 + (m_maxRect.Width() - width) * 0.5f;
  else
    m_renderRect.x1 = m_maxRect.x1 + m_label.offsetX;
  m_renderRect.x2 = m_renderRect.x1 + width;
  m_renderRect.y2 = m_renderRect.y1 + height;
}

float CGUILabel::GetMaxWidth() const
{
  if (m_label.width) return m_label.width;
  return m_maxRect.Width() - 2*m_label.offsetX;
}

void CGUILabel::CheckAndCorrectOverlap(CGUILabel &label1, CGUILabel &label2)
{
  CRect rect(label1.m_renderRect);
  if (rect.Intersect(label2.m_renderRect).IsEmpty())
    return; // nothing to do (though it could potentially encroach on the min_space requirement)

  static const float min_space = 10.0;
  // overlap vertically and horizontally - check alignment
  CGUILabel &left = label1.m_renderRect.x1 <= label2.m_renderRect.x1 ? label1 : label2;
  CGUILabel &right = label1.m_renderRect.x1 <= label2.m_renderRect.x1 ? label2 : label1;
  if ((left.m_label.align & 3) == 0 && right.m_label.align & XBFONT_RIGHT)
  {
    float chopPoint = (left.m_maxRect.x1 + left.GetMaxWidth() + right.m_maxRect.x2 - right.GetMaxWidth()) * 0.5f;
    // [1       [2...[2  1].|..........1]         2]
    // [1       [2.....[2   |      1]..1]         2]
    // [1       [2..........|.[2   1]..1]         2]
    if (right.m_renderRect.x1 > chopPoint)
      chopPoint = right.m_renderRect.x1 - min_space;
    else if (left.m_renderRect.x2 < chopPoint)
      chopPoint = left.m_renderRect.x2 + min_space;
    left.m_renderRect.x2 = chopPoint - min_space;
    right.m_renderRect.x1 = chopPoint + min_space;
  }
}
