/*
 *
 * Copyright (C) 2012, The Android OneWire Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.onewire;

import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;

/**
 * Note that this is a one-way interface so the server
 * does not block waiting for the client.
 * 
 * {@hide}
 */
oneway interface IOneWireListener {

    void onOneWireMasterAdded(in OneWireMasterID master);

	void onOneWireMasterRemoved(in OneWireMasterID master);

	void onOneWireSlaveAdded(in OneWireMasterID master, in OneWireSlaveID slaveOfTheMaster);

	void onOneWireSlaveRemoved(in OneWireMasterID master, in OneWireSlaveID slaveOfTheMaster);
}
