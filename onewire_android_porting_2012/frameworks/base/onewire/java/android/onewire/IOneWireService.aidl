/*
**
** Copyright (C) 2012, The Android OneWire Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


package android.onewire;


import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.onewire.IOneWireListener;

import android.os.Bundle;


/**
 * 
 * {@hide}
 */
interface IOneWireService
{

    void addOneWireListener(in IOneWireListener listener);


    void removeOneWireListener(in IOneWireListener listener);


    void oneWireCallbackFinished(in IOneWireListener listener);


	boolean isDebugEnabled();


	void setDebugEnabled(boolean enabled);
	

    boolean beginExclusive();


    void endExclusive();



    OneWireMasterID[] getCurrentMasters();
    

    OneWireMasterID[] listMasters();


    OneWireSlaveID[] searchSlaves(in OneWireMasterID masterId);


    boolean reset(in OneWireMasterID masterId);


    //dataOutLen is equal to dataInlen
    byte[] touch(in OneWireMasterID masterId, in byte[] dataIn, int dataInLen);


    byte[] read(in OneWireMasterID masterId, int readLen);


    boolean write(in OneWireMasterID masterId, in byte[] dataWriteIn);


}
