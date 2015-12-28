#include <string>
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> curResource;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
std::mutex curResourceLock;






void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{


 	bool state;
 	int power;
 	string name;

    try
    {
        if(eCode == OC_STACK_OK)
        {
            cout << "GET request was successful" << endl;
            cout << "Resource URI: " << rep.getUri() << endl;

            rep.getValue("state", state);
            rep.getValue("power", power);
            rep.getValue("name", name);

            cout << "state: " << state << endl;
            cout << "power: " << power << endl;
            cout << "name: " << name << endl;

        }
        else
        {
            cout << "onGET Response error: " << eCode << endl;
            exit(-1);
        }
    }
    catch(exception& e)
    {
        cout << "Exception: " << e.what() << " in onGet" << endl;
    }
}


void getRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
    	cout << "Getting representation..."<< endl;
 
        QueryParamsMap test;
        resource->get(test, &onGet);
    }
}
  




void foundResource(std::shared_ptr<OCResource> resource)
{
	string resourceURI;
	string hostAddress;


	if(resource)
	{
		cout << "A resource is discovered\n";
	
		    
        resourceURI = resource->uri();
        cout << "URI of the resource: " << resourceURI << std::endl;

    
        hostAddress = resource->host();
        cout << "Host address of the resource: " << hostAddress << std::endl;
        
        

        /*
         getResourceTypes() which is function to get the list of resource types. the return value is vector of resource types
        */

		cout << "List of resource types: " << std::endl;
		for(auto &resourceTypes : resource->getResourceTypes())
        {
        	cout << resourceTypes << std::endl;
        }

         
        cout << "List of resource interfaces: " << std::endl;
        for(auto &resourceInterfaces : resource->getResourceInterfaces())
        {
        	std::cout << resourceInterfaces << std::endl;
        }

        if(resourceURI == "/a/light")
        {
        	curResource = resource;
               
            getRepresentation(resource);
        }

	}
	
	else
	{
		cout << "Resource is invalid\n";
	}

}






int main(int argc, char* argv[])
{

	ostringstream requestURI;


	PlatformConfig cfg
	{
		ServiceType::InProc,
		ModeType::Client,
		"0.0.0.0", 0 , QualityOfService::LowQos
	};

	OCPlatform::Configure(cfg);

	requestURI << OC_RSRVD_WELL_KNOWN_URI;

    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, &foundResource);
    cout<< "Finding Resource... " << endl;


    mutex blocker;
    condition_variable cv;
    unique_lock<std::mutex> lock(blocker);
    cv.wait(lock);


	return 0;
}
