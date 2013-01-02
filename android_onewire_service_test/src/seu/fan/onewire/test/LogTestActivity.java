package seu.fan.onewire.test;


import com.example.android_onewire_service_test.R;

import android.app.Activity;
import android.os.Bundle;

public class LogTestActivity extends Activity {
	
	LogTestView myview;

	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        //setContentView(R.layout.main);
        setContentView(R.layout.log_test_layout);
        
        myview = new LogTestView(this);
    }
    

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
    
    
	
}
