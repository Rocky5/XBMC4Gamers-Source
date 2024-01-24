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

#include "ProgramDatabase.h"
#include "Util.h"
#include "xbox/xbeheader.h"
#include "windows/GUIWindowFileManager.h"
#include "FileItem.h"
#include "utils/Crc32.h"
#include "utils/URIUtils.h"
#include "utils/log.h"
#include "settings/AdvancedSettings.h"
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
#include "dialogs/GUIDialogOK.h"
#include "GUIWindowManager.h"
#include "LocalizeStrings.h"
#include "XMLUtils.h"

using namespace XFILE;

//********************************************************************************************************************************
CProgramDatabase::CProgramDatabase(void)
{
	m_strDatabaseFile=PROGRAM_DATABASE_NAME;
}

//********************************************************************************************************************************
CProgramDatabase::~CProgramDatabase(void)
{

}

//********************************************************************************************************************************
bool CProgramDatabase::CreateTables()
{

	try
	{
		CDatabase::CreateTables();

		CLog::Log(LOGINFO, "create files table");
		m_pDS->exec("CREATE TABLE files ( idFile integer primary key, strFilename text, titleId integer, xbedescription text, iTimesPlayed integer, lastAccessed integer, iRegion integer, iSize integer, altname text, developer text, publisher text, features_general text, features_online text, esrb text, esrb_descriptors text, genre text, release_date text, year text, rating text, platform text, exclusive text, title_id text, synopsis text, resources text, preview text,last_played text NOT NULL DEFAULT '')\n");
		CLog::Log(LOGINFO, "create trainers table");
		m_pDS->exec("CREATE TABLE trainers (idKey integer auto_increment primary key, idCRC integer, idTitle integer, strTrainerPath text, strSettings text, Active integer)\n");
		CLog::Log(LOGINFO, "create files index");
		m_pDS->exec("CREATE INDEX idxFiles ON files(strFilename)");
		CLog::Log(LOGINFO, "create files - titleid index");
		m_pDS->exec("CREATE INDEX idxTitleIdFiles ON files(titleId)");
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "programdatabase::unable to create tables:%lu", GetLastError());
		return false;
	}

	return true;
}

bool CProgramDatabase::UpdateOldVersion(int version)
{
	if (NULL == m_pDB.get()) return false;
	if (NULL == m_pDS.get()) return false;
	if (NULL == m_pDS2.get()) return false;

	try
	{	
		if (version < 4)
		{
			m_pDS->exec("ALTER TABLE files ADD COLUMN altname text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN developer text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN publisher text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN features_general text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN features_online text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN esrb text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN esrb_descriptors text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN genre text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN release_date text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN year text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN rating text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN platform text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN exclusive text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN title_id text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN synopsis text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN resources text NOT NULL DEFAULT ''");
			m_pDS->exec("ALTER TABLE files ADD COLUMN preview text NOT NULL DEFAULT ''");
			
			CGUIDialogOK *dialog = (CGUIDialogOK *)g_windowManager.GetWindow(WINDOW_DIALOG_OK);
			if (dialog)
			{
				dialog->SetHeading("Database Updated");
				dialog->SetLine(0, "If you have installed artwork from the artwork installer");
				dialog->SetLine(1, "you will need to refresh all synopsis info from the");
				dialog->SetLine(2, "context menu for it to show.");
				dialog->DoModal();
				CLog::Log(LOGINFO, "Updated database to version 4");
			}
		}
	} catch (...){}
	
	try
	{	
		if (version < 5)
		{
			m_pDS->exec("ALTER TABLE files ADD COLUMN last_played text NOT NULL DEFAULT ''");
			CLog::Log(LOGINFO, "Updated database to version 5");
		}
	} catch (...){}
	
	return true;
}

int CProgramDatabase::GetRegion(const CStdString& strFilenameAndPath)
{
	if (NULL == m_pDB.get()) return 0;
	if (NULL == m_pDS.get()) return 0;

	try
	{
		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFilenameAndPath.c_str());
		if (!m_pDS->query(strSQL.c_str()))
		return 0;

		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return 0;
		}
		int iRegion = m_pDS->fv("files.iRegion").get_asInt();
		m_pDS->close();

		return iRegion;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:GetRegion(%s) failed", strFilenameAndPath.c_str());
	}
	return 0;
}

int CProgramDatabase::GetTitleId(const CStdString& strFilenameAndPath)
{
	if (NULL == m_pDB.get()) return 0;
	if (NULL == m_pDS.get()) return 0;

	try
	{
		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFilenameAndPath.c_str());
		if (!m_pDS->query(strSQL.c_str()))
		return 0;

		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return 0;
		}
		int idTitle = m_pDS->fv("files.TitleId").get_asInt();
		m_pDS->close();
		return idTitle;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:GetTitleId(%s) failed", strFilenameAndPath.c_str());
	}
	return 0;
}

