
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

/*************************************************************************************************
 * Usefull
 *************************************************************************************************/

/**
 * Converts a boolean to a Qt::CheckState
 * true	 	-> Qt::Checked
 * false	-> Qt::Unchecked
 */
Qt::CheckState NVidiaMonitorApplet::boolToCheckState(bool state)
{
	return (state) ? Qt::Checked : Qt::Unchecked;
}

/**
 * Converts a Qt::CheckState to a boolean
 * Qt::Checked or Qt::PartiallyChecked	-> true
 * Qt::Unchecked						-> false
 */
bool NVidiaMonitorApplet::checkStateToBool(Qt::CheckState state)
{
	return (state == Qt::Checked || state == Qt::PartiallyChecked) ? true : false;
}

