
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
NVidiaMonitorApplet::NVidiaMonitorApplet(QObject* in_pParent, const QVariantList& in_args)
	: SM::Applet(in_pParent, in_args)
	, m_bIsBumblebee(false)
	, m_bIsCgOn(false)
	, m_bIsAnalog(true)
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
	Plasma::DataEngine * pEngine = dataEngine("nvidia-monitor");
	if(pEngine->isValid())
	{

		pEngine->disconnectSource("bumblebee", this);

		SourceMap::iterator it;
		for(it = m_smSources.begin(); it != m_smSources.end(); it++)
		{
			if(it->second.p_bIsConnected)
				pEngine->disconnectSource(it->first, this);
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
	m_smSources["temperature"]	= DataSource(&NVidiaMonitorApplet::addTempVisualization,
											 &NVidiaMonitorApplet::updateTempVisualization);
	m_smSources["frequencies"]	= DataSource(&NVidiaMonitorApplet::addFreqsVisualization,
											 &NVidiaMonitorApplet::updateFreqsVisualization);
	m_smSources["memory-usage"]	= DataSource(&NVidiaMonitorApplet::addMemVisualization,
											 &NVidiaMonitorApplet::updateMemVisualization);

	// Creating data containers
	DataMap & dmTemp	= m_smSources["temperature"].p_dmData;
	DataMap & dmFreqs	= m_smSources["frequencies"].p_dmData;
	DataMap & dmMem		= m_smSources["memory-usage"].p_dmData	;
																
	dmTemp["temperature"];                                    	
	dmFreqs["level"];                                             
	dmFreqs["graphic"];                                       	
	dmFreqs["memory"];                                        	
	dmFreqs["processor"];                                     	
	dmMem["percentage"];                                          
	dmMem["used"];                                            	
	dmMem["total"];

	// Getting previous saved config
	KConfigGroup conf = config();
	m_bIsAnalog										= conf.readEntry( "generalAnalog"	, true );

	m_smSources["temperature"].p_bIsInApplet		= conf.readEntry( "tempInApplet"	, true );
	m_smSources["temperature"].p_bIsInToolTip		= conf.readEntry( "tempInToolTip"	, true );
	m_smSources["temperature"].p_iPollingInterval	= conf.readEntry( "tempPolling"		, 5000 );

	m_smSources["frequencies"].p_bIsInApplet		= conf.readEntry( "freqsInApplet"	, true );
	m_smSources["frequencies"].p_bIsInToolTip		= conf.readEntry( "freqsInToolTip"	, true );
	m_smSources["frequencies"].p_iPollingInterval	= conf.readEntry( "freqsPolling"	, 5000 );

	m_smSources["memory-usage"].p_bIsInApplet		= conf.readEntry( "memInApplet"		, true );
	m_smSources["memory-usage"].p_bIsInToolTip		= conf.readEntry( "memInToolTip"	, true );
	m_smSources["memory-usage"].p_iPollingInterval	= conf.readEntry( "memPolling"		, 5000 );

	// Creating Tool Tip
	Plasma::ToolTipManager::self()->registerWidget(this);

	// Create first connection to DataEngine
	Plasma::DataEngine * pEngine = dataEngine("nvidia-monitor");
	if(pEngine->isValid())
		pEngine->connectSource("bumblebee", this);

	updateSources();

	// Create first display
	updateVisualizationsConfig();
} 

/*************************************************************************************************
 * Visualizations
 *************************************************************************************************/

/**
 * Call this when the visualization config has changed to apply it
 */
void NVidiaMonitorApplet::updateVisualizationsConfig()
{
	deleteVisualizations();

	reloadRender();

	if(m_bIsBumblebee && !m_bIsCgOn)
		displayBumblebeeOff();

	else
	{
		if(m_smSources["temperature"].p_bIsInApplet)
			(this->*(m_smSources["temperature"].p_pdAddVisualization))();

		if(m_smSources["frequencies"].p_bIsInApplet)
			(this->*(m_smSources["frequencies"].p_pdAddVisualization))();

		if(m_smSources["memory-usage"].p_bIsInApplet)
			(this->*(m_smSources["memory-usage"].p_pdAddVisualization))();
	}
}

/**
 * Add A visualisation to the Applet
 * \param in_strName	Id of the visualization
 * \param in_strTitle	Name to display in the applet
 * \param in_iMin The	min value for the graph
 * \param in_iMax The	max value for the graph
 * \param in_strSymbol	The symbol used when drawing the value
 */
void NVidiaMonitorApplet::addVisualizationEx(const std::string& in_strName, const std::string& in_strTitle,
												const int in_iMin, const int in_iMax, const std::string& in_strSymbol)
{
	SM::Plotter* pPlotter = new SM::Plotter(this);

	pPlotter->setTitle	(i18n(in_strTitle.c_str()));
	pPlotter->setAnalog	(m_bIsAnalog && mode() != SM::Applet::Panel);
	pPlotter->setMinMax	(in_iMin, in_iMax);
	pPlotter->setUnit	(i18n(in_strSymbol.c_str()));

	appendVisualization(in_strName.c_str(), pPlotter);
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
 * Append a data to the corresponding graph
 * \param in_strSource	The name of the source provinding the new data
 * \param in_strData	The name of the new data to append
 */
void NVidiaMonitorApplet::updateTempVisualizationEx(const std::string & in_strSource, const std::string & in_strData)
{
	SM::Plotter* plotter = qobject_cast<SM::Plotter*>(visualization(in_strSource.c_str()));
	if(plotter)
	{
		plotter->addSample(QList<double>() << (double) m_smSources[in_strSource.c_str()].p_dmData[in_strData.c_str()].p_iData);
	}
}

/**
 * Append to the Temperature graph the last get value.
 */
void NVidiaMonitorApplet::updateTempVisualization()
{
	updateTempVisualizationEx("temperature", "temperature");
}

/**
 * Append to the Frequency graph the last get value.
 */
void NVidiaMonitorApplet::updateFreqsVisualization()
{
	updateTempVisualizationEx("frequencies", "graphic");
}

/**
 * Append to the Memory graph the last get value.
 */
void NVidiaMonitorApplet::updateMemVisualization()
{
	updateTempVisualizationEx("memory-usage", "percentage");
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
	Plasma::DataEngine * pEngine = dataEngine("nvidia-monitor");
	if(pEngine->isValid())
	{
		SourceMap::iterator itSources;
		for(itSources = m_smSources.begin(); itSources != m_smSources.end(); itSources++)
		{
			DataSource & dataSource = itSources->second;

			// Connect
			if((dataSource.p_bIsInApplet || dataSource.p_bIsInToolTip) && !dataSource.p_bIsConnected)
			{
				pEngine->connectSource(itSources->first, this, dataSource.p_iPollingInterval);
				dataSource.p_bIsConnected = true;
			}

			// Deconnect
			else if(!(dataSource.p_bIsInApplet || dataSource.p_bIsInToolTip) && dataSource.p_bIsConnected)
			{
				pEngine->disconnectSource(itSources->first, this);
				dataSource.p_bIsConnected = false;
			}

			// Update (deco reco witSourcesh the new value)
			else if((dataSource.p_bIsInApplet || dataSource.p_bIsInToolTip) && dataSource.p_bIsConnected && dataSource.p_bIsUpdated)
			{
				pEngine->disconnectSource	(itSources->first, this);
				pEngine->connectSource		(itSources->first, this, dataSource.p_iPollingInterval);
				dataSource.p_bIsUpdated = false;
			}
		}
	}
}

/**
 * Called when a source has been updated
 * Set the new value(s) and update visualizations
 */
void NVidiaMonitorApplet::dataUpdated(QString const & in_strSource, Plasma::DataEngine::Data const & in_newData)
{
	if(in_newData.isEmpty())
		return;

	if(in_strSource == "bumblebee")
	{
		if(in_newData["status"].toString() == "no_bb")
		{
			m_bIsBumblebee = false;
			m_bIsCgOn = true;
		}
		else 
		{
			m_bIsBumblebee = true;

			bool bCgOn = m_bIsCgOn;
			if(in_newData["status"].toString() == "on")
			{
				bCgOn = true;
			}
			else
			{
				bCgOn = false;
			}

			if(bCgOn != m_bIsCgOn)
			{
				m_bIsCgOn = bCgOn;
				updateVisualizationsConfig();
			}
		}
	}
	else
	{
		SourceMap::iterator itSources = m_smSources.find(in_strSource);
		if(itSources != m_smSources.end())
		{
			DataMap::iterator itData;

			// Update data in container
			for(itData = itSources->second.p_dmData.begin(); itData != itSources->second.p_dmData.end(); itData++)
				itData->second.p_iData = in_newData[itData->first].toInt();

			// Update visualization
			(this->*(itSources->second.p_pdUpdateVisualization))();
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
 * \param pParent Pointer on the dialog to setup
 */
void NVidiaMonitorApplet::createConfigurationInterface(KConfigDialog * pParent)
{
	QWidget * qwConfigUi = new QWidget();
	m_configUi.setupUi(qwConfigUi);

	// Define ui values from config values
	m_configUi.generalAnalogCheckBox	-> setCheckState (boolToCheckState	( m_bIsAnalog										));

	m_configUi.tempInAppletCheckBox		-> setCheckState (boolToCheckState	( m_smSources["temperature"].p_bIsInApplet			));
	m_configUi.tempInToolTipCheckBox	-> setCheckState (boolToCheckState	( m_smSources["temperature"].p_bIsInToolTip			));
	m_configUi.tempPollingSpinBox		-> setValue							( m_smSources["temperature"].p_iPollingInterval		);

	m_configUi.freqsInAppletCheckBox	-> setCheckState (boolToCheckState	( m_smSources["frequencies"].p_bIsInApplet			));
	m_configUi.freqsInToolTipCheckBox	-> setCheckState (boolToCheckState	( m_smSources["frequencies"].p_bIsInToolTip			));
	m_configUi.freqsPollingSpinBox		-> setValue							( m_smSources["frequencies"].p_iPollingInterval		);

	m_configUi.memInAppletCheckBox		-> setCheckState (boolToCheckState	( m_smSources["memory-usage"].p_bIsInApplet			));
	m_configUi.memInToolTipCheckBox		-> setCheckState (boolToCheckState	( m_smSources["memory-usage"].p_bIsInToolTip		));
	m_configUi.memPollingSpinBox		-> setValue							( m_smSources["memory-usage"].p_iPollingInterval	);

	// Connect save config event 
	connect(pParent, SIGNAL(applyClicked())	, this, SLOT(configUpdated()));
	connect(pParent, SIGNAL(okClicked())	, this, SLOT(configUpdated()));

	// Add this config page
	pParent->addPage(qwConfigUi, i18n("General"), icon());

	// Connect modify event to update the "Apply" button
	connect( m_configUi.generalAnalogCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);

	connect( m_configUi.tempInAppletCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.tempInToolTipCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.tempPollingSpinBox 			, SIGNAL(valueChanged(int))	, pParent	, SLOT(settingsModified())		);

	connect( m_configUi.freqsInAppletCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.freqsInToolTipCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.freqsPollingSpinBox 		, SIGNAL(valueChanged(int))	, pParent	, SLOT(settingsModified())		);

	connect( m_configUi.memInAppletCheckBox			, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.memInToolTipCheckBox		, SIGNAL(toggled(bool))		, pParent	, SLOT(settingsModified())		);
	connect( m_configUi.memPollingSpinBox 			, SIGNAL(valueChanged(int))	, pParent	, SLOT(settingsModified())		);
}

/**
 * Called when the user has saved the Applet configuration from the configuration UI
 * Apply and save the new configuration
 */
void NVidiaMonitorApplet::configUpdated()
{
	// Get config from ui
	m_bIsAnalog										= checkStateToBool ( m_configUi.generalAnalogCheckBox->checkState()		);

	m_smSources["temperature"].p_bIsInApplet		= checkStateToBool ( m_configUi.tempInAppletCheckBox->checkState()		);
	m_smSources["temperature"].p_bIsInToolTip		= checkStateToBool ( m_configUi.tempInToolTipCheckBox->checkState()		);
	m_smSources["temperature"].p_iPollingInterval	= m_configUi.tempPollingSpinBox->value()								;
	m_smSources["temperature"].p_bIsUpdated			= true																	;

	m_smSources["frequencies"].p_bIsInApplet		= checkStateToBool ( m_configUi.freqsInAppletCheckBox->checkState()		);
	m_smSources["frequencies"].p_bIsInToolTip		= checkStateToBool ( m_configUi.freqsInToolTipCheckBox->checkState()	);
	m_smSources["frequencies"].p_iPollingInterval	= m_configUi.freqsPollingSpinBox->value()								;
	m_smSources["frequencies"].p_bIsUpdated			= true																	;

	m_smSources["memory-usage"].p_bIsInApplet		= checkStateToBool ( m_configUi.memInAppletCheckBox->checkState()		);
	m_smSources["memory-usage"].p_bIsInToolTip		= checkStateToBool ( m_configUi.memInToolTipCheckBox->checkState()		);
	m_smSources["memory-usage"].p_iPollingInterval	= m_configUi.memPollingSpinBox->value()									;
	m_smSources["memory-usage"].p_bIsUpdated		= true																	;

	// Write new config
	KConfigGroup conf = config();
	conf.writeEntry ( "generalAnalog"	, m_bIsAnalog										);

	conf.writeEntry ( "tempInApplet"	, m_smSources["temperature"].p_bIsInApplet			);
	conf.writeEntry ( "tempInToolTip"	, m_smSources["temperature"].p_bIsInToolTip			);
	conf.writeEntry ( "tempPolling"		, m_smSources["temperature"].p_iPollingInterval		);

	conf.writeEntry ( "freqsInApplet"	, m_smSources["frequencies"].p_bIsInApplet			);
	conf.writeEntry ( "freqsInToolTip"	, m_smSources["frequencies"].p_bIsInToolTip			);
	conf.writeEntry ( "freqsPolling"	, m_smSources["frequencies"].p_iPollingInterval		);

	conf.writeEntry ( "memInApplet"		, m_smSources["memory-usage"].p_bIsInApplet			);
	conf.writeEntry ( "memInToolTip"	, m_smSources["memory-usage"].p_bIsInToolTip		);
	conf.writeEntry ( "memPolling"		, m_smSources["memory-usage"].p_iPollingInterval	);

	emit configNeedsSaving();

	// Update to use new config
	updateSources();
	updateVisualizationsConfig();
	update();
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

	DataMap & temp	= m_smSources["temperature"].p_dmData;
	DataMap & freqs	= m_smSources["frequencies"].p_dmData;
	DataMap & mem	= m_smSources["memory-usage"].p_dmData;

	if(m_smSources["temperature"].p_bIsInToolTip)
		content += i18n("Temperature : %1 °C<br/>", temp["temperature"].p_iData);
	if(m_smSources["frequencies"].p_bIsInToolTip)
		content += i18n("Frequencies : Lvl. %1 - %2 Mhz %3 Mhz %4 Mhz<br/>", freqs["level"].p_iData, freqs["graphic"].p_iData, freqs["memory"].p_iData, freqs["processor"].p_iData);
	if(m_smSources["memory-usage"].p_bIsInToolTip)
		content += i18n("Memory Usage : %1% (%2 / %3 Mo)<br/>", mem["percentage"].p_iData, mem["used"].p_iData, mem["total"].p_iData);

	Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent(title(), content, KIcon(icon())));
}

/*************************************************************************************************
 * From SM::Applet
 *************************************************************************************************/

/**
 * \todo Properly merge
 */
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

/**
 * \todo Properly merge
 */
void NVidiaMonitorApplet::reloadRender()
{
	removeLayout();
	configureLayout();

	if (!m_smSources["temperature"].p_bIsInApplet &&
		!m_smSources["frequencies"].p_bIsInApplet &&
		!m_smSources["memory-usage"].p_bIsInApplet)
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