bool CProgramDatabase::SetRegion(const CStdString& strFileName, int iRegion)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFileName.c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return false;
		}
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		m_pDS->close();

		CLog::Log(LOGDEBUG, "CProgramDatabase::SetRegion(%s), idFile=%i, region=%i",
		strFileName.c_str(), idFile,iRegion);

		strSQL=PrepareSQL("update files set iRegion=%i where idFile=%i",
		iRegion, idFile);
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:SetDescription(%s) failed", strFileName.c_str());
	}

	return false;
}

bool CProgramDatabase::SetTitleId(const CStdString& strFileName, int idTitle)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFileName.c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return false;
		}
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		m_pDS->close();

		CLog::Log(LOGDEBUG, "CProgramDatabase::SetTitle(%s), idFile=%i, region=%i",
		strFileName.c_str(), idFile, idTitle);

		strSQL=PrepareSQL("update files set titleId=%i where idFile=%i",
		idTitle, idFile);
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:SetDescription(%s) failed", strFileName.c_str());
	}

	return false;
}

bool CProgramDatabase::SetLastPlayed(const CStdString& strFilenameAndPath)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString currenttime = CDateTime::GetCurrentDateTime().GetAsDBDateTime().c_str();
		
		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFilenameAndPath.c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return false;
		}
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		m_pDS->close();

		CLog::Log(LOGDEBUG, "CProgramDatabase::SetLastPlayed(%s), idFile=%i, last_played=%s",strFilenameAndPath.c_str(), idFile, currenttime.c_str());

		strSQL=PrepareSQL("update files set last_played='%s' where idFile=%i",currenttime.c_str(), idFile);
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:SetDescription(%s) failed", strFilenameAndPath.c_str());
	}

	return false;
}

bool CProgramDatabase::GetXBEPathByTitleId(const int idTitle, CStdString& strPathAndFilename)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL=PrepareSQL("select files.strFilename from files where files.titleId=%i", idTitle);
		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() > 0)
		{
			strPathAndFilename = m_pDS->fv("files.strFilename").get_asString();
			strPathAndFilename.Replace('/', '\\');
			m_pDS->close();
			return true;
		}
		m_pDS->close();
		return false;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::GetXBEPathByTitleId(%i) failed", idTitle);
	}
	return false;
}

bool CProgramDatabase::ItemHasTrainer(unsigned int iTitleId)
{
	CStdString strSQL;
	try
	{
		strSQL = PrepareSQL("select * from trainers where idTitle=%u", iTitleId);
		if (!m_pDS->query(strSQL.c_str()))
		return false;
		if (m_pDS->num_rows())
		return true;

		return false;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"error checking for title's trainers (%s)",strSQL.c_str());
	}
	return false;
}

bool CProgramDatabase::HasTrainer(const CStdString& strTrainerPath)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		strSQL = PrepareSQL("select * from trainers where idCRC=%u", (unsigned __int32) crc);
		if (!m_pDS->query(strSQL.c_str()))
		return false;
		if (m_pDS->num_rows())
		return true;

		return false;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"error checking for trainer existance (%s)",strSQL.c_str());
	}
	return false;
}

bool CProgramDatabase::AddTrainer(int iTitleId, const CStdString& strTrainerPath)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		char temp[101];
		for( int i=0;i<100;++i)
		temp[i] = '0';
		temp[100] = '\0';
		strSQL=PrepareSQL("insert into trainers (idKey,idCRC,idTitle,strTrainerPath,strSettings,Active) values(NULL,%u,%u,'%s','%s',%i)",(unsigned __int32)crc,iTitleId,strTrainerPath.c_str(),temp,0);
		if (!m_pDS->exec(strSQL.c_str()))
		return false;

		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"programdatabase: unable to add trainer (%s)",strSQL.c_str());
	}
	return false;
}

bool CProgramDatabase::RemoveTrainer(const CStdString& strTrainerPath)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		strSQL=PrepareSQL("delete from trainers where idCRC=%u", (unsigned __int32)crc);
		if (!m_pDS->exec(strSQL.c_str()))
		return false;

		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"programdatabase: unable to remove trainer (%s)",strSQL.c_str());
	}
	return false;
}

bool CProgramDatabase::GetTrainers(unsigned int iTitleId, std::vector<CStdString>& vecTrainers)
{
	vecTrainers.clear();
	CStdString strSQL;
	try
	{
		strSQL = PrepareSQL("select * from trainers where idTitle=%u", iTitleId);
		if (!m_pDS->query(strSQL.c_str()))
		return false;

		while (!m_pDS->eof())
		{
			vecTrainers.push_back(m_pDS->fv("strTrainerPath").get_asString());
			m_pDS->next();
		}

		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"programdatabase: error reading trainers for %i (%s)",iTitleId,strSQL.c_str());
	}
	return false;

}

