/*-----------------------------------------------------------------------
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agceements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"; you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agceed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-----------------------------------------------------------------------*/
#include "etpFesppDirectedDiscoveryProtocolHandlers.h"
#include "vtkEtpDocument.h"
//#include <QApplication>

void etpFesppDirectedDiscoveryProtocolHandlers::on_GetResourcesResponse(const Energistics::Etp::v12::Protocol::DirectedDiscovery::GetResourcesResponse & grr)
{
	Energistics::Etp::v12::Datatypes::Object::GraphResource graphResource =  grr.m_resource;

	etp_document->receive_resources(graphResource.m_uri,
			graphResource.m_contentType,
			graphResource.m_name,
			graphResource.m_resourceType,
			graphResource.m_sourceCount,
			graphResource.m_targetCount,
			graphResource.m_contentCount,
			graphResource.m_uuid,
			graphResource.m_lastChanged
			);
/*
	std::cout << "*************************************************" << std::endl;
		std::cout << "GraphResource received : " << std::endl;
		std::cout << "uri : " << graphResource.m_uri << std::endl;
		std::cout << "contentType : " << graphResource.m_contentType << std::endl;
		std::cout << "name : " << graphResource.m_name << std::endl;
		std::cout << "type : " << graphResource.m_resourceType << std::endl;
		std::cout << "sourceCount : " << graphResource.m_sourceCount << std::endl;
		std::cout << "targetCount : " << graphResource.m_targetCount << std::endl;
		std::cout << "contentCount : " << graphResource.m_contentCount << std::endl;
		std::cout << "uuid : " << graphResource.m_uuid << std::endl;
		std::cout << "lastChanged : " << graphResource.m_lastChanged << std::endl;
		std::cout << "*************************************************" << std::endl;
		*/
/*
	if (getObjectWhenDiscovered) {
		Energistics::Etp::v12::Protocol::Store::GetObject getO;
		getO.m_uri = graphResource.m_uri;
		session->send(getO);
	}
*/
}
