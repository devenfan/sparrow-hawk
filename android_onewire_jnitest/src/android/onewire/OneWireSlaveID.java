/*
 * Copyright (C) 2007 The Android Open Source Project
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

//package android.onewire;
package android.onewire;

import net.sh.android.onewire.ConvertCodec;

public class OneWireSlaveID {

	public static final int SIZE = 8;

	private boolean _isLittleEndian; //is little endian or big endian
	private byte[] _slaveRN; //8 bytes;
	
	/*
	private byte m_Family;
	private byte[] m_SN;	//6 bytes
	private byte m_CRC;
	*/
	
	public OneWireSlaveID() { }
	
	public OneWireSlaveID(long slaveRN) {
		byte[] bytes = new byte[8];
		for(int i = 0; i < bytes.length; i++){
			bytes[i] = (byte) (slaveRN >>> (8 * (bytes.length - i - 1)));
		}
		setSlaveRN(bytes, true);
	}
	
	public OneWireSlaveID(byte[] slaveRN, boolean isLittleEndian) {
		setSlaveRN(slaveRN, isLittleEndian);	
	}
	
	public byte getFamily() {
		if (_isLittleEndian) {
			return _slaveRN[0];
		} else {
			return _slaveRN[SIZE - 1];
		}
	}
	
	public String getSN(){
		return ConvertCodec.bytesToHexString(_slaveRN, 1, 6);
	}
	
	public void setSlaveRN(byte[] slaveRN, boolean isLittleEndian) {
		//TODO Check slaveRN...
		
		//Copy into the memory... 
		_slaveRN = new byte[SIZE];
		for (int i = 0; i < SIZE; i++) {
			_slaveRN[i] = slaveRN[i];
		}
		_isLittleEndian = isLittleEndian;
	}
	
	public String toString(){
		return ConvertCodec.bytesToHexString(_slaveRN);
	}
	

}