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
#include "utils/fanart.h"
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

int cutoffPoint = 700;

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
		m_pDS->exec(
			"CREATE TABLE files ( "
			"idFile integer primary key, "
			"strFilename text, "
			"titleId integer, "
			"xbedescription text, "
			"iTimesPlayed integer, "
			"lastAccessed integer, "
			"iRegion integer, "
			"iSize integer, "
			"altname text, "
			"developer text, "
			"publisher text, "
			"features_general text, "
			"features_online text, "
			"esrb text, "
			"esrb_descriptors text, "
			"genre text, "
			"release_date text, "
			"year text, "
			"rating text, "
			"platform text, "
			"exclusive text, "
			"title_id text, "
			"synopsis text, "
			"resources text, "
			"preview text, "
			"screenshot text, "
			"fanart text, "
			"last_played text, "
			"alt_xbe text NOT NULL DEFAULT ''"
			")\n"
		);
		CLog::Log(LOGINFO, "create trainers table");
		m_pDS->exec(
			"CREATE TABLE trainers ( "
			"idKey integer auto_increment primary key, "
			"idCRC integer, "
			"idTitle integer, "
			"strTrainerPath text, "
			"strSettings text, "
			"Active integer"
			")\n"
		);
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

	// try
	// {	
		// if (version < 4)
		// {
			// m_pDS->exec("ALTER TABLE files ADD COLUMN altname text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN developer text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN publisher text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN features_general text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN features_online text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN esrb text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN esrb_descriptors text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN genre text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN release_date text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN year text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN rating text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN platform text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN exclusive text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN title_id text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN synopsis text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN resources text NOT NULL DEFAULT ''");
			// m_pDS->exec("ALTER TABLE files ADD COLUMN preview text NOT NULL DEFAULT ''");

			// CLog::Log(LOGINFO, "Updated database to version 4");
		// }
	// } catch (...){}
	
	// try
	// {	
		// if (version < 5)
		// {
			// m_pDS->exec("ALTER TABLE files ADD COLUMN last_played text NOT NULL DEFAULT ''");
			// CLog::Log(LOGINFO, "Updated database to version 5");
		// }
	// } catch (...){}
	
	// try
	// {	
		// if (version < 6)
		// {
			// m_pDS->exec("ALTER TABLE files ADD COLUMN fanart text NOT NULL DEFAULT ''");
			// CLog::Log(LOGINFO, "Updated database to version 6");
		// }
	// } catch (...){}
	
	// try
	// {	
		// if (version < 7)
		// {
			// m_pDS->exec("ALTER TABLE files ADD COLUMN screenshot text NOT NULL DEFAULT ''");
			// CLog::Log(LOGINFO, "Updated database to version 7");
		// }
	// } catch (...){}
	
	return true;
}

int CProgramDatabase::GetRegion(const CStdString& strFilenameAndPath)
{
	if (NULL == m_pDB.get() || NULL == m_pDS.get()) return 0;

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
	if (NULL == m_pDB.get() || NULL == m_pDS.get()) return 0;

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
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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

bool CProgramDatabase::SetXBEType(const CStdString& strFilenameAndPath, const CStdString& strFilenamePath)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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

		CLog::Log(LOGDEBUG, "CProgramDatabase::Setting xbe type(%s), idFile=%i, alt_xbe=%s", strFilenameAndPath.c_str(), idFile, strFilenamePath.c_str());

		CStdString xbePath = strFilenamePath;
		int pos = xbePath.ReverseFind('\\');
		if (pos != -1)
			CStdString folderPath = xbePath.Left(pos);
			CStdString fileName = xbePath.Right(xbePath.GetLength() - pos - 1);

		strSQL = PrepareSQL("update files set alt_xbe='%s' where idFile=%i", fileName.c_str(), idFile);
		m_pDS->exec(strSQL.c_str());
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:SetXbeType(%s) failed", strFilenamePath.c_str());
	}

	return false;
}

