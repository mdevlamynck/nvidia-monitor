
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

#ifndef ENG__DATA_SOURCE__H
#define ENG__DATA_SOURCE__H

#include <vector>
#include <map>
#include <inttypes.h>

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
typedef	std::map<QString, int32_t>	DataMap;
typedef	bool(NVidiaMonitorDataEngine::*Update)(void);

/**
 * Handles Data : value and update function
 */
struct DataSource
{

public:

	DataSource(Update in_pdUpdate = NULL);

/**********************************************************************************************/

public:

	DataMap	p_dmData;
	Update	p_pdUpdate;

}; // struc DataSource

} // namespace eng

#endif // ENG__DATA_SOURCE__H

