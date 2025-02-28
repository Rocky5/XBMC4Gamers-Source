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
#include "GUIFixedListContainer.h"
#include "GUIListItem.h"
#include "GUIInfoManager.h"

CGUIFixedListContainer::CGUIFixedListContainer(int parentID, int controlID, float posX, float posY, float width, float height, ORIENTATION orientation, int scrollTime, int preloadItems, int fixedPosition, int cursorRange)
    : CGUIBaseContainer(parentID, controlID, posX, posY, width, height, orientation, scrollTime, preloadItems)
{
  ControlType = GUICONTAINER_FIXEDLIST;
  m_type = VIEW_TYPE_LIST;
  m_fixedCursor = fixedPosition;
  m_cursorRange = max(0, cursorRange);
  m_cursor = m_fixedCursor;
}

CGUIFixedListContainer::~CGUIFixedListContainer(void)
{
}

bool CGUIFixedListContainer::OnAction(const CAction &action)
{
  switch (action.GetID())
  {
  case ACTION_PAGE_UP:
    {
        Scroll(-m_itemsPerPage);
      return true;
    }
    break;
  case ACTION_PAGE_DOWN:
    {
        Scroll(m_itemsPerPage);
      return true;
    }
    break;
    // smooth scrolling (for analog controls)
  case ACTION_SCROLL_UP:
    {
      m_analogScrollCount += action.GetAmount() * action.GetAmount();
      bool handled = false;
      while (m_analogScrollCount > AnalogScrollSpeed())
      {
        handled = true;
        m_analogScrollCount -= AnalogScrollSpeed();
        Scroll(-1);
      }
      return handled;
    }
    break;
  case ACTION_SCROLL_DOWN:
    {
      m_analogScrollCount += action.GetAmount() * action.GetAmount();
      bool handled = false;
      while (m_analogScrollCount > AnalogScrollSpeed())
      {
        handled = true;
        m_analogScrollCount -= AnalogScrollSpeed();
        Scroll(1);
      }
      return handled;
    }
    break;
  }
  return CGUIBaseContainer::OnAction(action);
}

float CGUIFixedListContainer::AnalogScrollSpeed() const
{
  return 4.0f / m_itemsPerPage;
}

bool CGUIFixedListContainer::MoveUp(bool wrapAround)
{
  int item = GetSelectedItem();
  if (item > 0)
    SelectItem(item - 1);
  else if (wrapAround)
  {
    SelectItem((int)m_items.size() - 1);
    SetContainerMoving(-1);
  }
  else
    return false;
  return true;
}

bool CGUIFixedListContainer::MoveDown(bool wrapAround)
{
  int item = GetSelectedItem();
  if (item < (int)m_items.size() - 1)
    SelectItem(item + 1);
  else if (wrapAround)
  { // move first item in list
    SelectItem(0);
    SetContainerMoving(1);
  }
  else
    return false;
  return true;
}

// scrolls the said amount
void CGUIFixedListContainer::Scroll(int amount)
{
  // increase or decrease the offset
  int item = m_offset + m_cursor + amount;
  // check for our end points
  if (item >= (int)m_items.size() - 1)
    item = (int)m_items.size() - 1;
  if (item < 0)
    item = 0;
  SelectItem(item);
}

void CGUIFixedListContainer::ValidateOffset()
{
  if (!m_layout) return;
  // ensure our fixed cursor position is valid
  if (m_fixedCursor >= m_itemsPerPage)
    m_fixedCursor = m_itemsPerPage - 1;
  if (m_fixedCursor < 0)
    m_fixedCursor = 0;
  // compute our minimum and maximum cursor positions
  int minCursor, maxCursor;
  GetCursorRange(minCursor, maxCursor);
  // assure our cursor is between these limits
  m_cursor = max(m_cursor, minCursor);
  m_cursor = min(m_cursor, maxCursor);
  // and finally ensure our offset is valid
  if (m_offset + maxCursor >= (int)m_items.size() || m_scrollOffset > ((int)m_items.size() - maxCursor - 1) * m_layout->Size(m_orientation))
  {
    m_offset = m_items.size() - maxCursor - 1;
    m_scrollOffset = m_offset * m_layout->Size(m_orientation);
  }
  if (m_offset < -minCursor || m_scrollOffset < -minCursor * m_layout->Size(m_orientation))
  {
    m_offset = -minCursor;
    m_scrollOffset = m_offset * m_layout->Size(m_orientation);
  }
}

