#include <unistd.h>
#include <pthread.h>
#include <string>
#include <map>
#include <cstdlib>
#include <mutex>
#include <condition_variable>

#include <time.h>

#include "iotivity_config.h"
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

int GET_REQ_PERIOD = 10000;
int RESOURCE_SCAN_PERIOD = 10000;

string DEVICE_TEMP = "temp";
string DEVICE_HUMI = "humi";
string DEVICE_FAN = "fan";

string DEVICE_A = "A";
string DEVICE_B = "B";
string DEVICE_C = "C";
string DEVICE_D = "D";

string AGENT_REQ_TYPE_SENSOR = "sensor";
string AGENT_REQ_TYPE_RESOURCE = "resource";

string RESOURCE_URI_MANAGEMENT = "/mgt/";
string RESOURCE_URI_DEVICE = "/device/";
string RESOURCE_URI_SERVER = "/server";
string RESOURCE_URI_AGENT = "/agent";


string RESOURCE_TYPE_MANAGEMENT = "etri.device.mgt";
string RESOURCE_TYPE_DEVICE = "etri.device";
string RESOURCE_TYPE_SERVER = "etri.server";
string RESOURCE_TYPE_AGENT = "etri.agent";


// platform Info
std::string gPlatformId = "0A3E0D6F-DBF5-404E-8719-D6880042463A";
std::string gManufacturerName = "OCF";
std::string gManufacturerLink = "https://www.iotivity.org";
std::string gModelNumber = "myModelNumber";
std::string gDateOfManufacture = "2016-01-15";
std::string gPlatformVersion = "myPlatformVersion";
std::string gOperatingSystemVersion = "myOS";
std::string gHardwareVersion = "myHardwareVersion";
std::string gFirmwareVersion = "1.0";
std::string gSupportLink = "https://www.iotivity.org";
std::string gSystemTime = "2016-01-15T11.01";


// OCPlatformInfo Contains all the platform info to be stored
OCPlatformInfo platformInfo;

class DcResource;
class RemoteResource;
std::map<std::string,DcResource*> localDcResource;
std::map<std::string,RemoteResource*> remoteResource;

long now(){
    auto timeInMilis = std::time(nullptr);
    return timeInMilis;
}

class Resource
{
    protected:
	OCResourceHandle m_resourceHandle;
	OCRepresentation m_rep;
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)=0;

    public:
	void unregisterResource(){
	    OCStackResult result = OCPlatform::unregisterResource(m_resourceHandle);
	    if(OC_STACK_OK != result)
	    {
		throw std::runtime_error(
			std::string("Device Resource failed to unregister/delete") + std::to_string(result));
	    }
	}
};


class DeviceResource : public Resource
{
    public:
	int mValue;
	DcResource *mDcResource;

	DeviceResource(std::string device, DcResource& dcResource)
	{
	    mDcResource = &dcResource;
	    std::string resourceURI = RESOURCE_URI_DEVICE+device;
	    std::string resourceTypeName = RESOURCE_TYPE_DEVICE;
	    std::string resourceInterface = DEFAULT_INTERFACE;
	    //m_rep = rep;
	    EntityHandler cb = std::bind(&DeviceResource::entityHandler, this,PH::_1);
	    uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;


	    OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
		    resourceURI,
		    resourceTypeName,
		    resourceInterface,
		    cb,
		    resourceProperty);

