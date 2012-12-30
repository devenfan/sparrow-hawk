package com.example.android_onewire_service_test;


import android.app.Activity;
import android.os.Bundle;

public class OneWireTestActivity extends Activity {
	
	OneWireTestView oneWireTestView;
	

	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        //setContentView(R.layout.main);
        setContentView(R.layout.onewire_test_layout);
        
        oneWireTestView = new OneWireTestView(this);
    }
    

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
    
    
	
}
