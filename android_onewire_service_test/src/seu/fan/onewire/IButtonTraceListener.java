package seu.fan.onewire;

import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;

public interface IButtonTraceListener {
	
	void onTrace(OneWireMasterID master, OneWireSlaveID ibutton, String trace);

}
