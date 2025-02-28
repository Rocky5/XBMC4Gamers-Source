
#include "AladdinLCDSPI2PAR.h"
#include "conio.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "undocumented.h"

#define ALADDIN_SCROLL_SPEED_IN_MSEC 250

#define ALADDIN_REGISTER_IO 0xF70Du
#define ALADDIN_REGISTER_DISP_O 0xF700u
#define ALADDIN_SPI_CS_BIT 0x08
#define ALADDIN_SPI_MOSI_BIT 0x40
#define ALADDIN_SPI_CLK_BIT 0x20

#define SPI2PAR_CURSORHOME 1
#define SPI2PAR_HIDEDISPLAY 2
#define SPI2PAR_SHOWDISPLAY 3
#define SPI2PAR_HIDECURSOR 4
#define SPI2PAR_SHOWUNDERLINECURSOR 5
#define SPI2PAR_SHOWBLOCKCURSOR 6
#define SPI2PAR_SHOWINVERTEDCURSOR 7
#define SPI2PAR_BACKSPACE 8
#define SPI2PAR_MODULECONFIG 9
#define SPI2PAR_LINEFEED 10
#define SPI2PAR_DELETEINPLACE 11
#define SPI2PAR_FORMFEED 12
#define SPI2PAR_CARRIAGERETURN 13
#define SPI2PAR_SETBACKLIGHT 14
#define SPI2PAR_SETCONTRAST 15
#define SPI2PAR_SETCURSORPOSITION 17
#define SPI2PAR_DRAWBARGRAPH 18 
#define SPI2PAR_SCROLLON 19
#define SPI2PAR_SCROLLOFF 20
#define SPI2PAR_WRAPON 23
#define SPI2PAR_WRAPOFF 24
#define SPI2PAR_CUSTOMCHARACTER 25
#define SPI2PAR_REBOOT 26
#define SPI2PAR_CURSORMOVE 27 
#define SPI2PAR_LARGENUMBER 28

//*************************************************************************************************************
CAladdinLCDSPI2PAR::CAladdinLCDSPI2PAR()
{
  m_iActualpos=0;
  m_iRows    = 4;
  m_iColumns = 20;        // display rows each line
  m_iBackLight=32;
  m_iContrast=50; 
  m_iI2CAddress=0;
}

//*************************************************************************************************************
CAladdinLCDSPI2PAR::~CAladdinLCDSPI2PAR()
{
}

//*************************************************************************************************************
void CAladdinLCDSPI2PAR::Initialize()
{
  StopThread();
  ILCD::Initialize();
  Create();
  
}
void CAladdinLCDSPI2PAR::SetBackLight(int iLight)
{
  m_iBackLight=iLight;
}
void CAladdinLCDSPI2PAR::SetContrast(int iContrast)
{
	if (iContrast<0) iContrast=0;
	if (iContrast>100) iContrast=100;
	m_iContrast=iContrast;
}

//*************************************************************************************************************
void CAladdinLCDSPI2PAR::Stop()
{
  StopThread();
}

//*************************************************************************************************************
void CAladdinLCDSPI2PAR::SetLine(int iLine, const CStdString& strLine)
{
	if (iLine < 0 || iLine >= (int)m_iRows) 
		return;

	CStdString strLineLong=strLine;
	//strLineLong.Trim();
	StringToLCDCharSet(LCD_TYPE_HD44780, strLineLong);

	while (strLineLong.size() < m_iColumns) 
		strLineLong+=" ";
	if (strLineLong != m_strLine[iLine])
	{
		m_bUpdate[iLine] = true;
		m_strLine[iLine] = strLineLong;
		m_event.Set();
	}
}


//************************************************************************************************************************
// wait_us: delay routine
// Input: (wait in ~us)
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::wait_us(unsigned int value) 
{
	unsigned long long timeout = value;
	timeout *= -10; // 100ns units
	KeDelayExecutionThread(1, 0, (PLARGE_INTEGER)(&timeout));
}

