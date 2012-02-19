package net.sh.android.onewire;

import com.android.server.onewire.OneWireNativeService;

import android.app.Activity;
import android.onewire.OneWireListener;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class Android_onewire_jnitestActivity extends Activity {
	
	
	TextView _txtLog;
	TextView _txtStatus;
	Button _btnStart;
	Button _btnStop;
	
	OneWireNativeService _OneWireNativeService;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        _OneWireNativeService = OneWireNativeService.getInstance();
        _OneWireNativeService.setListener(new OneWireListener() {
			
			@Override
			public void oneWireSlaveRemoved(OneWireSlaveID slave) {
				_txtLog.append("oneWireSlaveRemoved: " + slave + "\n");
			}
			
			@Override
			public void oneWireSlaveAdded(OneWireSlaveID slave) {
				_txtLog.append("oneWireSlaveAdded: " + slave + "\n");
				
			}
			
			@Override
			public void oneWireMasterRemoved(OneWireMasterID master) {
				_txtLog.append("oneWireMasterRemoved: " + master + "\n");
				
			}
			
			@Override
			public void oneWireMasterAdded(OneWireMasterID master) {
				_txtLog.append("oneWireMasterAdded: " + master + "\n");
				
			}
		});
        
        _txtLog = (TextView) findViewById(R.id.txtLog);
        _txtStatus = (TextView) findViewById(R.id.txtStatus);
        _btnStart = (Button) findViewById(R.id.btnStart);
        _btnStop = (Button) findViewById(R.id.btnStop);
        
        //_btnStart.setEnabled(true);
        //_btnStop.setEnabled(false);
    
        _btnStart.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	_OneWireNativeService.start();
            	_txtStatus.setText(_OneWireNativeService.isStarted() ? "OneWire Started" : "OneWire Stopped");
            }  
         }); 
        
        _btnStop.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	_OneWireNativeService.stop();
            	_txtStatus.setText(_OneWireNativeService.isStarted() ? "OneWire Started" : "OneWire Stopped");
            }  
            
         }); 
        
    }
}