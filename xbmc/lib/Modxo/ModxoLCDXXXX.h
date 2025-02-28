#pragma once
#include "utils/thread.h"
#include "utils/lcd.h"

#define MAX_ROWS 20

class CModxoLCDXXXX : public ILCD
{
public:
    CModxoLCDXXXX();
    virtual ~CModxoLCDXXXX(void);
    virtual void Initialize();
    virtual void Stop();
    virtual void SetBackLight(int iLight);
    virtual void SetContrast(int iContrast);

protected:
    virtual void Process();
    virtual void SetLine(int iLine, const CStdString &strLine);
    void DisplayInit();
    void DisplaySetBacklight(unsigned char level);
    void DisplaySetContrast(unsigned char level);
    void DisplayProgressBar(unsigned char percent, unsigned char charcnt);
    void DisplayClearChars(unsigned char startpos, unsigned char line, unsigned char lenght);
    void DisplayWriteString(char *pointer);
    void DisplayWriteFixtext(const char *textstring);
    void DisplaySetPos(unsigned char pos, unsigned char line);
    void DisplayBuildCustomChars();
    void wait_us(unsigned int value);
    void expanderWrite(uint8_t value);
    void pulseEnable(uint8_t value);
    void write4bits(uint8_t value);
    void send(uint8_t val, uint8_t mode);
    void command(uint8_t value);
    void writeChar(uint8_t value);
    unsigned int m_iColumns;   // display columns for each line
    unsigned int m_iRows;      // total number of rows
    unsigned int m_iActualpos; // actual cursor possition
    int m_iBackLight;
    int m_iContrast;
    int m_iI2CAddress;
    bool m_bUpdate[MAX_ROWS];
    CStdString m_strLine[MAX_ROWS];
    int m_iPos[MAX_ROWS];
    DWORD m_dwSleep[MAX_ROWS];
    CEvent m_event;
};
