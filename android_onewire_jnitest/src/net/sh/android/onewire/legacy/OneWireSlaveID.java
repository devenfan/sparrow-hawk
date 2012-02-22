package net.sh.android.onewire.legacy;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

import net.sh.android.onewire.ConvertCodec;

public class OneWireSlaveID implements Parcelable {

	public static final int SIZE = 8;

	private boolean _isLittleEndian; //is little endian or big endian
	private byte[] _slaveRN; //8 bytes;
	
	/*
	private byte m_Family;	//1 byte
	private byte[] m_SN;	//6 bytes
	private byte m_CRC;		//1 byte
	*/
	
	//public OneWireSlaveID() { }
	
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
	
	private void setSlaveRN(byte[] slaveRN, boolean isLittleEndian) {
		//TODO Check slaveRN...
		
		//Copy into the memory... 
		_slaveRN = new byte[SIZE];
		for (int i = 0; i < SIZE; i++) {
			_slaveRN[i] = slaveRN[i];
		}
		_isLittleEndian = isLittleEndian;
	}
	
	
	// Extras Bundle ------------------------------------------------

    private Bundle _extras = null;
	
	
	/**
     * Returns additional provider-specific information about the
     * address as a Bundle.  The keys and values are determined
     * by the provider.  If no additional information is available,
     * null is returned.
     *
     * <!--
     * <p> A number of common key/value pairs are listed
     * below. Providers that use any of the keys on this list must
     * provide the corresponding value as described below.
     *
     * <ul>
     * </ul>
     * -->
     */
    public Bundle getExtras() {
        return _extras;
    }

    /**
     * Sets the extra information associated with this fix to the
     * given Bundle.
     */
    public void setExtras(Bundle extras) {
    	_extras = (extras == null) ? null : new Bundle(extras);
    }
	
    // toString impl ------------------------------------------------
	
	public String toString(){
		StringBuilder sb = new StringBuilder();
        sb.append(this.getClass().getSimpleName());
        sb.append("[");
		sb.append(ConvertCodec.bytesToHexString(_slaveRN));
		sb.append("]");
		return sb.toString();
	}
	
	// Parcelable Impl ------------------------------------------------
	
	public static final Parcelable.Creator<OneWireSlaveID> CREATOR =
        new Parcelable.Creator<OneWireSlaveID>() {
        public OneWireSlaveID createFromParcel(Parcel in) {
        	byte[] slaveRN = new byte[SIZE];
            in.readByteArray(slaveRN);
        	boolean isLittleEndian = in.readInt() != 0;
            return new OneWireSlaveID(slaveRN, isLittleEndian);
        }

        public OneWireSlaveID[] newArray(int size) {
            return new OneWireSlaveID[size];
        }
    };

	//@Override
	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}

	//@Override
	public void writeToParcel(Parcel parcel, int flags) {
		parcel.writeByteArray(_slaveRN);
		parcel.writeInt(_isLittleEndian ? 1 : 0);
	}

}