	    if(OC_STACK_OK != result)
	    {
		throw std::runtime_error(
			std::string("Light Resource failed to start")+std::to_string(result));
	    }else{
		mValue =99;
		std::vector<std::string> resourceTypes = {resourceTypeName};
		std::vector<std::string> resourceInterfaces = {resourceInterface};

		m_rep.setUri(resourceURI);
		m_rep.setResourceTypes(resourceTypes);
		m_rep.setResourceInterfaces(resourceInterfaces);
	    }
	}

	~DeviceResource(){
	    unregisterResource();
	    std::cout << "DeviceResource 소멸자 호출" << std::endl;
	}
	//private:
	OCRepresentation get()
	{
	    m_rep.setValue("value",mValue);
	    return m_rep;
	}



    protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
	    OCEntityHandlerResult ehResult = OC_EH_ERROR;
	    if(request)
	    {
		std::cout << "In entity handler for DeviceResource, URI is : "
		    << request->getResourceUri() << std::endl;

		if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
		{
		    auto pResponse = std::make_shared<OC::OCResourceResponse>();
		    pResponse->setRequestHandle(request->getRequestHandle());
		    pResponse->setResourceHandle(request->getResourceHandle());

		    if(request->getRequestType() == "GET")
		    {
			std::cout<<"DeviceResource Get Request"<<std::endl;
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{

			    ehResult = OC_EH_OK;
			}
		    }
		    else
		    {
			std::cout << "DeviceResource unsupported request type"
			    << request->getRequestType() << std::endl;
			pResponse->setResponseResult(OC_EH_ERROR);
			OCPlatform::sendResponse(pResponse);
			ehResult = OC_EH_ERROR;
		    }
		}
		else
		{
		    std::cout << "DeviceResource unsupported request flag" <<std::endl;
		}
	    }

	    return ehResult;
	}
};

class DcResource : public Resource
{
    public:
	bool mValue;
	std::string mDevice;
	DeviceResource *mDeviceResource;

	DcResource(std::string device,bool enable)
	{

	    std::string resourceURI = RESOURCE_URI_MANAGEMENT+device;
	    std::string resourceTypeName = RESOURCE_TYPE_MANAGEMENT;
	    std::string resourceInterface = DEFAULT_INTERFACE;
	    mDevice = device;
	    EntityHandler cb = std::bind(&DcResource::entityHandler, this,PH::_1);
	    uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;


	    OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
		    resourceURI,
		    resourceTypeName,
		    resourceInterface,
		    cb,
		    resourceProperty);

	    if(OC_STACK_OK != result)
	    {
		throw std::runtime_error(
			std::string("DcResource failed to start")+std::to_string(result));
	    }else{
		mValue = enable; 
		std::vector<std::string> resourceTypes = {resourceTypeName};
		std::vector<std::string> resourceInterfaces = {resourceInterface};

		m_rep.setUri(resourceURI);
		m_rep.setResourceTypes(resourceTypes);
		m_rep.setResourceInterfaces(resourceInterfaces);

	    }
	}

	~DcResource(){
	    std::cout << "DcResource 소멸자호출" << std::endl;
	    unregisterResource();
	    delete mDeviceResource;
	}

	void bindResource(DeviceResource &deviceResource){
	    mDeviceResource = &deviceResource;
	}
	// private:
	OCRepresentation get()
	{
	    m_rep.setValue("value",mValue);
	    return m_rep;
	}

	void put(const OCRepresentation& rep)
	{
	    rep.getValue("value", mValue);
	}


    protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
	    OCEntityHandlerResult ehResult = OC_EH_ERROR;
	    if(request)
	    {
		std::cout << "In entity handler for DcResource, URI is : "
		    << request->getResourceUri() << std::endl;

		if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
		{
		    auto pResponse = std::make_shared<OC::OCResourceResponse>();
		    pResponse->setRequestHandle(request->getRequestHandle());
		    pResponse->setResourceHandle(request->getResourceHandle());

		    if(request->getRequestType() == "GET")
		    {
			std::cout<<"DcResource Get Request"<<std::endl;
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{

			    ehResult = OC_EH_OK;
			}
		    }
		    else if(request->getRequestType() == "PUT")
		    {
			std::cout <<"DcResource Put Request"<<std::endl;
			put(request->getResourceRepresentation());

			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{
			    ehResult = OC_EH_OK;
			}
		    }
		    else
		    {
			std::cout << "DcResource unsupported request type"
			    << request->getRequestType() << std::endl;
			pResponse->setResponseResult(OC_EH_ERROR);
			OCPlatform::sendResponse(pResponse);
			ehResult = OC_EH_ERROR;
		    }
		}
		else
		{
		    std::cout << "DcResource unsupported request flag" <<std::endl;
		}
	    }

	    return ehResult;
	}
};


class DeviceServerResource:public Resource
{
    public:
	std::map<std::string,DcResource*> *mLocalDcResource;