int CGUIFixedListContainer::GetCursorFromPoint(const CPoint &point, CPoint *itemPoint) const
{
  if (!m_focusedLayout || !m_layout)
    return -1;
  int minCursor, maxCursor;
  GetCursorRange(minCursor, maxCursor);
  // see if the point is either side of our focus range
  float start = (minCursor + 0.2f) * m_layout->Size(m_orientation);
  float end = (maxCursor - 0.2f) * m_layout->Size(m_orientation) + m_focusedLayout->Size(m_orientation);
  float pos = (m_orientation == VERTICAL) ? point.y : point.x;
  if (pos >= start && pos <= end)
  { // select the appropriate item
    pos -= minCursor * m_layout->Size(m_orientation);
    for (int row = minCursor; row <= maxCursor; row++)
    {
      const CGUIListItemLayout *layout = (row == m_cursor) ? m_focusedLayout : m_layout;
      if (pos < layout->Size(m_orientation))
      {
        if (!InsideLayout(layout, point))
          return -1;
        return row;
      }
      pos -= layout->Size(m_orientation);
    }
  }
  return -1;
}

bool CGUIFixedListContainer::SelectItemFromPoint(const CPoint &point)
{
  if (!m_focusedLayout || !m_layout)
    return false;

  const float mouse_scroll_speed = 0.25f;
  const float mouse_max_amount = 1.5f;
  float sizeOfItem = m_layout->Size(m_orientation);
  int minCursor, maxCursor;
  GetCursorRange(minCursor, maxCursor);
  // see if the point is either side of our focus range
  float start = (minCursor + 0.2f) * sizeOfItem;
  float end = (maxCursor - 0.2f) * sizeOfItem + m_focusedLayout->Size(m_orientation);
  float pos = (m_orientation == VERTICAL) ? point.y : point.x;
  if (pos < start && m_offset > -minCursor)
  { // scroll backward
    if (!InsideLayout(m_layout, point))
      return false;
    float amount = min((start - pos) / sizeOfItem, mouse_max_amount);
    m_analogScrollCount += amount * amount * mouse_scroll_speed;
    if (m_analogScrollCount > 1)
    {
      ScrollToOffset(m_offset - 1);
      m_analogScrollCount = 0;
    }
    return true;
  }
  else if (pos > end && m_offset + maxCursor < (int)m_items.size() - 1)
  {
    if (!InsideLayout(m_layout, point))
      return false;
    // scroll forward
    float amount = min((pos - end) / sizeOfItem, mouse_max_amount);
    m_analogScrollCount += amount * amount * mouse_scroll_speed;
    if (m_analogScrollCount > 1)
    {
      ScrollToOffset(m_offset + 1);
      m_analogScrollCount = 0;
    }
    return true;
  }
  else
  { // select the appropriate item
    int cursor = GetCursorFromPoint(point);
    if (cursor < 0)
      return false;
    // calling SelectItem() here will focus the item and scroll, which isn't really what we're after
    m_cursor = cursor;
    return true;
  }
  return InsideLayout(m_focusedLayout, point);
}

void CGUIFixedListContainer::SelectItem(int item)
{
  // Check that m_offset is valid
  ValidateOffset();
  // only select an item if it's in a valid range
  if (item >= 0 && item < (int)m_items.size())
  {
    // Select the item requested - we first set the cursor position
    // which may be different at either end of the list, then the offset
    int minCursor, maxCursor;
    GetCursorRange(minCursor, maxCursor);

    int cursor = m_cursor;
    if ((int)m_items.size() - 1 - item <= maxCursor - m_fixedCursor)
      cursor = max(m_fixedCursor, maxCursor + item - (int)m_items.size() + 1);
    else if (item <= m_fixedCursor - minCursor)
      cursor = min(m_fixedCursor, minCursor + item);
    else
      cursor = m_fixedCursor;
    if (cursor != m_cursor)
      SetContainerMoving(cursor - m_cursor);
    m_cursor = cursor;
    ScrollToOffset(item - m_cursor);
  }
}

bool CGUIFixedListContainer::HasPreviousPage() const
{
  return (m_offset > 0);
}

bool CGUIFixedListContainer::HasNextPage() const
{
  return (m_offset != (int)m_items.size() - m_itemsPerPage && (int)m_items.size() >= m_itemsPerPage);
}

int CGUIFixedListContainer::GetCurrentPage() const
{
  int offset = CorrectOffset(m_offset, m_cursor);
  if (offset + m_itemsPerPage - m_cursor >= (int)GetRows())  // last page
    return (GetRows() + m_itemsPerPage - 1) / m_itemsPerPage;
  return offset / m_itemsPerPage + 1;
}

void CGUIFixedListContainer::GetCursorRange(int &minCursor, int &maxCursor) const
{
  minCursor = max(m_fixedCursor - m_cursorRange, 0);
  maxCursor = min(m_fixedCursor + m_cursorRange, m_itemsPerPage);

  if (!m_items.size())
  {
    minCursor = m_fixedCursor;
    maxCursor = m_fixedCursor;
    return;
  }

  while (maxCursor - minCursor > (int)m_items.size() - 1)
  {
    if (maxCursor - m_fixedCursor > m_fixedCursor - minCursor)
      maxCursor--;
    else
      minCursor++;
  }
}

