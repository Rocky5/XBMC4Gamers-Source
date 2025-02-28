
#include "LCDFactory.h"
#include "lib/SmartXX/SmartXXLCD.h"
#include "lib/libXenium/XeniumLCD.h"
#include "lib/X3/X3LCD.h"
#include "lib/Modxo/ModxoLCDHD44780.h"
#include "lib/Modxo/ModxoLCDXXXX.h"
#include "lib/Modxo/ModxoLCDSPI2PAR.h"
#include "lib/SMBUS/SMBUSLCDHD44780.h"
#include "lib/Aladdin/AladdinLCDSPI2PAR.h"
#include "settings/GUISettings.h"

ILCD* g_lcd = NULL;
CLCDFactory::CLCDFactory(void)
{}

CLCDFactory::~CLCDFactory(void)
{}

ILCD* CLCDFactory::Create()
{
  switch (g_guiSettings.GetInt("lcd.modchip"))
  {
  case MODCHIP_XENIUM:
    return new CXeniumLCD();
    break;
  case MODCHIP_SMARTXX_HD44780:
    return new CSmartXXLCD(LCD_TYPE_HD44780);
    break;
  case MODCHIP_SMARTXX_KS0073:
    return new CSmartXXLCD(LCD_TYPE_KS0073);
    break;
  case MODCHIP_SMARTXX_VFD_HD44780:
    return new CSmartXXLCD(LCD_TYPE_VFD | LCD_TYPE_HD44780);
    break;
  case MODCHIP_SMARTXX_VFD_KS0073:
    return new CSmartXXLCD(LCD_TYPE_VFD | LCD_TYPE_KS0073);
    break;
  case MODCHIP_XECUTER3_HD44780:
    return new CX3LCD(LCD_TYPE_HD44780);
    break;
  case MODCHIP_XECUTER3_KS0073:
    return new CX3LCD(LCD_TYPE_KS0073);
    break;
  case MODCHIP_MODXO_HD44780:
    return new CModxoLCDHD44780();
    break;
  case MODCHIP_MODXO_LCDXXXX:
    return new CModxoLCDXXXX();
    break;
  case MODCHIP_MODXO_SPI2PAR:
    return new CModxoLCDSPI2PAR();
    break;
  case MODCHIP_SMBUS_HD44780:
    return new CSMBUSLCDHD44780();
    break;
  case MODCHIP_ALADDIN_SPI2PAR:
    return new CAladdinLCDSPI2PAR();
    break;
  }
  return new CSmartXXLCD(LCD_TYPE_HD44780);
}