bool CProgramDatabase::GetAllTrainers(std::vector<CStdString>& vecTrainers)
{
	vecTrainers.clear();
	CStdString strSQL;
	try
	{
		strSQL = PrepareSQL("select distinct strTrainerPath from trainers");//PrepareSQL("select * from trainers");
		if (!m_pDS->query(strSQL.c_str()))
		return false;

		while (!m_pDS->eof())
		{
			vecTrainers.push_back(m_pDS->fv("strTrainerPath").get_asString());
			m_pDS->next();
		}

		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"programdatabase: error reading trainers (%s)",strSQL.c_str());
	}
	return false;
}

bool CProgramDatabase::SetTrainerOptions(const CStdString& strTrainerPath, unsigned int iTitleId, unsigned char* data, int numOptions)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		char temp[101];
		int i;
		for (i=0;i<numOptions && i<100;++i)
		{
			if (data[i] == 1)
			temp[i] = '1';
			else
			temp[i] = '0';
		}
		temp[i] = '\0';

		strSQL = PrepareSQL("update trainers set strSettings='%s' where idCRC=%u and idTitle=%u", temp, (unsigned __int32)crc,iTitleId);
		if (m_pDS->exec(strSQL.c_str()))
		return true;

		return false;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"CProgramDatabase::SetTrainerOptions failed (%s)",strSQL.c_str());
	}

	return false;
}

void CProgramDatabase::SetTrainerActive(const CStdString& strTrainerPath, unsigned int iTitleId, bool bActive)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		strSQL = PrepareSQL("update trainers set Active=%u where idCRC=%u and idTitle=%u", bActive?1:0, (unsigned __int32)crc, iTitleId);
		m_pDS->exec(strSQL.c_str());
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"CProgramDatabase::SetTrainerOptions failed (%s)",strSQL.c_str());
	}
}

CStdString CProgramDatabase::GetActiveTrainer(unsigned int iTitleId)
{
	CStdString strSQL;
	try
	{
		strSQL = PrepareSQL("select * from trainers where idTitle=%u and Active=1", iTitleId);
		if (!m_pDS->query(strSQL.c_str()))
		return "";

		if (!m_pDS->eof())
		return m_pDS->fv("strTrainerPath").get_asString();
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"programdatabase: error finding active trainer for %i (%s)",iTitleId,strSQL.c_str());
	}

	return "";
}

bool CProgramDatabase::GetTrainerOptions(const CStdString& strTrainerPath, unsigned int iTitleId, unsigned char* data, int numOptions)
{
	CStdString strSQL;
	Crc32 crc; crc.ComputeFromLowerCase(strTrainerPath);
	try
	{
		strSQL = PrepareSQL("select * from trainers where idCRC=%u and idTitle=%u", (unsigned __int32)crc, iTitleId);
		if (m_pDS->query(strSQL.c_str()))
		{
			CStdString strSettings = m_pDS->fv("strSettings").get_asString();
			for (int i=0;i<numOptions && i < 100;++i)
			data[i] = strSettings[i]=='1'?1:0;

			return true;
		}

		return false;
	}
	catch (...)
	{
		CLog::Log(LOGERROR,"CProgramDatabase::GetTrainerOptions failed (%s)",strSQL.c_str());
	}

	return false;
}

CStdString CProgramDatabase::ReplaceWithForwardSlash(const CStdString& strInput, CStdString& strtoReplace)
{
	CStdString Output = strInput;
	int i = 0;
	for (;;) {
		i = Output.find(strtoReplace, i);
		if (i == std::string::npos) {
			break;
		}
		Output.replace(i, 1, " /");
	}
	return Output;
}

