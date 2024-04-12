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

#include <ConIo.h>
#include "FanController.h"
#include "xbox/Undocumented.h"
#include "xbox/XKExports.h"
#include "settings/GUISettings.h"
#include "utils/log.h"

#define ADM_ADDRESS      0x98 // ADM1032 System Temperature Monitor
#define PIC_ADDRESS      0x20 // PIC16LC
#define XCALIBUR_ADDRESS 0xE0 // XCalibur/1.6 videochip
#define FAN_MODE         0x05 // Enable/ disable the custom fan speeds (0/1)
#define FAN_REGISTER     0x06 // Set custom fan speeds (0-50)
#define FAN_READBACK     0x10 // Current fan speed (0-50)
#define PIC_MB_TEMP      0x0A // MB Temperature
#define PIC_CPU_TEMP     0x09 // CPU Temperature
#define ADM_CPU_TEMP     0x01 // CPU Temperature
#define ADM_MB_TEMP      0x00 // MB Temperature

CFanController* CFanController::_Instance = NULL;

CFanController* CFanController::Instance()
{
  if (_Instance == NULL)
  {
    _Instance = new CFanController();
  }
  return _Instance;
}

void CFanController::RemoveInstance()
{
  if (_Instance)
  {
    _Instance->Stop();
    delete _Instance;
    _Instance=NULL;
  }
}

CFanController::CFanController()
{
  inCustomMode = false;
  systemFanSpeed = GetFanSpeed();
  currentFanSpeed = systemFanSpeed;
  calculatedFanSpeed = systemFanSpeed;
  unsigned long iDummy;
  bIs16Box = (HalReadSMBusValue(XCALIBUR_ADDRESS, 0, 0, (LPBYTE)&iDummy) == 0);
  cpuTempCount = 0;
  m_minFanspeed = 1;
}


CFanController::~CFanController()
{
  _Instance = NULL;
}

void CFanController::OnStartup()
{}

void CFanController::OnExit()
{}

void CFanController::Process()
{
  if (!g_guiSettings.GetBool("system.autotemperature")) return ;
  int interval = 500;
  tooHotLoopCount = 0;
  tooColdLoopCount = 0;
  while (!m_bStop)
  {
    GetGPUTempInternal();
    GetCPUTempInternal();
    GetFanSpeedInternal();

    // Use the highest temperature, if the temperatures are
    // equal, go with the CPU temperature.
    if (cpuTemp >= gpuTemp)
    {
      sensor = ST_CPU;
    }
    else
    {
      sensor = ST_GPU;
    }

    if (cpuLastTemp.IsValid())
    {
      CalcSpeed(targetTemp);

      SetFanSpeed(calculatedFanSpeed, false);
    }

    cpuLastTemp = cpuTemp;
    gpuLastTemp = gpuTemp;

    Sleep(interval);
  }
}

void CFanController::SetTargetTemperature(int targetTemperature)
{
  targetTemp = targetTemperature;
}

void CFanController::SetMinFanSpeed(int minFanspeed)
{
  m_minFanspeed = minFanspeed;
  if (m_minFanspeed < 1)
    m_minFanspeed=1;
  if (m_minFanspeed > 50)
    m_minFanspeed=50; // Should not be possible
}


void CFanController::RestoreStartupSpeed()
{
  SetFanSpeed(systemFanSpeed);
  Sleep(100);
  // if it's not a 1.6 box disable custom fanmode
  if (!bIs16Box)
  {
    // disable custom fanmode
    HalWriteSMBusValue(PIC_ADDRESS, FAN_MODE, 0, 0);
  }
  inCustomMode = false;
}

void CFanController::Start(int targetTemperature, int minFanspeed)
{
  StopThread();
  targetTemp = targetTemperature;
  SetMinFanSpeed(minFanspeed);
  Create();
}

void CFanController::Stop()
{
  StopThread();
  if (inCustomMode)
  {
    RestoreStartupSpeed();
  }
}

int CFanController::GetFanSpeed()
{
  if (m_ThreadHandle == NULL)
  {
    GetFanSpeedInternal();
  }
  return currentFanSpeed;
}

void CFanController::GetFanSpeedInternal()
{
  HalReadSMBusValue(PIC_ADDRESS, FAN_READBACK, 0, (LPBYTE)&currentFanSpeed);
}

