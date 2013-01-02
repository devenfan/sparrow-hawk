package seu.fan.onewire;

import seu.fan.onewire.utils.ConvertCodec;
import android.onewire.OneWireException;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;


public class IButtonBaseTester {

	private final byte COMMAND_READ_ROM = 0x33;	//only if there is a single slave on the bus
	private final byte COMMAND_MATCH_ROM = 0x55;	//followed by a 64- bit ROM sequence
	private final byte COMMAND_SEARCH_ROM = (byte) 0xF0;	//search all the ibuttons on the bus
	private final byte COMMAND_SKIP_ROM = (byte) 0xCC;	//only if there is a single slave on the bus

	
	protected OneWireManager _oneWireManager;
	
	protected OneWireMasterID _oneWireMaster;
	
	protected OneWireSlaveID _ibutton;
	
	protected byte _family;
	
	protected IButtonTraceListener _tranceListener;
	

	public IButtonBaseTester(OneWireManager oneWireManager, OneWireMasterID master){
		this._oneWireManager = oneWireManager;
		this._oneWireMaster = master;
	}

	
	public void registerTraceListener(IButtonTraceListener tranceListener) {
		_tranceListener = tranceListener;
	}
	
	public void unregisterTraceListener() {
		_tranceListener = null;
	}
	
	public boolean isFamilyCorrect(OneWireSlaveID ibutton) {
		return ibutton.getFamily() == _family;
	}
	
	public byte getFamily() {
		return _family;
	}
	
	public void changeIButton(OneWireSlaveID ibutton) {
		
		if(ibutton.getFamily() != _family)
			throw new IllegalArgumentException("iButton[" + ibutton.toString() + 
					"] IS NOT kind of Family[0x" + Integer.toString(_family, 16) + "]!");
		
		_ibutton = ibutton;
	}
	
	
	public void skipROM() throws OneWireException {
		
		if(_oneWireManager.reset(_oneWireMaster)){
			trace("RESET");
			
			//Skip ROM is a must... Otherwise, the command will not be correctly answered...
			_oneWireManager.write(_oneWireMaster, new byte[]{ (byte) 0xCC}); 
			trace("WRITE: CC");
			
		} else {
			throw new OneWireException("Cannot reset OneWire bus...");
		}
	}
	
	
	public void matchROM() throws OneWireException {
		
		if(_ibutton == null)
			throw new IllegalStateException("No ibutton...");
		
		if(_oneWireManager.reset(_oneWireMaster)){
			trace("RESET");
			
			//Match ROM: 0x55
			_oneWireManager.write(_oneWireMaster, new byte[]{ (byte) 0x55});
			
			byte[] rom = _ibutton.getBigEndianBytes(); //Right
			//byte[] rom = _ibutton.getLittleEndianBytes(); //Wrong
			_oneWireManager.write(_oneWireMaster, rom);
			trace("WRITE: 55" + ConvertCodec.bytesToHexString(rom));
			
		} else {
			throw new OneWireException("Cannot reset OneWire bus...");
		}
	}
	
	
	protected void trace(String trace) {
		if(_tranceListener != null)
			_tranceListener.onTrace(_oneWireMaster, _ibutton, trace);
	}
	
}
