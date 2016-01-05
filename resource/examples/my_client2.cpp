//#include <string>a
#include <map>
#include <iostream>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"
#include "base64.h"

#define BUF_SIZE 1024

using namespace OC;
using namespace std;
std::shared_ptr<OCResource> curResource;
condition_variable cv;
condition_variable cv2;
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



}



void onFput(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
 
	string s_buff;
 	int count;
    try
    {
    	if(eCode == OC_STACK_OK)
        {
        	cout << "----------------------------------------------" << endl;
            rep.getValue("fbuff", s_buff);
            cout << "file buffer : " << s_buff.c_str() << endl;
			rep.getValue("count", count);
			cout << "count : " << count << endl;

 			if(count == -1)
			{
				cout << "The firmware is transferred completely" << endl;
	       		count = 0;	
			}
		}
 
        else
        {
        	cout << "onFput Response error: " << eCode << endl;
            exit(-1);
        }
    }
    catch(exception& e)
    {
    	cout << "Exception: " << e.what() << " in onFput" << endl;
    }
 
    ready = true;
    cv.notify_one();
	cv2.notify_one();

 }




void putFirmware(std::shared_ptr<OCResource> resource, string input_filepath)
{
	FILE *fin;
	unsigned char buff[1024];
	string encoded_str;
	int n;
	int count = 0;
	
	if(resource)
	{
	
	
		#ifdef __arm__
		if((fin = fopen("/home/pi/Desktop/test_dir/in_dir/helloworld","rb")) == NULL)
		#else
		if((fin = fopen("/home/ysm/Desktop/temp_dir/in_dir/helloworld","rb")) == NULL)
		#endif

		{
			fputs("file open error!\n", stderr);
			return;
		}	 



		while((n = fread(buff, sizeof(char), BUF_SIZE, fin)) != 0)
		{


			OCRepresentation rep;
			count++;

			string encoded_str = base64_encode(buff, n);


			cout <<"buf size : " << encoded_str.size() << endl;
			rep.setValue("fbuff", encoded_str);
			rep.setValue("count",count);
			rep.setValue("n", n);
			resource->put(rep, QueryParamsMap(), &onFput);

            mutex blocker;
            unique_lock<std::mutex> lock(blocker);
            cv2.wait(lock);
	
		}

		if(n==0)
		{

			OCRepresentation rep;
			count = -1;
            rep.setValue("count",count);
			resource->put(rep, QueryParamsMap(), &onFput);

		}
			

		fclose(fin);
	
	}

}


int main(int argc, char* argv[])
{

	ostringstream requestURI;
	int menu;
	string input_state;
	int input_power;
	string input_filepath;

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
		cout << "3 : Put Firmware" << endl;
		cout << "4 : Exit" << endl;

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
				cout << "input filepath" << endl;
				cin >> input_filepath;
				putFirmware(curResource, input_filepath);
				break;
				
			case 4:
				return 0;
		}
        
		
	}	



}
