
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

#ifndef ENG_NVIDIA_MONITOR_ENGINE_H
#define ENG_NVIDIA_MONITOR_ENGINE_H

#include <map>
#include <string>

#include <QString>
#include <QTimer>
#include <Plasma/DataEngine>

#include "DataSource.h"

/**
 * The DataEngine part of nvidia-monitor
 */
namespace eng
{

	/**
	 * Provide Data about nvidia graphic cards using the proprietary driver (with nvidia-settings)
	 * Uses Plasma::DataEngine to be able to get requests from plasmoids
	 */
	class NVidiaMonitorDataEngine : public Plasma::DataEngine
	{

	// Needed to use signals/slots provided by Qt
	Q_OBJECT

	/*
	 * Inits & Releases
	 */
	public:

					NVidiaMonitorDataEngine		(QObject *parent, const QVariantList &args);
					~NVidiaMonitorDataEngine	();

		void		init						();

	protected:

		bool		initMem						();

	/*
	 * Data Handling
	 */
	 public:

		QStringList	sources						() const;

	protected:

		bool		sourceRequestEvent			(QString const & name);
		bool		updateSourceEvent			(QString const & name);

		bool		updateTemp					();
		bool		updateFreqs					();
		bool		updateMem					();

	/*
	 * Usefull
	 */
	protected:

		std::string	executeCommand				(const std::string& cmd) const;

	/**********************************************************************************************/

	protected:

		typedef std::map<QString, DataSource>	SourceMap;

		SourceMap	_sources;

	}; // class NVidiaMonitorDataEngine

} // namespace eng

#endif // ENG_NVIDIA_MONITOR_ENGINE_H

