
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

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <csignal>

#include <QPainter>
#include <QFontMetrics>
#include <QString>
 	
#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtDBus/QDBusInterface>
#include <QGraphicsLinearLayout>

#include <plasma/theme.h>

#include "NVidiaMonitorApplet.h"
#include "plotter.h"

using namespace std;

namespace app
{

	/*************************************************************************************************
	 * Inits & Releases
	 *************************************************************************************************/

	/**
	 * Applet Constructor
	 * Called by the Plasma framework
	 */
	NVidiaMonitorApplet::NVidiaMonitorApplet(QObject *parent, const QVariantList &args) :
		SM::Applet(parent, args),
		_isBumblebee(false),
		_isCgOn(false),
		_isAnalog(true),
		_sonLaunched(false)
	{
		setBackgroundHints(DefaultBackground);
		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

		setHasConfigurationInterface(true);
	}
	 
	/**
	 * Applet Destructor
	 */
	NVidiaMonitorApplet::~NVidiaMonitorApplet()
	{
		// Disconnect every source from DataEngine
		Plasma::DataEngine * engine = dataEngine("nvidia-monitor");
		if(engine->isValid())
		{

			engine->disconnectSource("bumblebee", this);

			SourceMap::iterator it;
			for(it = _sources.begin(); it != _sources.end(); it++)
			{
				if(it->second._isConnected)
					engine->disconnectSource(it->first, this);
			}
		}
	}

	/**
	 * Initialize the Applet
	 * Inits, Load configuration, First queries to the DataEngine
	 */
	void NVidiaMonitorApplet::init()
	{
		// Create locale
		KGlobal::locale()->insertCatalog("plasma_applet_nvidia-monitor");
		setTitle(i18n("nVidia Monitor"));

		// Define Visualization Functions
		_sources["temperature"]		= DataSource(&NVidiaMonitorApplet::addTempVisualization,
												 &NVidiaMonitorApplet::updateTempVisualization);
		_sources["frequencies"]		= DataSource(&NVidiaMonitorApplet::addFreqsVisualization,
												 &NVidiaMonitorApplet::updateFreqsVisualization);
		_sources["memory-usage"]	= DataSource(&NVidiaMonitorApplet::addMemVisualization,
												 &NVidiaMonitorApplet::updateMemVisualization);

		// Creating data containers
		DataMap & temp	= _sources["temperature"]._data;
		DataMap & freqs	= _sources["frequencies"]._data;
		DataMap & mem	= _sources["memory-usage"]._data;

		temp["temperature"];
		freqs["level"];
		freqs["graphic"];
		freqs["memory"];
		freqs["processor"];
		mem["percentage"];
		mem["used"];
		mem["total"];

		// Getting previous saved config
		KConfigGroup conf = config();
		_isAnalog									= conf.readEntry( "generalAnalog"	, true );
	
		_sources["temperature"]._isInApplet			= conf.readEntry( "tempInApplet"	, true );
		_sources["temperature"]._isInToolTip		= conf.readEntry( "tempInToolTip"	, true );
		_sources["temperature"]._pollingInterval	= conf.readEntry( "tempPolling"		, 5000 );

		_sources["frequencies"]._isInApplet			= conf.readEntry( "freqsInApplet"	, true );
		_sources["frequencies"]._isInToolTip		= conf.readEntry( "freqsInToolTip"	, true );
		_sources["frequencies"]._pollingInterval	= conf.readEntry( "freqsPolling"	, 5000 );

		_sources["memory-usage"]._isInApplet		= conf.readEntry( "memInApplet"		, true );
		_sources["memory-usage"]._isInToolTip		= conf.readEntry( "memInToolTip"	, true );
		_sources["memory-usage"]._pollingInterval	= conf.readEntry( "memPolling"		, 5000 );

		// Creating Tool Tip
		Plasma::ToolTipManager::self()->registerWidget(this);

		// Create first connection to DataEngine
		Plasma::DataEngine * engine = dataEngine("nvidia-monitor");
		if(engine->isValid())
			engine->connectSource("bumblebee", this);

		updateSources();

		// Create first display
		updateVisualizationsConfig();
	} 

