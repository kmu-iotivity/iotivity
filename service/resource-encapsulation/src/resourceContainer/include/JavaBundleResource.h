//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef JAVABUNDLERESOURCE_H_
#define JAVABUNDLERESOURCE_H_

#include <map>
#include <vector>
#include <string>
#include <jni.h>
#include "BundleResource.h"
#include "ResourceContainerImpl.h"

using namespace std;

namespace OIC
{
    namespace Service
    {
        class JavaBundleResource: public BundleResource
        {
        public:
            JavaBundleResource();
            JavaBundleResource(JNIEnv *env, jobject obj, jobject bundleResource, string bundleId,
                    jobjectArray attributes);
            virtual ~JavaBundleResource();

            virtual RCSResourceAttributes& getAttributes();

            virtual void setAttribute(std::string key, RCSResourceAttributes::Value&&);

            virtual RCSResourceAttributes::Value getAttribute(const std::string& key);
            virtual void initAttributes();
        private:
            // needs to be a GlobalRef
            jobject bundleResource;
            jobjectArray attributes;
            jclass bundleResourceClass;
            jmethodID attributeSetter;
            jmethodID attributeGetter;
            string m_bundleId;
        };
    }
}

#endif
