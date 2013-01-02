package seu.fan.onewire;

import java.util.Date;

import seu.fan.onewire.utils.Convert;
import seu.fan.onewire.utils.ConvertCodec;




import android.onewire.OneWireException;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;

public class DS1904Tester extends IButtonBaseTester {
	

	
	public DS1904Tester(OneWireManager oneWireManager, OneWireMasterID master) {
		super(oneWireManager, master);
		this._family = 0x24;
	}

	public boolean isDS1904(OneWireSlaveID ibutton){
		return isFamilyCorrect(ibutton);
	}
	
	
	private byte[] readState() throws OneWireException {
		
		//skipROM();
		matchROM();
		trace("MatchROM");

		_oneWireManager.write(_oneWireMaster, new byte[]{ (byte) 0x66});
		trace("WRITE: 66");
		
		byte[] state = _oneWireManager.read(_oneWireMaster, 5);
		trace("READ: " + ConvertCodec.bytesToHexString(state));
		
		return state;
	}
	
	private void writeState(byte[] state) throws OneWireException {
		
		if(state == null || state.length != 5) 
			throw new OneWireException("Wrong input of DS1904, should be 5 bytes...");
		/*
		byte[] data = new byte[6];
		data[0] = (byte) 0x99;
		for(int i = 0, j= 1; i < state.length; i++, j++){
			data[j] = state[i];
		}
		*/
		
		//skipROM();
		matchROM();
		trace("MatchROM");

		_oneWireManager.write(_oneWireMaster, new byte[]{ (byte) 0x99});
		_oneWireManager.write(_oneWireMaster, state);
		trace("WRITE: 99" + ConvertCodec.bytesToHexString(state));
	}
	
	
	
	
	public Date getTime() throws OneWireException{
		
		byte[] state = readState();
		
		long timeData = Convert.toLong(state, 1, 4) * 1000;
		
		return new Date(timeData);
	}
	
	public void setTime(Date time) throws OneWireException{
		
		byte[] state = readState();
		
		long timeData = time.getTime();
		
		Convert.toByteArray((timeData / 1000L), state, 1, 4);
		
		writeState(state);
	}
	
	public boolean isOscillatorEnabled() throws OneWireException {
		
		byte[] state = readState();
		
		return (state[0] & 0x0C) == 0x0C;
	}
	
	public void setTime(Date time, boolean isOscillatorEnabled) throws OneWireException{
		
		byte[] state = new byte[5];
		
		state[0] = 0x0C;
		
		long timeData = time.getTime();
		
		Convert.toByteArray((timeData / 1000L), state, 1, 4);
		
		writeState(state);
	}
	
	
}