	DeviceServerResource(std::map<std::string,DcResource*> &localDcResource)
	{


	    mLocalDcResource = &localDcResource;
	    std::string resourceURI = RESOURCE_URI_SERVER;
	    std::string resourceTypeName = RESOURCE_TYPE_SERVER;
	    std::string resourceInterface = DEFAULT_INTERFACE;
	    EntityHandler cb = std::bind(&DeviceServerResource::entityHandler, this,PH::_1);
	    uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

	    OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
		    resourceURI,
		    resourceTypeName,
		    resourceInterface,
		    cb,
		    resourceProperty);

	    if(OC_STACK_OK != result)
	    {
		throw std::runtime_error(
			std::string("DeviceServerResource failed to start")+std::to_string(result));
	    }else{

		std::vector<std::string> resourceTypes = {resourceTypeName};
		std::vector<std::string> resourceInterfaces = {resourceInterface};

		m_rep.setUri(resourceURI);
		m_rep.setResourceTypes(resourceTypes);
		m_rep.setResourceInterfaces(resourceInterfaces);

	    }
	}
	~DeviceServerResource(){
	    unregisterResource();
	}

	// private:
	OCRepresentation get()
	{
	    return m_rep;
	}

	void updateChildren(){
	    m_rep.clearChildren();
	    for(auto it = mLocalDcResource -> begin();it!= mLocalDcResource-> end();it++){
		auto dc = it->second;
		m_rep.addChild(dc->get());
		m_rep.addChild(dc->mDeviceResource -> get());

	    }
	}

    protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
	    OCEntityHandlerResult ehResult = OC_EH_ERROR;
	    if(request)
	    {
		std::cout << "In entity handler for DeviceServerResource, URI is : "
		    << request->getResourceUri() << std::endl;

		if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
		{
		    auto pResponse = std::make_shared<OC::OCResourceResponse>();
		    pResponse->setRequestHandle(request->getRequestHandle());
		    pResponse->setResourceHandle(request->getResourceHandle());

		    if(request->getRequestType() == "GET")
		    {
			std::cout<<"DeviceServerResource Get Request"<<std::endl;
			updateChildren();
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{

			    ehResult = OC_EH_OK;
			}
		    }
		    else
		    {
			std::cout << "DeviceServerResource unsupported request type"
			    << request->getRequestType() << std::endl;
			pResponse->setResponseResult(OC_EH_ERROR);
			OCPlatform::sendResponse(pResponse);
			ehResult = OC_EH_ERROR;
		    }
		}
		else
		{
		    std::cout << "DeviceServerResource unsupported request flag" <<std::endl;
		}
	    }

	    return ehResult;
	}

};

class RemoteResource{
    public:
	bool isRaspberry;
	long timestamp = now();
	std::string mHost;

	OCResource::Ptr device;
	OCResource::Ptr manage;

	OCRepresentation mRep_device;
	OCRepresentation mRep_manage;	

	RemoteResource(OCRepresentation &rep_device, OCRepresentation &rep_manage, std::string host) {
	    isRaspberry = true;
	    mRep_device = rep_device;
	    mRep_manage = rep_manage;
	    mHost = host;
	    createConstructResourceObject(mRep_device,true);
	    createConstructResourceObject(mRep_manage,false);

	}

	RemoteResource(OCRepresentation &rep_device, std::string host){
	    isRaspberry = false;
	    mRep_device = rep_device;
	    mHost = host;
	    createConstructResourceObject(mRep_device,true);
	}

	~RemoteResource(){

	}


	void createConstructResourceObject(OCRepresentation &rep, bool isDevice){
	    OCConnectivityType connectivityType = CT_ADAPTER_IP;
	    PutCallback cb = std::bind(&RemoteResource::onPut, this,PH::_1,PH::_2,PH::_3);

	    if(isDevice){
		device = OC::OCPlatform::constructResourceObject(
			mHost,
			rep.getUri(),
			connectivityType,
			false,
			rep.getResourceTypes(),
			rep.getResourceInterfaces());

	    }else{
		mRep_manage.setValue("value",true);
		manage = OC::OCPlatform::constructResourceObject(
			mHost,
			rep.getUri(),
			connectivityType,
			false,
			rep.getResourceTypes(),
			rep.getResourceInterfaces());


		std::cout << "tttttttttttttttttttttttttttttttttt\n";

		if(manage){
		    QueryParamsMap q;
		    manage->put(mRep_manage,q,cb,OC::QualityOfService::LowQos);
		}else{
		    std::cout << "Error: Resource Object construction returned null\n";
		}

	    }
	}
	void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
	{
	    if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CHANGED)
	    {
	    //std::cout << "PUT request was successful" << std::endl;
	    }
	    else
	    {
	    //std::cout << "onPut Response error: " << eCode << std::endl;
	    }
	}
};

