package seu.fan.onewire.test;


import seu.fan.onewire.IButtonBaseTester;
import seu.fan.onewire.test.OneWireTestView.OneWireMasterChangedListener;
import seu.fan.onewire.test.OneWireTestView.OneWireSlavesFoundListener;
import seu.fan.onewire.test.OneWireTestView.TraceListener;

import com.example.android_onewire_service_test.R;

import android.app.Activity;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.os.Bundle;
import android.widget.TabHost;

public abstract class IButtonTestActivity extends Activity {
	
	protected String TAG = this.getClass().getSimpleName();

	protected TabHost tabHost;   
	
	protected LogTestView logView;
	
	protected OneWireTestView oneWireView;
	
	protected IButtonTestView ibuttonView;
	
	protected IButtonBaseTester ibuttonTester;


	protected abstract int getLayoutID();
    
    protected abstract void duringCreation();
    
    protected abstract IButtonTestView createView();
    
    protected abstract IButtonBaseTester createTester(OneWireManager manager, OneWireMasterID master);
    
	
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
        setContentView(getLayoutID());
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

        
        duringCreation();
        
        
        ibuttonView = createView();
        
        oneWireView.addOneWireMasterChangedListener(new OneWireMasterChangedListener() {
			
			@Override
			public void onMasterChanged(OneWireMasterID oldMaster, OneWireMasterID newMaster) {

				if(newMaster != null) {
					
					ibuttonTester = createTester(oneWireView.getOneWireManager(), newMaster);
					
					ibuttonView.setIButtonTester(ibuttonTester);
					
					logView.appendLog("IButton View[" + ibuttonView.getClass().getSimpleName() 
							+ "] got its tester[" + ibuttonTester.getClass().getSimpleName() + "]");
				}
			}
		});
        
        oneWireView.addOneWireSlavesFoundListener(new OneWireSlavesFoundListener() {
			
			@Override
			public void onSlavesFound(OneWireSlaveID[] slaves) {

				if(ibuttonTester != null) {
					for(int i = 0; i < slaves.length; i++){
						if(ibuttonTester.isFamilyCorrect(slaves[i])) {
							ibuttonTester.changeIButton(slaves[i]);
							logView.appendLog("IButtonTester[" + ibuttonTester.getClass().getSimpleName() 
									+ "] changed its iButton[" + slaves[i] + "]");
							break;
						}
					}
				}
			}
		});
    }
    
    
}