int CProgramDatabase::GetProgramInfo(CFileItem *item)
{
	int SynopsisCheck = g_guiSettings.GetBool("mygames.gamesynopsisinfo");
	int idTitle = 0;
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select xbedescription,iTimesPlayed,lastAccessed,titleId,iSize,altname,developer,publisher,features_general,features_online,esrb,esrb_descriptors,genre,release_date,year,rating,platform,exclusive,title_id,synopsis,resources,preview from files where strFileName like '%s'", item->GetPath().c_str());
		m_pDS->query(strSQL.c_str());
		if (!m_pDS->eof())
		{ // get info - only set the label if not preformatted
			if (!item->IsLabelPreformated())
			if (!SynopsisCheck)
			{
				if(m_pDS->fv("files.altname").get_asString().empty()) 
				{
					item->SetLabel(m_pDS->fv("xbedescription").get_asString());
				}
				else
				{
					item->SetLabel(m_pDS->fv("files.altname").get_asString());
				}
			}
			else
			{
				item->SetLabel(m_pDS->fv("xbedescription").get_asString());	
			}
			item->m_iprogramCount = m_pDS->fv("iTimesPlayed").get_asInt();
			// item->m_strSynopsis = m_pDS->fv("synopsis").get_asString().c_str();
			item->m_strTitle = item->GetLabel();  // is this needed?
			item->m_dateTime = TimeStampToLocalTime(_atoi64(m_pDS->fv("lastAccessed").get_asString().c_str()));
			item->m_dwSize = _atoi64(m_pDS->fv("iSize").get_asString().c_str());
			idTitle = m_pDS->fv("titleId").get_asInt();

			item->SetLabelSynopsis_AltName(m_pDS->fv("files.altname").get_asString());
			item->SetLabelSynopsis_Developer(m_pDS->fv("files.developer").get_asString());
			item->SetLabelSynopsis_Publisher(m_pDS->fv("files.publisher").get_asString());
			
			CStdString Synopsis_FeaturesGeneral = m_pDS->fv("files.features_general").get_asString();
			item->SetLabelSynopsis_FeaturesGeneral(Synopsis_FeaturesGeneral);
			CStdString strTest = Synopsis_FeaturesGeneral;
			strTest.ToLower();
			if (strTest.Left(8).Equals("players "))
			{
				int pos = strTest.Find(" /");
				if (pos > 0)
				{
					CStdString strNew = strTest.Mid(8, pos - 8);
					item->SetLabelSynopsis_PlayerCount(strNew);
					// CLog::Log(LOGNOTICE, "1 - Found player count: %s - %s", strNew.c_str(), m_pDS->fv("xbedescription").get_asString().c_str());
				}
				else
				{
					int pos = strTest.Find(",");
					if (pos > 0)
					{
						CStdString strNew = strTest.Mid(8, pos - 8);
						item->SetLabelSynopsis_PlayerCount(strNew);
						// CLog::Log(LOGNOTICE, "1 - Found player count: %s - %s", strNew.c_str(), m_pDS->fv("xbedescription").get_asString().c_str());
					}
					else
					{
						CStdString strNew = strTest.Mid(8);
						item->SetLabelSynopsis_PlayerCount(strNew);
						// CLog::Log(LOGNOTICE, "2 - Found player count: %s - %s", strNew.c_str(), m_pDS->fv("xbedescription").get_asString().c_str());
					}
				}
			}
			else
			{
				item->SetLabelSynopsis_PlayerCount("1");
			}		
			
			item->SetLabelSynopsis_FeaturesOnline(m_pDS->fv("files.features_online").get_asString());
			item->SetLabelSynopsis_ESRB(m_pDS->fv("files.esrb").get_asString());
			item->SetLabelSynopsis_ESRBDescriptors(m_pDS->fv("files.esrb_descriptors").get_asString());
			item->SetLabelSynopsis_Genre(m_pDS->fv("files.genre").get_asString());
			item->SetLabelSynopsis_ReleaseDate(m_pDS->fv("files.release_date").get_asString());
			item->SetLabelSynopsis_Year(m_pDS->fv("files.year").get_asString());
			item->SetLabelSynopsis_Rating(m_pDS->fv("files.rating").get_asString());
			item->SetLabelSynopsis_Platform(m_pDS->fv("files.platform").get_asString());
			item->SetLabelSynopsis_Exclusive(m_pDS->fv("files.exclusive").get_asString());
			item->SetLabelSynopsis_TitleID(m_pDS->fv("files.title_id").get_asString());
			item->SetLabelSynopsis_Overview(m_pDS->fv("files.synopsis").get_asString());
			item->SetLabelSynopsis_Resources(m_pDS->fv("files.resources").get_asString());
			item->SetSynopsis_Preview(m_pDS->fv("files.preview").get_asString());

			if (item->m_dwSize == -1)
			{
				CStdString strPath1;
				URIUtils::GetDirectory(item->GetPath(),strPath1);
				__int64 iSize = CGUIWindowFileManager::CalculateFolderSize(strPath1);
				CStdString strSQL=PrepareSQL("update files set iSize=%I64u where strFileName like '%s'",iSize,item->GetPath().c_str());
				m_pDS->exec(strSQL.c_str());
			}
		}
		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::GetProgramInfo(%s) failed", item->GetPath().c_str());
	}
	return idTitle;
}