class AgentServerResource:public Resource
{
    public:
	std::map<std::string,RemoteResource*> *mRemoteResource;

	AgentServerResource(std::map<std::string,RemoteResource*> &remoteResource)
	{


	    mRemoteResource = &remoteResource;
	    std::string resourceURI = RESOURCE_URI_AGENT;
	    std::string resourceTypeName = RESOURCE_TYPE_AGENT;
	    std::string resourceInterface = DEFAULT_INTERFACE;
	    EntityHandler cb = std::bind(&AgentServerResource::entityHandler, this,PH::_1);
	    uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

	    OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
		    resourceURI,
		    resourceTypeName,
		    resourceInterface,
		    cb,
		    resourceProperty);

	    if(OC_STACK_OK != result)
	    {
		throw std::runtime_error(
			std::string("AgentServerResource failed to start")+std::to_string(result));
	    }else{

		std::vector<std::string> resourceTypes = {resourceTypeName};
		std::vector<std::string> resourceInterfaces = {resourceInterface};

		m_rep.setUri(resourceURI);
		m_rep.setResourceTypes(resourceTypes);
		m_rep.setResourceInterfaces(resourceInterfaces);

	    }
	}

	~AgentServerResource(){
	    unregisterResource();
	}
	// private:
	OCRepresentation get()
	{
	    return m_rep;
	}

	void updateChildren(){
	    m_rep.clearChildren();
	    for(auto it = mRemoteResource->begin();it!= mRemoteResource->end();it++){
		auto rr = it->second;

	    }
	}

    protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
	    OCEntityHandlerResult ehResult = OC_EH_ERROR;
	    if(request)
	    {
		std::cout << "In entity handler for AgentServerResource, URI is : "<< request->getResourceUri() << std::endl;

		if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
		{
		    auto pResponse = std::make_shared<OC::OCResourceResponse>();
		    pResponse->setRequestHandle(request->getRequestHandle());
		    pResponse->setResourceHandle(request->getResourceHandle());

		    if(request->getRequestType() == "GET")
		    {
			std::cout<<"AgentServerResource Get Request"<<std::endl;
			updateChildren();
			pResponse->setResourceRepresentation(get());
			if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
			{

			    ehResult = OC_EH_OK;
			}
		    }
		    else
		    {
			std::cout << "AgentServerResource unsupported request type"
			    << request->getRequestType() << std::endl;
			pResponse->setResponseResult(OC_EH_ERROR);
			OCPlatform::sendResponse(pResponse);
			ehResult = OC_EH_ERROR;
		    }
		}
		else
		{
		    std::cout << "AgentServerResource unsupported request flag" <<std::endl;
		}
	    }

	    return ehResult;
	}

};





