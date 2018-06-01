// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Dispatch", "0");

    // Make sure requests are serialized, this is required for the ordering test.
    properties->setProperty("Ice.ThreadPool.Server.Serialize", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints",
                                               getTestEndpoint() + ":" +
                                               getTestEndpoint("udp"));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyClassI), Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
