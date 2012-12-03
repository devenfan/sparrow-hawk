package android.onewire;

/**
 * 
 * Used by OneWireManager, which can be considered as Service Client Listener.
 *
 */
public interface OneWireListener {
	
	void onOneWireMasterAdded(OneWireMasterID master);

	void onOneWireMasterRemoved(OneWireMasterID master);

	void onOneWireSlaveAdded(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster);

	void onOneWireSlaveRemoved(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster);

}
