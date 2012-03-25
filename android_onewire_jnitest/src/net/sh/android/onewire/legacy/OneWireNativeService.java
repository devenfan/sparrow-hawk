package net.sh.android.onewire.legacy;


import android.util.Log;


public class OneWireNativeService {

    private static final String TAG = "OneWireNativeService";

    private static final boolean DEBUG = false;
    private static final boolean VERBOSE = false;

    private static OneWireNativeService s_instance = new OneWireNativeService();
    
    private boolean _isStarted = false;
    
    private OneWireListener _listener;

    static { 
    	System.loadLibrary("android_onewire");
    	//System.loadLibrary("android_servers");
    	
    	class_init_native(); 
    	
    	Log.i(TAG, "class_init_native... ");
    }
    
//    private OneWireNativeService(){
//    	
//    }
    
    // public --------------------------------------------------------------
    
    public static OneWireNativeService getInstance() {
    	return s_instance;
    }
    
    public static boolean isSupported(){
    	return native_is_supported();
    }
    
    public void setListener(OneWireListener listener){
    	_listener = listener;
    }

    public boolean isStarted(){
    	return _isStarted;
    }
    
    public boolean start(){
    	if(!_isStarted){
    		_isStarted = native_start();
    		Log.i(TAG, _isStarted ? "start successfully!" : "start failed!");
    	}
    	return _isStarted;
    } 
    
    public void stop() {
    	if(_isStarted) {
    		native_stop();
    		Log.i(TAG, "stopped!");
    		_isStarted = false;
    	}
    }
    
    public int getMasterId(){
    	return native_get_current_master();
    }
    
    
    // Callbacks (Invoked by JNI native codes...) --------------------------
    
    
    /**
     * masterId: 4 bytes
     * */
    private void masterAdded(int masterId){
    	OneWireMasterID ID = new OneWireMasterID(masterId);
    	Log.i(TAG, "masterAdded: " + ID);
    	if(_listener != null){
    		_listener.oneWireMasterAdded(ID);
    	}
    }

    /**
     * masterId: 4 bytes
     * */
    private void masterRemoved(int masterId){
    	OneWireMasterID ID = new OneWireMasterID(masterId);
    	Log.i(TAG, "masterRemoved: " + ID);
    	if(_listener != null){
    		_listener.oneWireMasterRemoved(ID);
    	}
    }

    /**
     * slaveRN: 8 bytes
     * */
    public void slaveAdded(long slaveRN){
    	OneWireSlaveID ID = new OneWireSlaveID(slaveRN);
    	Log.i(TAG, "slaveAdded: " + ID);
//    	if(_listener != null){
//    		_listener.oneWireSlaveAdded(ID);
//    	}
    }

    /**
     * slaveRN: 8 bytes
     * */
    public void slaveRemoved(long slaveRN){
    	OneWireSlaveID ID = new OneWireSlaveID(slaveRN);
    	Log.i(TAG, "slaveRemoved: " + ID);
//    	if(_listener != null){
//    		_listener.oneWireSlaveRemoved(ID);
//    	}
    }
    

    // native (JNI mapped functions) -------------------------------------
	
    private static native void class_init_native();
    
    private static native boolean native_is_supported();
    
    private native boolean native_start();
    
    private native void native_stop();
    
    //return masterId by integer...
    private native int native_get_current_master(); 
    
    //return the slave count...
    private native int native_get_current_slaves(long[] slaveRNs);
    
    private native boolean native_begin_exclusive(int masterId); 
    
    private native void native_end_exclusive(int masterId);
    
    //return the master count...
    //private native int native_list_masters(int[] masterIDs);
    
    //return the slave count...
    private native int native_search_slaves(int masterId, long[] slaveRNs);
    
    private native boolean native_master_reset(int masterId);
    
    private native boolean native_master_touch(
            int masterId, byte[] dataIn, int dataInLen, byte[] dataOut);
    
    private native boolean native_master_read(
            int masterId, int readLen, byte[] dataReadOut);
    
    private native boolean native_master_write(
            int masterId, int writeLen, byte[] dataWriteIn);
    
}
