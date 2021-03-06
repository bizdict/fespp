#include "ui_PQSelectionPanel.h"
#include "PQSelectionPanel.h"
// include API Resqml2
#include "resqml2_0_1/PolylineSetRepresentation.h"
#include "resqml2_0_1/TriangulatedSetRepresentation.h"
#include "resqml2_0_1/Horizon.h"
#include "resqml2_0_1/TectonicBoundaryFeature.h"
#include "resqml2_0_1/HorizonInterpretation.h"
#include "resqml2_0_1/FaultInterpretation.h"
#include "resqml2_0_1/PointSetRepresentation.h"
#include "resqml2_0_1/Grid2dRepresentation.h"
#include "common/AbstractObject.h"
#include "resqml2_0_1/SubRepresentation.h"
#include "resqml2_0_1/AbstractIjkGridRepresentation.h"
#include "resqml2/AbstractValuesProperty.h"
#include "resqml2_0_1/UnstructuredGridRepresentation.h"
#include "resqml2_0_1/WellboreTrajectoryRepresentation.h"
#include "resqml2_0_1/PropertyKindMapper.h"
#include "resqml2_0_1/SubRepresentation.h"

#include "resqml2/TimeSeries.h"

// include Qt
#include <QtGui>
#include <QFileInfo>
#include <QIcon>
#include <qtreewidget.h>
#include <QHeaderView>
#include <QMenu>
#include <QProgressDialog>
#include <QList>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qlayout.h>

// include ParaView
#include <PQToolsManager.h>
#include <pqPropertiesPanel.h>
#include <qobject.h>
#include <pqView.h>
#include <pqPipelineSource.h>
#include <pqApplicationCore.h>
#include <pqServerManagerModel.h>
#include <pqServer.h>
#include <pqOutputPort.h>
#include <pqActiveObjects.h>
#include <pqOutputPort.h>
#include <pqCoreUtilities.h>

// include VTK
#include <vtkSMProxy.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkInformation.h>

// include system
#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "VTK/VtkEpcDocumentSet.h"