/*
   class registerTask
   {
   public:

   std::shared_ptr<OCResource> mResource;
   registerTask(std::shared_ptr<OCResource> &resource){
   mResource = resource;
   QueryParamsMap q;
   GetCallback cb = std::bind(&registerTask::onGet, this,PH::_1,PH::_2,PH::_3);
   resource -> get(q,cb);

   }


// Callback handler on GET request
void onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep,int eCode)
{
try
{
if(eCode == OC_STACK_OK)

{
std::cout << "GET request was successful" << std::endl;
std::cout << "Resource URI: " << rep.getUri() << std::endl;
std::cout << "HOST : " << rep.getHost() << std::endl;
//std::cout << "Interface : " << rep.getHost() << std::endl;
registerRemoteResource(rep);
// Get resource header options
}
else
{
std::cout << "onGET Response error: " << eCode << std::endl;
std::exit(-1);
}
}
catch(std::exception& e)
{
std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
}
}



void registerRemoteResource(const OCRepresentation& rep){ 

std::map<std::string,OCRepresentation> rep_manage;
std::map<std::string,OCRepresentation> rep_device;

for(const OCRepresentation& r : rep.getChildren())
{
std::string uri(r.getUri());
std::string device = uri.substr(uri.find_last_of("/")+1);

if(std::string::npos != uri.find(RESOURCE_URI_MANAGEMENT))
{
rep_manage.insert(pair<std::string,OCRepresentation>(device,r));
}

if(std::string::npos != uri.find(RESOURCE_URI_DEVICE))
{
rep_device.insert(pair<std::string,OCRepresentation>(device,r));
}


//std::cout << rep.getHost() << std::endl;
//std::cout << r.getHost()<< std::endl;
//std::cout << rep.getUri() << std::endl;
//std::cout << r.getUri() << std::endl;

}

for(auto const& element : rep_device){
    std::string device =  element.first;

    if(rep_manage.count(device)){
	//rsapberry pi
	if(!checkResourceAdded(device)){	
	    RemoteResource *rr = ::new RemoteResource(rep_device.find(device)-> second,rep_manage.find(device) -> second,mResource); 
	    remoteResource.insert(pair<std::string,RemoteResource*>(device,rr));
	}else{
	    remoteResource.find(device) -> second -> timestamp = now();

	}

    }else{
	//ESP32

    }

} 

}

bool checkResourceAdded(std::string device){
    bool result = false;
    if(remoteResource.count(device)){
	result = true;
    }
    return result;
}

};
*/
bool checkResourceAdded(std::string device){
    bool result = false;
    if(remoteResource.count(device)){
	result = true;
    }
    return result;
}

void registerRemoteResource(const OCRepresentation& rep){ 

    std::map<std::string,OCRepresentation> rep_manage;
    std::map<std::string,OCRepresentation> rep_device;
    std::string host = rep.getHost();
    for(const OCRepresentation& r : rep.getChildren())
    {
	std::string uri(r.getUri());
	std::string device = uri.substr(uri.find_last_of("/")+1);

	if(std::string::npos != uri.find(RESOURCE_URI_MANAGEMENT))
	{
	    rep_manage.insert(pair<std::string,OCRepresentation>(device,r));
	}

	if(std::string::npos != uri.find(RESOURCE_URI_DEVICE))
	{
	    rep_device.insert(pair<std::string,OCRepresentation>(device,r));
	}


	//std::cout << rep.getHost() << std::endl;
	//std::cout << r.getHost()<< std::endl;
	//std::cout << rep.getUri() << std::endl;
	//std::cout << r.getUri() << std::endl;

    }

    for(auto const& element : rep_device){
	std::string device =  element.first;

	if(rep_manage.count(device)){
	    //rsapberry pi
	    if(!checkResourceAdded(device)){
		std::cout << device << "@@@@@@@@@@@@@@@@@\n";	
		RemoteResource *rr = ::new RemoteResource(rep_device.find(device)-> second,rep_manage.find(device) -> second,host); 
		remoteResource.insert(pair<std::string,RemoteResource*>(device,rr));
	    }else{
		remoteResource.find(device) -> second -> timestamp = now();

	    }

	}else{
	    //ESP32

	}

    } 

}
// Callback handler on GET request
void onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep,int eCode)
{
    try
    {
	if(eCode == OC_STACK_OK)

	{
	    std::cout << "GET request was successful" << std::endl;
	    std::cout << "Resource URI: " << rep.getUri() << std::endl;
	    std::cout << "HOST : " << rep.getHost() << std::endl;
	    //std::cout << "Interface : " << rep.getHost() << std::endl;
	    registerRemoteResource(rep);
	    // Get resource header options
	}
	else
	{
	    std::cout << "onGET Response error: " << eCode << std::endl;
	    std::exit(-1);
	}
    }
    catch(std::exception& e)
    {
	std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }
}




std::mutex curResourceLock;