bool CProgramDatabase::UpdateProgramInfo(CFileItem *item, unsigned int titleID)
{
	try
	{
		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", item->GetPath().c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		
		CStdString xbedescription;
		CStdString altname;
		CStdString developer;
		CStdString publisher;
		CStdString features_general;
		CStdString features_online;
		CStdString esrb;
		CStdString esrb_descriptors;
		CStdString genre;
		CStdString release_date;
		CStdString year;
		CStdString rating;
		CStdString platform;
		CStdString exclusive;
		CStdString title_id;
		CStdString synopsis;
		CStdString fanart;
		CStdString resources;
		CStdString preview;
		
		// Check if the _resources folder exists or not
		CStdString resources_path;
		URIUtils::GetDirectory(item->GetPath(),resources_path);
		URIUtils::AddFileToFolder(resources_path,"_resources\\",resources_path);
		resources = resources_path.c_str();
		if (!CDirectory::Exists(resources))
		resources = "";

		// Get XBE name, if its empty it uses the file name :/ will need to fix that
		CUtil::GetXBEDescription(item->GetPath(), xbedescription);
		
		// Gets the language of the system and then checks for the default.xml and then if there is a translated section
		CStdString strLanguage = g_guiSettings.GetString("locale.language");
		strLanguage.MakeLower();
		CStdString defaultxml;
		URIUtils::AddFileToFolder(resources,"default.xml",defaultxml);
		if (CFile::Exists(defaultxml))
		{
			TiXmlDocument xml_path_load;
			xml_path_load.LoadFile(defaultxml);
			TiXmlElement *pRootElement = xml_path_load.RootElement();
			TiXmlElement *pChildElement = pRootElement->FirstChildElement(strLanguage);
			if (pRootElement)
			{
				XMLUtils::GetString(pRootElement,"title", altname);
				XMLUtils::GetString(pRootElement,"developer", developer);
				XMLUtils::GetString(pRootElement,"publisher", publisher);
				XMLUtils::GetString(pRootElement,"features_general", features_general);
				XMLUtils::GetString(pRootElement,"features_online", features_online);
				XMLUtils::GetString(pRootElement,"esrb", esrb);
				XMLUtils::GetString(pRootElement,"esrb_descriptors", esrb_descriptors);
				XMLUtils::GetString(pRootElement,"genre", genre);
				XMLUtils::GetString(pRootElement,"release_date", release_date);
				XMLUtils::GetString(pRootElement,"year", year);
				XMLUtils::GetString(pRootElement,"rating", rating);
				XMLUtils::GetString(pRootElement,"platform", platform);
				XMLUtils::GetString(pRootElement,"exclusive", exclusive);
				XMLUtils::GetString(pRootElement,"titleid", title_id);
				XMLUtils::GetString(pRootElement,"overview", synopsis);
				
				if (pChildElement)
				{
					TiXmlElement *pChild1 = pChildElement->FirstChildElement("title");
					TiXmlElement *pChild2 = pChildElement->FirstChildElement("developer");
					TiXmlElement *pChild3 = pChildElement->FirstChildElement("publisher");
					TiXmlElement *pChild4 = pChildElement->FirstChildElement("features_general");
					TiXmlElement *pChild5 = pChildElement->FirstChildElement("features_online");
					TiXmlElement *pChild6 = pChildElement->FirstChildElement("esrb");
					TiXmlElement *pChild7 = pChildElement->FirstChildElement("esrb_descriptors");
					TiXmlElement *pChild8 = pChildElement->FirstChildElement("genre");
					TiXmlElement *pChild9 = pChildElement->FirstChildElement("release_date");
					TiXmlElement *pChild10 = pChildElement->FirstChildElement("year");
					TiXmlElement *pChild11 = pChildElement->FirstChildElement("rating");
					TiXmlElement *pChild12 = pChildElement->FirstChildElement("platform");
					TiXmlElement *pChild13 = pChildElement->FirstChildElement("exclusive");
					TiXmlElement *pChild14 = pChildElement->FirstChildElement("titleid");
					TiXmlElement *pChild15 = pChildElement->FirstChildElement("overview");
					if (pChild1)
					XMLUtils::GetString(pChildElement,"title", altname);
					if (pChild2)
					XMLUtils::GetString(pChildElement,"developer", developer);
					if (pChild3)
					XMLUtils::GetString(pChildElement,"publisher", publisher);
					if (pChild4)
					XMLUtils::GetString(pChildElement,"features_general", features_general);
					if (pChild5)
					XMLUtils::GetString(pChildElement,"features_online", features_online);
					if (pChild6)
					XMLUtils::GetString(pChildElement,"esrb", esrb);
					if (pChild7)
					XMLUtils::GetString(pChildElement,"esrb_descriptors", esrb_descriptors);
					if (pChild8)
					XMLUtils::GetString(pChildElement,"genre", genre);
					if (pChild9)
					XMLUtils::GetString(pChildElement,"release_date", release_date);
					if (pChild10)
					XMLUtils::GetString(pChildElement,"year", year);
					if (pChild11)
					XMLUtils::GetString(pChildElement,"rating", rating);
					if (pChild12)
					XMLUtils::GetString(pChildElement,"platform", platform);
					if (pChild13)
					XMLUtils::GetString(pChildElement,"exclusive", exclusive);
					if (pChild14)
					XMLUtils::GetString(pChildElement,"titleid", title_id);
					if (pChild15)
					XMLUtils::GetString(pChildElement,"overview", synopsis);
				}
			}
			CStdString previewfile;
			URIUtils::AddFileToFolder(resources,"media\\preview.mp4",previewfile);
			if (CFile::Exists(previewfile))
			{
				preview = "1";
			}
		}
		
		CStdString toReplace = ", ";
		features_general = ReplaceWithForwardSlash(features_general, toReplace);
		features_online = ReplaceWithForwardSlash(features_online, toReplace);
		esrb_descriptors = ReplaceWithForwardSlash(esrb_descriptors, toReplace);
		genre = ReplaceWithForwardSlash(genre, toReplace);
		platform = ReplaceWithForwardSlash(platform, toReplace);

		// special case - programs in root of sources
		CStdString strPath, strParent;
		URIUtils::GetDirectory(item->GetPath(),strPath);
		bool bIsShare=false;
		CUtil::GetMatchingSource(strPath,g_settings.m_programSources,bIsShare);
		__int64 iSize=0;
		if (bIsShare || !item->IsDefaultXBE())
		{
			__stat64 stat;
			if (CFile::Stat(item->GetPath(),&stat) == 0)
			iSize = stat.st_size;
		}
		else
		iSize = CGUIWindowFileManager::CalculateFolderSize(strPath);
		if (titleID == 0)
		titleID = (unsigned int) -1;
		
		strSQL=PrepareSQL("update files set strFileName='%s', titleId=%u, xbedescription='%s', iSize=%I64u, altname='%s', developer='%s', publisher='%s', features_general='%s', features_online='%s', esrb='%s', esrb_descriptors='%s', genre='%s', release_date='%s', year='%s', rating='%s', platform='%s', exclusive='%s', title_id='%s', synopsis='%s', resources='%s', preview='%s' where idFile=%i",
		item->GetPath().c_str(), titleID, xbedescription.c_str(), iSize, altname.c_str(), developer.c_str(), publisher.c_str(), features_general.c_str(), features_online.c_str(), esrb.c_str(), esrb_descriptors.c_str(), genre.c_str(), release_date.c_str(), year.c_str(), rating.c_str(), platform.c_str(), exclusive.c_str(), title_id.c_str(), synopsis.c_str(), resources.c_str(), preview.c_str(), idFile);
		m_pDS->exec(strSQL.c_str());

		item->m_dwSize = iSize;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::UpdatingProgramInfo(%s) failed", item->GetPath().c_str());
	}
	return true;
}

