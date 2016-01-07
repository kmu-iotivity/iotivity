#include <functional>
#include <pthread.h>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"
#include "base64.h"
#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <signal.h>


/*
	펌웨어 받는거 외의 기능은 다 삭제하자. 펌웨어 받는 것이 일단은 센서데이터를 보내는 형식(put)으로 돼있는데
	임시로 이렇게 하고 추후에 변경하도록하자.
*/

using namespace OC;
using namespace std;
namespace PH = std::placeholders;
shared_ptr<OCResource> curResource;
condition_variable cv;

int getProcIdByName(string procName)
{
    int pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
                ifstream cmdFile(cmdPath.c_str());
                string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (procName == cmdLine)
                        pid = id;
                }
            }
        }
    }

    closedir(dp);

    return pid;
}


 	void onUpgrade(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
    {
  	
  		try
      	{
        	if(eCode == OC_STACK_OK)
          	{
  				//Run program
				system("/home/ysm/Desktop/temp_dir/out_dir/copiedhello22");  				


          	}
         	else
         	{
            	cout << "onUpgrade Response error: " << eCode << endl;
            	exit(-1);
         	}
     	}
    	catch(exception& e)
     	{
       		cout << "Exception: " << e.what() << " in onUpgrade" << endl;
     	}
 
 	}

class Upgrader
{

public:

	string resource_name, resource_uri, resource_typename, resource_interface;
    int count;
    OCResourceHandle resource_handle;
	OCRepresentation resource_rep;
	uint8_t resource_property;
	string resource_fbuffer;
	FILE *fout;


	Upgrader() : resource_name("rpi upgrade server"), 
			  resource_uri("/a/upgrade"), resource_handle(nullptr),
			  resource_typename("rpi.upgrade"), resource_fbuffer(""), count(0)

	{
		resource_interface = DEFAULT_INTERFACE;
		resource_property = OC_DISCOVERABLE;
		resource_rep.setUri(resource_uri);
		resource_rep.setValue("name", resource_name);
		resource_rep.setValue("fbuff", resource_fbuffer);
	}


	void createResource()
	{

		EntityHandler eh = bind(&Upgrader::entityHandler, this, PH::_1);

		OCStackResult result = OCPlatform::registerResource(resource_handle,
		resource_uri, resource_typename, resource_interface, eh, resource_property);

		if(OC_STACK_OK != result)
		{
			cout << "Resource creation was unsuccessful\n";
		}
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
			}

	    	try
    	   	{
  
        		cout << "----------------------------------------------" << endl;
				string encoded_str;           	 		

				if(rep.getValue("fbuff", encoded_str))
           	 	{


				
					cout << "\t\t\t\t" << "buf size : " << encoded_str.size() << endl;
					cout << "\t\t\t\t" << "count : "<< count << endl;
					
					string decoded_str = base64_decode(encoded_str);


					if(fwrite(decoded_str.c_str(), sizeof(char), n, fout) != n)
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
			

			int sensor_server_pid = getProcIdByName("sensor_server");


			cout << "PID of sensor_server : " << sensor_server_pid << endl;
			kill(sensor_server_pid, SIGKILL);
						
			system("rm /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server");

			system("mv /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server2 /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server3");
			system("chmod 777 /home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server3");
			system("/home/ysm/iotivity/out/linux/x86_64/release/resource/examples/sensor_server3");

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
		

				if(requestType == "PUT")
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
        OC::ModeType::Both,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

	Upgrader upgrader;

	upgrader.createResource();

	cout << "upgrader is created!\n";

	mutex blocker;
    condition_variable cv;
    unique_lock<std::mutex> lock(blocker);
    cout <<"Waiting" << std::endl;
    cv.wait(lock, []{return false;});

	return 0;
}
