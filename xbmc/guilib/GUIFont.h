/*!
\file GUIFont.h
\brief
*/

#ifndef CGUILIB_GUIFONT_H
#define CGUILIB_GUIFONT_H
#pragma once

#include "utils/StdString.h"

#include <assert.h>

typedef uint32_t character_t;
typedef uint32_t color_t;
typedef std::vector<character_t> vecText;
typedef std::vector<color_t> vecColors;

class CGUIFontTTF;

// flags for alignment
#define XBFONT_LEFT       0x00000000
#define XBFONT_RIGHT      0x00000001
#define XBFONT_CENTER_X   0x00000002
#define XBFONT_CENTER_Y   0x00000004
#define XBFONT_BOTTOM     0x00000005
#define XBFONT_TRUNCATED  0x00000008
#define XBFONT_JUSTIFIED  0x00000010

#define FONT_STYLE_NORMAL       0
#define FONT_STYLE_BOLD         1
#define FONT_STYLE_ITALICS      2
#define FONT_STYLE_BOLD_ITALICS 3
#define FONT_STYLE_UPPERCASE    4
#define FONT_STYLE_LOWERCASE    8
#define FONT_STYLE_MASK       0xF

class CScrollInfo
{
public:
  CScrollInfo(unsigned int wait = 50, float pos = 0, int speed = defaultSpeed, const CStdString &scrollSuffix = " | ")
  {
    initialWait = wait;
    initialPos = pos;
    SetSpeed(speed ? speed : defaultSpeed);
    suffix = scrollSuffix;
    Reset();
  };
  void SetSpeed(int speed);
  void Reset()
  {
    waitTime = initialWait;
    characterPos = 0;
    // pixelPos is where we start the current letter, so is measured
    // to the left of the text rendering's left edge.  Thus, a negative
    // value will mean the text starts to the right
    pixelPos = -initialPos;
    // privates:
    m_averageFrameTime = 1000.f / abs(defaultSpeed);
    m_lastFrameTime = 0;
  }
  uint32_t GetCurrentChar(const vecText &text) const
  {
    assert(text.size());
    if (characterPos < text.size())
      return text[characterPos];
    else if (characterPos < text.size() + suffix.size())
      return suffix[characterPos - text.size()];
    return text[0];
  }
  float GetPixelsPerFrame();

  float pixelPos;
  float pixelSpeed;
  unsigned int waitTime;
  unsigned int characterPos;
  unsigned int initialWait;
  float initialPos;
  CStdString suffix;

  static const int defaultSpeed = 60;
private:
  float m_averageFrameTime;
  uint32_t m_lastFrameTime;
};

/*!
 \ingroup textures
 \brief
 */
class CGUIFont
{
public:
  CGUIFont(const CStdString& strFontName, uint32_t style, color_t textColor, color_t shadowColor, float lineSpacing, CGUIFontTTF *font);
  virtual ~CGUIFont();

  CStdString& GetFontName();

  void DrawText( float x, float y, color_t color, color_t shadowColor,
                 const vecText &text, uint32_t alignment, float maxPixelWidth)
  {
    vecColors colors;
    colors.push_back(color);
    DrawText(x, y, colors, shadowColor, text, alignment, maxPixelWidth);
  };

  void DrawText( float x, float y, const vecColors &colors, color_t shadowColor,
                 const vecText &text, uint32_t alignment, float maxPixelWidth);

  void DrawScrollingText( float x, float y, const vecColors &colors, color_t shadowColor,
                 const vecText &text, uint32_t alignment, float maxPixelWidth, CScrollInfo &scrollInfo);

  float GetTextWidth( const vecText &text );
  float GetCharWidth( character_t ch );
  float GetTextHeight(int numLines) const;
  float GetLineHeight() const;

  void Begin();
  void End();

  uint32_t GetStyle() const { return m_style; };

  static wchar_t RemapGlyph(wchar_t letter);

  CGUIFontTTF* GetFont() const
  {
     return m_font;
  }

  void SetFont(CGUIFontTTF* font)
  {
     m_font = font;
  }

protected:
  CStdString m_strFontName;
  uint32_t m_style;
  color_t m_shadowColor;
  color_t m_textColor;
  float m_lineSpacing;
  CGUIFontTTF *m_font; // the font object has the size information

private:
  bool ClippedRegionIsEmpty(float x, float y, float width, uint32_t alignment) const;
};

#endif
