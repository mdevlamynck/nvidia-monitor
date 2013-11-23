
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

#ifndef APP_DATA_SOURCE_H
#define APP_DATA_SOURCE_H

#include <map>

#include <QString>

namespace app
{

	/**********************************************************************************************
	 * Forward
	 **********************************************************************************************/
	class NVidiaMonitorApplet;

	/**
	 * Handles one Data get from the DataEngine : the value and either it is active or not
	 * Has a default constructor to set default values
	 */
	struct Data
	{

		Data();

	/************************************************************************************************/

		int		_data;

	};

	/**********************************************************************************************
	 * Typedefs
	 **********************************************************************************************/
	typedef	std::map<QString, Data>	DataMap;
	typedef	void(NVidiaMonitorApplet::*AddVisualization)(void);
	typedef	void(NVidiaMonitorApplet::*UpdateVisualization)(void);

	/**
	 * Handles a type of Data get from the DataEngine : Values get and options about them
	 */
	class DataSource
	{

	public:

		DataSource	(const AddVisualization addVisualization = NULL,
					 const UpdateVisualization updateVisualization = NULL);

	/**********************************************************************************************/

	public:

		DataMap				_data;

		bool				_isConnected;
		bool				_isUpdated;
		bool				_isInApplet;
		bool				_isInToolTip;

		int					_pollingInterval;

		AddVisualization	_addVisualization;
		UpdateVisualization	_updateVisualization;

	}; // struct DataSource

} // namespace app;

#endif // APP_DATA_SOURCE_H

