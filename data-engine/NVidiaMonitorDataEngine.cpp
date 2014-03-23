
/*
	NVidia Monitor - plasmoid that displays nvidia gpu's informations
	Copyright (C) 2013 Matthias Devlamynck

	This file is part of NVidia Monitor.

	NVidia Monitor is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NVidia Monitor is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NVidia Monitor. If not, see <http://www.gnu.org/licenses/>.

	To contact me, please send me an email at :
		matthias.devlamynck@mailoo.org

	The source code is aviable at :
		https://github.com/mdevlamynck/nvidia-monitor/

	If you wish to make a fork or maintain this project, please contact me.
*/

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <QString>
#include <QRegExp>

#include "NVidiaMonitorDataEngine.h"

using namespace std;

namespace eng
{

/**********************************************************************************************
 * Inits & Releases
 **********************************************************************************************/

/**
 * DataEngine Constructor
 * Called by the Plasma framework
 */
NVidiaMonitorDataEngine::NVidiaMonitorDataEngine(QObject* in_pParent, const QVariantList& in_args)
	: Plasma::DataEngine(in_pParent, in_args)
    , m_bIsInit			(false)
	, m_pXDisplay		(NULL)
	, m_strXDisplayId	(":0")
{
	setMinimumPollingInterval(1000);
}

/**
 * DataEngine Destructor
 */
NVidiaMonitorDataEngine::~NVidiaMonitorDataEngine()
{
}

/**
 * Init the DataEngine
 * Declare handled values
 */
void NVidiaMonitorDataEngine::init()
{
	m_smSources["temperature"]	= DataSource(&NVidiaMonitorDataEngine::updateTemp);
	m_smSources["frequencies"]	= DataSource(&NVidiaMonitorDataEngine::updateFreqs);
	m_smSources["memory-usage"]	= DataSource(&NVidiaMonitorDataEngine::updateMem);

    m_smSources["memory-usage"].p_dmData["total"] = -1;

	initBumblebee();
    if(!m_bIsBumblebee)
        initGPUConsts();
}

/**
 * Detect either this is a bumblebee setup or a regular one
 */
void NVidiaMonitorDataEngine::initBumblebee()
{
	if(isCgOn() == NotBumblebee)
		m_bIsBumblebee = false;
	else
	{
		m_bIsBumblebee	= true;

		// Retreive display used by Bumblebee
		std::ifstream bbconf("/etc/bumblebee/bumblebee.conf");
		if(bbconf.is_open())
		{
			QRegExp		findLine("^VirtualDisplay=:[0-9]");
			QRegExp		findXDisplay(":[0-9]");
			std::string	line;
			int			pos = 0;

			while( getline(bbconf, line) && (pos = findLine.indexIn(line.c_str(), 0)) == -1	);
			if(pos != -1)
			{
				QString tmp		= findLine.cap(0);
				m_strXDisplayId	= tmp.toUtf8().constData();
				pos = 0;
				findXDisplay.indexIn(tmp, 0);
				tmp				= findXDisplay.cap(0);
				m_strXDisplayId	= tmp.toUtf8().constData();
			}
		}
		bbconf.close();
	}
}

/**
 * Query
 */
void NVidiaMonitorDataEngine::initGPUConsts()
{
    if(!beforeQuery())
        return;

    // Total memory
    int				total	= -1;
	eng::DataMap &	dmMem	= m_smSources["memory-usage"].p_dmData;

	if(!XNVCTRLQueryTargetAttribute (m_pXDisplay, NV_CTRL_TARGET_TYPE_GPU, 0, 0, NV_CTRL_TOTAL_DEDICATED_GPU_MEMORY, &total))
        return;

	dmMem["total"]	= total;

    afterQuery();

    m_bIsInit = true;
}

/**********************************************************************************************
 * Data Handling
 **********************************************************************************************/

/**
 * Get the list of avaible sources provided by this DataEngine
 * \return Avaible sources
 */
QStringList NVidiaMonitorDataEngine::sources() const
{
	QStringList					aAvaible;
	SourceMap::const_iterator	itSources;

	aAvaible << "bumblebee";

	for(itSources = m_smSources.begin(); itSources != m_smSources.end(); itSources++)
	{
		aAvaible << itSources->first;
	}

	return aAvaible;
}

/**
 * Detect if the cg is on when using bumblbee
 * \return Wether the cg is currently on or not
 */
CGState NVidiaMonitorDataEngine::isCgOn()
{
	ifstream bbswitch("/proc/acpi/bbswitch");

	if(bbswitch.is_open())
	{
		std::string strOutput;
		getline(bbswitch, strOutput);
		bbswitch.close();

		if(strOutput.find("ON") != std::string::npos)
		{
            if(!m_bIsInit)
                initGPUConsts();

			setData("bumblebee", "status", "on");
			return On;
		}
		else
		{
			setData("bumblebee", "status", "off");
			return Off;
		}
	}
	else
		return NotBumblebee;
}

/**
 * Update the value(s) of a source
 * Called when an applet asks for the value of a source
 * \param in_qstrName Asked source in_qstrName
 * \return Wether the value will by updated or not
 */
bool NVidiaMonitorDataEngine::sourceRequestEvent(QString const & in_qstrName)
{
	if(in_qstrName == "bumblebee")
	{
		isCgOn();
		return true;
	}

	SourceMap::const_iterator itSources = m_smSources.find(in_qstrName);

	if(itSources == m_smSources.end())
		return false;
	else
	{
		if(!updateSourceEvent(in_qstrName))
			setData(in_qstrName, DataEngine::Data());

		return true;
	}
}

/**
 * Update the value(s) of a source
 * Called when the interval between two updated has expired
 * \param in_qstrName Name of the source to update
 * \return Wether the value will by updated or not
 */
bool NVidiaMonitorDataEngine::updateSourceEvent(QString const & in_qstrName)
{
	if(in_qstrName == "bumblebee")
	{
		isCgOn();
		return true;
	}

	SourceMap::const_iterator itSources = m_smSources.find(in_qstrName);

	if(itSources != m_smSources.end() && itSources->second.p_pdUpdate != NULL)
	{
        if(!beforeQuery())
            return false;

		// Update data
		bool isUpdated = (this->*(itSources->second.p_pdUpdate))();

		// Write data if updated
		if(isUpdated)
		{
			DataMap::const_iterator itData;

			for(itData = itSources->second.p_dmData.begin(); itData != itSources->second.p_dmData.end(); itData++)
				setData(itSources->first, itData->first, itData->second);
		}

        afterQuery();

		return isUpdated;
	}
	else
		return false;
}

/**
 * Update the value of the Temperature source
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::updateTemp()
{
	// Query GPU temperature
	int32_t temp;

	if(!XNVCTRLQueryAttribute (m_pXDisplay, 0, 0, NV_CTRL_GPU_CORE_TEMPERATURE, &temp))
        return false;

	eng::DataMap &	dmTemp	= m_smSources["temperature"].p_dmData;
	dmTemp["temperature"]	= temp;

	return true;
}

/**
 * Update the values of the Frequncies source
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::updateFreqs()
{
	// Query GPU Frequencies
	int32_t level				= -1;
    int16_t graphicMemory[2]	= {-1, -1};
	int32_t processor			= -1;

	bool levelSucceeded		= XNVCTRLQueryAttribute (m_pXDisplay, 0, 0, NV_CTRL_GPU_CURRENT_PERFORMANCE_LEVEL, &level);
	bool graphMemSucceeded	= XNVCTRLQueryAttribute (m_pXDisplay, 0, 0, NV_CTRL_GPU_CURRENT_CLOCK_FREQS, (int32_t*) graphicMemory);
	bool processorSucceeded	= XNVCTRLQueryAttribute (m_pXDisplay, 0, 0, NV_CTRL_GPU_CURRENT_PROCESSOR_CLOCK_FREQS, &processor);

    if(!levelSucceeded && !graphMemSucceeded && !processorSucceeded)
        return false;

	eng::DataMap &	dmFreqs = m_smSources["frequencies"].p_dmData;
	dmFreqs["level"]		= level;
	dmFreqs["memory"]		= graphicMemory[0] * 2;
	dmFreqs["graphic"]		= graphicMemory[1];
	dmFreqs["processor"]	= processor;

	return true;
}

/**
 * Update the values of the Memory-Usage source
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::updateMem()
{
	// Query GPU Frequencies
	int32_t used	= -1;

	eng::DataMap &	dmMem	= m_smSources["memory-usage"].p_dmData;

	if(!XNVCTRLQueryTargetAttribute (m_pXDisplay, NV_CTRL_TARGET_TYPE_GPU, 0, 0, NV_CTRL_USED_DEDICATED_GPU_MEMORY, &used))
		return false;

	dmMem["used"]	= used;

	if(dmMem["total"] > 0)
        dmMem["percentage"] = dmMem["used"] * 100 / dmMem["total"];

	return true;
}

/**
 * Check that the query will be possible and perform needed setup
 * \return true if will be able to query, false otherwise
 */
bool NVidiaMonitorDataEngine::beforeQuery()
{
	// Don't update if cg off using bumblebee
	if(m_bIsBumblebee && !isCgOn())
		return false;

	// Open X11 Display
	m_pXDisplay = XOpenDisplay(m_strXDisplayId.c_str());
	if(!m_pXDisplay)
		return false;

	// Check if connected to a nvidia GPU
	if(!XNVCTRLQueryExtension(m_pXDisplay, NULL, NULL))
	{
        afterQuery();
		return false;
	}

    return true;
}

/**
 * Clean up after a query
 */
void NVidiaMonitorDataEngine::afterQuery()
{
	XCloseDisplay(m_pXDisplay);
	m_pXDisplay = NULL;
}

} // namespace eng

K_EXPORT_PLASMA_DATAENGINE(nvidia-monitor, eng::NVidiaMonitorDataEngine)

#include "NVidiaMonitorDataEngine.moc"
