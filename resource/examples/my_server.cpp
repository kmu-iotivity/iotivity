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
	bool resource_state;
        int resource_power;
        OCResourceHandle resource_handle;
	OCRepresentation resource_rep;
	uint8_t resource_property;
	
	LightResource() : resource_name("Seongmin's light"), resource_state(false),
			  resource_power(0), resource_uri("/a/light"), resource_handle("nullptr"),
			  resource_typename("core.light"), resource_interface(DEFAULT_INTERFACE)		
		 	  resource_property(OC_DISCOVERABLE)
	{
		resource_rep.setUri(resource_uri);
		resource_rep.setValue("state", resource_state);
		resource_rep.setValue("power", resource_power);
		resource_rep.setValue("name", resource_name);
	}


	void createResource()
	{

		EntityHandler eh = bind(&LightResource::entityHandler, this, PH::_1);

		OCStackResult result = OCPlatform::registerResource(resource_handle,
		resource_uri, resource_typename, resource_interface, cb, resource_property);

		if(OC_STACK_OK != result)
		{
			cout << "Resource creation was unsuccessful\n";
		}
	}




}

