#include <unistd.h>
#include <pthread.h>
#include <string>
#include <map>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
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

// Set of strings for each of device info fields
std::string  deviceName = "Etri Server";
std::string  deviceType = "oic.wk.etri";
std::string  specVersion = "ocf.1.1.0";
std::vector<std::string> dataModelVersions = {"ocf.res.1.1.0", "ocf.sh.1.1.0"};
std::string  protocolIndependentID = "fa008167-3bbf-4c9d-8604-c9bcb96cb712";

// OCPlatformInfo Contains all the platform info to be stored
OCPlatformInfo platformInfo;

class Resource
{
    protected:
	OCResourceHandle m_resourceHandle;
	OCRepresentation m_rep;
	std::shared_ptr<OC::OCResource> m_resource;
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)=0;
};


class DcResource;
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

	    /*if(rep != NULL){
	      delete m_rep;
	      m_rep = *rep;
	      m_rep.getValue("mValue",mValue);
	      std::cout << "Test" <<std::endl;
	      std::cout << mValue << std::endl;
	      m_resource = OC::OCPlatform::constructResourceObject(
	      res->host(),
	      res->uri(),
	      res->connectivityType(),
	      false,
	      rep->getResourceTypes(),
	      rep->getResourceInterfaces());

	      }else{
	      mValue =14;  
	      }*/

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
	std::map<std::string,DcResource> *mLocalDcResource;

	DeviceServerResource(std::map<std::string,DcResource> &localDcResource)
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
   // private:
	OCRepresentation get()
	{
	    return m_rep;
	}

	void updateChildren(){
	    m_rep.clearChildren();
	    for(auto it = mLocalDcResource -> begin();it!= mLocalDcResource-> end();it++){
		auto dc = it->second;
		m_rep.addChild(dc.get());
		m_rep.addChild(dc.mDeviceResource -> get());

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




OCStackResult SetDeviceInfo()
{
    OCStackResult result = OC_STACK_ERROR;

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

std::map<std::string,DcResource> localDcResource;
int main (){ 

    int input;

    std::cout << "    1 - AgentServer\n";
    std::cout << "    2 - DeviceServer\n";
    std::cin >> input; 


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

	OCStackResult result = SetDeviceInfo();

	if(result != OC_STACK_OK){
	    cout << "Device Registration failed\n";
	    return -1;
	}	    

	if(input == 1){
	    std::cout << "Agent Server Resource Generating... \n";
	}else if(input ==2){

	    std::cout << "Device Server Resource Generating... \n";
	    // register local resources
	    auto devices = getAvailableDevices();
	    for(int i =0 ; i<devices.size();i++){
		std::cout<< devices[i] <<" registered" <<std::endl;

		DcResource *dc =::new DcResource(devices[i],false);
		DeviceResource *dv = ::new DeviceResource(devices[i],*dc);
		dc->bindResource(*dv);
		localDcResource.insert(pair<std::string,DcResource>(devices[i],*dc));
	    }

	    //register DeviceServer resource
	    DeviceServerResource *ds = new DeviceServerResource(localDcResource);
	}else{
	    std::cout << "invalid value\n";
	}


	//DeletePlatformInfo();
	if(input ==1 || input ==2){
	    std::mutex blocker;
	    std::condition_variable cv;
	    std::unique_lock<std::mutex> lock(blocker);
	    //std::cout <<"Waiting" << std::endl;
	    cv.wait(lock, []{return false;});
	}

	//std::cout << "waiting. Press \"Enter\""<< std::endl;
	// Ignoring all input except for EOL.
	//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);

    }
    catch(OCException& e){
	oclog() << "Exception in main : " << e.what();
    }
    return 0;

}



