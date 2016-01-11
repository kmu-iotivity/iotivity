#include <functional>
#include <pthread.h>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"
#include "base64.h"
#include <sys/stat.h>
#include <time.h>

/*
	observe, post 는 아직 미구현이고 get and put만 구현한 간단한 resourceserver를 구현할 것.
	slowResponse도 아직  미구현 그냥 노멀 리스폰스, secure 고려 안함

*/

using namespace OC;
using namespace std;
namespace PH = std::placeholders;


char* timeToString(struct tm *t) {
  static char s[20];

  sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
              t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
              t->tm_hour, t->tm_min, t->tm_sec
          );

  return s;
}


void print_fileinfo(char procname[])
{

  struct stat buf;
  char* filename = procname;


  if ( stat(filename, &buf) != 0 ) {
    switch (errno) {
      case ENOENT:
        fprintf(stderr, "File %s not found.\n", filename); break;
      case EINVAL:
        fprintf(stderr, "Invalid parameter to _stat.\n"); break;
      default:
        fprintf(stderr, "Unexpected error in _stat.\n");
    }
  }
  else {
    printf("%s\n", filename);
    printf( "\tTime Creation     : %s\n", timeToString(localtime(&buf.st_ctime)) );
    printf( "\tTime Last Written : %s\n", timeToString(localtime(&buf.st_mtime)) );
    printf( "\tTime Last Access  : %s\n", timeToString(localtime(&buf.st_atime)) );
  }

}


class LightResource
{

public:

	string resource_name, resource_uri, resource_typename, resource_interface;
	string resource_state;
    int resource_power;
	vector<string> resource_fbuffer;
    OCResourceHandle resource_handle;
	OCRepresentation resource_rep;
	uint8_t resource_property;
	FILE *fout;	

	LightResource() : resource_name("Seongmin's light"), resource_state("off"),
			  resource_power(0), resource_uri("/a/light"), resource_handle(nullptr),
			  resource_typename("core.light")

	{
		resource_interface = DEFAULT_INTERFACE;
		resource_property = OC_DISCOVERABLE;
		resource_rep.setUri(resource_uri);
		resource_rep.setValue("state", resource_state);
		resource_rep.setValue("power", resource_power);
		resource_rep.setValue("name", resource_name);
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


	void putFbuffer(OCRepresentation& rep)
    {
 
    	string encoded_str;
 
    #ifdef __arm__
        fout = fopen("/home/pi/Desktop/test_dir/out_dir/copiedhello22", "wb");
    #else
        fout = fopen("/home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server2", "wb");
    #endif
 
        if(fout == NULL)
        {
        	fputs("file open error!\n", stderr);
            return;
        }
 
        	else cout << "out file is opened" << endl;
 
        try
        {
 
        	cout << "----------------------------------------------" << endl;
 
 
            for(vector<string>::iterator it = resource_fbuffer.begin(); it != resource_fbuffer.end(); it++ )
            {
            	string decoded_str = base64_decode(*it);
                fwrite(decoded_str.c_str(), sizeof(char), decoded_str.size(), fout);
            }
 
 
        }
 
        catch(exception& e)
 		{
        	cout << e.what() << endl;
        }
 
            fclose(fout);
            cout << "\t\t\t\t" << "The firmware is transferred completely" << endl;
 
////            int sensor_server_pid = getProcIdByName("sensor_server");
 
////            cout << "PID of sensor_server : " << sensor_server_pid << endl;
////            kill(sensor_server_pid, SIGKILL);
 
////            system("rm /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server");

////            system("mv /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server2 /home/ysm/iotivity/out/linux/x    86_64/release/resource/examples/sensor_server3");
            system("chmod 777 /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server2");
			execl( "./sensor_server2", "sensor_server2", NULL );


			
////            system("/home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server3");
 
 
    }


    OCRepresentation getFbuffer()
    {
    	OCRepresentation rep;
        rep.setValue("fbuff", resource_fbuffer);
        return rep;
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


					if(rep.getValue("fbuff", resource_fbuffer) > 0) // file binary가 전송됐을 경우
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
	print_fileinfo(argv[0]);


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
