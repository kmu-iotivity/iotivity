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
std::shared_ptr<OCResource> curResource;
condition_variable cv;
bool ready;





void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{

 	string state;
 	int power;
 	string name;

    try
    {
        if(eCode == OC_STACK_OK)
        {
			cout << "----------------------------------------------" << endl;
            cout << "GET request was successful" << endl;
            cout << "esource URI: " << rep.getUri() << endl;

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

	ready = true;
	cv.notify_one();
}


void getRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        QueryParamsMap test;
        resource->get(test, &onGet);
    }
}
  

void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{

	string state;
	int power;
	string name;


    try
    {
        if(eCode == OC_STACK_OK)
        {
			cout << "----------------------------------------------" << endl;	
            cout << "PUT request was successful" << endl;

            rep.getValue("state", state);
            rep.getValue("power", power);
            rep.getValue("name", name);

            cout << "state: " << state << endl;
            cout << "power: " << power << endl;
            cout << "name: " << name << endl;

            
        }
        else
        {
            cout << "onPut Response error: " << eCode << endl;
            exit(-1);
        }
    }
    catch(exception& e)
    {
        cout << "Exception: " << e.what() << " in onPut" << endl;
    }

	ready = true;
	cv.notify_one();
}


void putRepresentation(std::shared_ptr<OCResource> resource, string state, int power)
{
	if(resource)
	{
    	OCRepresentation rep;
        rep.setValue("state", state);
        rep.setValue("power", power);
 
        resource->put(rep, QueryParamsMap(), &onPut);
    }
}


void foundResource(std::shared_ptr<OCResource> resource)
{
	string resourceURI;
	string hostAddress;


	if(resource)
	{
		cout << "A resource is founded." << endl;
	
		    
        resourceURI = resource->uri();
        cout << "URI of the resource: " << resourceURI << endl;

    
        hostAddress = resource->host();
        cout << "Host address of the resource: " << hostAddress << endl;
        
        

        
        // getResourceTypes() which is function to get the list of resource types. the return value is vector of resource types
        
		cout << "List of resource types: " << endl;
		for(auto &resourceTypes : resource->getResourceTypes())
        {
        	cout << resourceTypes << endl;
        }

         
        cout << "List of resource interfaces: " << endl;
        for(auto &resourceInterfaces : resource->getResourceInterfaces())
        {
        	std::cout << resourceInterfaces << endl;
        }

        if(resourceURI == "/a/light")
        {
        	curResource = resource;
               
            getRepresentation(curResource);
        }

	}
	
	else
	{
		cout << "Resource is invalid" << endl;
	}



	cv.notify_one();

}






int main(int argc, char* argv[])
{

	ostringstream requestURI;
	int menu;
	string input_state;
	int input_power;

	PlatformConfig cfg
	{
		ServiceType::InProc,
		ModeType::Client,
		"0.0.0.0", 0 , QualityOfService::LowQos
	};

	OCPlatform::Configure(cfg);

	requestURI << OC_RSRVD_WELL_KNOWN_URI;

    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, &foundResource);
    cout << "Finding Resource... " << endl;

	{
		mutex blocker;
		unique_lock<std::mutex> lock(blocker);
		cv.wait(lock);
	}

	while(curResource)
	{
		

		while(ready == false)
		{
			mutex blocker;
   	 		unique_lock<std::mutex> lock(blocker);
   	 		cv.wait(lock);
		}

		ready = false;	
		cout << "----------------------------------------------" << endl;
		cout << "select menu" << endl << "1 : Get representation" << endl;
		cout << "2 : Put representation" << endl;
		cout << "3 : Exit" << endl;

		cin >> menu;

		switch(menu)
		{
				
			case 1:
				getRepresentation(curResource);
				break;

			case 2:
				cout << "input state value (\"on\" or \"off\")" << endl;
				cin >> input_state;
				if( ((input_state.compare("on")==0) || (input_state.compare("off")==0)) != true  )
				{
					cout << "input_state is invalid! try again." << endl;
					ready = true;
					continue;
				}
				else
				{
				cout << "input power value" << endl;
				cin >> input_power;
				putRepresentation(curResource, input_state, input_power);
				}
				break;
				
			case 3:
				return 0;
		}
        
		
	}	



}
