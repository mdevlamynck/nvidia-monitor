
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

#ifndef ENG__NVIDIA_MONITOR_ENGINE__H
#define ENG__NVIDIA_MONITOR_ENGINE__H

#include <map>
#include <string>
#include <inttypes.h>

#include <QString>
#include <QTimer>
#include <Plasma/DataEngine>

extern "C"
{
#include <X11/Xlib.h>
#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>
}

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
 * Typdefs
 */
typedef std::map<QString, DataSource>	SourceMap;


/*
 * Inits & Releases
 */
public:

				NVidiaMonitorDataEngine		(QObject* in_pParent, const QVariantList& in_args);
				~NVidiaMonitorDataEngine	();

	void		init						();

protected:

	void		initBumblebee				();

/*
 * Data Handling
 */
public:

	QStringList	sources						() const;

protected:

	bool		isCgOn						();

	bool		sourceRequestEvent			(QString const & in_qstrName);
	bool		updateSourceEvent			(QString const & in_qstrName);

	bool		updateTemp					();
	bool		updateFreqs					();
	bool		updateMem					();

/*
 * Usefull
 */
protected:

	std::string	executeCommand				(const std::string& in_strCmd) const;

/**********************************************************************************************/

protected:

	SourceMap	m_smSources;
	bool		m_bIsBumblebee;

	Display*	m_pXDisplay;
	std::string	m_strXDisplayId;

}; // class NVidiaMonitorDataEngine

} // namespace eng

#endif // ENG__NVIDIA_MONITOR_ENGINE__H
