
#include "SMBUSLCDHD44780.h"
#include "conio.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "undocumented.h"

#define XBOX_SCROLL_SPEED_IN_MSEC 250
#define XBOX_LCD_COMMAND_MODE 0x80
#define XBOX_LCD_DATA_MODE 0x40

#define HalWriteSMBusByte(SlaveAddress, CommandCode, DataValue) HalWriteSMBusValue(SlaveAddress, CommandCode, FALSE, DataValue)

//*************************************************************************************************************
CSMBUSLCDHD44780::CSMBUSLCDHD44780()
{
  m_iActualpos=0;
  m_iRows    = 4;
  m_iColumns = 20;        // display rows each line
  m_iBackLight=32;
  m_iContrast=50;  
  m_iI2CAddress=0;
}

//*************************************************************************************************************
CSMBUSLCDHD44780::~CSMBUSLCDHD44780()
{
}

//*************************************************************************************************************
void CSMBUSLCDHD44780::Initialize()
{
  StopThread();
  ILCD::Initialize();
  Create();
  
}
void CSMBUSLCDHD44780::SetBackLight(int iLight)
{
  m_iBackLight=iLight;
}
void CSMBUSLCDHD44780::SetContrast(int iContrast)
{
  m_iContrast=iContrast;
}

//*************************************************************************************************************
void CSMBUSLCDHD44780::Stop()
{
  StopThread();
}

//*************************************************************************************************************
void CSMBUSLCDHD44780::SetLine(int iLine, const CStdString& strLine)
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
void CSMBUSLCDHD44780::wait_us(unsigned int value) 
{
	unsigned long long timeout = value;
	timeout *= -10; // 100ns units
	KeDelayExecutionThread(1, 0, (PLARGE_INTEGER)(&timeout));
}

//************************************************************************************************************************
//DisplayBuildCustomChars: load customized characters to character ram of display, resets cursor to pos 0
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplayBuildCustomChars() 
{

}


//************************************************************************************************************************
// DisplaySetPos: sets cursor position
// Input: (row position, line number from 0 to 3)
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplaySetPos(unsigned char pos, unsigned char line) 
{
	int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
	uint8_t address = i2c_addresses[m_iI2CAddress];

	int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };
	uint8_t cursor = 0x80 | (pos + row_offsets[line]);
	HalWriteSMBusByte(address << 1, 0x80, cursor);
}

//************************************************************************************************************************
// DisplayWriteFixText: write a fixed text to actual cursor position
// Input: ("fixed text like")
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplayWriteFixtext(const char *textstring)
{ 
  int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
  uint8_t address = i2c_addresses[m_iI2CAddress];

  unsigned char  c;
  while (c = *textstring++) {
    HalWriteSMBusByte(address << 1, 0x40, c);
  }
} 


//************************************************************************************************************************
// DisplayWriteString: write a string to acutal cursor position 
// Input: (pointer to a 0x00 terminated string)
//************************************************************************************************************************

void CSMBUSLCDHD44780::DisplayWriteString(char *pointer) 
{
  int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
  uint8_t address = i2c_addresses[m_iI2CAddress];

  /* display a normal 0x00 terminated string on the LCD display */
  unsigned char c;
  do {
    c = *pointer;
    if (c == 0x00)
      break;

	HalWriteSMBusByte(address << 1, 0x40, c);
    *pointer++;
    } while(1);
}		


//************************************************************************************************************************
// DisplayClearChars:  clears a number of chars in a line and resets cursor position to it's startposition
// Input: (Startposition of clear in row, row number, number of chars to clear)
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplayClearChars(unsigned char startpos , unsigned char line, unsigned char lenght) 
{
  int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
  uint8_t address = i2c_addresses[m_iI2CAddress];

  int i;

  DisplaySetPos(startpos,line);
  for (i=0;i<lenght; i++){
    HalWriteSMBusByte(address << 1, 0x40, 0x20);
  }
  DisplaySetPos(startpos,line);
}