	/*************************************************************************************************
	 * Visualizations
	 *************************************************************************************************/

	/**
	 * Add A visualisation to the Applet
	 * \param name Id of the visualization
	 * \param title Name to display in the applet
	 * \param min The min value for the graph
	 * \param max The max value for the graph
	 * \param symbol The symbol used when drawing the value
	 */
	void NVidiaMonitorApplet::addVisualizationEx(
		const std::string& name, const std::string& title,
		const int min, const int max, const std::string& symbol
	)
	{
		SM::Plotter* plotter = new SM::Plotter(this);
		plotter->setTitle(i18n(title.c_str()));
		plotter->setAnalog(_isAnalog && mode() != SM::Applet::Panel);
		plotter->setMinMax(min, max);
		plotter->setUnit(i18n(symbol.c_str()));

		appendVisualization(name.c_str(), plotter);
	}

	/**
	 * Call this when the visualization config has changed to apply it
	 */
	void NVidiaMonitorApplet::updateVisualizationsConfig()
	{
		deleteVisualizations();

		reloadRender();

		if(_isBumblebee && !_isCgOn)
			displayBumblebeeOff();

		else
		{
			if(_sources["temperature"]._isInApplet)
				(this->*(_sources["temperature"]._addVisualization))();

			if(_sources["frequencies"]._isInApplet)
				(this->*(_sources["frequencies"]._addVisualization))();

			if(_sources["memory-usage"]._isInApplet)
				(this->*(_sources["memory-usage"]._addVisualization))();
		}
	}

	/**
	 * Add a Temperature visualization
	 */
	void NVidiaMonitorApplet::addTempVisualization()
	{
		addVisualizationEx("temperature", "Temperature", 0, 150, "°C");
	}

	/**
	 * Add a Frequency visualization
	 */
	void NVidiaMonitorApplet::addFreqsVisualization()
	{
		addVisualizationEx("frequencies", "Frequencies", 0, 1200, "Mhz");
	}

	/**
	 * Add a Memory visualization
	 */
	void NVidiaMonitorApplet::addMemVisualization()
	{
		addVisualizationEx("memory-usage", "Memory Usage", 0, 100, "%");
	}

	/**
	 * Append to the Temperature graph the last get value.
	 */
	void NVidiaMonitorApplet::updateTempVisualization()
	{
		SM::Plotter* plotter = qobject_cast<SM::Plotter*>(visualization("temperature"));
		if(plotter)
		{
			plotter->addSample(QList<double>() << (double) _sources["temperature"]._data["temperature"]._data);
		}
	}

	/**
	 * Append to the Frequency graph the last get value.
	 */
	void NVidiaMonitorApplet::updateFreqsVisualization()
	{
		SM::Plotter* plotter = qobject_cast<SM::Plotter*>(visualization("frequencies"));
		if(plotter)
		{
			plotter->addSample(QList<double>() << (double) _sources["frequencies"]._data["graphic"]._data);
		}
	}

	/**
	 * Append to the Memory graph the last get value.
	 */
	void NVidiaMonitorApplet::updateMemVisualization()
	{
		SM::Plotter* plotter = qobject_cast<SM::Plotter*>(visualization("memory-usage"));
		if(plotter)
		{
			plotter->addSample(QList<double>() << (double) _sources["memory-usage"]._data["percentage"]._data);
		}
	}

	/**
	 * 
	 */
	void NVidiaMonitorApplet::displayBumblebeeOff()
	{
		displayNoAvailableSources();
//		KIcon appletIcon(icon());
//		m_noSourcesIcon = new Plasma::IconWidget(appletIcon, QString(), this);
//		mainLayout()->addItem(m_noSourcesIcon);
//
//		m_preferredItemHeight = MINIMUM;
//
//		setConfigurationRequired(true, i18n("CG Off"));
	}

	/*************************************************************************************************
	 * Data from DataEngine Handling
	 *************************************************************************************************/

