package seu.fan.onewire;



import seu.fan.onewire.utils.CRC8;
import seu.fan.onewire.utils.ConvertCodec;
import android.onewire.OneWireException;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;

public class DS1920Tester extends IButtonBaseTester {

	//--------
	//-------- Static Final Variables
	//--------
	
	/**
	* default temperature resolution for this <code>OneWireContainer10</code>
	* device.
	*/
	public static final double RESOLUTION_NORMAL = 0.5;
	
	/**
	* maximum temperature resolution for this <code>OneWireContainer10</code>
	* device. Use <code>RESOLUTION_MAXIMUM</code> in
	* <code>setResolution()</code> if higher resolution is desired.
	*/
	public static final double RESOLUTION_MAXIMUM = 0.1;
	
	/** DS1920 convert temperature command  */
	private static final byte CONVERT_TEMPERATURE_COMMAND = 0x44;
	
	/** DS1920 read data from scratchpad command*/
	private static final byte READ_SCRATCHPAD_COMMAND = ( byte ) 0xBE;
	
	/** DS1920 write data to scratchpad command  */
	private static final byte WRITE_SCRATCHPAD_COMMAND = ( byte ) 0x4E;
	
	/** DS1920 copy data from scratchpad to EEPROM command  */
	private static final byte COPY_SCRATCHPAD_COMMAND = ( byte ) 0x48;
	
	/** DS1920 recall EEPROM command */
	private static final byte RECALL_EEPROM_COMMAND = ( byte ) 0xB8;

	
	
	public DS1920Tester(OneWireManager oneWireManager, OneWireMasterID master) {
		super(oneWireManager, master);
		this._family = 0x10;
	}
	
	
	private byte[] readScratchPad() throws OneWireException {

		byte[] state = new byte[8];
		
		_oneWireManager.write(_oneWireMaster, new byte[]{ READ_SCRATCHPAD_COMMAND});
		trace("WRITE: BE");
		
		byte[] dataWithCRC = _oneWireManager.read(_oneWireMaster, 9);
		trace("READ: " + ConvertCodec.bytesToHexString(state));
		
		//compute first 8 bytes with the CRC byte, the result should be 0 
		if (CRC8.compute(dataWithCRC, 0, 9) == 0)
            System.arraycopy(dataWithCRC, 0, state, 0, 8);
         else
            throw new OneWireException("CRC error when read ScratchPad: " + ConvertCodec.bytesToHexString(dataWithCRC));
		
		return state;
	}
	
	
	private void writeScratchPad(byte[] state) throws OneWireException {
		
		if(state == null || state.length != 8) 
			throw new OneWireException("Wrong input of DS1920, should be 8 bytes...");

		byte[] dataOut = new byte[3];
		dataOut[0] = WRITE_SCRATCHPAD_COMMAND;
		dataOut[1] = state[2];
		dataOut[2] = state[3];

		//write scratchpad
		_oneWireManager.write(_oneWireMaster, dataOut);
		trace("WRITE: " + ConvertCodec.bytesToHexString(dataOut));
	}
	
	
	private void copyScratchPad() throws OneWireException {
		
		//copy scratchpad
		_oneWireManager.write(_oneWireMaster, new byte[] {COPY_SCRATCHPAD_COMMAND});
		trace("WRITE: 48");

        // delay for 10 ms
        try
        {
           Thread.sleep(10);
        }
        catch (InterruptedException e){}
		trace("SLEEP: 10ms");
	}
	
	
	private void convertTemperature() throws OneWireException {
		
		// send the convert temperature command
		_oneWireManager.write(_oneWireMaster, new byte[]{ CONVERT_TEMPERATURE_COMMAND});
		trace("WRITE: 44");

        // delay for 750 ms
        try
        {
           Thread.sleep(750);
        }
        catch (InterruptedException e){}
		trace("SLEEP: 750ms");
	}
	
	
	
	public byte[] readState() throws OneWireException {
		
		//skipROM();
		matchROM();
		trace("MatchROM");
		
		convertTemperature();
		
		return readScratchPad();
	}
	
	
	private void writeState(byte[] state) throws OneWireException {

		//skipROM();
		matchROM();
		trace("MatchROM");
		
		writeScratchPad(state);
		
		copyScratchPad();
	}
	
	
	private double getTemperature(byte[] state) throws OneWireException {

		//on some parts, namely the 18S20, you can get invalid readings.
		//basically, the detection is that all the upper 8 bits should
		//be the same by sign extension. the error condition (DS18S20
		//returns 185.0+) violated that condition
		if (((state [1] & 0x0ff) != 0x00) && ((state [1] & 0x0ff) != 0x0FF))
		throw new OneWireException("Invalid temperature data: " + ConvertCodec.bytesToHexString(state, 0, 2));

		short temp = ( short ) ((state [0] & 0x0ff) | (state [1] << 8));
		
		if (state [4] == 1) { //max resolution: 0.5
			
			temp = ( short ) (temp >> 1);//lop off the last bit
			
			//also takes care of the / 2.0
			double tmp = ( double ) temp;
			double cr  = (state [6] & 0x0ff);
			double cpc = (state [7] & 0x0ff);
			
			//just let the thing throw a divide by zero exception
			tmp = tmp - ( double ) 0.25 + (cpc - cr) / cpc;
			
			return tmp;
		} else { //normal resolution: 0.1
			
			//do normal resolution
			return temp / 2.0;
		}
	}
	
	
	public double readTemperature() throws OneWireException {
		
		byte[] state = readState();
		
		return getTemperature(state);
	}
	
	
}