namespace
{
pqPropertiesPanel* getpqPropertiesPanel()
{
	// get multi-block inspector panel
	pqPropertiesPanel *panel = 0;
	foreach(QWidget *widget, qApp->topLevelWidgets())
	{
		panel = widget->findChild<pqPropertiesPanel *>();

		if (panel)
		{
			break;
		}
	}
	return panel;
}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::constructor()
{
	setWindowTitle("Selection widget");
	QWidget* t_widget = new QWidget(this);
	Ui::panelSelection ui;
	ui.setupUi(t_widget);
	setWidget(t_widget);
	treeWidget = ui.treeWidget;

	treeWidget->setStyleSheet("QWidget::branch:has-siblings:!adjoins-item{border-image: url(:vline.png) 0;}"
			"QWidget::branch:has-siblings:adjoins-item{border-image: url(:branch-more.png) 0;}"
			"QWidget::branch:!has-children:!has-siblings:adjoins-item{border-image: url(:branch-end.png) 0;}"
			"QWidget::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings{border-image: none; image: url(:branch-closed.png);}"
			"QWidget::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings{border-image: none; image: url(:branch-open.png);}");


	treeWidget->header()->setStretchLastSection(false);
	treeWidget->header()->resizeSection(1, 20);
#if QT_VERSION >= 0x050000
	treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
	treeWidget->header()->setResizeMode(0, QHeaderView::Stretch);
#endif
	treeWidget->expandToDepth(0);
	treeWidget->header()->close();

	//	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(clicSelection(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onItemCheckedUnchecked(QTreeWidgetItem*, int)));

	//*** bouton recule/avance
	button_Time_After = ui.button_Time_After;
	button_Time_Before = ui.button_Time_Before;
	button_Time_Play = ui.button_Time_Play;
	button_Time_Pause = ui.button_Time_Pause;
	button_Time_Stop = ui.button_Time_Stop;
	QIcon icon1;
	icon1.addFile(QString::fromUtf8(":time-after.png"), QSize(), QIcon::Normal, QIcon::Off);
	QIcon icon2;
	icon2.addFile(QString::fromUtf8(":time-before.png"), QSize(), QIcon::Normal, QIcon::Off);
	QIcon icon3;
	icon3.addFile(QString::fromUtf8(":time-play.png"), QSize(), QIcon::Normal, QIcon::Off);
	QIcon icon4;
	icon4.addFile(QString::fromUtf8(":time-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
	QIcon icon5;
	icon5.addFile(QString::fromUtf8(":time-stop.png"), QSize(), QIcon::Normal, QIcon::Off);

	button_Time_After->setIcon(icon1);
	button_Time_Before->setIcon(icon2);
	button_Time_Play->setIcon(icon3);
	button_Time_Pause->setIcon(icon4);
	button_Time_Stop->setIcon(icon5);


	connect(button_Time_After, SIGNAL (released()), this, SLOT (handleButtonAfter()));
	connect(button_Time_Before, SIGNAL (released()), this, SLOT (handleButtonBefore()));
	connect(button_Time_Play, SIGNAL (released()), this, SLOT (handleButtonPlay()));
	connect(button_Time_Pause, SIGNAL (released()), this, SLOT (handleButtonPause()));
	connect(button_Time_Stop, SIGNAL (released()), this, SLOT (handleButtonStop()));
	//***

	//*** combo Box des différents Time_series
	time_series = ui.time_series;
	QStringList time_series_list;
	time_series->addItems(time_series_list);

	connect(time_series, SIGNAL (currentIndexChanged(int)), this, SLOT (timeChangedComboBox(int)));
	//***

	//*** Timer Play option
	timer = new QTimer(this);

	connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
	//***

	//*** slider des différents Time_series
	slider_Time_Step = ui.slider_Time_Step;
	slider_Time_Step->setTickInterval(10);
	slider_Time_Step->setSingleStep(1);
	slider_Time_Step->setMinimum(0);

	connect(slider_Time_Step, SIGNAL (sliderMoved(int)), this, SLOT (sliderMoved(int)));

	//***
	radioButtonCount = 0;

	indexFile = 0;

	save_time = 0;

	vtkEpcDocumentSet = new VtkEpcDocumentSet(0, 0, VtkEpcCommon::TreeView);

}

//******************************* ACTIONS ************************************

//----------------------------------------------------------------------------
void PQSelectionPanel::clicSelection(QTreeWidgetItem* item, int column)
{
	auto pickedBlocks = itemUuid[item];
	if (pickedBlocks!="root")
	{
		pqPipelineSource * source = findPipelineSource("EpcDocument");
		if (source)
		{
			pqActiveObjects *activeObjects = &pqActiveObjects::instance();
			activeObjects->setActiveSource(source);
		}
		if (!(uuidToFilename[pickedBlocks] == pickedBlocks))
		{
			emit selectionName(uuidToFilename[pickedBlocks], pickedBlocks, pcksave[uuidToFilename[pickedBlocks]]);
		}
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::onItemCheckedUnchecked(QTreeWidgetItem * item, int column)
{
	auto uuid = itemUuid[item];

	if (!(uuid == ""))
	{
		if (item->checkState(0) == Qt::Checked)
		{
			this->loadUuid(uuid);
		}
		else
		{
			// Property exist
			if (uuidItem[uuid]->childCount() > 0 && mapUuidWithProperty[uuid] != "")
			{
				auto uuidProperty = mapUuidWithProperty[uuid];
				this->removeUuid(uuidProperty);

				mapUuidWithProperty[uuid] = "";
				mapUuidParentButtonInvisible[uuid]->setChecked(true);
				uuidItem[uuid]->setData(0, Qt::CheckStateRole, QVariant());
			}

			this->removeUuid(uuid);

		}
	}

}

//----------------------------------------------------------------------------
void PQSelectionPanel::deleteTreeView()
{
	treeWidget->clear();

	indexFile = 0;
	radioButtonCount = 0;
	allFileName.clear();
	uuidToFilename.clear();
	filenameToUuids.clear();
	filenameToUuidsPartial.clear();

	uuidItem.clear();
	itemUuid.clear();
	uuidParentItem.clear();

	mapUuidProperty.clear();
	mapUuidWithProperty.clear();

	displayUuid.clear();

	radioButton_to_id.clear();

	mapUuidParentButtonInvisible.clear();

	pcksave.clear();
	uuidParent_to_groupButton.clear();
	radioButton_to_uuid.clear();

	delete vtkEpcDocumentSet;
	vtkEpcDocumentSet = new VtkEpcDocumentSet(0, 0, VtkEpcCommon::TreeView);
}

//----------------------------------------------------------------------------
void PQSelectionPanel::checkedRadioButton(int rbNo)
{
	emit button_Time_Pause->released();
	if (radioButton_to_id.contains(rbNo))
	{
		std::string uuid = radioButton_to_id[rbNo];

		std::string uuidParent = itemUuid[uuidParentItem[uuid]];
		uuidParentItem[uuid]->setCheckState(0, Qt::Checked);

		std::string uuidOld = mapUuidWithProperty[uuidParent];

		if (!(uuidOld == ""))
		{
			if (ts_timestamp_to_uuid.count(uuidOld) > 0) // time series
			{
				QDateTime date = QDateTime::fromString(time_series->currentText());
				time_t time = date.toTime_t();

				this->removeUuid(ts_timestamp_to_uuid[uuidOld][time]);

				this->updateTimeSeries(uuidOld, false);
			}
			else // basic property
			{
				this->removeUuid(uuidOld);
			}
		}

		mapUuidWithProperty[uuidParent] = uuid;

		if (ts_timestamp_to_uuid.count(uuid) > 0) // time series
		{
			this->updateTimeSeries(uuid, true);

			QDateTime date = QDateTime::fromString(time_series->currentText());
			time_t time = date.toTime_t();

			this->loadUuid(ts_timestamp_to_uuid[uuid][time]);
		}
		else // basic property
		{
			this->loadUuid(uuid);
		}
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::handleButtonAfter()
{
	auto idx = time_series->currentIndex() ;
	if (++idx < time_series->count() )
	{
		// change the TimeStamp
		time_series->setCurrentIndex(idx);
	}
	else
	{
		time_Changed = false;
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::handleButtonBefore()
{
	auto idx = time_series->currentIndex() ;
	if (idx != 0)
	{
		--idx;
		// change the TimeStamp
		time_series->setCurrentIndex(idx);
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::handleButtonPlay()
{
	timer->start(1000);
	time_Changed = true;
}

//----------------------------------------------------------------------------
void PQSelectionPanel::handleButtonPause()
{
	timer->stop();
}

//----------------------------------------------------------------------------
void PQSelectionPanel::handleButtonStop()
{
	time_series->setCurrentIndex(0);
	timer->stop();
}

//----------------------------------------------------------------------------
void PQSelectionPanel::updateTimer()
{
	if (time_Changed)
	{
		emit button_Time_After->released();
	}
	else
	{
		timer->stop();
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::timeChangedComboBox(int)
{
	slider_Time_Step->setValue(time_series->currentIndex());

	QDateTime date = QDateTime::fromString(time_series->currentText());
	time_t time = date.toTime_t();

	if (time != save_time)
	{
		// remove old time properties
		if (save_time != 0)
		{
			for (auto &ts :ts_displayed)
			{
				this->removeUuid(ts_timestamp_to_uuid[ts][save_time]);
				this->loadUuid(ts_timestamp_to_uuid[ts][time]);
			}
		}
		else
		{
			// load time properties
			for (auto &ts :ts_displayed)
			{
				this->loadUuid(ts_timestamp_to_uuid[ts][time]);
			}
		}
		save_time = time;
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::sliderMoved(int value)
{
	time_series->setCurrentIndex(value);
}

//----------------------------------------------------------------------------
void PQSelectionPanel::updateTimeSeries(const std::string & uuid, bool newUuid)
{
	if (newUuid)
	{
		ts_displayed.push_back(uuid);
	}else
	{
		auto index_to_delete = ts_displayed.indexOf(uuid);
		ts_displayed.remove(index_to_delete);
	}

	QList<time_t> list;
	QStringList large_list;

	for (const auto &uuid_displayed : ts_displayed)
	{
		foreach(time_t t, ts_timestamp_to_uuid[uuid_displayed].keys())
			list << t;
	}

	qSort(list.begin(), list.end());

	for (auto &it : list)
	{
		QDateTime dt;
		dt.setTime_t(it);
		large_list << dt.toString();
	}

	large_list.removeDuplicates();

	disconnect(time_series, SIGNAL (currentIndexChanged(int)), this, SLOT (timeChangedComboBox(int)));
	time_series->clear();
	time_series->addItems(large_list);
	connect(time_series, SIGNAL (currentIndexChanged(int)), this, SLOT (timeChangedComboBox(int)));

	slider_Time_Step->setMaximum(large_list.size());
}

//****************************************************************************

//********************************* TreeView *********************************
//----------------------------------------------------------------------------
void PQSelectionPanel::addFileName(const std::string & fileName)
{
	if (std::find(allFileName.begin(), allFileName.end(),fileName)==allFileName.end())
	{
		vtkEpcDocumentSet->addEpcDocument(fileName);
		allFileName.push_back(fileName);
		uuidToFilename[fileName] = fileName;


		QMap<std::string, std::string> name_to_uuid;
		auto treeView = vtkEpcDocumentSet->getTreeView();
		for (auto &feuille : treeView)
		{

			if (feuille->getTimeIndex()<0 )
			{
				populateTreeView(feuille->getParent(), feuille->getParentType(), feuille->getUuid(), feuille->getName(), feuille->getType());
			}else
			{
				if(name_to_uuid.count(feuille->getName())<=0)
				{
					populateTreeView(feuille->getParent(), feuille->getParentType(), feuille->getUuid(), feuille->getName(), feuille->getType());
					name_to_uuid[feuille->getName()]=feuille->getUuid();
				}

				ts_timestamp_to_uuid[name_to_uuid[feuille->getName()]][feuille->getTimestamp()] = feuille->getUuid();
			}
		}
	}
}

void PQSelectionPanel::populateTreeView(const std::string & parent, VtkEpcCommon::Resqml2Type parentType, const std::string & uuid, const std::string &  name, VtkEpcCommon::Resqml2Type type)
{
	if (uuid != "")
	{
		if (!uuidItem[uuid]){
			if (parentType==VtkEpcCommon::Resqml2Type::PARTIAL && !uuidItem[parent])
			{
			}
			else
			{
				if (!uuidItem[parent])
				{
					QTreeWidgetItem *treeItem = new QTreeWidgetItem(treeWidget);

					treeItem->setExpanded(true);
					treeItem->setText(0, parent.c_str());
					treeItem->setText(1, "");
					uuidItem[parent] = treeItem;
					itemUuid[treeItem] = parent;
				}

				QIcon icon;
				if (type==VtkEpcCommon::PROPERTY || type==VtkEpcCommon::TIME_SERIES)
				{
					addTreeProperty(uuidItem[parent], parent, name, uuid);
				}
				else
				{
					switch (type)
					{
					case VtkEpcCommon::Resqml2Type::GRID_2D:
					{
						icon.addFile(QString::fromUtf8(":Grid2D.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::POLYLINE_SET:
					{
						icon.addFile(QString::fromUtf8(":Polyline.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::TRIANGULATED_SET:
					{
						icon.addFile(QString::fromUtf8(":Triangulated.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::WELL_TRAJ:
					{
						icon.addFile(QString::fromUtf8(":WellTraj.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::IJK_GRID:
					{
						icon.addFile(QString::fromUtf8(":IjkGrid.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::UNSTRUC_GRID:
					{
						icon.addFile(QString::fromUtf8(":UnstructuredGrid.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					case VtkEpcCommon::Resqml2Type::SUB_REP:
					{
						icon.addFile(QString::fromUtf8(":SubRepresentation.png"), QSize(), QIcon::Normal, QIcon::Off);
						break;
					}
					default:
						break;
					}

					QTreeWidgetItem *treeItem = new QTreeWidgetItem();

					treeItem->setText(0, name.c_str());
					treeItem->setIcon(0, icon);
					treeItem->setCheckState(0, Qt::Unchecked);
					treeItem->setFlags(treeItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);


					uuidItem[parent]->addChild(treeItem);

					uuidItem[uuid] = treeItem;
					itemUuid[treeItem] = uuid;

					filenameToUuids[uuidToFilename[uuid]].push_back(uuid);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::addTreeProperty(QTreeWidgetItem *parent, const std::string & parentUuid,const std::string & name, const std::string & uuid)
{
	QButtonGroup *buttonGroup;
	if (uuidParent_to_groupButton.count(parentUuid) < 1)
	{
		buttonGroup = new QButtonGroup();
		buttonGroup->setExclusive(true);
		QRadioButton *radioButtonInvisible = new QRadioButton("invisible");
		buttonGroup->addButton(radioButtonInvisible, radioButtonCount++);
		mapUuidParentButtonInvisible[parentUuid] = radioButtonInvisible;
		connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(checkedRadioButton(int)));
	}
	else
	{
		buttonGroup = uuidParent_to_groupButton[itemUuid[parent]];
	}
	QTreeWidgetItem *treeItem = new QTreeWidgetItem();
	QRadioButton *radioButton = new QRadioButton(QString::fromStdString(name));
	radioButton_to_id[radioButtonCount] = uuid;
	buttonGroup->addButton(radioButton, radioButtonCount);
	parent->addChild(treeItem);
	treeWidget->setItemWidget(treeItem, 0, radioButton);
	uuidItem[uuid] = treeItem;
	uuidParentItem[uuid] = parent;
	itemUuid[treeItem] = uuid;
	uuidToFilename[uuid] = uuidToFilename[itemUuid[parent]];

	radioButtonCount++;
	radioButton_to_uuid[radioButton] = uuid;
	parent->setData(0, Qt::CheckStateRole, QVariant());

	uuidParent_to_groupButton[itemUuid[parent]] = buttonGroup;
}
//****************************************************************************
void PQSelectionPanel::deleteUUID(QTreeWidgetItem *item)
{
	for (int o = 0; item->childCount(); o++)
	{
		deleteUUID(item->child(0));
	}
	delete item;
}
//********************************* Interfacing ******************************
//----------------------------------------------------------------------------
void PQSelectionPanel::loadUuid(const std::string & uuid)
{
	pqPipelineSource * source = findPipelineSource("EpcDocument");
	if (source)
	{
		pqActiveObjects *activeObjects = &pqActiveObjects::instance();
		activeObjects->setActiveSource(source);

		PQToolsManager* manager = PQToolsManager::instance();
		auto fesppReader = manager->getFesppReader();

		if (fesppReader)
		{
			pqActiveObjects *activeObjects = &pqActiveObjects::instance();
			activeObjects->setActiveSource(fesppReader);

			// add uuid to property panel
			vtkSMProxy* fesppReaderProxy = fesppReader->getProxy();

			vtkSMPropertyHelper( fesppReaderProxy, "uuidList" ).SetStatus(uuid.c_str(),1);

			fesppReaderProxy->UpdatePropertyInformation();
			fesppReaderProxy->UpdateVTKObjects();
			getpqPropertiesPanel()->update();
			getpqPropertiesPanel()->apply();
		}
	}
}

//----------------------------------------------------------------------------
void PQSelectionPanel::removeUuid(const std::string & uuid)
{
	pqPipelineSource * source = findPipelineSource("EpcDocument");
	if (source)
	{
		pqActiveObjects *activeObjects = &pqActiveObjects::instance();
		activeObjects->setActiveSource(source);

		PQToolsManager* manager = PQToolsManager::instance();
		auto fesppReader = manager->getFesppReader();

		if (fesppReader)
		{
			pqActiveObjects *activeObjects = &pqActiveObjects::instance();
			activeObjects->setActiveSource(fesppReader);

			// add file to property
			vtkSMProxy* fesppReaderProxy = fesppReader->getProxy();

			vtkSMPropertyHelper( fesppReaderProxy, "uuidList" ).SetStatus(uuid.c_str(),0);

			fesppReaderProxy->UpdatePropertyInformation();
			fesppReaderProxy->UpdateVTKObjects();
			getpqPropertiesPanel()->update();
			getpqPropertiesPanel()->apply();
		}
	}
}

//****************************************************************************

//************************** Pipeline & Server *******************************

//----------------------------------------------------------------------------
pqPipelineSource * PQSelectionPanel::findPipelineSource(const char *SMName)
{
	pqApplicationCore *core = pqApplicationCore::instance();
	pqServerManagerModel *smModel = core->getServerManagerModel();

	QList<pqPipelineSource*> sources = smModel->findItems<pqPipelineSource*>(this->getActiveServer());
	foreach(pqPipelineSource *s, sources)
	{
		if (strcmp(s->getSMName().toStdString().c_str(), SMName) == 0) return s;
	}
	return nullptr;
}

//----------------------------------------------------------------------------
pqServer * PQSelectionPanel::getActiveServer()
{
	pqApplicationCore *app = pqApplicationCore::instance();
	pqServerManagerModel *smModel = app->getServerManagerModel();
	pqServer *server = smModel->getItemAtIndex<pqServer*>(0);

	return server;
}

//****************************************************************************//----------------------------------------------------------------------------
void PQSelectionPanel::uuidKO(const std::string & uuid)
{
	uuidItem[uuid]->setDisabled(true);
	for (int idx_child = 0; idx_child < uuidItem[uuid]->childCount(); ++idx_child)
	{
		uuidItem[uuid]->child(idx_child)->setHidden(true);
	}
}
