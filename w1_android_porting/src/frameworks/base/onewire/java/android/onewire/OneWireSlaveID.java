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

package android.onewire;


public class OneWireSlaveID {

	public static final int SIZE = 8;

	private boolean m_IsLittleEndian; //is little endian or big endian
	private byte[] m_SlaveRN; //8 bytes;
	
	/*
	private byte m_Family;
	private byte[] m_SN;	//6 bytes
	private byte m_CRC;
	*/
	
	public OneWireMasterID() { }
	
	public OneWireMasterID(byte[] slaveRN, boolean isLittleEndian) {
		setSlaveRN(slaveRN, isLittleEndian);	
	}
	
	public byte getFamily(){
		if(m_IsLittleEndian)
		{
			return m_SlaveRN[0];
		}
		else
		{
			return m_SlaveRN[SIZE - 1];
		}
	}
	
	public byte getSN(){
		return ConvertCodec.byteArrayToHexString(m_SlaveRN, 1, 6);
	}
	
	public void setSlaveRN(byte[] slaveRN, boolean isLittleEndian) {
		//TODO Check slaveRN...
		//Copy into the memory... 
		m_SlaveRN = new byte[SIZE];
		for(int i = 0; i < SIZE; i++)
		{
			m_SlaveRN[i] = slaveRN[i];
		}
		m_IsLittleEndian = isLittleEndian;
	}
	
	public String toString(){
		return ConvertCodec.byteArrayToHexString(m_SlaveRN);
	}
	

}