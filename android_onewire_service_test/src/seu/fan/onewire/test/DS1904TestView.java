package seu.fan.onewire.test;


import java.util.Date;

import com.example.android_onewire_service_test.R;

import seu.fan.onewire.DS1904Tester;
import seu.fan.onewire.IButtonTraceListener;

import android.app.Activity;

import android.onewire.OneWireException;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.TextView;

public class DS1904TestView {
	
	private ScrollView _view;
	private TextView _txtLogOutput;
	private Button _btnDS1904GetTime;
	private Button _btnDS1904SetTime;
	private Button _btnDS1904IsTimerEnabled;
	private Button _btnDS1904SetTimer;
	private Button _btnLogClear;
	
	private DS1904Tester _DS1904Tester;
	
	public DS1904TestView(Activity activity) {

		_view = new ScrollView(activity);   
		_view.setScrollContainer(true);   
		_view.setFocusable(true);   

		_txtLogOutput = (TextView) activity.findViewById(R.id.txtDS1904LogOutput);
		_btnDS1904GetTime = (Button) activity.findViewById(R.id.btnDS1904GetTime);
		_btnDS1904SetTime = (Button) activity.findViewById(R.id.btnDS1904SetTime);
		_btnDS1904IsTimerEnabled = (Button) activity.findViewById(R.id.btnDS1904IsTimerEnabled);
		_btnDS1904SetTimer = (Button) activity.findViewById(R.id.btnDS1904SetTimer);
		_btnLogClear = (Button) activity.findViewById(R.id.btnDS1904LogClear);

        //enable Horizontally scrolling...
        //_txtLog.setHorizontallyScrolling(true);  
        
        //let it move itself...
        _txtLogOutput.setMovementMethod(ScrollingMovementMethod.getInstance());
        
        _btnDS1904GetTime.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_DS1904Tester != null){
            		try {
						Date time = _DS1904Tester.getTime();
						appendLog("Get Time: " + time.toString());
					} catch (OneWireException e) {
						appendLog(e.toString());
					}
            	}
            }  
            
         }); 
        
        _btnDS1904SetTime.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_DS1904Tester != null){
            		try {
            			Date now = new Date();
						_DS1904Tester.setTime(now);
						appendLog("Set Time: " + now.toString());
					} catch (OneWireException e) {
						appendLog(e.toString());
					}
            	}
            }  
            
         }); 
        

        _btnDS1904IsTimerEnabled.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_DS1904Tester != null){
            		try {
						boolean enabled = _DS1904Tester.isOscillatorEnabled();
						appendLog("Timer Enabled: " + enabled);
					} catch (OneWireException e) {
						appendLog(e.toString());
					}
            	}
            }  
            
         }); 
        
        _btnDS1904SetTimer.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_DS1904Tester != null){
            		try {
            			Date now = new Date();
						_DS1904Tester.setTime(now, true);
						appendLog("Set Timer: " + now.toString());
					} catch (OneWireException e) {
						appendLog(e.toString());
					}
            	}
            }  
            
         }); 
        
        
        _btnLogClear.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	_txtLogOutput.setText("");
            }  
            
         });
	}
	

	public void setDS1904Tester(DS1904Tester tester){
		if(_DS1904Tester != null)
			_DS1904Tester.unregisterTraceListener();
		
		_DS1904Tester = tester;
		_DS1904Tester.registerTraceListener(new IButtonTraceListener() {
			
			@Override
			public void onTrace(OneWireMasterID master, OneWireSlaveID ibutton,
					String trace) {
				appendLog("iButton[" + ibutton + "] on Master[" + master.getId() + "] - " + trace);
			}
		});
		
	}
	
	
	private void appendLog(String log) {
		_txtLogOutput.append(log);
		_txtLogOutput.append("\n");
	}
	
}
