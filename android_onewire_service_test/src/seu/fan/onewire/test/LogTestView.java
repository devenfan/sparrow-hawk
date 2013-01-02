package seu.fan.onewire.test;


import com.example.android_onewire_service_test.R;

import android.app.Activity;

import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

public class LogTestView {
	
	private ScrollView _view;
	private EditText _txtLogInput;
	private TextView _txtLogOutput;
	private Button _btnLogAdd;
	private Button _btnLogClear;
	
	public LogTestView(Activity activity) {

		_view = new ScrollView(activity);   
		_view.setScrollContainer(true);   
		_view.setFocusable(true);   

        _txtLogInput = (EditText) activity.findViewById(R.id.txtLogInput);
		_txtLogOutput = (TextView) activity.findViewById(R.id.txtLogOutput);
        _btnLogAdd = (Button) activity.findViewById(R.id.btnLogAdd);
        _btnLogClear = (Button) activity.findViewById(R.id.btnLogClear);

        //enable Horizontally scrolling...
        //_txtLog.setHorizontallyScrolling(true);  
        
        //let it move itself...
        _txtLogOutput.setMovementMethod(ScrollingMovementMethod.getInstance());
        
        _btnLogAdd.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	appendLog(_txtLogInput.getText().toString());
            }  
            
         }); 

        
        _btnLogClear.setOnClickListener(new View.OnClickListener() {  
        	
            public void onClick(View v) {  
            	clearLog();
            }  
            
         });
	}
	
	
	public void appendLog(String log) {
		_txtLogOutput.append(log);
		_txtLogOutput.append("\r\n");
	}
	
	public void clearLog() {
		_txtLogOutput.setText("");
	}
}