void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n";
    std::string resourceURI;
    std::string hostAddress;
    try
    {	

	/*if(resource){
	  {
	  std::lock_guard<std::mutex> lock(curResourceLock);
	  }
	  }*/

	// Do some operations with resource object.
	if(resource)
	{
	    std::cout<<"DISCOVERED Resource:"<<std::endl;
	    // Get the resource URI
	    resourceURI = resource->uri();
	    std::cout << "\tURI of the resource: " << resourceURI << std::endl;

	    // Get the resource host address
	    hostAddress = resource->host();
	    std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

	    // Get the resource types
	    std::cout << "\tList of resource types: " << std::endl;
	    for(auto &resourceTypes : resource->getResourceTypes())
	    {
		std::cout << "\t\t" << resourceTypes << std::endl;
	    }

	    // Get the resource interfaces
	    std::cout << "\tList of resource interfaces: " << std::endl;
	    for(auto &resourceInterfaces : resource->getResourceInterfaces())
	    {
		std::cout << "\t\t" << resourceInterfaces << std::endl;
	    }

	    // Get Resource current host
	    std::cout << "\tHost of resource: " << std::endl;
	    std::cout << "\t\t" << resource->host() << std::endl;

	    // Get Resource Endpoint Infomation
	    std::cout << "\tList of resource endpoints: " << std::endl;
	    for(auto &resourceEndpoints : resource->getAllHosts())
	    {
		std::cout << "\t\t" << resourceEndpoints << std::endl;
	    }

	    if(std::string::npos != resource ->uri().find(RESOURCE_URI_SERVER))
	    {
		//new registerTask(resource);
		QueryParamsMap q;
		resource -> get(q,&onGet);

		std::cout << "\tAddress of selected resource: " << resource->host() << std::endl;
	    }
	}
	else
	{
	    // Resource is invalid
	    std::cout << "Resource is invalid" << std::endl;
	}

    }
    catch(std::exception& e)
    {
	std::cerr << "Exception in foundResource: "<< e.what() << std::endl;
    }
}


void scanDeviceServerResource(){

    std::ostringstream requestURI;
    requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt="+ RESOURCE_TYPE_SERVER;

    OCPlatform::findResource("",requestURI.str(),CT_DEFAULT,&foundResource);

}


void DuplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

OCStackResult SetPlatformInfo(std::string platformID, std::string manufacturerName,
	std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
	std::string platformVersion, std::string operatingSystemVersion,
	std::string hardwareVersion, std::string firmwareVersion, std::string supportUrl,
	std::string systemTime)
{
    DuplicateString(&platformInfo.platformID, platformID);
    DuplicateString(&platformInfo.manufacturerName, manufacturerName);
    DuplicateString(&platformInfo.manufacturerUrl, manufacturerUrl);
    DuplicateString(&platformInfo.modelNumber, modelNumber);
    DuplicateString(&platformInfo.dateOfManufacture, dateOfManufacture);
    DuplicateString(&platformInfo.platformVersion, platformVersion);
    DuplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion);
    DuplicateString(&platformInfo.hardwareVersion, hardwareVersion);
    DuplicateString(&platformInfo.firmwareVersion, firmwareVersion);
    DuplicateString(&platformInfo.supportUrl, supportUrl);
    DuplicateString(&platformInfo.systemTime, systemTime);

    return OC_STACK_OK;
}


void DeletePlatformInfo()
{
    delete[] platformInfo.platformID;
    delete[] platformInfo.manufacturerName;
    delete[] platformInfo.manufacturerUrl;
    delete[] platformInfo.modelNumber;
    delete[] platformInfo.dateOfManufacture;
    delete[] platformInfo.platformVersion;
    delete[] platformInfo.operatingSystemVersion;
    delete[] platformInfo.hardwareVersion;
    delete[] platformInfo.firmwareVersion;
    delete[] platformInfo.supportUrl;
    delete[] platformInfo.systemTime;
}




