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

#include <limits.h>

#include "utils/log.h"
#include "settings/AdvancedSettings.h"
#include "Application.h"
#include "network/DNSNameCache.h"
#include "FileSystem/File.h"
#include "utils/LangCodeExpander.h"
#include "LangInfo.h"
#include "utils/URIUtils.h"
#include "settings/Settings.h"

using namespace XFILE;

CAdvancedSettings g_advancedSettings;

CAdvancedSettings::CAdvancedSettings()
{
  m_DisableModChipDetection = true;
  m_bPowerSave = true;

  m_audioHeadRoom = 0;
  m_ac3Gain = 12.0f;
  m_audioApplyDrc = true;
  m_dvdplayerIgnoreDTSinWAV = false;

  m_audioPlayCountMinimumPercent = 90.0f;

  m_videoSubsDelayRange = 10;
  m_videoAudioDelayRange = 10;
  m_videoSmallStepBackSeconds = 7;
  m_videoSmallStepBackTries = 3;
  m_videoSmallStepBackDelay = 300;
  m_videoUseTimeSeeking = true;
  m_videoTimeSeekForward = 30;
  m_videoTimeSeekBackward = -30;
  m_videoTimeSeekForwardBig = 600;
  m_videoTimeSeekBackwardBig = -600;
  m_videoPercentSeekForward = 2;
  m_videoPercentSeekBackward = -2;
  m_videoPercentSeekForwardBig = 10;
  m_videoPercentSeekBackwardBig = -10;
  m_videoBlackBarColour = 0;
  m_videoPPFFmpegDeint = "linblenddeint";
  m_videoPPFFmpegPostProc = "hb:a,vb:a,dr:a";
  m_videoIgnoreSecondsAtStart = 3*60;
  m_videoIgnorePercentAtEnd   = 8.0f;
  m_videoPlayCountMinimumPercent = 90.0f;

  m_musicUseTimeSeeking = true;
  m_musicTimeSeekForward = 10;
  m_musicTimeSeekBackward = -10;
  m_musicTimeSeekForwardBig = 60;
  m_musicTimeSeekBackwardBig = -60;
  m_musicPercentSeekForward = 1;
  m_musicPercentSeekBackward = -1;
  m_musicPercentSeekForwardBig = 10;
  m_musicPercentSeekBackwardBig = -10;
  m_musicResample = 48000;

  m_cacheMemBufferSize = 1024 * 1024;

  m_slideshowPanAmount = 2.5f;
  m_slideshowZoomAmount = 5.0f;
  m_slideshowBlackBarCompensation = 20.0f;

  m_lcdRows = 4;
  m_lcdColumns = 20;
  m_lcdAddress1 = 0;
  m_lcdAddress2 = 0x40;
  m_lcdAddress3 = 0x14;
  m_lcdAddress4 = 0x54;

  m_autoDetectPingTime = 30;

  m_songInfoDuration = 10;
  m_busyDialogDelay = 2000;
  m_logLevel     = LOG_LEVEL_NORMAL;
  m_logLevelHint = LOG_LEVEL_NORMAL;
  m_cddbAddress = "freedb.freedb.org";
  m_usePCDVDROM = false;
  m_fullScreenOnMovieStart = true;
  m_noDVDROM = false;
  m_enableintro = false;
  m_slowscrolling = true;
  m_splashImage = true;
  m_videoPatch = true;
  m_cachePath = "Z:\\";
  m_displayRemoteCodes = false;
  
  // resources images
  m_bannerImage = "";
  m_cdImage = "";
  m_cdposterImage = "";
  m_cdsmallImage = "";
  m_dual3dImage = "";
  m_fanartImage = "";
  m_fanartblurImage = "";
  m_fanartthumbImage = "";
  m_fogImage = "";
  m_iconImage = "";
  m_opencaseImage = "";
  m_posterImage = "";
  m_postersmallImage = "";
  m_synopsisImage = "";
  m_thumbImage = "";
  
  // wallpapers
  m_wallpaperImage = "";

  m_videoCleanDateTimeRegExp = "(.*[^ _\\,\\.\\(\\)\\[\\]\\-])[ _\\.\\(\\)\\[\\]\\-]+(19[0-9][0-9]|20[0-1][0-9])([ _\\,\\.\\(\\)\\[\\]\\-]|[^0-9]$)";

  m_videoCleanStringRegExps.push_back("[ _\\,\\.\\(\\)\\[\\]\\-](ac3|dts|custom|dc|remastered|divx|divx5|dsr|dsrip|dutch|dvd|dvd5|dvd9|dvdrip|dvdscr|dvdscreener|screener|dvdivx|cam|fragment|fs|hdtv|hdrip|hdtvrip|internal|limited|multisubs|ntsc|ogg|ogm|pal|pdtv|proper|repack|rerip|retail|r3|r5|bd5|se|svcd|swedish|german|read.nfo|nfofix|unrated|extended|ws|telesync|ts|telecine|tc|brrip|bdrip|480p|480i|576p|576i|720p|720i|1080p|1080i|hrhd|hrhdtv|hddvd|bluray|x264|h264|xvid|xvidvd|xxx|www.www|cd[1-9]|\\[.*\\])([ _\\,\\.\\(\\)\\[\\]\\-]|$)");
  m_videoCleanStringRegExps.push_back("(\\[.*\\])");

  m_moviesExcludeFromScanRegExps.push_back("-trailer");
  m_moviesExcludeFromScanRegExps.push_back("[-._ \\\\/]sample[-._ \\\\/]");
  m_tvshowExcludeFromScanRegExps.push_back("[-._ \\\\/]sample[-._ \\\\/]");

  m_videoStackRegExps.push_back("(.*?)([ _.-]*(?:cd|dvd|p(?:(?:ar)?t)|dis[ck]|d)[ _.-]*[0-9]+)(.*?)(\\.[^.]+)$");
  m_videoStackRegExps.push_back("(.*?)([ _.-]*(?:cd|dvd|p(?:(?:ar)?t)|dis[ck]|d)[ _.-]*[a-d])(.*?)(\\.[^.]+)$");
  m_videoStackRegExps.push_back("(.*?)([ ._-]*[a-d])(.*?)(\\.[^.]+)$");
  // This one is a bit too greedy to enable by default.  It will stack sequels
  // in a flat dir structure, but is perfectly safe in a dir-per-vid one.
  //m_videoStackRegExps.push_back("(.*?)([ ._-]*[0-9])(.*?)(\\.[^.]+)$");

  // foo.s01.e01, foo.s01_e01, S01E02 foo, S01 - E02
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[Ss]([0-9]+)[][ ._-]*[Ee]([0-9]+)([^\\\\/]*)$"));
  // foo.ep01, foo.EP_01
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\._ -]()[Ee][Pp]_?([0-9]+)([^\\\\/]*)$"));
  // foo.yyyy.mm.dd.* (byDate=true)
  m_tvshowStackRegExps.push_back(TVShowRegexp(true,"([0-9]{4})[\\.-]([0-9]{2})[\\.-]([0-9]{2})"));
  // foo.mm.dd.yyyy.* (byDate=true)
  m_tvshowStackRegExps.push_back(TVShowRegexp(true,"([0-9]{2})[\\.-]([0-9]{2})[\\.-]([0-9]{4})"));
  // foo.1x09* or just /1x09*
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\\\/\\._ \\[\\(-]([0-9]+)x([0-9]+)([^\\\\/]*)$"));
  // foo.103*, 103 foo
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\\\/\\._ -]([0-9]+)([0-9][0-9])([\\._ -][^\\\\/]*)$"));
  // Part I, Pt.VI
  m_tvshowStackRegExps.push_back(TVShowRegexp(false,"[\\/._ -]p(?:ar)?t[_. -]()([ivx]+)([._ -][^\\/]*)$"));

  m_tvshowMultiPartStackRegExp = "^[-_EeXx]+([0-9]+)";

  m_remoteRepeat = 480;
  m_controllerDeadzone = 0.2f;
  m_FTPShowCache = false;

  m_playlistAsFolders = true;
  m_detectAsUdf = false;

  m_thumbSize = 192;
  m_fanartHeight = 480;
  m_useddsfanart = false;

  m_sambaclienttimeout = 10;
  m_sambadoscodepage = "";
  m_sambastatfiles = true;

  m_bHTTPDirectoryStatFilesize = false;

  m_bFTPThumbs = false;

  m_musicThumbs = "folder.jpg|Folder.jpg|folder.JPG|Folder.JPG|cover.jpg|Cover.jpg|cover.jpeg|thumb.jpg|Thumb.jpg|thumb.JPG|Thumb.JPG";
  m_dvdThumbs = "folder.jpg|Folder.jpg|folder.JPG|Folder.JPG";
  m_fanartImages = "fanart.jpg|fanart.png";

  m_bMusicLibraryHideAllItems = false;
  m_bMusicLibraryAllItemsOnBottom = false;
  m_bMusicLibraryAlbumsSortByArtistThenYear = false;
  m_iMusicLibraryRecentlyAddedItems = 25;
  m_strMusicLibraryAlbumFormat = "";
  m_strMusicLibraryAlbumFormatRight = "";
  m_prioritiseAPEv2tags = false;
  m_musicItemSeparator = " / ";
  m_videoItemSeparator = " / ";

  m_bVideoLibraryHideAllItems = false;
  m_bVideoLibraryAllItemsOnBottom = false;
  m_iVideoLibraryRecentlyAddedItems = 25;
  m_bVideoLibraryHideRecentlyAddedItems = false;
  m_bVideoLibraryHideEmptySeries = false;
  m_bVideoLibraryCleanOnUpdate = false;
  m_bVideoLibraryExportAutoThumbs = false;
  m_bVideoLibraryMyMoviesCategoriesToGenres = false;
  m_bVideoLibraryImportWatchedState = false;
  m_bVideoScannerIgnoreErrors = false;

  m_iTuxBoxStreamtsPort = 31339;
  m_bTuxBoxAudioChannelSelection = false;
  m_bTuxBoxSubMenuSelection = false;
  m_bTuxBoxPictureIcon= true;
  m_bTuxBoxSendAllAPids= false;
  m_iTuxBoxEpgRequestTime = 10; //seconds
  m_iTuxBoxDefaultSubMenu = 4;
  m_iTuxBoxDefaultRootMenu = 0; //default TV Mode
  m_iTuxBoxZapWaitTime = 0; // Time in sec. Default 0:OFF
  m_bTuxBoxZapstream = true;
  m_iTuxBoxZapstreamPort = 31344;

  m_iMythMovieLength = 0; // 0 == Off

  m_bEdlMergeShortCommBreaks = false;      // Off by default
  m_iEdlMaxCommBreakLength = 8 * 30 + 10;  // Just over 8 * 30 second commercial break.
  m_iEdlMinCommBreakLength = 3 * 30;       // 3 * 30 second commercial breaks.
  m_iEdlMaxCommBreakGap = 4 * 30;          // 4 * 30 second commercial breaks.
  m_iEdlMaxStartGap = 5 * 60;              // 5 minutes.
  m_iEdlCommBreakAutowait = 0;             // Off by default
  m_iEdlCommBreakAutowind = 0;             // Off by default

  m_curlconnecttimeout = 10;
  m_curllowspeedtime = 20;
  m_curlretries = 2;

  m_playlistRetries = 100;
  m_playlistTimeout = 20; // 20 seconds timeout
  m_bVirtualShares = true;
  m_bNavVKeyboard = false;

  m_bPythonVerbose = false;

  m_bgInfoLoaderMaxThreads = 3;
}

