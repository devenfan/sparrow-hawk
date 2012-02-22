/* 
**
** Copyright 2008, The Android Open Source Project
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


/**
 * {@hide}
 */
oneway interface IOneWireListener
{
    void oneWireMasterAdded(in OneWireMasterID master);

    void oneWireMasterRemoved(in OneWireMasterID master);

    void oneWireSlaveAdded(in OneWireSlaveID slave);

    void oneWireSlaveRemoved(in OneWireSlaveID slave);
}


