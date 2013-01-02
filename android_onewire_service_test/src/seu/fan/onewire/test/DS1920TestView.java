package seu.fan.onewire.test;



import com.example.android_onewire_service_test.R;

import seu.fan.onewire.DS1920Tester;
import seu.fan.onewire.utils.ConvertCodec;

import android.app.Activity;

import android.onewire.OneWireException;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class DS1920TestView extends IButtonTestView {
	
	private TextView _txtLogOutput;
	private Button _btnLogClear;
	private Button _btnDS1920GetTemperature;
	private Button _btnDS1920GetState;
	
	public DS1920TestView(Activity activity) {
		
		super(activity);

		_txtLogOutput = (TextView) activity.findViewById(R.id.txtDS1920LogOutput);
		_btnDS1920GetTemperature = (Button) activity.findViewById(R.id.btnDS1920GetTemperature);
		_btnDS1920GetState = (Button) activity.findViewById(R.id.btnDS1920GetState);
		_btnLogClear = (Button) activity.findViewById(R.id.btnDS1920LogClear);

        //enable Horizontally scrolling...
        //_txtLog.setHorizontallyScrolling(true);  
        
        //let it move itself...
        _txtLogOutput.setMovementMethod(ScrollingMovementMethod.getInstance());
        
        
        _btnDS1920GetTemperature.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_tester != null && _tester instanceof DS1920Tester){
            		try {
						double temp = ((DS1920Tester)_tester).readTemperature();
						appendLog("Get Temperature: " + temp);
					} catch (OneWireException e) {
						appendLog(e.toString());
					}
            	}
            } 
            
         }); 
        
        _btnDS1920GetState.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	if(_tester != null && _tester instanceof DS1920Tester){
            		try {
            			byte[] state = ((DS1920Tester)_tester).readState();
						appendLog("Get State: " + ConvertCodec.bytesToHexString(state));
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
	

	protected void appendLog(String log) {
		_txtLogOutput.append(log);
		_txtLogOutput.append("\n");
	}
	
}