//************************************************************************************************************************
// DisplayProgressBar: shows a grafic bar staring at actual cursor position
// Input: (percent of bar to display, lenght of whole bar in chars when 100 %)
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplayProgressBar(unsigned char percent, unsigned char charcnt) 
{

}
//************************************************************************************************************************
//Set brightness level 
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplaySetBacklight(unsigned char level) 
{
  if (level<0) level=0;
  if (level>100) level=100;
}
//************************************************************************************************************************
//Set Contrast level
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplaySetContrast(unsigned char level)
{
	if (level<0) level=0;
	if (level>100) level=100;

	int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
	uint8_t address = i2c_addresses[m_iI2CAddress];

	HalWriteSMBusByte(address << 1, 0x80, 0x2A);
	HalWriteSMBusByte(address << 1, 0x80, 0x79);
	HalWriteSMBusByte(address << 1, 0x80, 0x81);
	HalWriteSMBusByte(address << 1, 0x80, (uint8_t)(level * 2.55f));
	HalWriteSMBusByte(address << 1, 0x80, 0x78);
	HalWriteSMBusByte(address << 1, 0x80, 0x28);
}
//************************************************************************************************************************
void CSMBUSLCDHD44780::DisplayInit()
{
	int contrast = g_guiSettings.GetInt("lcd.contrast");
	if (contrast<0) contrast=0;
	if (contrast>100) contrast=100;

	int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
	uint8_t address = i2c_addresses[m_iI2CAddress];

	HalWriteSMBusByte(address << 1, 0x80, 0x2a); 
	HalWriteSMBusByte(address << 1, 0x80, 0x71); 
	HalWriteSMBusByte(address << 1, 0x40, 0x00);
	HalWriteSMBusByte(address << 1, 0x80, 0x28);
	HalWriteSMBusByte(address << 1, 0x80, 0x08);
	HalWriteSMBusByte(address << 1, 0x80, 0x2a);
	HalWriteSMBusByte(address << 1, 0x80, 0x79); 
	HalWriteSMBusByte(address << 1, 0x80, 0xd5);
	HalWriteSMBusByte(address << 1, 0x80, 0x70);
	HalWriteSMBusByte(address << 1, 0x80, 0x78);
	HalWriteSMBusByte(address << 1, 0x80, 0x08 | 0x01); //0x01 is 3-4 rows
	HalWriteSMBusByte(address << 1, 0x80, 0x06);
	HalWriteSMBusByte(address << 1, 0x80, 0x72);
	HalWriteSMBusByte(address << 1, 0x40, 0x00); 
	HalWriteSMBusByte(address << 1, 0x80, 0x2a); 
	HalWriteSMBusByte(address << 1, 0x80, 0x79);
	HalWriteSMBusByte(address << 1, 0x80, 0xda);
	HalWriteSMBusByte(address << 1, 0x80, 0x10);
	HalWriteSMBusByte(address << 1, 0x80, 0xdc);
	HalWriteSMBusByte(address << 1, 0x80, 0x00);
	HalWriteSMBusByte(address << 1, 0x80, 0x81);
	HalWriteSMBusByte(address << 1, 0x80, (uint8_t)(contrast * 2.55f));
	HalWriteSMBusByte(address << 1, 0x80, 0xd9);
	HalWriteSMBusByte(address << 1, 0x80, 0xf1);
	HalWriteSMBusByte(address << 1, 0x80, 0xdb); 
	HalWriteSMBusByte(address << 1, 0x80, 0x40);
	HalWriteSMBusByte(address << 1, 0x80, 0x78); 
	HalWriteSMBusByte(address << 1, 0x80, 0x28); 
	HalWriteSMBusByte(address << 1, 0x80, 0x01);
	HalWriteSMBusByte(address << 1, 0x80, 0x80);
	HalWriteSMBusByte(address << 1, 0x80, 0x08 | 0x04);
}

//************************************************************************************************************************
void CSMBUSLCDHD44780::Process()
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
    Sleep(XBOX_SCROLL_SPEED_IN_MSEC);
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
  int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
  uint8_t address = i2c_addresses[m_iI2CAddress];
  HalWriteSMBusByte(address << 1, 0x80, 0x08);
}
