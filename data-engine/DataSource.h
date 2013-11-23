
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

#ifndef ENG_DATA_SOURCE_H
#define ENG_DATA_SOURCE_H

#include <vector>
#include <map>

#include <QString>

namespace eng
{

	/**********************************************************************************************
	 * Forward Declaration
	 **********************************************************************************************/
	class NVidiaMonitorDataEngine;

	/**********************************************************************************************
	 * Typedefs
	 **********************************************************************************************/
	typedef	std::map<QString, int>	DataMap;
	typedef	bool(NVidiaMonitorDataEngine::*Update)(void);

	/**
	 * Handles Data : value and update function
	 */
	struct DataSource
	{

	public:

		DataSource(Update update = NULL);

	/**********************************************************************************************/

	public:

		DataMap	_data;
		Update	_update;

	}; // struc DataSource

} // namespace eng

#endif // ENG_DATA_SOURCE_H

