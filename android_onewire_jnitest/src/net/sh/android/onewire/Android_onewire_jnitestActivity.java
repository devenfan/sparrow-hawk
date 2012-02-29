package net.sh.android.onewire;


import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;

import net.sh.android.onewire.legacy.*;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class Android_onewire_jnitestActivity extends Activity {
	
	
	TextView _txtLog;
	TextView _txtStatus;
	Button _btnStart;
	Button _btnStop;
	Button _btnTest;
	
	OneWireNativeService _OneWireNativeService;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        runRootCommand("ls"); 
        
        _OneWireNativeService = new OneWireNativeService();
        _OneWireNativeService.setListener(new OneWireListener() {
			
			//@Override
			public void oneWireSlaveRemoved(OneWireSlaveID slave) {
				_txtLog.append("oneWireSlaveRemoved: " + slave + "\n");
			}
			
			//@Override
			public void oneWireSlaveAdded(OneWireSlaveID slave) {
				_txtLog.append("oneWireSlaveAdded: " + slave + "\n");
				
			}
			
			//@Override
			public void oneWireMasterRemoved(OneWireMasterID master) {
				_txtLog.append("oneWireMasterRemoved: " + master + "\n");
				
			}
			
			//@Override
			public void oneWireMasterAdded(OneWireMasterID master) {
				_txtLog.append("oneWireMasterAdded: " + master + "\n");
				
			}
		});
        
        _txtLog = (TextView) findViewById(R.id.txtLog);
        _txtStatus = (TextView) findViewById(R.id.txtStatus);
        _btnStart = (Button) findViewById(R.id.btnStart);
        _btnStop = (Button) findViewById(R.id.btnStop);
        _btnTest = (Button) findViewById(R.id.btnTest);
        
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
        
        _btnTest.setOnClickListener(new View.OnClickListener() {  
            public void onClick(View v) {  
            	
            	_txtStatus.setText(OneWireNativeService.isSupported() ? 
            			"OneWire supported!!!" : "OneWire not supported...");
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
			process.waitFor();

			BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(
					process.getInputStream()));
			
			// BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(
			// process.getErrorStream())); //如果出错用这个输出一下看结果，一定要获取root权限才可以执行
			
			String line = null;
			while ((line = bufferedReader.readLine()) != null) {
				Log.d("*********", line);
			}
			try {
				bufferedReader.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (Exception e) {
			Log.d("*********",
					"the device is not rooted, error message: "
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
			}
		}
		return true;
	}
    
    
}