package com.example.android_onewire_service_test;


import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.util.Arrays;

import android.app.Activity;
import android.content.Context;
import android.onewire.OneWireListener;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class AndroidOneWireServiceTestActivity extends Activity {
	
	static String TAG = "Activity";
	
	TextView _txtLog;
	TextView _txtStatus;
	Button _btnTest1;
	Button _btnTest2;
	Button _btnTest3;
	
	OneWireManager oneWireManager;
	
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        /*
        if(runRootCommand("ls")){
        	Log.i(TAG, "OK, Got su permission...");
        } else {
        	Log.e(TAG, "Cannot switch to su...");
        }
        */
        
        oneWireManager = (OneWireManager) this.getSystemService(Context.ONEWIRE_SERVICE);
        
        oneWireManager.addOneWireListener(new OneWireListener() {
			
			@Override
			public void onOneWireSlaveRemoved(OneWireMasterID paramOneWireMasterID,
					OneWireSlaveID paramOneWireSlaveID) {
				_txtLog.append("oneWireSlaveRemoved: slave[" + 
					paramOneWireSlaveID + "] on master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireSlaveAdded(OneWireMasterID paramOneWireMasterID,
					OneWireSlaveID paramOneWireSlaveID) {
				_txtLog.append("onOneWireSlaveAdded: slave[" + 
						paramOneWireSlaveID + "] on master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireMasterRemoved(OneWireMasterID paramOneWireMasterID) {
				_txtLog.append("onOneWireMasterRemoved: master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireMasterAdded(OneWireMasterID paramOneWireMasterID) {
				_txtLog.append("onOneWireMasterAdded: master[" + paramOneWireMasterID + "] \n");
			}
		});
        
        
        _txtLog = (TextView) findViewById(R.id.txtLog);
        _txtStatus = (TextView) findViewById(R.id.txtStatus);
        _btnTest1 = (Button) findViewById(R.id.btnTest1);
        _btnTest2 = (Button) findViewById(R.id.btnTest2);
        _btnTest3 = (Button) findViewById(R.id.btnTest3);
        
        
        _btnTest1.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	
            	_txtStatus.setText("List Masters: " + Arrays.toString(oneWireManager.listMasters()));
            }  
            
         }); 
    }
    
    
    
	public static boolean runRootCommand(String command) {
		
		Process process = null;
		DataOutputStream outputStream = null;
		try {
			process = Runtime.getRuntime().exec("su");

			outputStream = new DataOutputStream(process.getOutputStream());
			outputStream.writeBytes(command + "\n");

			outputStream.writeBytes("exit\n");
			outputStream.flush();
			
			Log.d(TAG, "su");
			
			process.waitFor();

			BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(
					process.getInputStream()));
			
			// You can use below line to check the error, only if the board has been rooted...
			// BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getErrorStream())); 
			
			String line = null;
			while ((line = bufferedReader.readLine()) != null) {
				Log.d(TAG, line);
			}
			try {
				bufferedReader.close();
			} catch (Exception e) {
				e.printStackTrace();
				Log.e(TAG, e.toString());
			}
		} catch (Exception e) {
			Log.d(TAG, "the device is not rooted, error message: "
							+ e.getMessage());
			return false;
			
		} finally {
			try {
				if (outputStream != null) {
					outputStream.close();
				}
				if (process != null) {
					process.destroy();
				}
			} catch (Exception e) {
				e.printStackTrace();
				Log.e(TAG, e.toString());
			}
		}
		return true;
	}
    
    
}