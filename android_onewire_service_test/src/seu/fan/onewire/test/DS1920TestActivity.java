package seu.fan.onewire.test;



import com.example.android_onewire_service_test.R;

import seu.fan.onewire.DS1904Tester;
import seu.fan.onewire.DS1920Tester;
import seu.fan.onewire.IButtonBaseTester;

import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;

public class DS1920TestActivity extends IButtonTestActivity {
	
	

	@Override
	protected int getLayoutID() {
		return R.layout.main_with_ds1920;
	}


	@Override
	protected void duringCreation() {
		// TODO Auto-generated method stub
		
	}



	@Override
	protected IButtonTestView createView() {
		return new DS1920TestView(this);
	}



	@Override
	protected IButtonBaseTester createTester(OneWireManager manager, OneWireMasterID master) {
		return new DS1920Tester(manager, master);
	}
    
    
    
    
    
}