CStdString CProgramDatabase::GetXBEType(const CStdString& strFilenameAndPath)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return "";

		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", strFilenameAndPath.c_str());
		if (!m_pDS->query(strSQL.c_str())) return "";
		int iRowsFound = m_pDS->num_rows();
		if (iRowsFound == 0)
		{
			m_pDS->close();
			return "";
		}
		CStdString alt_xbe = m_pDS->fv("files.alt_xbe").get_asString();
		m_pDS->close();
		return alt_xbe;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase:GetXBEType(%s) failed", strFilenameAndPath.c_str());
	}

	return "";
}


bool CProgramDatabase::SetLastPlayed(const CStdString& strFilenameAndPath)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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

bool CProgramDatabase::StringToFileTime(const CStdString& str, FILETIME& ft) {
	SYSTEMTIME st = {0};
	int ret = _stscanf(str, _T("%04d-%02d-%02d %02d:%02d"), &st.wYear, &st.wMonth, &st.wDay, &st.wHour, &st.wMinute);
	CLog::Log(LOGDEBUG, "Parsed SYSTEMTIME: %04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	
	if (ret != 5) {
		return false;
	}
	if (!SystemTimeToFileTime(&st, &ft)) {
		return false;
	}
	return true;
}

CStdString CProgramDatabase::RemoveTrailingTime(CStdString str) {
	if (str.IsEmpty()) {
		return _T("");
	}
	
	if (str.GetLength() > 3 &&
		str.Right(3).GetAt(0) == ':' &&
		_istdigit(str.Right(3).GetAt(1)) &&
		_istdigit(str.Right(3).GetAt(2))) {
		str = str.Left(str.GetLength() - 3);
	}

	CLog::Log(LOGDEBUG, "Result after RemoveTrailingTime: %s", str.c_str());
	return str;
}

bool CProgramDatabase::GetXBEPathByTitleId(const int idTitle, CStdString& strPathAndFilename)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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

CStdString CProgramDatabase::ReplaceWith(const CStdString& strInput, CStdString& strToReplace, CStdString& strToReplaceWith)
{
	CStdString output = strInput;
	size_t pos = 0;
	while ((pos = output.find(strToReplace, pos)) != std::string::npos) {
		output.replace(pos, strToReplace.length(), strToReplaceWith);
		pos += strToReplaceWith.length();
	}
	return output;
}

int CProgramDatabase::GetProgramInfo(CFileItem* item)
{
	int synopsisCheck = g_guiSettings.GetBool("mygames.gamesynopsisinfo");
	int idTitle = 0;
	try
	{
		if (!m_pDB.get() || !m_pDS.get()) return false;

		CStdString strSQL = PrepareSQL("select xbedescription,iTimesPlayed,lastAccessed,titleId,iSize,altname,developer,publisher,features_general,features_online,esrb,esrb_descriptors,genre,release_date,year,rating,platform,exclusive,title_id,synopsis,resources,preview,screenshot,fanart,last_played from files where strFileName like '%s'", item->GetPath().c_str());
		m_pDS->query(strSQL.c_str());
		if (!m_pDS->eof())
		{
			// get info - only set the label if not pre-formatted
			if (!item->IsLabelPreformated()) {
				if (!synopsisCheck) {
					item->SetLabel(m_pDS->fv("files.altname").get_asString().empty() ? m_pDS->fv("xbedescription").get_asString() : m_pDS->fv("files.altname").get_asString());
				}
				else {
					item->SetLabel(m_pDS->fv("xbedescription").get_asString());
				}
			}
			
			// Check if the _resources folder exists
			CStdString resources;
			CStdString resources_path;
			URIUtils::GetDirectory(item->GetPath(), resources_path);
			resources = resources_path + "_resources\\";
			
			item->m_iprogramCount = m_pDS->fv("iTimesPlayed").get_asInt();
			item->m_strTitle = item->GetLabel();  // is this needed?
			item->m_dateTime = TimeStampToLocalTime(_atoi64(m_pDS->fv("lastAccessed").get_asString().c_str()));
			item->m_dwSize = _atoi64(m_pDS->fv("iSize").get_asString().c_str());
			idTitle = m_pDS->fv("titleId").get_asInt();

			item->SetLabelSynopsis_AltName(m_pDS->fv("files.altname").get_asString());
			item->SetLabelSynopsis_Developer(m_pDS->fv("files.developer").get_asString());
			item->SetLabelSynopsis_Publisher(m_pDS->fv("files.publisher").get_asString());
			item->SetLabelSynopsis_FeaturesGeneral(m_pDS->fv("files.features_general").get_asString());
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
			
            CStdString synopsis = m_pDS->fv("files.synopsis").get_asString();
            item->SetProperty("Synopsis_Overview_Full", synopsis);
            size_t cutoffPointSizeT = static_cast<size_t>(cutoffPoint);
            if (synopsis.length() > cutoffPointSizeT)
            {
              size_t posComma = synopsis.find_first_of(',', cutoffPointSizeT);
              size_t posSpace = synopsis.find_first_of(' ', cutoffPointSizeT);
              if (posComma != std::string::npos && posComma < cutoffPointSizeT + 200) {
                  synopsis = synopsis.substr(0, posComma) + "...";
              } else if (posSpace != std::string::npos && posSpace < cutoffPointSizeT + 200) {
                  synopsis = synopsis.substr(0, posSpace) + "...";
              } else {
                  synopsis = synopsis.substr(0, cutoffPointSizeT) + "...";
              }
            }
            item->SetLabelSynopsis_Overview(synopsis);
			
			// item->SetLabelSynopsis_Overview(m_pDS->fv("files.synopsis").get_asString());
			item->SetLabelSynopsis_Resources(m_pDS->fv("files.resources").get_asString());
			item->SetLabelLastPlayed(m_pDS->fv("files.last_played").get_asString());
			item->SetSynopsis_Preview(m_pDS->fv("files.preview").get_asString());
			item->SetSynopsis_Screenshot(m_pDS->fv("files.screenshot").get_asString());
			item->SetSynopsis_Fanart(m_pDS->fv("files.fanart").get_asString());
			
			// Eats up 5MB :/ but it works fine, I just don't like seeing it goto 20/21MB of free memory with over 1000 games
			// if (!m_pDS->fv("files.resources").get_asString().empty())
				// item->SetProperty("alt_synopsis_image", resources_path + "_resources\\artwork\\alt_synopsis.jpg");
				// item->SetProperty("banner_image", resources_path + "_resources\\artwork\\banner.png");
				// item->SetProperty("cd_image", resources_path + "_resources\\artwork\\cd.png");
				// item->SetProperty("cd_small_image", resources_path + "_resources\\artwork\\cd_small.jpg");
				// item->SetProperty("cdposter_image", resources_path + "_resources\\artwork\\cdposter.png");
				// item->SetProperty("dual3d_image", resources_path + "_resources\\artwork\\dual3d.png");
				// item->SetProperty("fanart_image", resources_path + "_resources\\artwork\\fanart.jpg");
				// item->SetProperty("fanart_thumb_image", resources_path + "_resources\\artwork\\fanart_thumb.jpg");
				// item->SetProperty("fanart-blur_image", resources_path + "_resources\\artwork\\fanart-blur.jpg");
				// item->SetProperty("fog_image", resources_path + "_resources\\artwork\\fog.jpg");
				// item->SetProperty("icon_image", resources_path + "_resources\\artwork\\icon.png");
				// item->SetProperty("opencase_image", resources_path + "_resources\\artwork\\opencase.png");
				// item->SetProperty("poster_image", resources_path + "_resources\\artwork\\poster.jpg");
				// item->SetProperty("poster_small_image", resources_path + "_resources\\artwork\\poster_small.jpg");
				// item->SetProperty("poster_small_blurred_image", resources_path + "_resources\\artwork\\poster_small_blurred.jpg");
				// item->SetProperty("synopsis_image", resources_path + "_resources\\artwork\\synopsis.jpg");
				// item->SetProperty("thumb_image", resources_path + "_resources\\artwork\\thumb.jpg");
				// item->SetProperty("screenshot_images", resources_path + "_resources\\screenshots");
				// item->SetProperty("preview_video", resources_path + "_resources\\media\\preview.mp4");

			// Handling player count
			CStdString synopsisFeaturesGeneral = m_pDS->fv("files.features_general").get_asString();
			item->SetLabelSynopsis_FeaturesGeneral(synopsisFeaturesGeneral);
			CStdString strTest = synopsisFeaturesGeneral;
			strTest.ToLower();

			if (strTest.Left(8).Equals("players ")) {
				int pos = strTest.Find(" /");
				if (pos > 0) {
					item->SetLabelSynopsis_PlayerCount(strTest.Mid(8, pos - 8));
				}
				else if ((pos = strTest.Find(",")) > 0) {
					item->SetLabelSynopsis_PlayerCount(strTest.Mid(8, pos - 8));
				}
				else {
					item->SetLabelSynopsis_PlayerCount(strTest.Mid(8));
				}
			}
			else {
				item->SetLabelSynopsis_PlayerCount("1");
			}

			if (item->m_dwSize == -1) {
				CStdString strPath1;
				URIUtils::GetDirectory(item->GetPath(), strPath1);
				__int64 iSize = CGUIWindowFileManager::CalculateFolderSize(strPath1);
				strSQL = PrepareSQL("update files set iSize=%I64u where strFileName like '%s'", iSize, item->GetPath().c_str());
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


bool CProgramDatabase::UpdateProgramInfo(CFileItem* item, unsigned int titleID)
{
	try
	{
		// Prepare SQL query to get the file's id
		CStdString strSQL = PrepareSQL("select * from files where files.strFileName like '%s'", item->GetPath().c_str());
		if (!m_pDS->query(strSQL.c_str())) return false;
		int idFile = m_pDS->fv("files.idFile").get_asInt();

		// Declare variables to hold the program information
		CStdString xbedescription, altname, developer, publisher, features_general, features_online, esrb, esrb_descriptors, genre, release_date, year, rating, platform, exclusive, title_id, synopsis, resources, preview, screenshot, fanart;

		// Check if the _resources folder exists
		CStdString resources_path;
		URIUtils::GetDirectory(item->GetPath(), resources_path);
		resources = resources_path + "_resources\\";
		if (!CDirectory::Exists(resources))
			resources = "";

		// Get XBE description
		if (!CUtil::GetXBEDescription(item->GetPath(), xbedescription))
			CUtil::GetDirectoryName(item->GetPath(), xbedescription);

		// Get system language and check for the default.xml and translated section
		CStdString strLanguage = g_guiSettings.GetString("locale.language");
		if (strLanguage.Find('(') != -1) {
			strLanguage.Replace(" (", "_");
			strLanguage.Replace(")", "");
		}
		strLanguage.MakeLower();

		CStdString defaultxml;
		URIUtils::AddFileToFolder(resources, "default.xml", defaultxml);
		if (CFile::Exists(defaultxml))
		{
			TiXmlDocument xml_path_load;
			xml_path_load.LoadFile(defaultxml);
			TiXmlElement* pRootElement = xml_path_load.RootElement();
			TiXmlElement* pChildElement = pRootElement->FirstChildElement(strLanguage);
			if (pRootElement)
			{
				XMLUtils::GetString(pRootElement, "title", altname);
				XMLUtils::GetString(pRootElement, "developer", developer);
				XMLUtils::GetString(pRootElement, "publisher", publisher);
				XMLUtils::GetString(pRootElement, "features_general", features_general);
				XMLUtils::GetString(pRootElement, "features_online", features_online);
				XMLUtils::GetString(pRootElement, "esrb", esrb);
				XMLUtils::GetString(pRootElement, "esrb_descriptors", esrb_descriptors);
				XMLUtils::GetString(pRootElement, "genre", genre);
				XMLUtils::GetString(pRootElement, "release_date", release_date);
				XMLUtils::GetString(pRootElement, "year", year);
				XMLUtils::GetString(pRootElement, "rating", rating);
				XMLUtils::GetString(pRootElement, "platform", platform);
				XMLUtils::GetString(pRootElement, "exclusive", exclusive);
				XMLUtils::GetString(pRootElement, "titleid", title_id);
				XMLUtils::GetString(pRootElement, "overview", synopsis);
				
				// Check for child elements with translated information
				if (pChildElement)
				{
					XMLUtils::GetString(pChildElement, "title", altname);
					XMLUtils::GetString(pChildElement, "developer", developer);
					XMLUtils::GetString(pChildElement, "publisher", publisher);
					XMLUtils::GetString(pChildElement, "features_general", features_general);
					XMLUtils::GetString(pChildElement, "features_online", features_online);
					XMLUtils::GetString(pChildElement, "esrb", esrb);
					XMLUtils::GetString(pChildElement, "esrb_descriptors", esrb_descriptors);
					XMLUtils::GetString(pChildElement, "genre", genre);
					XMLUtils::GetString(pChildElement, "release_date", release_date);
					XMLUtils::GetString(pChildElement, "year", year);
					XMLUtils::GetString(pChildElement, "rating", rating);
					XMLUtils::GetString(pChildElement, "platform", platform);
					XMLUtils::GetString(pChildElement, "exclusive", exclusive);
					XMLUtils::GetString(pChildElement, "titleid", title_id);
					XMLUtils::GetString(pChildElement, "overview", synopsis);
				}
			}
		}
		
		// Check for media files
		CStdString screenshotfile, previewfile, fanartfile1;
		URIUtils::AddFileToFolder(resources, "screenshots\\Screenshot-1.jpg", screenshotfile);
		if (CFile::Exists(screenshotfile))
			screenshot = "1";

		URIUtils::AddFileToFolder(resources, "media\\preview.mp4", previewfile);
		if (CFile::Exists(previewfile))
			preview = "1";

		URIUtils::AddFileToFolder(resources, "artwork\\fanart.jpg", fanartfile1);
		if (CFile::Exists(fanartfile1))
			fanart = "1";

		// Remove new lines and spaces from the start of the synopsis
		size_t start = 0;
		while (start < synopsis.size() && isspace(static_cast<unsigned char>(synopsis[start]))) {
			++start;
		}
		synopsis.erase(0, start);
		
		// This is disabled here as I do it when the database is pulling info for the games
		// Truncate the synopsis
		// size_t cutoffPointSizeT = static_cast<size_t>(cutoffPoint);
		// if (synopsis.length() > cutoffPointSizeT)
		// {
		  // size_t posComma = synopsis.find_first_of(',', cutoffPointSizeT);
		  // size_t posSpace = synopsis.find_first_of(' ', cutoffPointSizeT);
		  // if (posComma != std::string::npos && posComma < cutoffPointSizeT + 200) {
			  // synopsis = synopsis.substr(0, posComma) + "...";
		  // } else if (posSpace != std::string::npos && posSpace < cutoffPointSizeT + 200) {
			  // synopsis = synopsis.substr(0, posSpace) + "...";
		  // } else {
			  // synopsis = synopsis.substr(0, cutoffPointSizeT) + "...";
		  // }
		// }

		// Replace certain strings with other strings for formatting
		CStdString commaReplace = ", ";
		CStdString withSlash = " / ";
		features_general = ReplaceWith(features_general, commaReplace, withSlash);
		features_online = ReplaceWith(features_online, commaReplace, withSlash);
		esrb_descriptors = ReplaceWith(esrb_descriptors, commaReplace, withSlash);
		genre = ReplaceWith(genre, commaReplace, withSlash);
		platform = ReplaceWith(platform, commaReplace, withSlash);

		CStdString notRatedReplace = "Not Rated";
		CStdString withNA = "N/A";
		esrb = ReplaceWith(esrb, notRatedReplace, withNA);
		esrb_descriptors = ReplaceWith(esrb_descriptors, notRatedReplace, withNA);

		// Special case for programs in the root of sources
		CStdString strPath;
		URIUtils::GetDirectory(item->GetPath(), strPath);
		bool bIsShare = false;
		CUtil::GetMatchingSource(strPath, g_settings.m_programSources, bIsShare);
		__int64 iSize = 0;

		if (bIsShare || !item->IsDefaultXBE())
		{
			__stat64 stat;
			if (CFile::Stat(item->GetPath(), &stat) == 0)
				iSize = stat.st_size;
		}
		else
		{
			iSize = CGUIWindowFileManager::CalculateFolderSize(strPath);
		}

		if (titleID == 0)
			titleID = (unsigned int)-1;

		// Prepare SQL query to update the file information
		strSQL = PrepareSQL(
			"update files set "
			"strFileName='%s', "
			"titleId=%u, "
			"xbedescription='%s', "
			"iSize=%I64u, "
			"altname='%s', "
			"developer='%s', "
			"publisher='%s', "
			"features_general='%s', "
			"features_online='%s', "
			"esrb='%s', "
			"esrb_descriptors='%s', "
			"genre='%s', "
			"release_date='%s', "
			"year='%s', "
			"rating='%s', "
			"platform='%s', "
			"exclusive='%s', "
			"title_id='%s', "
			"synopsis='%s', "
			"resources='%s', "
			"preview='%s', "
			"screenshot='%s', "
			"fanart='%s' "
			"where idFile=%i",
			item->GetPath().c_str(), 
			titleID, 
			xbedescription.c_str(), 
			iSize, 
			altname.c_str(), 
			developer.c_str(), 
			publisher.c_str(), 
			features_general.c_str(), 
			features_online.c_str(), 
			esrb.c_str(), 
			esrb_descriptors.c_str(), 
			genre.c_str(), 
			release_date.c_str(), 
			year.c_str(), 
			rating.c_str(), 
			platform.c_str(), 
			exclusive.c_str(), 
			title_id.c_str(), 
			synopsis.c_str(), 
			resources.c_str(), 
			preview.c_str(), 
			screenshot.c_str(), 
			fanart.c_str(), 
			idFile
		);
		m_pDS->exec(strSQL.c_str());
		item->m_dwSize = iSize;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::UpdateProgramInfo(%s) failed", item->GetPath().c_str());
		return false;
	}
	return true;
}

bool CProgramDatabase::AddProgramInfo(CFileItem* item, unsigned int titleID)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

		int iRegion = -1;
		if (g_guiSettings.GetBool("myprograms.gameautoregion"))
		{
			CXBE xbe;
			iRegion = xbe.ExtractGameRegion(item->GetPath());
			if (iRegion < 1 || iRegion > 7) iRegion = 0;
		}

		FILETIME time;
		item->m_dateTime = CDateTime::GetCurrentDateTime();
		item->m_dateTime.GetAsTimeStamp(time);

		ULARGE_INTEGER lastAccessed;
		lastAccessed.u.LowPart = time.dwLowDateTime;
		lastAccessed.u.HighPart = time.dwHighDateTime;

		// Initialize variables to empty strings
		CStdString altname = "", developer = "", publisher = "", features_general = "", features_online = "", esrb = "", esrb_descriptors = "", genre = "", release_date = "", year = "", rating = "", platform = "", exclusive = "", title_id = "", synopsis = "", resources = "", preview = "", screenshot = "", fanart = "", last_played = "", alt_xbe = "";

		// Check if the _resources folder exists
		CStdString resources_path;
		URIUtils::GetDirectory(item->GetPath(), resources_path);
		resources = resources_path + "_resources\\";
		if (!CDirectory::Exists(resources))
			resources = "";

		// Get system language and check for the default.xml and translated section
		CStdString strLanguage = g_guiSettings.GetString("locale.language");
		if (strLanguage.Find('(') != -1)
		{
			strLanguage.Replace(" (", "_");
			strLanguage.Replace(")", "");
		}
		strLanguage.MakeLower();

		CStdString defaultxml;
		URIUtils::AddFileToFolder(resources, "default.xml", defaultxml);
		if (CFile::Exists(defaultxml))
		{
			TiXmlDocument xml_path_load;
			xml_path_load.LoadFile(defaultxml);
			TiXmlElement* pRootElement = xml_path_load.RootElement();
			TiXmlElement* pChildElement = pRootElement->FirstChildElement(strLanguage);
			if (pRootElement)
			{
				XMLUtils::GetString(pRootElement, "title", altname);
				XMLUtils::GetString(pRootElement, "developer", developer);
				XMLUtils::GetString(pRootElement, "publisher", publisher);
				XMLUtils::GetString(pRootElement, "features_general", features_general);
				XMLUtils::GetString(pRootElement, "features_online", features_online);
				XMLUtils::GetString(pRootElement, "esrb", esrb);
				XMLUtils::GetString(pRootElement, "esrb_descriptors", esrb_descriptors);
				XMLUtils::GetString(pRootElement, "genre", genre);
				XMLUtils::GetString(pRootElement, "release_date", release_date);
				XMLUtils::GetString(pRootElement, "year", year);
				XMLUtils::GetString(pRootElement, "rating", rating);
				XMLUtils::GetString(pRootElement, "platform", platform);
				XMLUtils::GetString(pRootElement, "exclusive", exclusive);
				XMLUtils::GetString(pRootElement, "titleid", title_id);
				XMLUtils::GetString(pRootElement, "overview", synopsis);

				// Check for child elements with translated information
				if (pChildElement)
				{
					XMLUtils::GetString(pChildElement, "title", altname);
					XMLUtils::GetString(pChildElement, "developer", developer);
					XMLUtils::GetString(pChildElement, "publisher", publisher);
					XMLUtils::GetString(pChildElement, "features_general", features_general);
					XMLUtils::GetString(pChildElement, "features_online", features_online);
					XMLUtils::GetString(pChildElement, "esrb", esrb);
					XMLUtils::GetString(pChildElement, "esrb_descriptors", esrb_descriptors);
					XMLUtils::GetString(pChildElement, "genre", genre);
					XMLUtils::GetString(pChildElement, "release_date", release_date);
					XMLUtils::GetString(pChildElement, "year", year);
					XMLUtils::GetString(pChildElement, "rating", rating);
					XMLUtils::GetString(pChildElement, "platform", platform);
					XMLUtils::GetString(pChildElement, "exclusive", exclusive);
					XMLUtils::GetString(pChildElement, "titleid", title_id);
					XMLUtils::GetString(pChildElement, "overview", synopsis);
				}
			}
		}
		
		// Check for media files
		CStdString screenshotfile, previewfile, fanartfile1;
		URIUtils::AddFileToFolder(resources, "screenshots\\Screenshot-1.jpg", screenshotfile);
		if (CFile::Exists(screenshotfile))
			screenshot = "1";

		URIUtils::AddFileToFolder(resources, "media\\preview.mp4", previewfile);
		if (CFile::Exists(previewfile))
			preview = "1";

		URIUtils::AddFileToFolder(resources, "artwork\\fanart.jpg", fanartfile1);
		if (CFile::Exists(fanartfile1))
			fanart = "1";

		// Remove new lines and spaces from the start of the synopsis
		size_t start = 0;
		while (start < synopsis.size() && isspace(static_cast<unsigned char>(synopsis[start]))) {
			++start;
		}
		synopsis.erase(0, start);

		// This is disabled here as I do it when the database is pulling info for the games
		// Truncate the synopsis
		// size_t cutoffPointSizeT = static_cast<size_t>(cutoffPoint);
		// if (synopsis.length() > cutoffPointSizeT)
		// {
		  // size_t posComma = synopsis.find_first_of(',', cutoffPointSizeT);
		  // size_t posSpace = synopsis.find_first_of(' ', cutoffPointSizeT);
		  // if (posComma != std::string::npos && posComma < cutoffPointSizeT + 200) {
			  // synopsis = synopsis.substr(0, posComma) + "...";
		  // } else if (posSpace != std::string::npos && posSpace < cutoffPointSizeT + 200) {
			  // synopsis = synopsis.substr(0, posSpace) + "...";
		  // } else {
			  // synopsis = synopsis.substr(0, cutoffPointSizeT) + "...";
		  // }
		// }

		// Replace strings for formatting
		CStdString commaReplace = ", ";
		CStdString withSlash = " / ";
		features_general = ReplaceWith(features_general, commaReplace, withSlash);
		features_online = ReplaceWith(features_online, commaReplace, withSlash);
		esrb_descriptors = ReplaceWith(esrb_descriptors, commaReplace, withSlash);
		genre = ReplaceWith(genre, commaReplace, withSlash);
		platform = ReplaceWith(platform, commaReplace, withSlash);

		CStdString notRatedReplace = "Not Rated";
		CStdString withNA = "N/A";
		esrb = ReplaceWith(esrb, notRatedReplace, withNA);
		esrb_descriptors = ReplaceWith(esrb_descriptors, notRatedReplace, withNA);

		// Handle special case for programs in the root of sources
		CStdString strPath6;
		URIUtils::GetDirectory(item->GetPath(), strPath6);
		bool bIsShare = false;
		CUtil::GetMatchingSource(strPath6, g_settings.m_programSources, bIsShare);
		__int64 iSize = 0;
		if (bIsShare || !item->IsDefaultXBE())
		{
			__stat64 stat;
			if (CFile::Stat(item->GetPath(), &stat) == 0)
				iSize = stat.st_size;
		}
		else
		{
			iSize = CGUIWindowFileManager::CalculateFolderSize(strPath6);
		}

		if (titleID == 0)
			titleID = (unsigned int)-1;

		// Prepare SQL query to insert the program information
		CStdString strSQL = PrepareSQL(
			"insert into files ("
			"idFile, "
			"strFileName, "
			"titleId, "
			"xbedescription, "
			"iTimesPlayed, "
			"lastAccessed, "
			"iRegion, "
			"iSize, "
			"altname, "
			"developer, "
			"publisher, "
			"features_general, "
			"features_online, "
			"esrb, "
			"esrb_descriptors, "
			"genre, "
			"release_date, "
			"year, "
			"rating, "
			"platform, "
			"exclusive, "
			"title_id, "
			"synopsis, "
			"resources, "
			"preview, "
			"screenshot, "
			"fanart, "
			"last_played, "
			"alt_xbe"
			") values ("
			"NULL, "
			"'%s', "
			"%u, "
			"'%s', "
			"%i, "
			"%I64u, "
			"%i, "
			"%I64u, "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s', "
			"'%s'"
			")",
			item->GetPath().c_str(),
			titleID,
			item->GetLabel().c_str(),
			0,
			lastAccessed.QuadPart,
			iRegion,
			iSize,
			altname.c_str(),
			developer.c_str(),
			publisher.c_str(),
			features_general.c_str(),
			features_online.c_str(),
			esrb.c_str(),
			esrb_descriptors.c_str(),
			genre.c_str(),
			release_date.c_str(),
			year.c_str(),
			rating.c_str(),
			platform.c_str(),
			exclusive.c_str(),
			title_id.c_str(),
			synopsis.c_str(),
			resources.c_str(),
			preview.c_str(),
			screenshot.c_str(),
			fanart.c_str(),
			last_played.c_str(),
			alt_xbe.c_str()
		);
		m_pDS->exec(strSQL.c_str());
		item->m_dwSize = iSize;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "CProgramDatabase::AddProgramInfo(%s) failed", item->GetPath().c_str());
		return false;
	}
	return true;
}

FILETIME CProgramDatabase::TimeStampToLocalTime( unsigned __int64 timeStamp )
{
	FILETIME fileTime;
	::FileTimeToLocalFileTime( (const FILETIME *)&timeStamp, &fileTime);

	SYSTEMTIME st;
	FileTimeToSystemTime(&fileTime, &st);
	CLog::Log(LOGDEBUG, "Local SYSTEMTIME: %04d/%02d/%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	
	return fileTime;
}

bool CProgramDatabase::IncTimesPlayed(const CStdString& strFileName)
{
	try
	{
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;

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
		if (NULL == m_pDB.get() || NULL == m_pDS.get()) return false;
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