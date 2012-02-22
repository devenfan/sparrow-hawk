package android.onewire;


public interface OneWireListener {
	
	void oneWireMasterAdded(OneWireMasterID master);

	void oneWireMasterRemoved(OneWireMasterID master);

	void oneWireSlaveAdded(OneWireSlaveID slave);

	void oneWireSlaveRemoved(OneWireSlaveID slave);

}
