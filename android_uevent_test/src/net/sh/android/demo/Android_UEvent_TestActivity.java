package net.sh.android.demo;


import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import android.os.*;

public class Android_UEvent_TestActivity extends Activity {
	
	static final String TAG = Android_UEvent_TestActivity.class.getSimpleName();
	
	
	
	long _minTimeInterval = 1000;	//by millisecond
	float _minDistanceChange = 0;
	
	TextView _txtView;
	
	
	UEventObserver uEventObserver = new UEventObserver() {
		
		@Override
		public void onUEvent(UEvent event) {
			
			String text = "UEvent coming: " + event.toString();
			
			debug(text);
			
			Toast.makeText(Android_UEvent_TestActivity.this, text, Toast.LENGTH_LONG).show();
			
		}
	};
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gps_demo);
        
        _txtView = (TextView) findViewById(R.id.textUEvent);
        
//        // Use an existing ListAdapter that will map an array
//        // of strings to TextViews
//        ArrayAdapter arrayAdapter = new ArrayAdapter<String>(this,
//                android.R.layout.simple_list_item_1, mStrings);
        
        uEventObserver.startObserving("");
        
        
    }


	@Override
	protected void onDestroy() {
		
		uEventObserver.stopObserving();
		
		super.onDestroy();
	}
    
    
    void debug(String text){
    	Log.e(TAG, text);
    	_txtView.setText(text);
    }
	
}