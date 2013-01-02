package seu.fan.onewire;

import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;

public class DS1972Tester extends IButtonBaseTester {
	
	

	public DS1972Tester(OneWireManager oneWireManager, OneWireMasterID master) {
		super(oneWireManager, master);
		this._family = 0x2D;
	}
	
	
	
	
	
	
	

}
