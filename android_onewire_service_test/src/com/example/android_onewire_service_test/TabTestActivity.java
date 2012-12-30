package com.example.android_onewire_service_test;


import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.util.Arrays;


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

public class TabTestActivity extends Activity {
	

	private TabHost tabHost;    
	
	//private LogTestView logView;
	
	
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
        setContentView(R.layout.tab_test_layout);
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
        
        
        //logView = new LogTestView(this);
        
        /*
        oneWireView = new OneWireTestView(this);
        
        oneWireView.registerTraceListener(new TraceListener() {
			
			@Override
			public void onTrace(String str) {
				logView.appendLog(str);
			}
		});
        */
    }
    
    
    
    
}