bool CAdvancedSettings::Load()
{
  // NOTE: This routine should NOT set the default of any of these parameters
  //       it should instead use the versions of GetString/Integer/Float that
  //       don't take defaults in.  Defaults are set in the constructor above
  CStdString advancedSettingsXML;
  advancedSettingsXML  = g_settings.GetUserDataItem("advancedsettings.xml");
  TiXmlDocument advancedXML;
  if (!CFile::Exists(advancedSettingsXML))
  { // tell the user it doesn't exist
    CLog::Log(LOGNOTICE, "No advancedsettings.xml to load (%s)", advancedSettingsXML.c_str());
    return false;
  }

  if (!advancedXML.LoadFile(advancedSettingsXML))
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", advancedSettingsXML.c_str(), advancedXML.ErrorRow(), advancedXML.ErrorDesc());
    return false;
  }

  TiXmlElement *pRootElement = advancedXML.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"advancedsettings") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, no <advancedsettings> node", advancedSettingsXML.c_str());
    return false;
  }

  // succeeded - tell the user it worked
  CLog::Log(LOGNOTICE, "Loaded advancedsettings.xml from %s", advancedSettingsXML.c_str());

  // Dump contents of AS.xml to debug log
  TiXmlPrinter printer;
  printer.SetLineBreak("\n");
  printer.SetIndent("  ");
  advancedXML.Accept(&printer);
  CLog::Log(LOGNOTICE, "Contents of %s are...\n%s", advancedSettingsXML.c_str(), printer.CStr());

  TiXmlElement *pElement = pRootElement->FirstChildElement("audio");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "ac3downmixgain", m_ac3Gain, -96.0f, 96.0f);
    XMLUtils::GetInt(pElement, "headroom", m_audioHeadRoom, 0, 12);
    XMLUtils::GetFloat(pElement, "karaokesyncdelay", m_karaokeSyncDelay, -3.0f, 3.0f);
    // 101 on purpose - can be used to never automark as watched
    XMLUtils::GetFloat(pElement, "playcountminimumpercent", m_audioPlayCountMinimumPercent, 0.0f, 101.0f);

    XMLUtils::GetBoolean(pElement, "usetimeseeking", m_musicUseTimeSeeking);
    XMLUtils::GetInt(pElement, "timeseekforward", m_musicTimeSeekForward, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackward", m_musicTimeSeekBackward, -6000, 0);
    XMLUtils::GetInt(pElement, "timeseekforwardbig", m_musicTimeSeekForwardBig, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackwardbig", m_musicTimeSeekBackwardBig, -6000, 0);

    XMLUtils::GetInt(pElement, "percentseekforward", m_musicPercentSeekForward, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackward", m_musicPercentSeekBackward, -100, 0);
    XMLUtils::GetInt(pElement, "percentseekforwardbig", m_musicPercentSeekForwardBig, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackwardbig", m_musicPercentSeekBackwardBig, -100, 0);

    XMLUtils::GetInt(pElement, "resample", m_musicResample, 0, 48000);

    TiXmlElement* pAudioExcludes = pElement->FirstChildElement("excludefromlisting");
    if (pAudioExcludes)
      GetCustomRegexps(pAudioExcludes, m_audioExcludeFromListingRegExps);

    pAudioExcludes = pElement->FirstChildElement("excludefromscan");
    if (pAudioExcludes)
      GetCustomRegexps(pAudioExcludes, m_audioExcludeFromScanRegExps);

    XMLUtils::GetBoolean(pElement, "applydrc", m_audioApplyDrc);
    XMLUtils::GetBoolean(pElement, "dvdplayerignoredtsinwav", m_dvdplayerIgnoreDTSinWAV);
  }

  pElement = pRootElement->FirstChildElement("video");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "subsdelayrange", m_videoSubsDelayRange, 10, 600);
    XMLUtils::GetFloat(pElement, "audiodelayrange", m_videoAudioDelayRange, 10, 600);
    XMLUtils::GetInt(pElement, "blackbarcolour", m_videoBlackBarColour, 0, 255);
    XMLUtils::GetBoolean(pElement, "fullscreenonmoviestart", m_fullScreenOnMovieStart);
    // 101 on purpose - can be used to never automark as watched
    XMLUtils::GetFloat(pElement, "playcountminimumpercent", m_videoPlayCountMinimumPercent, 0.0f, 101.0f);
    XMLUtils::GetInt(pElement, "ignoresecondsatstart", m_videoIgnoreSecondsAtStart, 0, 900);
    XMLUtils::GetFloat(pElement, "ignorepercentatend", m_videoIgnorePercentAtEnd, 0, 100.0f);

    XMLUtils::GetInt(pElement, "smallstepbackseconds", m_videoSmallStepBackSeconds, 1, INT_MAX);
    XMLUtils::GetInt(pElement, "smallstepbacktries", m_videoSmallStepBackTries, 1, 10);
    XMLUtils::GetInt(pElement, "smallstepbackdelay", m_videoSmallStepBackDelay, 100, 5000); //MS

    XMLUtils::GetBoolean(pElement, "usetimeseeking", m_videoUseTimeSeeking);
    XMLUtils::GetInt(pElement, "timeseekforward", m_videoTimeSeekForward, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackward", m_videoTimeSeekBackward, -6000, 0);
    XMLUtils::GetInt(pElement, "timeseekforwardbig", m_videoTimeSeekForwardBig, 0, 6000);
    XMLUtils::GetInt(pElement, "timeseekbackwardbig", m_videoTimeSeekBackwardBig, -6000, 0);

    XMLUtils::GetInt(pElement, "percentseekforward", m_videoPercentSeekForward, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackward", m_videoPercentSeekBackward, -100, 0);
    XMLUtils::GetInt(pElement, "percentseekforwardbig", m_videoPercentSeekForwardBig, 0, 100);
    XMLUtils::GetInt(pElement, "percentseekbackwardbig", m_videoPercentSeekBackwardBig, -100, 0);

    TiXmlElement* pVideoExcludes = pElement->FirstChildElement("excludefromlisting");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_videoExcludeFromListingRegExps);

    pVideoExcludes = pElement->FirstChildElement("excludefromscan");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_moviesExcludeFromScanRegExps);

    pVideoExcludes = pElement->FirstChildElement("excludetvshowsfromscan");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_tvshowExcludeFromScanRegExps);

    pVideoExcludes = pElement->FirstChildElement("cleanstrings");
    if (pVideoExcludes)
      GetCustomRegexps(pVideoExcludes, m_videoCleanStringRegExps);

    XMLUtils::GetString(pElement,"cleandatetime", m_videoCleanDateTimeRegExp);
    XMLUtils::GetString(pElement,"ppffmpegdeinterlacing",m_videoPPFFmpegDeint);
    XMLUtils::GetString(pElement,"ppffmpegpostprocessing",m_videoPPFFmpegPostProc);
  }

  pElement = pRootElement->FirstChildElement("musiclibrary");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "hideallitems", m_bMusicLibraryHideAllItems);
    XMLUtils::GetInt(pElement, "recentlyaddeditems", m_iMusicLibraryRecentlyAddedItems, 1, INT_MAX);
    XMLUtils::GetBoolean(pElement, "prioritiseapetags", m_prioritiseAPEv2tags);
    XMLUtils::GetBoolean(pElement, "allitemsonbottom", m_bMusicLibraryAllItemsOnBottom);
    XMLUtils::GetBoolean(pElement, "albumssortbyartistthenyear", m_bMusicLibraryAlbumsSortByArtistThenYear);
    XMLUtils::GetString(pElement, "albumformat", m_strMusicLibraryAlbumFormat);
    XMLUtils::GetString(pElement, "albumformatright", m_strMusicLibraryAlbumFormatRight);
    XMLUtils::GetString(pElement, "itemseparator", m_musicItemSeparator);
  }

  pElement = pRootElement->FirstChildElement("videolibrary");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "hideallitems", m_bVideoLibraryHideAllItems);
    XMLUtils::GetBoolean(pElement, "allitemsonbottom", m_bVideoLibraryAllItemsOnBottom);
    XMLUtils::GetInt(pElement, "recentlyaddeditems", m_iVideoLibraryRecentlyAddedItems, 1, INT_MAX);
    XMLUtils::GetBoolean(pElement, "hiderecentlyaddeditems", m_bVideoLibraryHideRecentlyAddedItems);
    XMLUtils::GetBoolean(pElement, "hideemptyseries", m_bVideoLibraryHideEmptySeries);
    XMLUtils::GetBoolean(pElement, "cleanonupdate", m_bVideoLibraryCleanOnUpdate);
    XMLUtils::GetString(pElement, "itemseparator", m_videoItemSeparator);
    XMLUtils::GetBoolean(pElement, "exportautothumbs", m_bVideoLibraryExportAutoThumbs);
    XMLUtils::GetBoolean(pElement, "importwatchedstate", m_bVideoLibraryImportWatchedState);

    TiXmlElement* pMyMovies = pElement->FirstChildElement("mymovies");
    if (pMyMovies)
      XMLUtils::GetBoolean(pMyMovies, "categoriestogenres", m_bVideoLibraryMyMoviesCategoriesToGenres);
  }

  pElement = pRootElement->FirstChildElement("videoscanner");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "ignoreerrors", m_bVideoScannerIgnoreErrors);
  }

  pElement = pRootElement->FirstChildElement("slideshow");
  if (pElement)
  {
    XMLUtils::GetFloat(pElement, "panamount", m_slideshowPanAmount, 0.0f, 20.0f);
    XMLUtils::GetFloat(pElement, "zoomamount", m_slideshowZoomAmount, 0.0f, 20.0f);
    XMLUtils::GetFloat(pElement, "blackbarcompensation", m_slideshowBlackBarCompensation, 0.0f, 50.0f);
  }

  pElement = pRootElement->FirstChildElement("lcd");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "rows", m_lcdRows, 1, 4);
    XMLUtils::GetInt(pElement, "columns", m_lcdColumns, 1, 40);
    XMLUtils::GetInt(pElement, "address1", m_lcdAddress1, 0, 0x100);
    XMLUtils::GetInt(pElement, "address2", m_lcdAddress2, 0, 0x100);
    XMLUtils::GetInt(pElement, "address3", m_lcdAddress3, 0, 0x100);
    XMLUtils::GetInt(pElement, "address4", m_lcdAddress4, 0, 0x100);
  }

  pElement = pRootElement->FirstChildElement("network");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "autodetectpingtime", m_autoDetectPingTime, 1, 240);
    XMLUtils::GetInt(pElement, "curlclienttimeout", m_curlconnecttimeout, 1, 1000);
    XMLUtils::GetInt(pElement, "curllowspeedtime", m_curllowspeedtime, 1, 1000);
    XMLUtils::GetInt(pElement, "curlretries", m_curlretries, 0, 10);
    XMLUtils::GetUInt(pElement, "cachemembuffersize", m_cacheMemBufferSize);
  }

  pElement = pRootElement->FirstChildElement("samba");
  if (pElement)
  {
    XMLUtils::GetString(pElement,  "doscodepage",   m_sambadoscodepage);
    XMLUtils::GetInt(pElement, "clienttimeout", m_sambaclienttimeout, 5, 100);
    XMLUtils::GetBoolean(pElement, "statfiles", m_sambastatfiles);
  }

  pElement = pRootElement->FirstChildElement("httpdirectory");
  if (pElement)
    XMLUtils::GetBoolean(pElement, "statfilesize", m_bHTTPDirectoryStatFilesize);

  pElement = pRootElement->FirstChildElement("ftp");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "remotethumbs", m_bFTPThumbs);
  }

  pElement = pRootElement->FirstChildElement("loglevel");
  if (pElement)
  { // read the loglevel setting, so set the setting advanced to hide it in GUI
    // as altering it will do nothing - we don't write to advancedsettings.xml
    XMLUtils::GetInt(pRootElement, "loglevel", m_logLevelHint, LOG_LEVEL_NONE, LOG_LEVEL_MAX);
    CSettingBool *setting = (CSettingBool *)g_guiSettings.GetSetting("debug.showloginfo");
    if (setting)
    {
      const char* hide;
      if (!((hide = pElement->Attribute("hide")) && strnicmp("false", hide, 4) == 0))
        setting->SetAdvanced();
    }
	if (m_logLevelHint == -1) 
	{
		CLog::SetLogLevel(m_logLevelHint);
	}
	else
	{
		g_advancedSettings.m_logLevel = std::max(g_advancedSettings.m_logLevel, g_advancedSettings.m_logLevelHint);
		CLog::SetLogLevel(g_advancedSettings.m_logLevel);
	}
  }

  pElement = pRootElement->FirstChildElement("python");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "verbose", m_bPythonVerbose);
  }

  XMLUtils::GetString(pRootElement, "cddbaddress", m_cddbAddress);

  XMLUtils::GetBoolean(pRootElement, "usepcdvdrom", m_usePCDVDROM);
  XMLUtils::GetBoolean(pRootElement, "nodvdrom", m_noDVDROM);
  XMLUtils::GetBoolean(pRootElement, "enableintro", m_enableintro);
  XMLUtils::GetBoolean(pRootElement, "slowscrolling", m_slowscrolling);
  XMLUtils::GetBoolean(pRootElement, "splash", m_splashImage);
  XMLUtils::GetBoolean(pRootElement, "removekernelpatch", m_videoPatch);
  XMLUtils::GetBoolean(pRootElement, "disablemodchipdetection", m_DisableModChipDetection);
  XMLUtils::GetBoolean(pRootElement, "powersave", m_bPowerSave);
  
  pElement = pRootElement->FirstChildElement("images");
  if (pElement)
  { 
    // Get wallpaper value
    XMLUtils::GetString(pElement, "wallpaper", m_wallpaperImage);
	
	MEMORYSTATUS status;
	GlobalMemoryStatus( &status );
	if (status.dwTotalPhys > 67108864)
		XMLUtils::GetString(pElement, "wallpaper_128mb", m_wallpaperImage);

    // Get <_resources> values
    pElement = pElement->FirstChildElement("_resources");
    if (pElement)
	{
        XMLUtils::GetString(pElement, "banner", m_bannerImage);
        XMLUtils::GetString(pElement, "cd", m_cdImage);
        XMLUtils::GetString(pElement, "cd_small", m_cdsmallImage);
        XMLUtils::GetString(pElement, "cdposter", m_cdposterImage);
        XMLUtils::GetString(pElement, "dual3d", m_dual3dImage);
        XMLUtils::GetString(pElement, "fanart", m_fanartImage);
        XMLUtils::GetString(pElement, "fanart_blur", m_fanartblurImage);
        XMLUtils::GetString(pElement, "fanart_thumb", m_fanartthumbImage);
        XMLUtils::GetString(pElement, "fog", m_fogImage);
        XMLUtils::GetString(pElement, "icon", m_iconImage);
        XMLUtils::GetString(pElement, "opencase", m_opencaseImage);
        XMLUtils::GetString(pElement, "poster", m_posterImage);
        XMLUtils::GetString(pElement, "poster_small", m_postersmallImage);
        XMLUtils::GetString(pElement, "synopsis", m_synopsisImage);
        XMLUtils::GetString(pElement, "thumb", m_thumbImage);
    }
  }

  XMLUtils::GetInt(pRootElement, "songinfoduration", m_songInfoDuration, 0, INT_MAX);
  XMLUtils::GetInt(pRootElement, "busydialogdelay", m_busyDialogDelay, 0, 5000);
  XMLUtils::GetInt(pRootElement, "playlistretries", m_playlistRetries, -1, 5000);
  XMLUtils::GetInt(pRootElement, "playlisttimeout", m_playlistTimeout, 0, 5000);

  XMLUtils::GetBoolean(pRootElement,"virtualshares", m_bVirtualShares);
  XMLUtils::GetBoolean(pRootElement,"navigatevirtualkeyboard", m_bNavVKeyboard);

  //Tuxbox
  pElement = pRootElement->FirstChildElement("tuxbox");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "streamtsport", m_iTuxBoxStreamtsPort, 0, 65535);
    XMLUtils::GetBoolean(pElement, "audiochannelselection", m_bTuxBoxAudioChannelSelection);
    XMLUtils::GetBoolean(pElement, "submenuselection", m_bTuxBoxSubMenuSelection);
    XMLUtils::GetBoolean(pElement, "pictureicon", m_bTuxBoxPictureIcon);
    XMLUtils::GetBoolean(pElement, "sendallaudiopids", m_bTuxBoxSendAllAPids);
    XMLUtils::GetInt(pElement, "epgrequesttime", m_iTuxBoxEpgRequestTime, 0, 3600);
    XMLUtils::GetInt(pElement, "defaultsubmenu", m_iTuxBoxDefaultSubMenu, 1, 4);
    XMLUtils::GetInt(pElement, "defaultrootmenu", m_iTuxBoxDefaultRootMenu, 0, 4);
    XMLUtils::GetInt(pElement, "zapwaittime", m_iTuxBoxZapWaitTime, 0, 120);
    XMLUtils::GetBoolean(pElement, "zapstream", m_bTuxBoxZapstream);
    XMLUtils::GetInt(pElement, "zapstreamport", m_iTuxBoxZapstreamPort, 0, 65535);
  }

  // Myth TV
  pElement = pRootElement->FirstChildElement("myth");
  if (pElement)
  {
    XMLUtils::GetInt(pElement, "movielength", m_iMythMovieLength);
  }

  // EDL commercial break handling
  pElement = pRootElement->FirstChildElement("edl");
  if (pElement)
  {
    XMLUtils::GetBoolean(pElement, "mergeshortcommbreaks", m_bEdlMergeShortCommBreaks);
    XMLUtils::GetInt(pElement, "maxcommbreaklength", m_iEdlMaxCommBreakLength, 0, 10 * 60); // Between 0 and 10 minutes
    XMLUtils::GetInt(pElement, "mincommbreaklength", m_iEdlMinCommBreakLength, 0, 5 * 60);  // Between 0 and 5 minutes
    XMLUtils::GetInt(pElement, "maxcommbreakgap", m_iEdlMaxCommBreakGap, 0, 5 * 60);        // Between 0 and 5 minutes.
    XMLUtils::GetInt(pElement, "maxstartgap", m_iEdlMaxStartGap, 0, 10 * 60);               // Between 0 and 10 minutes
    XMLUtils::GetInt(pElement, "commbreakautowait", m_iEdlCommBreakAutowait, 0, 10);        // Between 0 and 10 seconds
    XMLUtils::GetInt(pElement, "commbreakautowind", m_iEdlCommBreakAutowind, 0, 10);        // Between 0 and 10 seconds
  }

  // picture exclude regexps
  TiXmlElement* pPictureExcludes = pRootElement->FirstChildElement("pictureexcludes");
  if (pPictureExcludes)
    GetCustomRegexps(pPictureExcludes, m_pictureExcludeFromListingRegExps);

  // picture extensions
  CStdString extraExtensions;
  TiXmlElement* pExts = pRootElement->FirstChildElement("pictureextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_settings.m_pictureExtensions);

  // music extensions
  pExts = pRootElement->FirstChildElement("musicextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_settings.m_musicExtensions);

  // video extensions
  pExts = pRootElement->FirstChildElement("videoextensions");
  if (pExts)
    GetCustomExtensions(pExts,g_settings.m_videoExtensions);

  m_vecTokens.clear();
  CLangInfo::LoadTokens(pRootElement->FirstChild("sorttokens"),m_vecTokens);

  XMLUtils::GetBoolean(pRootElement, "displayremotecodes", m_displayRemoteCodes);

  // TODO: Should cache path be given in terms of our predefined paths??
  //       Are we even going to have predefined paths??
  CSettings::GetPath(pRootElement, "cachepath", m_cachePath);
  URIUtils::AddSlashAtEnd(m_cachePath);

  XMLUtils::GetBoolean(pRootElement, "ftpshowcache", m_FTPShowCache);

  g_LangCodeExpander.LoadUserCodes(pRootElement->FirstChildElement("languagecodes"));

  // trailer matching regexps
  TiXmlElement* pTrailerMatching = pRootElement->FirstChildElement("trailermatching");
  if (pTrailerMatching)
    GetCustomRegexps(pTrailerMatching, m_trailerMatchRegExps);

  //everything thats a trailer is not a movie
  m_moviesExcludeFromScanRegExps.insert(m_moviesExcludeFromScanRegExps.end(),
                                        m_trailerMatchRegExps.begin(),
                                        m_trailerMatchRegExps.end());

  // stacking regexps
  TiXmlElement* pVideoStacking = pRootElement->FirstChildElement("moviestacking");
  if (pVideoStacking)
    GetCustomRegexps(pVideoStacking, m_videoStackRegExps);

  //tv stacking regexps
  TiXmlElement* pTVStacking = pRootElement->FirstChildElement("tvshowmatching");
  if (pTVStacking)
    GetCustomTVRegexps(pTVStacking, m_tvshowStackRegExps);

  // path substitutions
  TiXmlElement* pPathSubstitution = pRootElement->FirstChildElement("pathsubstitution");
  if (pPathSubstitution)
  {
    m_pathSubstitutions.clear();
    CLog::Log(LOGDEBUG,"Configuring path substitutions");
    TiXmlNode* pSubstitute = pPathSubstitution->FirstChildElement("substitute");
    while (pSubstitute)
    {
      CStdString strFrom, strTo;
      TiXmlNode* pFrom = pSubstitute->FirstChild("from");
      if (pFrom)
        strFrom = pFrom->FirstChild()->Value();
      TiXmlNode* pTo = pSubstitute->FirstChild("to");
      if (pTo)
        strTo = pTo->FirstChild()->Value();

      if (!strFrom.IsEmpty() && !strTo.IsEmpty())
      {
        CLog::Log(LOGDEBUG,"  Registering substition pair:");
        CLog::Log(LOGDEBUG,"    From: [%s]", strFrom.c_str());
        CLog::Log(LOGDEBUG,"    To:   [%s]", strTo.c_str());
        // keep literal commas since we use comma as a seperator
        strFrom.Replace(",",",,");
        strTo.Replace(",",",,");
        m_pathSubstitutions.push_back(strFrom + " , " + strTo);
      }
      else
      {
        // error message about missing tag
        if (strFrom.IsEmpty())
          CLog::Log(LOGERROR,"  Missing <from> tag");
        else
          CLog::Log(LOGERROR,"  Missing <to> tag");
      }

      // get next one
      pSubstitute = pSubstitute->NextSiblingElement("substitute");
    }
  }

  XMLUtils::GetInt(pRootElement, "remoterepeat", m_remoteRepeat, 1, INT_MAX);
  XMLUtils::GetFloat(pRootElement, "controllerdeadzone", m_controllerDeadzone, 0.0f, 1.0f);
  XMLUtils::GetInt(pRootElement, "thumbsize", m_thumbSize, 0, 1024);
  XMLUtils::GetInt(pRootElement, "fanartheight", m_fanartHeight, 0, 1080);
  //dds support
  XMLUtils::GetBoolean(pRootElement, "useddsfanart", m_useddsfanart);


  XMLUtils::GetBoolean(pRootElement, "playlistasfolders", m_playlistAsFolders);
  XMLUtils::GetBoolean(pRootElement, "detectasudf", m_detectAsUdf);

  // music thumbs
  CStdString extraThumbs;
  TiXmlElement* pThumbs = pRootElement->FirstChildElement("musicthumbs");
  if (pThumbs)
  {
    // remove before add so that the defaults can be restored after user defined ones
    // (ie, the list can be:cover.jpg|cover.png|folder.jpg)
    CSettings::GetString(pThumbs, "remove", extraThumbs, "");
    if (extraThumbs != "")
    {
      CStdStringArray thumbs;
      StringUtils::SplitString(extraThumbs, "|", thumbs);
      for (unsigned int i = 0; i < thumbs.size(); ++i)
      {
        int iPos = m_musicThumbs.Find(thumbs[i]);
        if (iPos == -1)
          continue;
        m_musicThumbs.erase(iPos, thumbs[i].size() + 1);
      }
    }
    CSettings::GetString(pThumbs, "add", extraThumbs,"");
    if (extraThumbs != "")
    {
      if (!m_musicThumbs.IsEmpty())
        m_musicThumbs += "|";
      m_musicThumbs += extraThumbs;
    }
  }

  // dvd thumbs
  pThumbs = pRootElement->FirstChildElement("dvdthumbs");
  if (pThumbs)
  {
    // remove before add so that the defaults can be restored after user defined ones
    // (ie, the list can be:cover.jpg|cover.png|folder.jpg)
    CSettings::GetString(pThumbs, "remove", extraThumbs, "");
    if (extraThumbs != "")
    {
      CStdStringArray thumbs;
      StringUtils::SplitString(extraThumbs, "|", thumbs);
      for (unsigned int i = 0; i < thumbs.size(); ++i)
      {
        int iPos = m_dvdThumbs.Find(thumbs[i]);
        if (iPos == -1)
          continue;
        m_dvdThumbs.erase(iPos, thumbs[i].size() + 1);
      }
    }
    CSettings::GetString(pThumbs, "add", extraThumbs,"");
    if (extraThumbs != "")
    {
      if (!m_dvdThumbs.IsEmpty())
        m_dvdThumbs += "|";
      m_dvdThumbs += extraThumbs;
    }
  }

  // movie fanarts
  TiXmlElement* pFanart = pRootElement->FirstChildElement("fanart");
  if (pFanart)
    GetCustomExtensions(pFanart,m_fanartImages);

  // music filename->tag filters
  TiXmlElement* filters = pRootElement->FirstChildElement("musicfilenamefilters");
  if (filters)
  {
    TiXmlNode* filter = filters->FirstChild("filter");
    while (filter)
    {
      if (filter->FirstChild())
        m_musicTagsFromFileFilters.push_back(filter->FirstChild()->ValueStr());
      filter = filter->NextSibling("filter");
    }
  }

  TiXmlElement* pHostEntries = pRootElement->FirstChildElement("hosts");
  if (pHostEntries)
  {
    TiXmlElement* element = pHostEntries->FirstChildElement("entry");
    while(element)
    {
      CStdString name  = element->Attribute("name");
      CStdString value;
      if(element->GetText())
        value = element->GetText();

      if(name.length() > 0 && value.length() > 0)
        CDNSNameCache::Add(name, value);
      element = element->NextSiblingElement("entry");
    }
  }

  XMLUtils::GetInt(pRootElement, "bginfoloadermaxthreads", m_bgInfoLoaderMaxThreads);
  m_bgInfoLoaderMaxThreads = std::max(1, m_bgInfoLoaderMaxThreads);

  // load in the GUISettings overrides:
  g_guiSettings.LoadXML(pRootElement, true);  // true to hide the settings we read in

  return true;
}

void CAdvancedSettings::Clear()
{
  m_videoCleanStringRegExps.clear();
  m_moviesExcludeFromScanRegExps.clear();
  m_tvshowExcludeFromScanRegExps.clear();
  m_videoExcludeFromListingRegExps.clear();
  m_videoStackRegExps.clear();
  m_audioExcludeFromScanRegExps.clear();
  m_audioExcludeFromListingRegExps.clear();
  m_pictureExcludeFromListingRegExps.clear();
}

void CAdvancedSettings::GetCustomTVRegexps(TiXmlElement *pRootElement, SETTINGS_TVSHOWLIST& settings)
{
  int iAction = 0; // overwrite
  // for backward compatibility
  const char* szAppend = pRootElement->Attribute("append");
  if ((szAppend && stricmp(szAppend, "yes") == 0))
    iAction = 1;
  // action takes precedence if both attributes exist
  const char* szAction = pRootElement->Attribute("action");
  if (szAction)
  {
    iAction = 0; // overwrite
    if (stricmp(szAction, "append") == 0)
      iAction = 1; // append
    else if (stricmp(szAction, "prepend") == 0)
      iAction = 2; // prepend
  }
  if (iAction == 0)
    settings.clear();
  TiXmlNode* pRegExp = pRootElement->FirstChild("regexp");
  int i = 0;
  while (pRegExp)
  {
    if (pRegExp->FirstChild())
    {
      bool bByDate = false;
      if (pRegExp->ToElement())
      {
        CStdString byDate = pRegExp->ToElement()->Attribute("bydate");
        if(byDate && stricmp(byDate, "true") == 0)
        {
          bByDate = true;
        }
      }
      CStdString regExp = pRegExp->FirstChild()->Value();
      regExp.MakeLower();
      if (iAction == 2)
        settings.insert(settings.begin() + i++, 1, TVShowRegexp(bByDate,regExp));
      else
        settings.push_back(TVShowRegexp(bByDate,regExp));
    }
    pRegExp = pRegExp->NextSibling("regexp");
  }
}

void CAdvancedSettings::GetCustomRegexps(TiXmlElement *pRootElement, CStdStringArray& settings)
{
  TiXmlElement *pElement = pRootElement;
  while (pElement)
  {
    int iAction = 0; // overwrite
    // for backward compatibility
    const char* szAppend = pElement->Attribute("append");
    if ((szAppend && stricmp(szAppend, "yes") == 0))
      iAction = 1;
    // action takes precedence if both attributes exist
    const char* szAction = pElement->Attribute("action");
    if (szAction)
    {
      iAction = 0; // overwrite
      if (stricmp(szAction, "append") == 0)
        iAction = 1; // append
      else if (stricmp(szAction, "prepend") == 0)
        iAction = 2; // prepend
    }
    if (iAction == 0)
      settings.clear();
    TiXmlNode* pRegExp = pElement->FirstChild("regexp");
    int i = 0;
    while (pRegExp)
    {
      if (pRegExp->FirstChild())
      {
        CStdString regExp = pRegExp->FirstChild()->Value();
        regExp.MakeLower();
        if (iAction == 2)
          settings.insert(settings.begin() + i++, 1, regExp);
        else
          settings.push_back(regExp);
      }
      pRegExp = pRegExp->NextSibling("regexp");
    }

    pElement = pElement->NextSiblingElement(pRootElement->Value());
  }
}

void CAdvancedSettings::GetCustomExtensions(TiXmlElement *pRootElement, CStdString& extensions)
{
  CStdString extraExtensions;
  CSettings::GetString(pRootElement,"add",extraExtensions,"");
  if (extraExtensions != "")
    extensions += "|" + extraExtensions;
  CSettings::GetString(pRootElement,"remove",extraExtensions,"");
  if (extraExtensions != "")
  {
    CStdStringArray exts;
    StringUtils::SplitString(extraExtensions,"|",exts);
    for (unsigned int i=0;i<exts.size();++i)
    {
      int iPos = extensions.Find(exts[i]);
      if (iPos == -1)
        continue;
      extensions.erase(iPos,exts[i].size()+1);
    }
  }
}

void CAdvancedSettings::SetDebugMode(bool debug)
{
  if (debug)
  {
    int level = std::max(m_logLevelHint, LOG_LEVEL_DEBUG_FREEMEM);
    m_logLevel = level;
    CLog::SetLogLevel(level);
    CLog::Log(LOGNOTICE, "Enabled debug logging due to GUI setting. Level %d.", level);
  }
  else
  {
    int level = std::min(m_logLevelHint, LOG_LEVEL_DEBUG/*LOG_LEVEL_NORMAL*/);
    CLog::Log(LOGNOTICE, "Disabled debug logging due to GUI setting. Level %d.", level);
    m_logLevel = level;
    CLog::SetLogLevel(level);
  }
}
