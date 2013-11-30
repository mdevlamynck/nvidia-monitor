 
/*
	NVidia Monitor - plasmoid that displays nvidia gpu's informations
	Copyright (C) 2013  Matthias Devlamynck

	This file is part of NVidia Monitor.

	NVidia Monitor is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NVidia Monitor is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NVidia Monitor.  If not, see <http://www.gnu.org/licenses/>.

	To contact me, please send me an email at :
		wildfier@hotmail.fr

	The source code is aviable at :
		http://sourceforge.net/projects/nvidia-monitor/	

	If you wish to make a fork or maintain this project, please contact me.
*/

#include <iostream>
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
	NVidiaMonitorDataEngine::NVidiaMonitorDataEngine(QObject *parent, const QVariantList &args) :
		Plasma::DataEngine(parent, args)
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
		_sources["temperature"]		= DataSource(&NVidiaMonitorDataEngine::updateTemp);
		_sources["frequencies"]		= DataSource(&NVidiaMonitorDataEngine::updateFreqs);
		_sources["memory-usage"]	= DataSource(&NVidiaMonitorDataEngine::updateMem);

		initBumblebee();
		std::cout << "bb : " << _isBumblebee << std::endl;

		if(initMem())
			setData("memory-usage", "total", _sources["memory-usage"]._data["total"]);
	}

	/**
	 * Detect either this is a bumblebee setup or a regular one
	 */
	void NVidiaMonitorDataEngine::initBumblebee()
	{
		std::string output = executeCommand("[ -f /proc/acpi/bbswitch ] && printf yes || printf no");
		if(output == "yes")
		{
			setData("bumblebee", "status", "off");
			_isBumblebee = true;
		}
		else
		{
			setData("bumblebee", "status", "no_bb");
			_isBumblebee = false;
		}
	}

	/**
	 * Get the GPU Total VRam used to calcul the percentage of used memory
	 * \return Either the function succeeded or not
	 */
	bool NVidiaMonitorDataEngine::initMem()
	{
		std::string output;

		if(_isBumblebee)
			if(isCgOn())
				output = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/TotalDedicatedGPUMemory -t");
			else
				return false;
		else
			output = executeCommand("nvidia-settings -q [gpu:0]/TotalDedicatedGPUMemory -t");
		if(output != "")
		{
			istringstream	data(output.c_str());
			eng::DataMap &	mem = _sources["memory-usage"]._data;

			data >> mem["total"];

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
		QStringList list;
		SourceMap::const_iterator it;

		list << "bumblebee";

		for(it = _sources.begin(); it != _sources.end(); it++)
		{
			list << it->first;
		}

		return list;
	}

	/**
	 * Detect if the cg is on when using bumblbee
	 * \return Either the cg is currently on or not
	 */
	bool NVidiaMonitorDataEngine::isCgOn()
	{
		std::string output = executeCommand("cat /proc/acpi/bbswitch");

		std::cout << output << std::endl;
		if(output != "")
		{
			if(output.find("ON") != std::string::npos)
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
	 * \param name Asked source name
	 * \return Either the value will by updated or not
	 */
	bool NVidiaMonitorDataEngine::sourceRequestEvent(QString const & name)
	{
		if(name == "bumblebee")
			return true;

		SourceMap::const_iterator it = _sources.find(name);

		if(it == _sources.end())
			return false;
		else
			return updateSourceEvent(name);
	}

	/**
	 * Update the value(s) of a source
	 * Called when the interval between two updated has expired
	 * \param name Name of the source to update
	 * \return Either the value will by updated or not
	 */
	bool NVidiaMonitorDataEngine::updateSourceEvent(QString const & name)
	{
		if(name == "bumblebee")
		{
			isCgOn();
			return true;
		}

		SourceMap::const_iterator it = _sources.find(name);

		if(it != _sources.end() && it->second._update != NULL && (this->*(it->second._update))())
		{
			DataMap::const_iterator data;

			for(data = it->second._data.begin(); data != it->second._data.end(); data++)
				setData(it->first, data->first, data->second);

			return true;
		}

		else
			return false;
	}

	/**
	 * Update the value of the Temperature source
	 * \return Either the function succeeded or not
	 */
	bool NVidiaMonitorDataEngine::updateTemp()
	{
		std::string output;
		if(_isBumblebee)
			if(isCgOn())
				output = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q GPUCoreTemp -t");
			else
				return true;
		else
			output = executeCommand("nvidia-settings -q GPUCoreTemp -t");

		if(output != "")
		{
			istringstream	data(output);
			eng::DataMap &	temp = _sources["temperature"]._data;

			data >> temp["temperature"];

			return true;
		}

		else
			return false;
	}

	/**
	 * Update the values of the Frequncies source
	 * \return Either the function succeeded or not
	 */
	bool NVidiaMonitorDataEngine::updateFreqs()
	{
		std::string output;

		if(_isBumblebee)
		{
			if(isCgOn())
				output = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/GPUCurrentPerfLevel -q [gpu:0]/GPUCurrentClockFreqs -q [gpu:0]/GPUCurrentProcessorClockFreqs -t | sed -e 's/,/\\n/'");
			else
				return true;
		}
		else
			output = executeCommand("nvidia-settings -q [gpu:0]/GPUCurrentPerfLevel -q [gpu:0]/GPUCurrentClockFreqs -q [gpu:0]/GPUCurrentProcessorClockFreqs -t | sed  -e 's/,/\\n/'");

		if(output != "")
		{
			istringstream	data(output);
			eng::DataMap &	freqs = _sources["frequencies"]._data;

			data >> freqs["level"];
			data >> freqs["graphic"];
			data >> freqs["memory"];
			data >> freqs["processor"];

			return true;
		}

		else
			return false;
	}

	/**
	 * Update the values of the Memory-Usage source
	 * \return Either the function succeeded or not
	 */
	bool NVidiaMonitorDataEngine::updateMem()
	{
		std::string output;

		if(_isBumblebee)
		{
			if(isCgOn())
				output = executeCommand("optirun -b virtualgl nvidia-settings -c :8 -q [gpu:0]/UsedDedicatedGPUMemory -t");
			else
				return true;
		}
		else
			output = executeCommand("nvidia-settings -q [gpu:0]/UsedDedicatedGPUMemory -t");

		if(output != "")
		{
			istringstream	data(output.c_str());
			eng::DataMap &	mem = _sources["memory-usage"]._data;

			data >> mem["used"];

			if(mem["total"] > 0)
				initMem();

			if(mem["percentage"] != -1 && mem["total"] > 0)
				mem["percentage"] = mem["used"] * 100 / mem["total"];

			return true;
		}

		else
			return false;
	}

	/**********************************************************************************************
	 * Usefull
	 **********************************************************************************************/
	 
	/**
	 * Execute a shell command and get its standard output
	 * \param The command to execute
	 * \return The standard output of the command
	 */
	std::string NVidiaMonitorDataEngine::executeCommand(const std::string & cmd) const
	{
		FILE*			in;
		char			buff[8];
		ostringstream	result;

		if(!( in = popen(cmd.c_str(), "r") ))
		{
			return "";
		}

		while(fgets(buff, sizeof(buff), in)!=NULL)
		{
			result << buff;
		}
		pclose(in);

		return result.str();
	}


} // namespace eng

K_EXPORT_PLASMA_DATAENGINE(nvidia-monitor, eng::NVidiaMonitorDataEngine)

#include "NVidiaMonitorDataEngine.moc"