void CFanController::SetFanSpeed(const int fanspeed, const bool force)
{
  if (fanspeed < 0) return ;
  if (fanspeed > 50) return ;
  if ((currentFanSpeed == fanspeed) && (!force)) return ;
  if (force)
  {
    // on boot or first time set it needs a kickstart in releasemode for some reason
    // it works fine without this block in debugmode...
    HalWriteSMBusValue(PIC_ADDRESS, FAN_MODE, 0, 1);
    Sleep(10);
    HalWriteSMBusValue(PIC_ADDRESS, FAN_REGISTER, 0, fanspeed);
  }
  // enable custom fanspeeds
  HalWriteSMBusValue(PIC_ADDRESS, FAN_MODE, 0, 1);
  Sleep(10);
  HalWriteSMBusValue(PIC_ADDRESS, FAN_REGISTER, 0, fanspeed);
  Sleep(10);
  currentFanSpeed = fanspeed;
  inCustomMode = true;
}

const CTemperature& CFanController::GetGPUTemp()
{
  if (m_ThreadHandle == NULL)
  {
    GetGPUTempInternal();
  }
  return gpuTemp;
}

void CFanController::GetGPUTempInternal()
{
  unsigned long temp;
  if (!bIs16Box)
  {
    // 1.0-1.5 Read ADM1032
    HalReadSMBusValue(ADM_ADDRESS, ADM_MB_TEMP, FALSE, (LPBYTE)&temp);
  }
  else
  {
    // 1.6 Read SMC instead
    HalReadSMBusValue(PIC_ADDRESS, PIC_MB_TEMP, FALSE, (LPBYTE)&temp);
  }
  gpuTemp = CTemperature::CreateFromCelsius((double)temp);

  // The XBOX v1.6 shows the temp to high! Let's recalc it! It will only do ~minus 10 degress
  if (bIs16Box)
  {
    gpuTemp *= 0.8f;
  }
}

const CTemperature& CFanController::GetCPUTemp()
{
  if (m_ThreadHandle == NULL)
  {
    GetCPUTempInternal();
  }
  return cpuTemp;
}

void CFanController::GetCPUTempInternal()
{
  unsigned long temp;
  if (!bIs16Box)
  {
    // 1.0-1.5 Read ADM1032
    HalReadSMBusValue(ADM_ADDRESS, ADM_CPU_TEMP, FALSE, (LPBYTE)&temp);
  }
  else
  {
    // 1.6 Read SMC instead
    HalReadSMBusValue(PIC_ADDRESS, PIC_CPU_TEMP, FALSE, (LPBYTE)&temp);
  }
  cpuTemp = CTemperature::CreateFromCelsius((double)temp);
}

void CFanController::CalcSpeed(int targetTemp)
{
  CTemperature temp;
  CTemperature tempOld;
  CTemperature targetTempFloor;
  CTemperature targetTempCeiling;

  if (sensor == ST_GPU)
  {
    temp = gpuTemp;
    tempOld = gpuLastTemp;
  }
  else
  {
    temp = cpuTemp;
    tempOld = cpuLastTemp;
  }
  targetTempFloor = CTemperature::CreateFromCelsius((float)targetTemp - 0.75f);
  targetTempCeiling = CTemperature::CreateFromCelsius((float)targetTemp + 0.75f);

  if ((temp >= targetTempFloor) && (temp <= targetTempCeiling))
  {
    // within range, try to keep it steady
    tooHotLoopCount = 0;
    tooColdLoopCount = 0;
    if (temp > tempOld)
    {
      calculatedFanSpeed++;
    }
    else if (temp < tempOld)
    {
      calculatedFanSpeed--;
    }
  }

  else if (temp < targetTempFloor)
  {
    // cool, lower speed unless it's getting hotter
    if (temp == tempOld)
    {
      tooColdLoopCount++;
    }
    else if (temp > tempOld)
    {
      tooColdLoopCount--;
    }
    if ((temp < tempOld) || (tooColdLoopCount == 12))
    {
      calculatedFanSpeed--;
      // CLog::Log(LOGDEBUG,"Lowering fanspeed to %i, tooHotLoopCount=%i tooColdLoopCount=%i", calculatedFanSpeed, tooHotLoopCount, tooColdLoopCount);
      tooColdLoopCount = 0;
    }
  }

  else if (temp > targetTempCeiling)
  {
    // hot, increase fanspeed if it's still getting hotter or not getting any cooler for at leat loopcount*sleepvalue
    if (temp == tempOld)
    {
      tooHotLoopCount++;
    }
    else if (temp < tempOld)
    {
      tooHotLoopCount--;
    }
    if ((temp > tempOld) || (tooHotLoopCount == 12))
    {
      calculatedFanSpeed++;
      // CLog::Log(LOGDEBUG,"Increasing fanspeed to %i, tooHotLoopCount=%i tooColdLoopCount=%i", calculatedFanSpeed, tooHotLoopCount, tooColdLoopCount);
      tooHotLoopCount = 0;
    }
  }

  if (calculatedFanSpeed < m_minFanspeed)
  {
    calculatedFanSpeed = m_minFanspeed;
  }
  if (calculatedFanSpeed > 50) {calculatedFanSpeed = 50;}
}