//************************************************************************************************************************
//DisplayBuildCustomChars: load customized characters to character ram of display, resets cursor to pos 0
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplayBuildCustomChars() 
{

}


//************************************************************************************************************************
// DisplaySetPos: sets cursor position
// Input: (row position, line number from 0 to 3)
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplaySetPos(unsigned char pos, unsigned char line) 
{
	writeValue(SPI2PAR_SETCURSORPOSITION);
	writeValue(pos);
	writeValue(line);
	wait_us(2000);
}

//************************************************************************************************************************
// DisplayWriteFixText: write a fixed text to actual cursor position
// Input: ("fixed text like")
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplayWriteFixtext(const char *textstring)
{ 
	unsigned char  c;
	while (c = *textstring++) 
	{
		writeValue(c);
		wait_us(2000);
	}
} 


//************************************************************************************************************************
// DisplayWriteString: write a string to acutal cursor position 
// Input: (pointer to a 0x00 terminated string)
//************************************************************************************************************************

void CAladdinLCDSPI2PAR::DisplayWriteString(char *pointer) 
{
	/* display a normal 0x00 terminated string on the LCD display */
	unsigned char c;
	do {
		c = *pointer;
		if (c == 0x00)
		break;

		writeValue(c);
		wait_us(2000);
		*pointer++;
	} while(1);
}		


//************************************************************************************************************************
// DisplayClearChars:  clears a number of chars in a line and resets cursor position to it's startposition
// Input: (Startposition of clear in row, row number, number of chars to clear)
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplayClearChars(unsigned char startpos , unsigned char line, unsigned char lenght) 
{
	int i;

	DisplaySetPos(startpos,line);
	for (i=0;i<lenght; i++)
	{
		writeValue(0x20);
		wait_us(2000);
	}
	DisplaySetPos(startpos,line);
}


//************************************************************************************************************************
// DisplayProgressBar: shows a grafic bar staring at actual cursor position
// Input: (percent of bar to display, lenght of whole bar in chars when 100 %)
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplayProgressBar(unsigned char percent, unsigned char charcnt) 
{

}
//************************************************************************************************************************
//Set brightness level 
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplaySetBacklight(unsigned char level) 
{
	if (level<0) level=0;
	if (level>100) level=100;

	writeValue(SPI2PAR_SETBACKLIGHT); 
	writeValue(level); 
	wait_us(2000);
}
//************************************************************************************************************************
//Set Contrast level
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplaySetContrast(unsigned char level)
{
	if (level<0) level=0;
	if (level>100) level=100;

	writeValue(SPI2PAR_SETCONTRAST); 
	writeValue(level); 
	wait_us(2000);
}
//************************************************************************************************************************
void CAladdinLCDSPI2PAR::DisplayInit()
{
	int contrast = g_guiSettings.GetInt("lcd.contrast");
	if (contrast<0) contrast=0;
	if (contrast>100) contrast=100;

	int backlight = g_guiSettings.GetInt("lcd.backlight");
	if (backlight<0) backlight=0;
	if (backlight>100) backlight=100;

	writeValue(SPI2PAR_SHOWDISPLAY); 
	wait_us(2000);
	writeValue(SPI2PAR_FORMFEED);  
	wait_us(2000);
	writeValue(SPI2PAR_HIDECURSOR);
	wait_us(2000);
	writeValue(SPI2PAR_SCROLLOFF); 
	wait_us(2000);
	writeValue(SPI2PAR_WRAPOFF);
	wait_us(2000);
	writeValue(SPI2PAR_SETBACKLIGHT); 
	writeValue(backlight); 
	wait_us(2000);
	writeValue(SPI2PAR_SETCONTRAST); 
	writeValue(contrast); 
	wait_us(2000);
}

