package android.onewire;

/**
 * 
 * Used by OneWireManager, which can be considered as Service Client Listener.
 *
 */
public interface OneWireListener {
	
	void oneWireMasterAdded(OneWireMasterID master);

	void oneWireMasterRemoved(OneWireMasterID master);

	void oneWireSlaveAdded(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster);

	void oneWireSlaveRemoved(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster);

}
