
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

#ifndef APP_NVIDIA_MONITOR_APPLET_H
#define APP_NVIDIA_MONITOR_APPLET_H

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

						NVidiaMonitorApplet				(QObject *parent, const QVariantList &args);
						~NVidiaMonitorApplet			();

	public slots:

		void 			init							();

	/*
	 * Visualizations
	 */
	protected:

		void			addVisualizationEx				(const std::string& name, const std::string& title,
														 const int min, const int max,
														 const std::string& symbol);
		void			updateVisualizationsConfig();

		void			addTempVisualization			();
		void			addFreqsVisualization			();
		void			addMemVisualization				();

		void			updateTempVisualization			();
		void			updateFreqsVisualization		();
		void			updateMemVisualization			();

	/*
	 * Data from DataEngine Handling
	 */
	protected:

		void			updateSources					();

	protected slots:

		void			dataUpdated						(QString const & sourceName,
														 Plasma::DataEngine::Data const & data);
	/*
	 * Config UI Handling
	 */
	public:

		void			createConfigurationInterface	(KConfigDialog * parent);

	protected slots:

		void			configUpdated					();

	/*
	 * Click to lauch nvidia-settings Handling
	 */
	protected:

		void			mousePressEvent					(QGraphicsSceneMouseEvent * event);
		void			mouseReleaseEvent				(QGraphicsSceneMouseEvent * event);

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
		Qt::CheckState	boolToCheckState				(bool state);
		inline 
		bool			checkStateToBool				(Qt::CheckState state);

        void reloadRender();
        virtual void constraintsEvent(Plasma::Constraints constraints);

	/**********************************************************************************************/

	private:

		// Data get from the DataEngine 
		typedef	std::map<QString, DataSource>	SourceMap;

		SourceMap				_sources;

		// The plasmoid configuration 
		Ui::NVidiaMonitorConfig	_configUi;
		bool					_isAnalog;

		// Handling click on the applet and launching nvidia-settings from here
		QPoint					_mousePressLoc;
		bool					_sonLaunched;
		pid_t					_sonPid;

	}; // class NVidiaMonitorApplet

} // namespace app

K_EXPORT_PLASMA_APPLET(nvidia-monitor, app::NVidiaMonitorApplet)

#endif // APP_NVIDIA_MONITOR_APPLET_H