// Set of strings for each of device info fields
std::string  deviceName = "Device Server";
std::string  deviceType = RESOURCE_TYPE_SERVER;
std::string  specVersion = "1.0.0";
std::vector<std::string> dataModelVersions = { "etri.ocf.1.0.0"};
std::string  protocolIndependentID = "fa008167-3bbf-4c9d-8604-c9bcb96cb712";
OCStackResult SetDeviceInfo(int opt)
{
    OCStackResult result = OC_STACK_ERROR;

    if(opt == 1){
	deviceName = "Agent Server";
	deviceType = RESOURCE_TYPE_AGENT;
    }

    OCResourceHandle handle = OCPlatform::getResourceHandleAtUri(OC_RSRVD_DEVICE_URI);
    if (handle == NULL)
    {
	cout << "Failed to find resource " << OC_RSRVD_DEVICE_URI << endl;
	return result;
    }

    result = OCPlatform::bindTypeToResource(handle, deviceType);
    if (result != OC_STACK_OK)
    {
	cout << "Failed to add device type" << endl;
	return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DEVICE_NAME, deviceName);
    if (result != OC_STACK_OK)
    {
	cout << "Failed to set device name" << endl;
	return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DATA_MODEL_VERSION,
	    dataModelVersions);
    if (result != OC_STACK_OK)
    {
	cout << "Failed to set data model versions" << endl;
	return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_SPEC_VERSION, specVersion);
    if (result != OC_STACK_OK)
    {
	cout << "Failed to set spec version" << endl;
	return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_PROTOCOL_INDEPENDENT_ID,
	    protocolIndependentID);
    if (result != OC_STACK_OK)
    {
	cout << "Failed to set piid" << endl;
	return result;
    }

    return OC_STACK_OK;
}

std::vector<string> getAvailableDevices(){
    std::vector<string> devices = {
	DEVICE_TEMP,
	DEVICE_HUMI,
	DEVICE_FAN
    } ;
    return devices;

}


void startDeviceServer(){ 
    std::cout << "Device Server Resource Generating... \n";
    // register local resources
    auto devices = getAvailableDevices();
    for(int i =0 ; i<devices.size();i++){
	std::cout<< devices[i] <<" registered" <<std::endl;

	DcResource *dc =::new DcResource(devices[i],false);
	DeviceResource *dv = ::new DeviceResource(devices[i],*dc);
	dc->bindResource(*dv);
	localDcResource.insert(pair<std::string,DcResource*>(devices[i],dc));
    }

    //register DeviceServer resource
    DeviceServerResource *ds = new DeviceServerResource(localDcResource);

}

void startAgentServer(){

    startDeviceServer();

    std::cout << "Agent Server Resource Generating... \n";
    AgentServerResource *as = new AgentServerResource(remoteResource);

}


int main (){ 

    int opt;

    std::cout << "    1 - AgentServer\n";
    std::cout << "    2 - DeviceServer\n";
    std::cin >> opt; 


    PlatformConfig cfg(
	    OC::ServiceType::InProc,
	    OC::ModeType::Both,
	    nullptr
	    );
    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP | OCTransportAdapter::OC_ADAPTER_TCP);
    cfg.QoS = OC::QualityOfService::LowQos;
    OCPlatform::Configure(cfg);


    try
    {
	OC_VERIFY(OCPlatform::start() == OC_STACK_OK);
	std::cout << "Starting server & setting platform info\n";


	/*OCStackResult result = SetPlatformInfo(gPlatformId, gManufacturerName, gManufacturerLink,
	  gModelNumber, gDateOfManufacture, gPlatformVersion, gOperatingSystemVersion,
	  gHardwareVersion, gFirmwareVersion, gSupportLink, gSystemTime);

	  result = OCPlatform::registerPlatformInfo(platformInfo);

	  if(result != OC_STACK_OK){
	  cout <<  "Platform Registration failed\n";
	  return -1;
	  }
	  */

	OCStackResult result = SetDeviceInfo(opt);

	if(result != OC_STACK_OK){
	    cout << "Device Registration failed\n";
	    return -1;
	}	    
	if(opt == 1){
	    startAgentServer();
	    scanDeviceServerResource();
	}else if(opt ==2){
	    startDeviceServer();
	}else{
	    std::cout << "invalid value\n";
	    return -1;
	}

	DeletePlatformInfo();
	std::mutex blocker;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(blocker);
	std::cout <<"Waiting" << std::endl;
	cv.wait(lock, []{return false;});

	OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);

    }
    catch(OCException& e){
	oclog() << "Exception in main : " << e.what();
    }
    return 0;

}