bool CProgramDatabase::AddProgramInfo(CFileItem *item, unsigned int titleID)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		int iRegion = -1;
		if (g_guiSettings.GetBool("myprograms.gameautoregion"))
		{
			CXBE xbe;
			iRegion = xbe.ExtractGameRegion(item->GetPath());
			if (iRegion < 1 || iRegion > 7)
			iRegion = 0;
		}
		FILETIME time;
		item->m_dateTime=CDateTime::GetCurrentDateTime();
		item->m_dateTime.GetAsTimeStamp(time);

		ULARGE_INTEGER lastAccessed;
		lastAccessed.u.LowPart = time.dwLowDateTime; 
		lastAccessed.u.HighPart = time.dwHighDateTime;

		CStdString altname;
		CStdString developer;
		CStdString publisher;
		CStdString features_general;
		CStdString features_online;
		CStdString esrb;
		CStdString esrb_descriptors;
		CStdString genre;
		CStdString release_date;
		CStdString year;
		CStdString rating;
		CStdString platform;
		CStdString exclusive;
		CStdString title_id;
		CStdString synopsis;
		CStdString fanart;
		CStdString resources;
		CStdString preview;
		
		// Check if the _resources folder exists or not
		CStdString resources_path;
		URIUtils::GetDirectory(item->GetPath(),resources_path);
		URIUtils::AddFileToFolder(resources_path,"_resources\\",resources_path);
		resources = resources_path.c_str();
		if (!CDirectory::Exists(resources))
		resources = "";

		// Gets the language of the system and then checks for the default.xml and then if there is a translated section
		CStdString strLanguage = g_guiSettings.GetString("locale.language");
		strLanguage.MakeLower();
		CStdString defaultxml;
		URIUtils::AddFileToFolder(resources,"default.xml",defaultxml);
		if (CFile::Exists(defaultxml))
		{
			TiXmlDocument xml_path_load;
			xml_path_load.LoadFile(defaultxml);
			TiXmlElement *pRootElement = xml_path_load.RootElement();
			TiXmlElement *pChildElement = pRootElement->FirstChildElement(strLanguage);
			if (pRootElement)
			{
				XMLUtils::GetString(pRootElement,"title", altname);
				XMLUtils::GetString(pRootElement,"developer", developer);
				XMLUtils::GetString(pRootElement,"publisher", publisher);
				XMLUtils::GetString(pRootElement,"features_general", features_general);
				XMLUtils::GetString(pRootElement,"features_online", features_online);
				XMLUtils::GetString(pRootElement,"esrb", esrb);
				XMLUtils::GetString(pRootElement,"esrb_descriptors", esrb_descriptors);
				XMLUtils::GetString(pRootElement,"genre", genre);
				XMLUtils::GetString(pRootElement,"release_date", release_date);
				XMLUtils::GetString(pRootElement,"year", year);
				XMLUtils::GetString(pRootElement,"rating", rating);
				XMLUtils::GetString(pRootElement,"platform", platform);
				XMLUtils::GetString(pRootElement,"exclusive", exclusive);
				XMLUtils::GetString(pRootElement,"titleid", title_id);
				XMLUtils::GetString(pRootElement,"overview", synopsis);
				
				if (pChildElement)
				{
					TiXmlElement *pChild1 = pChildElement->FirstChildElement("title");
					TiXmlElement *pChild2 = pChildElement->FirstChildElement("developer");
					TiXmlElement *pChild3 = pChildElement->FirstChildElement("publisher");
					TiXmlElement *pChild4 = pChildElement->FirstChildElement("features_general");
					TiXmlElement *pChild5 = pChildElement->FirstChildElement("features_online");
					TiXmlElement *pChild6 = pChildElement->FirstChildElement("esrb");
					TiXmlElement *pChild7 = pChildElement->FirstChildElement("esrb_descriptors");
					TiXmlElement *pChild8 = pChildElement->FirstChildElement("genre");
					TiXmlElement *pChild9 = pChildElement->FirstChildElement("release_date");
					TiXmlElement *pChild10 = pChildElement->FirstChildElement("year");
					TiXmlElement *pChild11 = pChildElement->FirstChildElement("rating");
					TiXmlElement *pChild12 = pChildElement->FirstChildElement("platform");
					TiXmlElement *pChild13 = pChildElement->FirstChildElement("exclusive");
					TiXmlElement *pChild14 = pChildElement->FirstChildElement("titleid");
					TiXmlElement *pChild15 = pChildElement->FirstChildElement("overview");
					if (pChild1)
					XMLUtils::GetString(pChildElement,"title", altname);
					if (pChild2)
					XMLUtils::GetString(pChildElement,"developer", developer);
					if (pChild3)
					XMLUtils::GetString(pChildElement,"publisher", publisher);
					if (pChild4)
					XMLUtils::GetString(pChildElement,"features_general", features_general);
					if (pChild5)
					XMLUtils::GetString(pChildElement,"features_online", features_online);
					if (pChild6)
					XMLUtils::GetString(pChildElement,"esrb", esrb);
					if (pChild7)
					XMLUtils::GetString(pChildElement,"esrb_descriptors", esrb_descriptors);
					if (pChild8)
					XMLUtils::GetString(pChildElement,"genre", genre);
					if (pChild9)
					XMLUtils::GetString(pChildElement,"release_date", release_date);
					if (pChild10)
					XMLUtils::GetString(pChildElement,"year", year);
					if (pChild11)
					XMLUtils::GetString(pChildElement,"rating", rating);
					if (pChild12)
					XMLUtils::GetString(pChildElement,"platform", platform);
					if (pChild13)
					XMLUtils::GetString(pChildElement,"exclusive", exclusive);
					if (pChild14)
					XMLUtils::GetString(pChildElement,"titleid", title_id);
					if (pChild15)
					XMLUtils::GetString(pChildElement,"overview", synopsis);
				}
			}
			CStdString previewfile;
			URIUtils::AddFileToFolder(resources,"media\\preview.mp4",previewfile);
			if (CFile::Exists(previewfile))
			preview = "1";
		}
		
		CStdString toReplace = ", ";
		features_general = ReplaceWithForwardSlash(features_general, toReplace);
		features_online = ReplaceWithForwardSlash(features_online, toReplace);
		esrb_descriptors = ReplaceWithForwardSlash(esrb_descriptors, toReplace);
		genre = ReplaceWithForwardSlash(genre, toReplace);
		platform = ReplaceWithForwardSlash(platform, toReplace);

		// special case - programs in root of sources
		CStdString strPath6, strParent;
		URIUtils::GetDirectory(item->GetPath(),strPath6);
		bool bIsShare=false;
		CUtil::GetMatchingSource(strPath6,g_settings.m_programSources,bIsShare);
		__int64 iSize=0;
		if (bIsShare || !item->IsDefaultXBE())
		{
			__stat64 stat;
			if (CFile::Stat(item->GetPath(),&stat) == 0)
			iSize = stat.st_size;
		}
		else
		iSize = CGUIWindowFileManager::CalculateFolderSize(strPath6);
		if (titleID == 0)
		titleID = (unsigned int) -1;
		CStdString strSQL=PrepareSQL("insert into files (idFile, strFileName, titleId, xbedescription, iTimesPlayed, lastAccessed, iRegion, iSize, altname, developer, publisher, features_general, features_online, esrb, esrb_descriptors, genre, release_date, year, rating, platform, exclusive, title_id, synopsis, resources, preview) values(NULL, '%s', %u, '%s', %i, %I64u, %i, %I64u, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
		item->GetPath().c_str(), titleID, item->GetLabel().c_str(), 0, lastAccessed.QuadPart, iRegion, iSize, altname.c_str(), developer.c_str(), publisher.c_str(), features_general.c_str(), features_online.c_str(), esrb.c_str(), esrb_descriptors.c_str(), genre.c_str(), release_date.c_str(), year.c_str(), rating.c_str(), platform.c_str(), exclusive.c_str(), title_id.c_str(), synopsis.c_str(), resources.c_str(), preview.c_str());
		
		m_pDS->exec(strSQL.c_str());
		item->m_dwSize = iSize;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::AddProgramInfo(%s) failed", item->GetPath().c_str());
	}
	return true;
}

