
#include "ModxoLCDXXXX.h"
#include "conio.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "undocumented.h"

#define MODXO_SCROLL_SPEED_IN_MSEC 250
#define MODXO_LCD_COMMAND_MODE 0x80
#define MODXO_LCD_DATA_MODE 0x40

#define MODXO_REGISTER_LCD_COMMAND 0xDEA8
#define MODXO_REGISTER_LCD_DATA 0xDEA9

#define MODXO_LCD_SPI 0x00
#define MODXO_LCD_I2C 0x01
#define MODXO_LCD_REMOVE_I2C_PREFIX 2
#define MODXO_LCD_SET_I2C_PREFIX 3
#define MODXO_LCD_SET_CLK 4
#define MODXO_LCD_SET_SPI_MODE 5

//*************************************************************************************************************
CModxoLCDXXXX::CModxoLCDXXXX()
{
  m_iActualpos=0;
  m_iRows    = 4;
  m_iColumns = 20;        // display rows each line
  m_iBackLight=32;
  m_iContrast=50; 
  m_iI2CAddress=0;
}

//*************************************************************************************************************
CModxoLCDXXXX::~CModxoLCDXXXX()
{
}

//*************************************************************************************************************
void CModxoLCDXXXX::Initialize()
{
  StopThread();
  ILCD::Initialize();
  Create();
  
}
void CModxoLCDXXXX::SetBackLight(int iLight)
{
  m_iBackLight=iLight;
}
void CModxoLCDXXXX::SetContrast(int iContrast)
{
	if (iContrast<0) iContrast=0;
	if (iContrast>100) iContrast=100;
	m_iContrast=iContrast;
}

//*************************************************************************************************************
void CModxoLCDXXXX::Stop()
{
  StopThread();
}

//*************************************************************************************************************
void CModxoLCDXXXX::SetLine(int iLine, const CStdString& strLine)
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
void CModxoLCDXXXX::wait_us(unsigned int value) 
{
	unsigned long long timeout = value;
	timeout *= -10; // 100ns units
	KeDelayExecutionThread(1, 0, (PLARGE_INTEGER)(&timeout));
}

//************************************************************************************************************************
//DisplayBuildCustomChars: load customized characters to character ram of display, resets cursor to pos 0
//************************************************************************************************************************
void CModxoLCDXXXX::DisplayBuildCustomChars() 
{

}


//************************************************************************************************************************
// DisplaySetPos: sets cursor position
// Input: (row position, line number from 0 to 3)
//************************************************************************************************************************
void CModxoLCDXXXX::DisplaySetPos(unsigned char pos, unsigned char line) 
{
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	uint8_t cursor = 0x80 | (pos + row_offsets[line]);
	command(cursor);
}

//************************************************************************************************************************
// DisplayWriteFixText: write a fixed text to actual cursor position
// Input: ("fixed text like")
//************************************************************************************************************************
void CModxoLCDXXXX::DisplayWriteFixtext(const char *textstring)
{ 
	unsigned char  c;
	while (c = *textstring++) 
	{
		writeChar(c);
	}
} 


//************************************************************************************************************************
// DisplayWriteString: write a string to acutal cursor position 
// Input: (pointer to a 0x00 terminated string)
//************************************************************************************************************************

void CModxoLCDXXXX::DisplayWriteString(char *pointer) 
{
	/* display a normal 0x00 terminated string on the LCD display */
	unsigned char c;
	do {
		c = *pointer;
		if (c == 0x00)
		break;

		writeChar(c);
		*pointer++;
	} while(1);
}		


//************************************************************************************************************************
// DisplayClearChars:  clears a number of chars in a line and resets cursor position to it's startposition
// Input: (Startposition of clear in row, row number, number of chars to clear)
//************************************************************************************************************************
void CModxoLCDXXXX::DisplayClearChars(unsigned char startpos , unsigned char line, unsigned char lenght) 
{
	int i;

	DisplaySetPos(startpos,line);
	for (i=0;i<lenght; i++){
		writeChar(0x20);
	}
	DisplaySetPos(startpos,line);
}


//************************************************************************************************************************
// DisplayProgressBar: shows a grafic bar staring at actual cursor position
// Input: (percent of bar to display, lenght of whole bar in chars when 100 %)
//************************************************************************************************************************
void CModxoLCDXXXX::DisplayProgressBar(unsigned char percent, unsigned char charcnt) 
{

}
//************************************************************************************************************************
//Set brightness level 
//************************************************************************************************************************
void CModxoLCDXXXX::DisplaySetBacklight(unsigned char level) 
{
	if (level<0) level=0;
	if (level>100) level=100;
}
//************************************************************************************************************************
//Set Contrast level
//************************************************************************************************************************
void CModxoLCDXXXX::DisplaySetContrast(unsigned char level)
{
	if (level<0) level=0;
	if (level>100) level=100;
}
//************************************************************************************************************************
void CModxoLCDXXXX::DisplayInit()
{
	int i2c_addresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
	uint8_t address = i2c_addresses[m_iI2CAddress];

	_outp(MODXO_REGISTER_LCD_COMMAND, MODXO_LCD_SET_CLK);
	_outp(MODXO_REGISTER_LCD_COMMAND, 100);
	_outp(MODXO_REGISTER_LCD_COMMAND, MODXO_LCD_REMOVE_I2C_PREFIX);
	_outp(MODXO_REGISTER_LCD_COMMAND, MODXO_LCD_I2C);
	_outp(MODXO_REGISTER_LCD_COMMAND, address);
	wait_us(2000);

	write4bits(0x03 << 4);
	wait_us(4500);
	write4bits(0x03 << 4);
	wait_us(4500);
	write4bits(0x03 << 4);
	wait_us(4500);
	write4bits(0x02 << 4);
	command(0x20 | 0x08);
	command(0x08 | 0x04);
	command(0x01);
	wait_us(2000);
	command(0x04 | 0x02);
	command(0x02);
	wait_us(2000);
}

//************************************************************************************************************************
void CModxoLCDXXXX::Process()
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
    Sleep(MODXO_SCROLL_SPEED_IN_MSEC);
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
  command(0x08);
}


void CModxoLCDXXXX::expanderWrite(uint8_t value) 
{
    uint8_t bl = 0x08;
	uint8_t data = (uint8_t)(value | bl);
	_outp(MODXO_REGISTER_LCD_DATA, data);
}

void CModxoLCDXXXX::pulseEnable(uint8_t value) 
{
    uint8_t En = 0x04;

    expanderWrite(value | En);
	wait_us(1);

    expanderWrite(value & ~En);
    wait_us(50);
}

void CModxoLCDXXXX::write4bits(uint8_t value) 
{
    expanderWrite(value);
    pulseEnable(value);
}

void CModxoLCDXXXX::send(uint8_t value, uint8_t mode) 
{
    uint8_t highnib = value & 0xf0;
    uint8_t lownib = (value << 4) & 0xf0;
    write4bits((highnib) | mode);
    write4bits((lownib) | mode); 
}

void CModxoLCDXXXX::command(uint8_t value) 
{
    send(value, 0);
}

void CModxoLCDXXXX::writeChar(uint8_t value) 
{
    send(value, 1);
}