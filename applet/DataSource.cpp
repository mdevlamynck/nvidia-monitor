
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

#include "DataSource.h"

namespace app
{

	/**
	 * Data Constructor
	 */
	Data::Data() :
		_data(0)
	{
	}

	/**
	 * DataSource Constructor
	 * \param addVisualization The function to call to add the DataSource corresponding visualization to the Applet
	 * \param updateVisualization The function to call to update the DataSource corresponding visualization in the Applet
	 */
	DataSource::DataSource(
		const AddVisualization addVisualization,
		const UpdateVisualization updateVisualization
	) :
		_isConnected(false)		, _isUpdated(false),
		_isInApplet(true)		, _isInToolTip(true),
		_pollingInterval(5000)	,
		_addVisualization(addVisualization),
		_updateVisualization(updateVisualization)
	{
	}

} // namespace app

