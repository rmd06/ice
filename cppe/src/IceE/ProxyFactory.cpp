// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ProxyFactory.h>
#include <IceE/Thread.h>
#include <IceE/Time.h>
#include <IceE/Instance.h>
#include <IceE/Proxy.h>
#include <IceE/Reference.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/LocatorInfo.h>
#include <IceE/Properties.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/LocalException.h>
#include <IceE/SafeStdio.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ProxyFactory* p) { p->__incRef(); }
void IceInternal::decRef(ProxyFactory* p) { p->__decRef(); }

ObjectPrx
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str);
    return referenceToProxy(ref);
}

string
IceInternal::ProxyFactory::proxyToString(const ObjectPrx& proxy) const
{
    if(proxy)
    {
	return proxy->__reference()->toString();
    }
    else
    {
	return "";
    }
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s) const
{
    Identity ident;
    ident.__read(s);

    ReferencePtr ref = _instance->referenceFactory()->create(ident, s);
    return referenceToProxy(ref);
}

void
IceInternal::ProxyFactory::proxyToStream(const ObjectPrx& proxy, BasicStream* s) const
{
    if(proxy)
    {
	proxy->__reference()->getIdentity().__write(s);
	proxy->__reference()->streamWrite(s);
    }
    else
    {
	Identity ident;
	ident.__write(s);
    }
}

ObjectPrx
IceInternal::ProxyFactory::referenceToProxy(const ReferencePtr& ref) const
{
    if(ref)
    {
        ObjectPrx proxy = new ::IceProxy::Ice::Object;
        proxy->setup(ref);
        return proxy;
    }
    else
    {
        return 0;
    }
}

void
IceInternal::ProxyFactory::checkRetryAfterException(const LocalException& ex, const ReferencePtr& ref, int& cnt) const
{
    //
    // We retry ObjectNotExistException if the reference is
    // indirect. Otherwise, we don't retry other *NotExistException,
    // which are all derived from RequestFailedException.
    //
#ifdef ICEE_HAS_LOCATOR
    if(dynamic_cast<const ObjectNotExistException*>(&ex))
    {
	IndirectReferencePtr ir = IndirectReferencePtr::dynamicCast(ref);
	if(!ir || !ir->getLocatorInfo())
	{
	    ex.ice_throw();
	}
	ir->getLocatorInfo()->clearObjectCache(ir);
    }
    else
#else
    if(dynamic_cast<const RequestFailedException*>(&ex))
    {
	ex.ice_throw();
    }
#endif

    ++cnt;
    
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();
    
    //
    // Instance components may be null if the communicator has been
    // destroyed.
    //
    if(traceLevels && logger)
    {
        if(cnt > static_cast<int>(_retryIntervals.size()))
        {
            if(traceLevels->retry >= 1)
            {
                Trace out(logger, traceLevels->retryCat);
                out << "cannot retry operation call because retry limit has been exceeded\n" << ex.toString();
            }
            ex.ice_throw();
        }

        if(traceLevels->retry >= 1)
        {
            Trace out(logger, traceLevels->retryCat);
            out << "re-trying operation call";
            if(cnt > 0 && _retryIntervals[cnt - 1] > 0)
            {
                out << Ice::printfToString(" in %dms", _retryIntervals[cnt - 1]);
            }
            out << " because of exception\n" << ex.toString();
        }

        if(cnt > 0)
        {
            //
            // Sleep before retrying.
            //
            Ice::ThreadControl::sleep(Ice::Time::milliSeconds(_retryIntervals[cnt - 1]));
        }
    }
    else
    {
        //
        // Impossible to retry after the communicator has been
        // destroyed.
        //
        ex.ice_throw();
    }
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
    string str = _instance->properties()->getPropertyWithDefault("IceE.RetryIntervals", "0");

    string::size_type beg;
    string::size_type end = 0;

    while(true)
    {
	const string delim = " \t";
    
	beg = str.find_first_not_of(delim, end);
	if(beg == string::npos)
	{
	    if(_retryIntervals.empty())
	    {
		_retryIntervals.push_back(0);
	    }
	    break;
	}

	end = str.find_first_of(delim, beg);
	if(end == string::npos)
	{
	    end = str.length();
	}
	
	if(beg == end)
	{
	    break;
	}

	string value = str.substr(beg, end - beg);
	int v = atoi(value.c_str());

	//
	// If -1 is the first value, no retry and wait intervals.
	//
	if(v == -1 && _retryIntervals.empty())
	{
	    break;
	}

	_retryIntervals.push_back(v > 0 ? v : 0);
    }
}

IceInternal::ProxyFactory::~ProxyFactory()
{
}
