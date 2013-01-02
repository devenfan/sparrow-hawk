package seu.fan.onewire.test;



import seu.fan.onewire.DS1904Tester;
import seu.fan.onewire.test.OneWireTestView.OneWireMasterChangedListener;
import seu.fan.onewire.test.OneWireTestView.OneWireSlavesFoundListener;
import seu.fan.onewire.test.OneWireTestView.TraceListener;

import com.example.android_onewire_service_test.R;

import android.app.Activity;
import android.app.TabActivity;
import android.content.Context;
import android.onewire.OneWireListener;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.TabHost;
import android.widget.TextView;

public class DS1904TestActivity extends Activity {
	
	static String TAG = DS1904TestActivity.class.getSimpleName();

	private TabHost tabHost;    
	
	private LogTestView logView;
	
	private OneWireTestView oneWireView;
	
	private DS1904TestView ds1904View;
	
	private DS1904Tester ds1904Tester;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	
        super.onCreate(savedInstanceState);

        
        
        /*
         * <p>Call setup() before adding tabs if loading TabHost using findViewById(). 
         * <i><b>However</i></b>: You do not need to call setup() after getTabHost() 
         * in {@link android.app.TabActivity TabActivity}.
         * 
         */
        //----------------------------------------------------------------------
        setContentView(R.layout.main_with_ds1904);
        tabHost = (TabHost) findViewById(R.id.tabhost);
        //tabHost = (TabHost) findViewById(com.android.internal.R.id.tabhost);
        tabHost.setup();

        //LayoutInflater loyoutInflater = LayoutInflater.from(this);   
        //inflater_tab1.inflate(R.layout.tab1, tabHost.getTabContentView());  
        //inflater_tab1.inflate(R.layout.tab2, tabHost.getTabContentView());   
        //inflater_tab1.inflate(R.layout.log_test_layout, tabHost.getTabContentView());  
        //inflater_tab1.inflate(R.layout.log_test_layout, tabHost.getTabContentView());  
        //inflater_tab1.inflate(R.layout.log_test_layout, tabHost.getTabContentView());  
        //tabHost.addTab(tabHost.newTabSpec("tab_test1").setIndicator("TAB 11").setContent(R.id.LinearLayout01));  
        //tabHost.addTab(tabHost.newTabSpec("tab_test1").setIndicator("TAB 11").setContent(R.id.FrameLayout02)); 
        //----------------------------------------------------------------------
        
        
        //----------------------------------------------------------------------
        //tabHost = getTabHost();
        //LayoutInflater.from(this).inflate(R.layout.tab_test_layout, tabHost.getTabContentView(), true);  
        tabHost.addTab(tabHost.newTabSpec("tab1").setIndicator("TAB1", null).setContent(R.id.tab1)); 
        tabHost.addTab(tabHost.newTabSpec("tab2").setIndicator("TAB2", null).setContent(R.id.tab2));
        tabHost.addTab(tabHost.newTabSpec("tab3").setIndicator("TAB3", null).setContent(R.id.tab3));  
        //this.setContentView(tabHost);
        //----------------------------------------------------------------------
        
        
        
        logView = new LogTestView(this);
        

        oneWireView = new OneWireTestView(this);
        
        oneWireView.registerTraceListener(new TraceListener() {
			
			@Override
			public void onTrace(String str) {
				logView.appendLog(str);
			}
		});

        
        ds1904View = new DS1904TestView(this);
        
        oneWireView.addOneWireMasterChangedListener(new OneWireMasterChangedListener() {
			
			@Override
			public void onMasterChanged(OneWireMasterID oldMaster,
					OneWireMasterID newMaster) {

				if(newMaster != null) {
					ds1904Tester = new DS1904Tester(oneWireView.getOneWireManager(), newMaster);
					
					ds1904View.setDS1904Tester(ds1904Tester);
					
					logView.appendLog("DS1904View got its tester...");
				}
			}
		});
        
        oneWireView.addOneWireSlavesFoundListener(new OneWireSlavesFoundListener() {
			
			@Override
			public void onSlavesFound(OneWireSlaveID[] slaves) {

				for(int i = 0; i < slaves.length; i++){
					if(!ds1904Tester.isDS1904(slaves[i])){
						logView.appendLog("iButton[" + slaves[i].toString() + "] is not DS1904!");
					} else {
						ds1904Tester.changeIButton(slaves[i]);
						break;
					}
				}
				
			}
		});
        
    }
    
    
    
    
    
}