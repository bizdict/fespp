﻿#include "VtkGrid2DRepresentation.h"

#include <sstream>

// include VTK library
#include <vtkInformation.h>

// include F2i-consulting Energistics Standards API
#include <common/EpcDocument.h>

// include F2i-consulting Energistics Standards ParaView Plugin
#include "VtkGrid2DRepresentationPoints.h"
#include "VtkGrid2DRepresentationCells.h"

//----------------------------------------------------------------------------
VtkGrid2DRepresentation::VtkGrid2DRepresentation(const std::string & fileName, const std::string & name, const std::string & uuid, const std::string & uuidParent, common::EpcDocument *pckEPCRep, common::EpcDocument *pckEPCSubRep) :
VtkResqml2MultiBlockDataSet(fileName, name, uuid, uuidParent), epcPackageRepresentation(pckEPCRep), epcPackageSubRepresentation(pckEPCSubRep), grid2DPoints(getFileName(), name, uuid+"-Points", uuidParent, epcPackageRepresentation, epcPackageSubRepresentation)
{

	//BUG in PARAVIEW
//	std::stringstream grid2DCellsUuid;
//	grid2DCellsUuid << uuid << "-Cells";
//	
//	grid2DCells = new VtkGrid2DRepresentationCells(getFileName(), name, grid2DCellsUuid.str(), uuidParent, epcPackage);
}

VtkGrid2DRepresentation::~VtkGrid2DRepresentation()
{
	if (epcPackageRepresentation != nullptr) {
		epcPackageRepresentation = nullptr;
	}

	if (epcPackageSubRepresentation != nullptr) {
		epcPackageSubRepresentation = nullptr;
	}

	/*
	if (grid2DCells != nullptr) {
		delete grid2DCells;
		grid2DCells = nullptr;
	}
	*/
}

//----------------------------------------------------------------------------
void VtkGrid2DRepresentation::createTreeVtk(const std::string & uuid, const std::string & uuidParent, const std::string & name, const VtkEpcCommon::Resqml2Type & type)
{
	if (uuid != getUuid())
	{
		grid2DPoints.createTreeVtk(uuid, uuidParent, name, type);
//		grid2DCells->createTreeVtk(uuid, uuidParent, name, type);
	}
}

//----------------------------------------------------------------------------
int VtkGrid2DRepresentation::createOutput(const std::string & uuid)
{
	grid2DPoints.createOutput(uuid);
//	grid2DCells->createOutput(uuid);
	
	return 1;
}
//----------------------------------------------------------------------------
// load toutes les sub Representation
void VtkGrid2DRepresentation::visualize(const std::string & uuid)
{
	createOutput(uuid);

	this->attach();
}

//----------------------------------------------------------------------------
void VtkGrid2DRepresentation::remove(const std::string & uuid)
{
	if (uuid == getUuid())
	{
		this->detach();
		std::stringstream grid2DPointsUuid;
		grid2DPointsUuid << uuid << "-Points";

		grid2DPoints.remove(uuid);

		//	std::stringstream grid2DCellsUuid;
		//	grid2DCellsUuid << uuid << "-Cells";
		//	
		//	grid2DCells = new VtkGrid2DRepresentationCells(getFileName(), getName(), grid2DCellsUuid.str(), getParent(), epcPackage);
	}
}

//----------------------------------------------------------------------------
void VtkGrid2DRepresentation::attach()
{
	unsigned int index =0;

	vtkOutput->SetBlock(index, grid2DPoints.getOutput());
	vtkOutput->GetMetaData(index++)->Set(vtkCompositeDataSet::NAME(),grid2DPoints.getUuid().c_str());

//	vtkOutput->SetBlock(index, grid2DCells->getOutput());
//	vtkOutput->GetMetaData(index++)->Set(vtkCompositeDataSet::NAME(),grid2DCells->getUuid().c_str());
}

void VtkGrid2DRepresentation::addProperty(const std::string & uuidProperty, vtkDataArray* dataProperty)
{
	grid2DPoints.addProperty(uuidProperty, dataProperty);
}

long VtkGrid2DRepresentation::getAttachmentPropertyCount(const std::string & uuid, const VtkEpcCommon::FesppAttachmentProperty propertyUnit)
{
	return grid2DPoints.getAttachmentPropertyCount(uuid, propertyUnit);
}
