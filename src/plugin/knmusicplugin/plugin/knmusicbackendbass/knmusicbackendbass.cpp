/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QApplication>
#include <QDir>

#ifdef Q_OS_WIN64
#include "basswasapi.h"
#endif

#include "knconfigure.h"
#include "knglobal.h"

#include "knmusicbackendbassthread.h"

#include "knmusicbackendbass.h"

#include <QDebug>

KNMusicBackendBass::KNMusicBackendBass(QObject *parent) :
    KNMusicStandardBackend(parent),
    m_pluginList(QList<HPLUGIN>()),
    m_playbackConfigure(knGlobal->systemConfigure()->getConfigure("Backend"))
  #ifdef Q_OS_WIN64
  ,
    m_wasapiOutputDevice(-1),
    m_wasapiFlag(BASS_WASAPI_AUTOFORMAT | BASS_WASAPI_BUFFER |
                 BASS_WASAPI_EXCLUSIVE),
    m_wasapiEnabled(false)
  #endif
{
    //Initial a empty thread flags.
    DWORD threadFlag=0;
    //Initial the bass library.
    if(!initialBass(threadFlag))
    {
        return;
    }
    //Load plugins from application resource dir and global plugin dir.
#ifdef Q_OS_MACX
    //This is a special OS, on Mac OS X, all the bass related dylib should be
    //placed at the same path of the executable file.
    loadPlugin(qApp->applicationDirPath());
#else
    //Load the user plugins.
    loadPlugin(knGlobal->dirPath(KNGlobal::PluginDir) + "/Bass");
    //Load the plugins in the application folder.
    loadPlugin(knGlobal->dirPath(KNGlobal::ResourceDir) + "/Plugins/Bass");
#endif
    //Initial the main and preview thread.
    setMainThread(generateThread(threadFlag));
    setPreviewThread(generateThread(threadFlag));
}

KNMusicBackendBass::~KNMusicBackendBass()
{
    //Free all the plugin to recover the memory.
    while(!m_pluginList.isEmpty())
    {
        BASS_PluginFree(m_pluginList.takeFirst());
    }
    //Close the bass.
    BASS_Free();
}

int KNMusicBackendBass::volume() const
{
    return BASS_GetConfig(BASS_CONFIG_GVOL_STREAM);
}

int KNMusicBackendBass::minimalVolume() const
{
    return 0;
}

int KNMusicBackendBass::maximumVolume() const
{
    return 10000;
}

qint64 KNMusicBackendBass::previewDuration() const
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    //Disable the preview thread duration fetch.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Return an invalid value.
        return -1;
    }
#endif
    return KNMusicStandardBackend::previewDuration();
}

qint64 KNMusicBackendBass::previewPosition() const
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    //Disable the preview thread position.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Return an invalid value.
        return -1;
    }
#endif
    return KNMusicStandardBackend::previewPosition();
}

bool KNMusicBackendBass::previewLoadMusic(const QString &filePath,
                                          const qint64 &start,
                                          const qint64 &duration)
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    //Check the WASAPI flag.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Failed to load music for exclusive mode.
        return false;
    }
#endif
    //Do original event.
    return KNMusicStandardBackend::previewLoadMusic(filePath, start, duration);
}

int KNMusicBackendBass::previewState() const
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    //Check the WASAPI flag.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Failed to fetch the playing state when enabled exclusive mode.
        return Stopped;
    }
#endif
    //Do orignal event.
    return KNMusicStandardBackend::previewState();
}

void KNMusicBackendBass::previewPlay()
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Do nothing.
        return;
    }
#endif
    //Do orignal event.
    KNMusicStandardBackend::previewPlay();
}

void KNMusicBackendBass::previewPause()
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Do nothing.
        return;
    }
#endif
    //Do orignal event.
    KNMusicStandardBackend::previewPause();
}

void KNMusicBackendBass::previewStop()
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Do nothing.
        return;
    }
#endif
    //Do orignal event.
    KNMusicStandardBackend::previewStop();
}

void KNMusicBackendBass::previewReset()
{
#ifdef Q_OS_WIN64
    //When exclusive, only main thread would working.
    if(m_wasapiEnabled && (m_wasapiFlag & BASS_WASAPI_EXCLUSIVE))
    {
        //Do nothing.
        return;
    }
#endif
    //Do orignal event.
    KNMusicStandardBackend::previewReset();
}

void KNMusicBackendBass::setGlobalVolume(const int &volume)
{
#ifdef Q_OS_WIN64
    if(m_wasapiEnabled)
    {
        //Use the WASAPI function to change the volume.
        BASS_WASAPI_SetVolume(BASS_WASAPI_CURVE_WINDOWS,
                              ((float)volume)/10000.0);
    }
    else
    {
#endif
        //Change the global volume size of the bass.
        BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, volume);
#ifdef Q_OS_WIN64
    }
#endif
    //Emit the volume changed signal.
    emit volumeChanged(volume);
}

int KNMusicBackendBass::volumeLevel() const
{
    return 1000;
}

qreal KNMusicBackendBass::smartVolumeScale() const
{
    return 0.125;
}

