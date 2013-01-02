package seu.fan.onewire.test;

import seu.fan.onewire.IButtonBaseTester;
import seu.fan.onewire.IButtonTraceListener;
import android.app.Activity;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.widget.ScrollView;

public abstract class IButtonTestView {

	protected ScrollView _view;
	
	protected IButtonBaseTester _tester;
	
	
	public IButtonTestView(Activity activity) {
		
		_view = new ScrollView(activity);   
		_view.setScrollContainer(true);   
		_view.setFocusable(true);   
	}
	
	public void setIButtonTester(IButtonBaseTester tester){
		
		if(_tester != null)
			_tester.unregisterTraceListener();
		
		_tester = tester;
		_tester.registerTraceListener(new IButtonTraceListener() {
			
			@Override
			public void onTrace(OneWireMasterID master, OneWireSlaveID ibutton,
					String trace) {
				appendLog("iButton[" + ibutton + "] on Master[" + master.getId() + "] - " + trace);
			}
		});
		
	}
	

	protected abstract void appendLog(String log);
}