FILETIME CProgramDatabase::TimeStampToLocalTime( unsigned __int64 timeStamp )
{
	FILETIME fileTime;
	::FileTimeToLocalFileTime( (const FILETIME *)&timeStamp, &fileTime);
	return fileTime;
}

bool CProgramDatabase::IncTimesPlayed(const CStdString& strFileName)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFileName.c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return false;
		}
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		int iTimesPlayed = m_pDS->fv("files.iTimesPlayed").get_asInt();
		m_pDS->close();

		CLog::Log(LOGDEBUG, "CProgramDatabase::IncTimesPlayed(%s), idFile=%i, iTimesPlayed=%i",
		strFileName.c_str(), idFile, iTimesPlayed);

		strSQL=PrepareSQL("update files set iTimesPlayed=%i where idFile=%i",
		++iTimesPlayed, idFile);
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:IncTimesPlayed(%s) failed", strFileName.c_str());
	}

	return false;
}

bool CProgramDatabase::SetDescription(const CStdString& strFileName, const CStdString& strDescription)
{
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFileName.c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return false;
		}
		int idFile = m_pDS->fv("files.idFile").get_asInt();
		m_pDS->close();

		CLog::Log(LOGDEBUG, "CProgramDatabase::SetDescription(%s), idFile=%i, description=%s",
		strFileName.c_str(), idFile, strDescription.c_str());
		if (!g_guiSettings.GetBool("mygames.gamesynopsisinfo"))
		{
			strSQL=PrepareSQL("update files set altname='%s' where idFile=%i",strDescription.c_str(), idFile);
		}
		else
		{
			strSQL=PrepareSQL("update files set xbedescription='%s' where idFile=%i",strDescription.c_str(), idFile);
		}
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:SetDescription(%s) failed", strFileName.c_str());
	}

	return false;
}

bool CProgramDatabase::GetArbitraryQuery(const CStdString& strQuery,      const CStdString& strOpenRecordSet, const CStdString& strCloseRecordSet,
const CStdString& strOpenRecord, const CStdString& strCloseRecord,   const CStdString& strOpenField, 
const CStdString& strCloseField,       CStdString& strResult)
{
	try
	{
		strResult = "";
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;
		CStdString strSQL=strQuery;
		if (!m_pDS->query(strSQL.c_str()))
		{
			strResult = m_pDB->getErrorMsg();
			return false;
		}
		strResult=strOpenRecordSet;
		while (!m_pDS->eof())
		{
			strResult += strOpenRecord;
			for (int i=0; i<m_pDS->fieldCount(); i++)
			{
				strResult += strOpenField + CStdString(m_pDS->fv(i).get_asString()) + strCloseField;
			}
			strResult += strCloseRecord;
			m_pDS->next();
		}
		strResult += strCloseRecordSet;
		m_pDS->close();
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
	try
	{
		if (NULL == m_pDB.get()) return false;
		strResult = m_pDB->getErrorMsg();
	}
	catch (...)
	{

	}

	return false;
}
