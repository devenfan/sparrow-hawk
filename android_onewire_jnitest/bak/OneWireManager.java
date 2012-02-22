package android.onewire;

import android.content.Context;
import android.os.Binder;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.ParcelFileDescriptor;
import android.os.Process;
import android.os.RemoteException;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;

import android.util.Log;

public class OneWireManager {

	private static final String TAG = "OneWireManager";
	
    private IOneWireService _oneWireService;

    public OneWireManager() {
	
    	_oneWireService = IOneWireService.Stub.asInterface(
                             ServiceManager.getService("OneWire"));

    	if (_oneWireService != null) {
            Log.i(TAG, "The OneWireManager object is ready.");
    	}
    }

	
}
