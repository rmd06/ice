// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackDouble
    extends Functional_TwowayCallback implements Ice.TwowayCallbackDouble
{
    public Functional_TwowayCallbackDouble(Functional_DoubleCallback responseCb, 
                                           Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                           Functional_BoolCallback sentCb)
    {
        super(responseCb != null, localExceptionCb, sentCb);
        __responseCb = responseCb;
    }
    
    protected Functional_TwowayCallbackDouble(boolean userExceptionCb,
                                              Functional_DoubleCallback responseCb, 
                                              Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                              Functional_BoolCallback sentCb)
    {
        super(localExceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && localExceptionCb != null));
        __responseCb = responseCb;
    }
    
    public void response(byte arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }
    
    final private Functional_DoubleCallback __responseCb;
};
