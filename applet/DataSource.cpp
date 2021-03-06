
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

#include "DataSource.h"

namespace app
{

	/**
	 * Data Constructor
	 */
	Data::Data()
		: p_iData(0)
	{
	}

	/**
	 * DataSource Constructor
	 * \param addVisualization The function to call to add the DataSource corresponding visualization to the Applet
	 * \param updateVisualization The function to call to update the DataSource corresponding visualization in the Applet
	 */
	DataSource::DataSource(const AddVisualization in_pdAddVisualization, const UpdateVisualization in_pdUpdateVisualization)
		: p_bIsConnected(false)
		, p_bIsUpdated(false)
		, p_bIsInApplet(true)
		, p_bIsInToolTip(true)
		, p_iPollingInterval(5000)
		, p_pdAddVisualization(in_pdAddVisualization)
		, p_pdUpdateVisualization(in_pdUpdateVisualization)
	{
	}

} // namespace app

