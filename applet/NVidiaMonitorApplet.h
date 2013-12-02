
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

#ifndef APP__NVIDIA_MONITOR_APPLET__H
#define APP__NVIDIA_MONITOR_APPLET__H

#include <map>

#include <QString>
#include <QSizeF>
#include <QtCore/QVector>
#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <KConfigDialog>
#include "applet.h"

#include "DataSource.h"

#include "ui_NVidiaMonitorConfig.h"

/**
 * Contains the plasmoid part of nvidia-monitor
 */
namespace app
{

/**
 * The main class handling the plasmoid behaviour and definition
 * Uses SM::Applet for predefined visualizations it offers
 */
class NVidiaMonitorApplet : public SM::Applet
{

// Needed to use signals/slots provided by Qt
Q_OBJECT

/*
 * Inits & Releases
 */
public:

					NVidiaMonitorApplet				(QObject* in_pParent, const QVariantList& in_args);
					~NVidiaMonitorApplet			();

public slots:

	void 			init							();

/*
 * Visualizations
 */
protected:

	void			updateVisualizationsConfig();

	void			addVisualizationEx				(const std::string& in_strName, const std::string& in_strTitle,
													 const int in_iMin, const int in_iMax,
													 const std::string& in_strSymbol);

	void			addTempVisualization			();
	void			addFreqsVisualization			();
	void			addMemVisualization				();

	void			updateTempVisualizationEx		(const std::string & in_strSource, const std::string & in_strData);

	void			updateTempVisualization			();
	void			updateFreqsVisualization		();
	void			updateMemVisualization			();

	void			displayBumblebeeOff				();

/*
 * Data from DataEngine Handling
 */
protected:

	void			updateSources					();

protected slots:

	void			dataUpdated						(QString const & in_qstrSourceName,
													 Plasma::DataEngine::Data const & in_newData);
/*
 * Config UI Handling
 */
public:

	void			createConfigurationInterface	(KConfigDialog * in_pParent);

protected slots:

	void			configUpdated					();

/*
 * Tool Tip on hover Handling
 */
protected slots:
	void			toolTipAboutToShow				();

/**
 * Usefull
 */
protected:

	inline 
	Qt::CheckState	boolToCheckState				(bool in_bState);
	inline 
	bool			checkStateToBool				(Qt::CheckState in_csState);

/**
 * From SM::Applet
 */
protected:
	void			reloadRender					();
	virtual void	constraintsEvent				(Plasma::Constraints in_constraints);

/**********************************************************************************************/

private:

	// Data get from the DataEngine 
	typedef	std::map<QString, DataSource>	SourceMap;

	SourceMap				m_smSources;
	bool					m_bIsBumblebee;
	bool					m_bIsCgOn;

	// The plasmoid configuration 
	Ui::NVidiaMonitorConfig	m_configUi;
	bool					m_bIsAnalog;

}; // class NVidiaMonitorApplet

} // namespace app

K_EXPORT_PLASMA_APPLET(nvidia-monitor, app::NVidiaMonitorApplet)

#endif // APP__NVIDIA_MONITOR_APPLET__H

