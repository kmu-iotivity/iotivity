#include <functional>
#include <pthread.h>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"


/*
	observe, post 는 아직 미구현이고 get and put만 구현한 간단한 resourceserver를 구현할 것.
	slowResponse도 아직  미구현 그냥 노멀 리스폰스, secure 고려 안함

*/

using namespace OC;
using namespace std;
namespace PH = std::placeholders;


class LightResource
{

public:

	string resource_name, resource_uri, resource_typename, resource_interface;
	string resource_state;
    int resource_power, count;
    OCResourceHandle resource_handle;
	OCRepresentation resource_rep;
	uint8_t resource_property;
	string resource_fbuffer;
	FILE *fout;


	LightResource() : resource_name("Seongmin's light"), resource_state("off"),
			  resource_power(0), resource_uri("/a/light"), resource_handle(nullptr),
			  resource_typename("core.light"), resource_fbuffer(""), count(0)

	{
		resource_interface = DEFAULT_INTERFACE;
		resource_property = OC_DISCOVERABLE;
		resource_rep.setUri(resource_uri);
		resource_rep.setValue("state", resource_state);
		resource_rep.setValue("power", resource_power);
		resource_rep.setValue("name", resource_name);
		resource_rep.setValue("fbuff", resource_fbuffer);
	}


	void createResource()
	{

		EntityHandler eh = bind(&LightResource::entityHandler, this, PH::_1);

		OCStackResult result = OCPlatform::registerResource(resource_handle,
		resource_uri, resource_typename, resource_interface, eh, resource_property);

		if(OC_STACK_OK != result)
		{
			cout << "Resource creation was unsuccessful\n";
		}
	}

	OCRepresentation get()
    {
		resource_rep.setValue("state", resource_state);
       	resource_rep.setValue("power", resource_power);
        return resource_rep;
    }


    OCRepresentation getFbuffer()
    {

		OCRepresentation rep;
    	rep.setValue("count", count);
		rep.setValue("fbuff", resource_fbuffer);
    	return rep;
    }



	void putFbuffer(OCRepresentation& rep)
	{
		
		rep.getValue("count", count);
	
		int n;
		rep.getValue("n", n);

		if(count > 0)
		{
			if(count == 1)
			{
				fout = fopen("/home/ysm/Desktop/temp_dir/out_dir/copiedhello2", "wb");

			    if(fout == NULL)
       			{
     	       		fputs("file open error!\n", stderr);
        	  		return;
        		}
				
				else cout << "out file is opened" << endl;
			}

	    	try
    	   	{
  
        		cout << "----------------------------------------------" << endl;
				string temp_fbuffer;           	 		

				if(rep.getValue("fbuff", temp_fbuffer))
           	 	{


				
					cout << "\t\t\t\t" << "buf size : " << temp_fbuffer.size() << endl;
            		cout << "\t\t\t\t" << "file buffer : " << resource_fbuffer.c_str() << endl;
					cout << "\t\t\t\t" << "count : "<< count << endl;
					
					if(fwrite(temp_fbuffer.c_str(), sizeof(char), n, fout) != n)
						cout << "쓰기 갯수가 다르다 !\n";
            	}
  
            	else
            	{
               		cout << "\t\t\t\t" << "fbuff not found in the representation" << endl;
            	}
 
        	}	
 
        	catch(exception& e)
        	{
            	cout << e.what() << endl;
       		}
	
		}

		else if(count == -1)
		{
			fclose(fout);
			resource_fbuffer = "";
			cout << "\t\t\t\t" << "The firmware is transferred completely" << endl;
		}

	}


	


    void put(OCRepresentation& rep)
    {
        
		try 
		{

			cout << "----------------------------------------------" << endl;
			if(rep.getValue("state", resource_state))
			{
				cout << "\t\t\t\t" << "state: " << resource_state << endl;
       		}
          
			else
       	 	{
       	   		cout << "\t\t\t\t" << "state not found in the representation" << endl;
       		}

       	 	if(rep.getValue("power", resource_power))
      	  	{
       	 		cout << "\t\t\t\t" << "power: " << resource_power << endl;
          	}
           
			else
        	{
        		cout << "\t\t\t\t" << "power not found in the representation" << endl;
       	 	}
        }

    	catch(exception& e)
    	{
    		cout << e.what() << endl;
    	}
    
	}


private:

	OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
	{

		OCEntityHandlerResult ehResult = OC_EH_ERROR;
	
		if(request)
		{

			string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

        	if(requestFlag & RequestHandlerFlag::RequestFlag)		//observe 기능은 미구현이므로 ObserverFlag에 대한 처리는 안하였다.
       	 	{
	
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());
		
				if(requestType == "GET")
				{
					//normal response로 처리하는 것임, slowresponse는 스레드를 생성하여 핸들러를 사용하는데, 이 프로그램에선 아직 사용하지 않겠다.
					pResponse->setErrorCode(200); 
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
					{
						ehResult = OC_EH_OK;
					}
				}


				else if(requestType == "PUT")
				{

					OCRepresentation rep = request->getResourceRepresentation();

					if(rep.getValue("count",count) > 0)		// count 값이 0 초과가 아니라 count라는 값이 전달됐을 경우를 말한다. (헷갈리기 쉽다.)
					{

						putFbuffer(rep);
 	                    pResponse->setErrorCode(200);
    	                pResponse->setResponseResult(OC_EH_OK);
        	            pResponse->setResourceRepresentation(getFbuffer());
     
                    	if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    	{   
                    		ehResult = OC_EH_OK;
                    	}
					}

					else
					{
						put(rep);
						pResponse->setErrorCode(200);
						pResponse->setResponseResult(OC_EH_OK);
						pResponse->setResourceRepresentation(get());
	
						if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
   	    	    		{
   	   						ehResult = OC_EH_OK;
   	         			}
					}
				}

			}

		}

		else
		{	
			cout << "Request invalid" << endl;
		}
	
		return ehResult;	
	}

};

int main(int argc, char* argv[])
{



    PlatformConfig cfg 
	{
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);


	LightResource virtual_light;

	virtual_light.createResource();

	cout << "virtual light is created!\n";

	mutex blocker;
    condition_variable cv;
    unique_lock<std::mutex> lock(blocker);
    cout <<"Waiting" << std::endl;
    cv.wait(lock, []{return false;});

	return 0;
}
