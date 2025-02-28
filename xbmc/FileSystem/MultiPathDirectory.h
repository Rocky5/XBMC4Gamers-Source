#pragma once
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

#include "IDirectory.h"
#include "ProgramDatabase.h"

namespace XFILE
{
class CMultiPathDirectory :
      public IDirectory
{
public:
  CProgramDatabase m_database;
  CMultiPathDirectory(void);
  virtual ~CMultiPathDirectory(void);
  virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
  virtual bool Exists(const char* strPath);
  virtual bool Remove(const char* strPath);

  static CStdString GetFirstPath(const CStdString &strPath);
  static bool SupportsWriteFileOperations(const CStdString &strPath);
  static bool GetPaths(const CStdString& strPath, std::vector<CStdString>& vecPaths);
  static bool HasPath(const CStdString& strPath, const CStdString& strPathToFind);
  static CStdString ConstructMultiPath(const std::vector<CStdString> &vecPaths);

private:
  void MergeItems(CFileItemList &items);
  static void AddToMultiPath(CStdString& strMultiPath, const CStdString& strPath);
  CStdString ConstructMultiPath(const CFileItemList& items, const std::vector<int> &stack);
};
}