inline bool KNMusicBackendBass::initialBass(DWORD &channelFlags)
{
    //Detect operating system version and enable option for using WASAPI.
#ifdef Q_OS_WIN64
    m_wasapiEnabled=m_playbackConfigure->data("WASAPI", false).toBool();
#endif
    //Check the bass library version first.
    if(HIWORD(BASS_GetVersion()) > BASSVERSION)
    {
        //Failed to load a higher version bass library.
        return false;
    }
    //Enabled float digital signal processing.
    //DON'T MOVE THIS, this should config before bass init.
    BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
    //Get the setting sample rate.
    int userSampleRate=m_playbackConfigure->data("SampleRate", 44100).toInt();
#ifdef Q_OS_WIN64
    if(m_wasapiEnabled)
    {
        //For 64-bit Windows, we will enable WASAPI as the playing API instead
        //of using DirectX.
        //Find the output device.
        BASS_WASAPI_DEVICEINFO deviceInfo;
        for(int device=0;
            BASS_WASAPI_GetDeviceInfo(device, &deviceInfo);
            ++device)
        {
            //Check the device flag.
            if((deviceInfo.flags & (BASS_DEVICE_DEFAULT |
                                    BASS_DEVICE_LOOPBACK |
                                    BASS_DEVICE_INPUT))==BASS_DEVICE_DEFAULT)
            {
                //Save the default device index.
                m_wasapiOutputDevice=device;
                break;
            }
        }
        // Check the device index.
        if (m_wasapiOutputDevice==-1)
        {
            //Failed to find the output device.
            return false;
        }
        //Because we won't playing anything via BASS, so don't need an update
        //thread.
        BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 0);
        //Setup BASS - "no sound" device with the "mix" sample rate (default for
        //MOD music)
        BASS_Init(0, deviceInfo.mixfreq, 0, 0, NULL);
    }
    else
    {
#endif
        //Normal bass initialize.
        //Prepare the bass initial flag.
        DWORD initFlag=BASS_DEVICE_FREQ;
        //Check the preference setting.
        if(m_playbackConfigure->data("Stero", false).toBool())
        {
            //Add stereo flag.
            initFlag |= BASS_DEVICE_STEREO;
        }
        //Initial bass library.
        if(!BASS_Init(-1, userSampleRate, initFlag, NULL, NULL))
        {
            //Failed to initial the library bass.
            return false;
        }
#ifdef Q_OS_WIN64
    }
#endif
    //Clear the channel flags.
    channelFlags=0;
    //Check float dsp supporting.
    DWORD fdpsCheck=BASS_StreamCreate(userSampleRate,
                                      2, BASS_SAMPLE_FLOAT, NULL, 0);
    //If support the float dsp,
    if(fdpsCheck)
    {
        //Free the check channel, recover the memory.
        BASS_StreamFree(fdpsCheck);
        //Set fdps support flag.
        channelFlags |= BASS_SAMPLE_FLOAT;
    }
    //Load complete.
    return true;
}

inline void KNMusicBackendBass::loadPlugin(const QString &dirPath)
{
    //Get the dynamic linked library suffix name.
    QString dylibSuffix;
#ifdef Q_OS_WIN
    dylibSuffix=QString("dll");
#endif
#ifdef Q_OS_MACX
    dylibSuffix=QString("dylib");
#endif
#ifdef Q_OS_LINUX
    dylibSuffix=QString("so");
#endif
    //Check if we have already get the suffix for current OS.
    if(dylibSuffix.isEmpty())
    {
        //Ignore the load request if we cannot get the suffix.
        return;
    }
    //Get the plugin directory.
    QDir pluginDir(dirPath);
    //Check the diretory existence.
    if(!pluginDir.exists())
    {
        return;
    }
    //Get all the file from the plugin dir.
    QFileInfoList pluginList=pluginDir.entryInfoList();
    //Check all the file.
    for(auto i=pluginList.constBegin(); i!=pluginList.constEnd(); ++i)
    {
        //If the item is file, and the suffix is dynamic linked library, then
        //we have to tried to load the plugin.
        if((*i).isFile() && (*i).suffix().toLower()==dylibSuffix)
        {
            //Load the plugin.
            HPLUGIN plugin=
            //For windows, we have to use stdWString.
#ifdef Q_OS_WIN
            BASS_PluginLoad((*i).absoluteFilePath().toStdWString().data(), 0);
#endif
#ifdef Q_OS_UNIX
            BASS_PluginLoad((*i).absoluteFilePath().toStdString().data(), 0);
#endif
            //Add the plugin to the plugin list.
            m_pluginList.append(plugin);
        }
    }
}

KNMusicBackendBassThread *KNMusicBackendBass::generateThread(
        const DWORD &channelFlags)
{
    //Generate a thread.
    KNMusicBackendBassThread *thread=new KNMusicBackendBassThread(this);
    //Set the channel create flag to the thread.
    thread->setCreateFlags(channelFlags);
#ifdef Q_OS_WIN64
    //Set the WASAPI parameters to the thread.
    thread->setWasapiData(m_wasapiEnabled, m_wasapiOutputDevice, m_wasapiFlag);
#endif
    //Give back the thread.
    return thread;
}