	/**
	 * Perform connection, deconnection and reconnection of sources to DataEngine according to the configuration
	 */
	void NVidiaMonitorApplet::updateSources()
	{
		Plasma::DataEngine * engine = dataEngine("nvidia-monitor");
		if(engine->isValid())
		{
			SourceMap::iterator it;
			for(it = _sources.begin(); it != _sources.end(); it++)
			{
				DataSource & data = it->second;

				// Connect
				if((data._isInApplet || data._isInToolTip) && !data._isConnected)
				{
					engine->connectSource(it->first, this, data._pollingInterval);
					data._isConnected = true;
				}

				// Deconnect
				else if(!(data._isInApplet || data._isInToolTip) && data._isConnected)
				{
					engine->disconnectSource(it->first, this);
					data._isConnected = false;
				}

				// Update (deco reco with the new value)
				else if((data._isInApplet || data._isInToolTip) && data._isConnected && data._isUpdated)
				{
					engine->disconnectSource(it->first, this);
					engine->connectSource(it->first, this, data._pollingInterval);
					data._isUpdated = false;
				}
			}
		}
	}

	/**
	 * Called when a source has been updated
	 * Set the new value(s) and update visualizations
	 */
	void NVidiaMonitorApplet::dataUpdated(QString const & sourceName, Plasma::DataEngine::Data const & data)
	{
		if(data.isEmpty())
			return;

		if(sourceName == "bumblebee")
		{
			if(data["status"].toString() == "no_bb")
			{
				_isBumblebee = false;
				_isCgOn = true;
			}
			else 
			{
				_isBumblebee = true;

				bool cgOn = _isCgOn;
				if(data["status"].toString() == "on")
				{
					cgOn = true;
				}
				else
				{
					cgOn = false;
				}

				if(cgOn != _isCgOn)
				{
					_isCgOn = cgOn;
					updateVisualizationsConfig();
				}
			}
		}
		else
		{
			SourceMap::iterator it = _sources.find(sourceName);
			if(it != _sources.end())
			{
				DataMap::iterator dataIt;

				// Update data in container
				for(dataIt = it->second._data.begin(); dataIt != it->second._data.end(); dataIt++)
					dataIt->second._data = data[dataIt->first].toInt();

				// Update visualization
				(this->*(it->second._updateVisualization))();
			}
		}

		if (Plasma::ToolTipManager::self()->isVisible(this))
			toolTipAboutToShow();

		update();
	}

	/*************************************************************************************************
	 * Config UI Handling
	 *************************************************************************************************/