//************************************************************************************************************************
void CAladdinLCDSPI2PAR::Process()
{
  int iOldLight=-1;
  int iOldContrast=-1;

  m_iColumns = g_advancedSettings.m_lcdColumns;
  m_iRows    = g_advancedSettings.m_lcdRows;
  m_iBackLight= g_guiSettings.GetInt("lcd.backlight");
  m_iContrast = g_guiSettings.GetInt("lcd.contrast");
  m_iI2CAddress = g_guiSettings.GetInt("lcd.i2caddress");
  if (m_iRows >= MAX_ROWS) m_iRows=MAX_ROWS-1;

  DisplayInit();
  while (!m_bStop)
  {
    Sleep(ALADDIN_SCROLL_SPEED_IN_MSEC);
    if (m_iBackLight != iOldLight)
    {
      // backlight setting changed
      iOldLight=m_iBackLight;
      DisplaySetBacklight(m_iBackLight);
    }
    if (m_iContrast != iOldContrast)
    {
      // contrast setting changed
      iOldContrast=m_iContrast;
      DisplaySetContrast(m_iContrast);
    }
    DisplayBuildCustomChars();
    for (int iLine=0; iLine < (int)m_iRows; ++iLine)
    {
      if (m_bUpdate[iLine])
      {
        CStdString strTmp=m_strLine[iLine];
        if (strTmp.size() > m_iColumns)
        {
          strTmp=m_strLine[iLine].Left(m_iColumns);
        }
        m_iPos[iLine]=0;
        DisplaySetPos(0,iLine);
        DisplayWriteFixtext(strTmp.c_str());
        m_bUpdate[iLine]=false;
        m_dwSleep[iLine]=GetTickCount();
      }
      else if ( (GetTickCount()-m_dwSleep[iLine]) > 1000)
      {
        int iSize=m_strLine[iLine].size();
        if (iSize > (int)m_iColumns)
        {
          //scroll line
          CStdString strRow=m_strLine[iLine]+"   -   ";
          int iSize=strRow.size();
          m_iPos[iLine]++;
          if (m_iPos[iLine]>=iSize) m_iPos[iLine]=0;
          int iPos=m_iPos[iLine];
          CStdString strLine="";
          for (int iCol=0; iCol < (int)m_iColumns;++iCol)
          {
            strLine +=strRow.GetAt(iPos);
            iPos++;
            if (iPos >= iSize) iPos=0;
          }
          DisplaySetPos(0,iLine);
          DisplayWriteFixtext(strLine.c_str());
        }
      }
    }
  }
  for (int i=0; i < (int)m_iRows; i++)
  {
    DisplayClearChars(0,i,m_iColumns);
  }
  writeValue(SPI2PAR_HIDEDISPLAY); 
}


void CAladdinLCDSPI2PAR::writeValue(unsigned char value) 
{
	unsigned char spi = m_iSpi;

	spi |= ALADDIN_SPI_CLK_BIT;
	spi &= ~ALADDIN_SPI_CS_BIT;
	_outp(ALADDIN_REGISTER_DISP_O, spi);
	wait_us(0x1);

	unsigned char i = 8;
	do
	{
		if ((value & 0x80) > 0)
		{
			spi |= ALADDIN_SPI_MOSI_BIT;
		} 
		else
		{
			spi &= ~ALADDIN_SPI_MOSI_BIT;
		}

		spi &= ~ALADDIN_SPI_CLK_BIT;
		_outp(ALADDIN_REGISTER_DISP_O, spi);
		wait_us(0x1);

		spi |= ALADDIN_SPI_CLK_BIT;
		_outp(ALADDIN_REGISTER_DISP_O, spi);
		wait_us(0x1);

		value = ((value << 1) & 0xFF);
		i--;
	}
	while (i > 0);

	spi |= ALADDIN_SPI_CS_BIT;
	_outp(ALADDIN_REGISTER_DISP_O, spi);
	wait_us(0x1);

	m_iSpi = spi;
}
