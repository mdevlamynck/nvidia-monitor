 
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

#include <sstream>
#include <stdio.h>

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

	initBumblebee();

	if(initMem())
		setData("memory-usage", "total", m_smSources["memory-usage"].p_dmData["total"]);
}

/**
 * Detect either this is a bumblebee setup or a regular one
 */
void NVidiaMonitorDataEngine::initBumblebee()
{
	std::string strOutput = executeCommand("[ -f /proc/acpi/bbswitch ] && printf yes || printf no");
	if(strOutput == "yes")
	{
		setData("bumblebee", "status", "off");
		m_bIsBumblebee = true;
	}
	else
	{
		setData("bumblebee", "status", "no_bb");
		m_bIsBumblebee = false;
	}
}

/**
 * Get the GPU Total VRam used to calcul the percentage of used memory
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::initMem()
{
	std::string strOutput;

	if(m_bIsBumblebee)
		if(isCgOn())
			strOutput = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/TotalDedicatedGPUMemory -t");
		else
			return false;
	else
		strOutput = executeCommand("nvidia-settings -q [gpu:0]/TotalDedicatedGPUMemory -t");
	if(strOutput != "")
	{
		istringstream	issData(strOutput.c_str());
		eng::DataMap &	dmMem = m_smSources["memory-usage"].p_dmData;

		issData >> dmMem["total"];

		return true;
	}

	else
		return false;
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
bool NVidiaMonitorDataEngine::isCgOn()
{
	std::string strOutput = executeCommand("cat /proc/acpi/bbswitch");

	if(strOutput != "")
	{
		if(strOutput.find("ON") != std::string::npos)
		{
			setData("bumblebee", "status", "on");
			return true;
		}
		else
		{
			setData("bumblebee", "status", "off");
			return false;
		}
	}
	else
		return false;
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
		return true;

	SourceMap::const_iterator itSources = m_smSources.find(in_qstrName);

	if(itSources == m_smSources.end())
		return false;
	else
		return updateSourceEvent(in_qstrName);
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

	if(itSources != m_smSources.end() && itSources->second.p_pdUpdate != NULL && (this->*(itSources->second.p_pdUpdate))())
	{
		DataMap::const_iterator itData;

		for(itData = itSources->second.p_dmData.begin(); itData != itSources->second.p_dmData.end(); itData++)
			setData(itSources->first, itData->first, itData->second);

		return true;
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
	std::string strOutput;
	if(m_bIsBumblebee)
	{
		if(isCgOn())
			strOutput = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q GPUCoreTemp -t");
		else
			return true;
	}
	else
		strOutput = executeCommand("nvidia-settings -q GPUCoreTemp -t");

	if(strOutput != "")
	{
		istringstream	issData(strOutput);
		eng::DataMap &	dmTemp = m_smSources["temperature"].p_dmData;

		issData >> dmTemp["temperature"];

		return true;
	}
	else
		return false;
}

/**
 * Update the values of the Frequncies source
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::updateFreqs()
{
	std::string strOutput;

	if(m_bIsBumblebee)
	{
		if(isCgOn())
			strOutput = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/GPUCurrentPerfLevel -q [gpu:0]/GPUCurrentClockFreqs -q [gpu:0]/GPUCurrentProcessorClockFreqs -t | sed -e 's/,/\\n/'");
		else
			return true;
	}
	else
		strOutput = executeCommand("nvidia-settings -q [gpu:0]/GPUCurrentPerfLevel -q [gpu:0]/GPUCurrentClockFreqs -q [gpu:0]/GPUCurrentProcessorClockFreqs -t | sed  -e 's/,/\\n/'");

	if(strOutput != "")
	{
		istringstream	issData(strOutput);
		eng::DataMap &	dmFreqs = m_smSources["frequencies"].p_dmData;

		issData >> dmFreqs["level"];
		issData >> dmFreqs["graphic"];
		issData >> dmFreqs["memory"];
		issData >> dmFreqs["processor"];

		return true;
	}
	else
		return false;
}

/**
 * Update the values of the Memory-Usage source
 * \return Wether the function succeeded or not
 */
bool NVidiaMonitorDataEngine::updateMem()
{
	std::string strOutput;

	if(m_bIsBumblebee)
	{
		if(isCgOn())
			strOutput = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/UsedDedicatedGPUMemory -t");
		else
			return true;
	}
	else
		strOutput = executeCommand("nvidia-settings -q [gpu:0]/UsedDedicatedGPUMemory -t");

	if(strOutput != "")
	{
		istringstream	issData(strOutput.c_str());
		eng::DataMap &	dmMem = m_smSources["memory-usage"].p_dmData;

		issData >> dmMem["used"];

		if(dmMem["total"] > 0)
			initMem();

		if(dmMem["percentage"] != -1 && dmMem["total"] > 0)
			dmMem["percentage"] = dmMem["used"] * 100 / dmMem["total"];

		return true;
	}
	else
		return false;
}

/**********************************************************************************************
 * Usefull
 **********************************************************************************************/
 
/**
 * Execute a shell command and get its standard strOutput
 * \param The command to execute
 * \return The standard strOutput of the command
 */
std::string NVidiaMonitorDataEngine::executeCommand(const std::string & in_strCmd) const
{
	FILE*			fIn;
	char			acBuff[8];
	ostringstream	ossResult;

	if(!( fIn = popen(in_strCmd.c_str(), "r") ))
		return "";

	while(fgets(acBuff, sizeof(acBuff), fIn)!=NULL)
		ossResult << acBuff;

	pclose(fIn);

	return ossResult.str();
}

} // namespace eng

K_EXPORT_PLASMA_DATAENGINE(nvidia-monitor, eng::NVidiaMonitorDataEngine)

#include "NVidiaMonitorDataEngine.moc"