	/**
	 * Define the configuration UI of the Applet
	 */
	void NVidiaMonitorApplet::createConfigurationInterface(KConfigDialog * parent)
	{
		QWidget * qwConfigUi = new QWidget();
		_configUi.setupUi(qwConfigUi);

		// Define ui values from config values
		_configUi.generalAnalogCheckBox		-> setCheckState (boolToCheckState	( _isAnalog									));

		_configUi.tempInAppletCheckBox		-> setCheckState (boolToCheckState	( _sources["temperature"]._isInApplet		));
		_configUi.tempInToolTipCheckBox		-> setCheckState (boolToCheckState	( _sources["temperature"]._isInToolTip		));
		_configUi.tempPollingSpinBox		-> setValue							( _sources["temperature"]._pollingInterval	);

		_configUi.freqsInAppletCheckBox		-> setCheckState (boolToCheckState	( _sources["frequencies"]._isInApplet		));
		_configUi.freqsInToolTipCheckBox	-> setCheckState (boolToCheckState	( _sources["frequencies"]._isInToolTip		));
		_configUi.freqsPollingSpinBox		-> setValue							( _sources["frequencies"]._pollingInterval	);

		_configUi.memInAppletCheckBox		-> setCheckState (boolToCheckState	( _sources["memory-usage"]._isInApplet		));
		_configUi.memInToolTipCheckBox		-> setCheckState (boolToCheckState	( _sources["memory-usage"]._isInToolTip		));
		_configUi.memPollingSpinBox			-> setValue							( _sources["memory-usage"]._pollingInterval	);

		// Connect save config event 
		connect(parent, SIGNAL(applyClicked())	, this, SLOT(configUpdated()));
		connect(parent, SIGNAL(okClicked())		, this, SLOT(configUpdated()));

		// Add this config page
		parent->addPage(qwConfigUi, i18n("General"), icon());

		// Connect modify event to update the "Apply" button
 		connect( _configUi.generalAnalogCheckBox		, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);

 		connect( _configUi.tempInAppletCheckBox			, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
 		connect( _configUi.tempInToolTipCheckBox		, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
		connect( _configUi.tempPollingSpinBox 			, SIGNAL(valueChanged(int))	, parent	, SLOT(settingsModified())		);

 		connect( _configUi.freqsInAppletCheckBox		, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
 		connect( _configUi.freqsInToolTipCheckBox		, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
		connect( _configUi.freqsPollingSpinBox 			, SIGNAL(valueChanged(int))	, parent	, SLOT(settingsModified())		);

 		connect( _configUi.memInAppletCheckBox			, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
 		connect( _configUi.memInToolTipCheckBox			, SIGNAL(toggled(bool))		, parent	, SLOT(settingsModified())		);
		connect( _configUi.memPollingSpinBox 			, SIGNAL(valueChanged(int))	, parent	, SLOT(settingsModified())		);
	}

	/**
	 * Called when the user has saved the Applet configuration from the configuration UI
	 * Apply and save the new configuration
	 */
	void NVidiaMonitorApplet::configUpdated()
	{
		// Get config from ui
		_isAnalog									= checkStateToBool ( _configUi.generalAnalogCheckBox->checkState()	);

		_sources["temperature"]._isInApplet			= checkStateToBool ( _configUi.tempInAppletCheckBox->checkState()	);
		_sources["temperature"]._isInToolTip		= checkStateToBool ( _configUi.tempInToolTipCheckBox->checkState()	);
		_sources["temperature"]._pollingInterval	= _configUi.tempPollingSpinBox->value()								;
		_sources["temperature"]._isUpdated			= true																;

		_sources["frequencies"]._isInApplet			= checkStateToBool ( _configUi.freqsInAppletCheckBox->checkState()	);
		_sources["frequencies"]._isInToolTip		= checkStateToBool ( _configUi.freqsInToolTipCheckBox->checkState()	);
		_sources["frequencies"]._pollingInterval	= _configUi.freqsPollingSpinBox->value()							;
		_sources["frequencies"]._isUpdated			= true																;

		_sources["memory-usage"]._isInApplet		= checkStateToBool ( _configUi.memInAppletCheckBox->checkState()	);
		_sources["memory-usage"]._isInToolTip		= checkStateToBool ( _configUi.memInToolTipCheckBox->checkState()	);
		_sources["memory-usage"]._pollingInterval	= _configUi.memPollingSpinBox->value()								;
		_sources["memory-usage"]._isUpdated			= true																;

		// Write new config
		KConfigGroup conf = config();
		conf.writeEntry ( "generalAnalog"	, _isAnalog									);

		conf.writeEntry ( "tempInApplet"	, _sources["temperature"]._isInApplet		);
		conf.writeEntry ( "tempInToolTip"	, _sources["temperature"]._isInToolTip		);
		conf.writeEntry ( "tempPolling"		, _sources["temperature"]._pollingInterval	);

		conf.writeEntry ( "freqsInApplet"	, _sources["frequencies"]._isInApplet		);
		conf.writeEntry ( "freqsInToolTip"	, _sources["frequencies"]._isInToolTip		);
		conf.writeEntry ( "freqsPolling"	, _sources["frequencies"]._pollingInterval	);

		conf.writeEntry ( "memInApplet"		, _sources["memory-usage"]._isInApplet		);
		conf.writeEntry ( "memInToolTip"	, _sources["memory-usage"]._isInToolTip		);
		conf.writeEntry ( "memPolling"		, _sources["memory-usage"]._pollingInterval	);

		emit configNeedsSaving();

		// Update to use new config
		updateSources();
		updateVisualizationsConfig();
		update();
	}

	/*************************************************************************************************
	 * Click to launch nvidia-settings Handling
	 *************************************************************************************************/
        
	/**
	 * Called when the user clicks the applet
	 */
	void NVidiaMonitorApplet::mousePressEvent(QGraphicsSceneMouseEvent * event)
	{
		if(event->button() == Qt::LeftButton)
			_mousePressLoc = event->screenPos();
	}

	/**
	 * Called when the user releases the mouse on the applet
	 */
	void NVidiaMonitorApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
	{
		if(event->button() == Qt::LeftButton &&
				((event->screenPos() - _mousePressLoc).manhattanLength()) <
				QApplication::startDragDistance())
		{
			if(!_sonLaunched)
			{
				if((_sonPid = fork()) == 0)
					execl("/usr/bin/nvidia-settings", "/usr/bin/nvidia-settings", NULL);
				else
					_sonLaunched = true;
			}
			else
			{
				kill(_sonPid, SIGTERM);
				_sonLaunched = false;
			}
		}
	}

	/*************************************************************************************************
	 * Tool Tip on Hover Handling
	 *************************************************************************************************/
	
	/**
	 * Set the Applet tool tip content
	 * Automatically called when the tool tip is displayed and should be called when the content needs to be updated (only if displayed of course)
	 */
	void NVidiaMonitorApplet::toolTipAboutToShow()
	{
		QString content;

		DataMap & temp = _sources["temperature"]._data;
		DataMap & freqs = _sources["frequencies"]._data;
		DataMap & mem = _sources["memory-usage"]._data;

		if(_sources["temperature"]._isInToolTip)
			content += i18n("Temperature : %1 °C<br/>", temp["temperature"]._data);
		if(_sources["frequencies"]._isInToolTip)
			content += i18n("Frequencies : Lvl. %1 - %2 Mhz %3 Mhz %4 Mhz<br/>", freqs["level"]._data, freqs["graphic"]._data, freqs["memory"]._data, freqs["processor"]._data);
		if(_sources["memory-usage"]._isInToolTip)
			content += i18n("Memory Usage : %1% (%2 / %3 Mo)<br/>", mem["percentage"]._data, mem["used"]._data, mem["total"]._data);

		Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent(title(), content, KIcon(icon())));
	}

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

	void NVidiaMonitorApplet::constraintsEvent(Plasma::Constraints constraints)
	{
		if (constraints & Plasma::FormFactorConstraint) {
			if (m_mode == Monitor) {
				setBackgroundHints(NoBackground);
				m_orientation = Qt::Vertical;
			} else {
				SM::Applet::Mode mode = m_mode;
				switch (formFactor()) {
					case Plasma::Planar:
					case Plasma::MediaCenter:
						mode = Desktop;
						m_orientation = Qt::Vertical;
						break;
					case Plasma::Horizontal:
						mode = Panel;
						m_orientation = Qt::Horizontal;
						break;
					case Plasma::Vertical:
						mode = Panel;
						m_orientation = Qt::Vertical;
						break;

					case Plasma::Application:
					default:
						mode = Panel;
						m_orientation = Qt::Vertical;
						break;
				}
				if (mode != m_mode) {
					m_mode = mode;
					updateVisualizationsConfig();
				}
			}
		} else if (constraints & Plasma::SizeConstraint) {
			checkGeometry();
		}
	}

	void NVidiaMonitorApplet::reloadRender()
	{
		removeLayout();
		configureLayout();

		if (!_sources["temperature"]._isInApplet &&
			!_sources["frequencies"]._isInApplet &&
			!_sources["memory-usage"]._isInApplet)
		{
			displayNoAvailableSources();
			constraintsEvent(Plasma::SizeConstraint);
			return;
		}

		mainLayout()->activate();
		constraintsEvent(Plasma::SizeConstraint);
	}


} // namespace app

#include "NVidiaMonitorApplet.moc"

