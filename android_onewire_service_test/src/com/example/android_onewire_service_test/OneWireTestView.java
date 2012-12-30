package com.example.android_onewire_service_test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.onewire.OneWireException;
import android.onewire.OneWireListener;
import android.onewire.OneWireManager;
import android.onewire.OneWireMasterID;
import android.onewire.OneWireSlaveID;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;

public class OneWireTestView {

	
	//UI Attributes ------------------------------------------------------------
	
	private ScrollView _view;
	
	private TextView _txtOneWireStatus;
	private Button _btnListMasters;
	private Button _btnGetMasters;
	
	private Button _btnEnableDebug;
	private Button _btnDisableDebug;
	
	private Button _btnBeginExclusive;
	private Button _btnEndExclusive;
	private Button _btnMasterSearch;
	private Button _btnMasterReset;
	
	private EditText _txtOneWireInput;
	private Button _btnMasterRead;
	private Button _btnMasterWrite;
	private Button _btnMasterTouch;
	private Button _btnClearOneWireInput;
	
	private Spinner _spinnerMasters;  

	//OneWire Attributes -------------------------------------------------------

	private OneWireManager _oneWireManager;
	
	//must be initialized, otherwise ArrayAdapter will not be initialized...
	//private OneWireMasterID[] _masters = new OneWireMasterID[10]; 
	private List<OneWireMasterID> _masters = new ArrayList<OneWireMasterID>();
	
	private OneWireMasterID _currentMaster;
	
	private ArrayAdapter<OneWireMasterID> _mastersAdapter;
	
	//About Trace --------------------------------------------------------------
	
	private TraceListener _tranceListener;
	
	
	public interface TraceListener {
		
		void onTrace(String str);
	}
	
	
	public void registerTraceListener(TraceListener tranceListener) {
		_tranceListener = tranceListener;
	}
	
	public void unregisterTraceListener() {
		_tranceListener = null;
	}
	
	
	private void trace(String str) {
		if(_tranceListener != null)
			_tranceListener.onTrace(str);
		
		_txtOneWireStatus.setText(str);
	}
	
	
	private boolean isSupported(){
		return _oneWireManager != null;
	}
	
	
	//Constructors -------------------------------------------------------------
	
	
	public OneWireTestView(Activity activity) {
		
		
		findUIControls(activity);

		
		initializeUIEffects(activity);
		
		
		initializeOneWireManager(activity);
		
		
		
		initializeOneWireButtons();
		
	}
	
	
	
	
	private void findUIControls(Activity activity){
		
		_view = new ScrollView(activity);   
		_view.setScrollContainer(true);   
		_view.setFocusable(true);   
		
		_txtOneWireStatus = (TextView) activity.findViewById(R.id.txtOneWireStatus);
		_btnListMasters = (Button) activity.findViewById(R.id.btnListMasters);
		_btnGetMasters = (Button) activity.findViewById(R.id.btnGetMasters);
		

		_btnEnableDebug = (Button) activity.findViewById(R.id.btnEnableDebug);
		_btnDisableDebug = (Button) activity.findViewById(R.id.btnDisableDebug);
		
		_btnBeginExclusive = (Button) activity.findViewById(R.id.btnBeginExclusive);
		_btnEndExclusive = (Button) activity.findViewById(R.id.btnEndExclusive);
		_btnMasterSearch = (Button) activity.findViewById(R.id.btnMasterSearch);
		_btnMasterReset = (Button) activity.findViewById(R.id.btnMasterReset);
		
		_txtOneWireInput = (EditText) activity.findViewById(R.id.txtOneWireInput);
		_btnMasterRead = (Button) activity.findViewById(R.id.btnMasterRead);
		_btnMasterWrite = (Button) activity.findViewById(R.id.btnMasterWrite);
		_btnMasterTouch = (Button) activity.findViewById(R.id.btnMasterTouch);
		_btnClearOneWireInput = (Button) activity.findViewById(R.id.btnClearOneWireInput);
		
		_spinnerMasters = (Spinner) activity.findViewById(R.id.spinnerMasters);  
	}
	
	
	
	
	private void initializeUIEffects(Activity activity){

        
        //let it move itself...
		_txtOneWireStatus.setMovementMethod(ScrollingMovementMethod.getInstance());
		
		

		
        //将可选内容与ArrayAdapter连接起来  
		_mastersAdapter = new ArrayAdapter<OneWireMasterID>(activity, android.R.layout.simple_spinner_item, _masters);  
          
        //设置下拉列表的风格  
		_mastersAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);  
          
        //添加事件Spinner事件监听    
		_spinnerMasters.setOnItemSelectedListener(new OnItemSelectedListener(){

			@Override
			public void onItemSelected(AdapterView<?> paramAdapterView,
					View paramView, int paramInt, long paramLong) {
				
				if(_mastersAdapter.getCount() > 0) {
					_currentMaster = _mastersAdapter.getItem(paramInt);
					
					trace("Master Selected: " + _currentMaster);
				}
			}

			@Override
			public void onNothingSelected(AdapterView<?> paramAdapterView) {
				_currentMaster = null;
				
				trace("Master Selected: NULL");
			}

		});  
		

        //将adapter 添加到spinner中  
		_spinnerMasters.setAdapter(_mastersAdapter);  

        //设置默认值  
		_spinnerMasters.setVisibility(View.VISIBLE);  
		
	}
	
	
	
	
	private void initializeOneWireManager(Activity activity){

        _oneWireManager = (OneWireManager) activity.getSystemService(Context.ONEWIRE_SERVICE);
        
        _oneWireManager.addOneWireListener(new OneWireListener() {
			
			@Override
			public void onOneWireSlaveRemoved(OneWireMasterID paramOneWireMasterID,
					OneWireSlaveID paramOneWireSlaveID) {
				trace("oneWireSlaveRemoved: slave[" + 
					paramOneWireSlaveID + "] on master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireSlaveAdded(OneWireMasterID paramOneWireMasterID,
					OneWireSlaveID paramOneWireSlaveID) {
				trace("onOneWireSlaveAdded: slave[" + 
						paramOneWireSlaveID + "] on master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireMasterRemoved(OneWireMasterID paramOneWireMasterID) {
				trace("onOneWireMasterRemoved: master[" + paramOneWireMasterID + "] \n");
			}
			
			@Override
			public void onOneWireMasterAdded(OneWireMasterID paramOneWireMasterID) {
				trace("onOneWireMasterAdded: master[" + paramOneWireMasterID + "] \n");
			}
		});
        
        if(_oneWireManager.isDebugEnabled()) {
        	_btnEnableDebug.setEnabled(false);
        	_btnDisableDebug.setEnabled(true);
        } else {
        	_btnEnableDebug.setEnabled(true);
        	_btnDisableDebug.setEnabled(false);
        }
	}
	
	
	
	
	
	private void initializeOneWireButtons(){
		
		_btnListMasters.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				OneWireMasterID[] masters = null;
				
				try {
					masters = _oneWireManager.listMasters();
					
				} catch (OneWireException e) {

					trace(e.toString());
					return;
				}

				String tr = "List Masters: "
						+ (masters == null ? "NULL" : Arrays.toString(masters));
				
				_mastersAdapter.clear();
				
				if(masters != null){
					for(OneWireMasterID master : masters){
						_mastersAdapter.add(master);
					}
				} 

				trace(tr);
			}

		});
		
		
		_btnGetMasters.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				OneWireMasterID[] masters = _oneWireManager.getCurrentMasters();

				String tr = "Get Current Masters: "
						+ (masters == null ? "NULL" : Arrays.toString(masters));
				
				_mastersAdapter.clear();
				
				if(masters != null){
					for(OneWireMasterID master : masters){
						_mastersAdapter.add(master);
					}
				} 

				trace(tr);
			}

		});
		
		_btnEnableDebug.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				_oneWireManager.setDebugEnabled(true);
				
				if(_oneWireManager.isDebugEnabled()) {
		        	_btnEnableDebug.setEnabled(false);
		        	_btnDisableDebug.setEnabled(true);
		        } else {
		        	_btnEnableDebug.setEnabled(true);
		        	_btnDisableDebug.setEnabled(false);
		        }

				String tr = "Set Debug Enabled... ";
				
				trace(tr);
			}

		});
		
		_btnDisableDebug.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				_oneWireManager.setDebugEnabled(false);
				
				if(_oneWireManager.isDebugEnabled()) {
		        	_btnEnableDebug.setEnabled(false);
		        	_btnDisableDebug.setEnabled(true);
		        } else {
		        	_btnEnableDebug.setEnabled(true);
		        	_btnDisableDebug.setEnabled(false);
		        }

				String tr = "Set Debug Disabled... ";
				
				trace(tr);
			}

		});
		
		
		_btnBeginExclusive.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				boolean result = _oneWireManager.beginExclusive();

				String tr = "OneWire begnExclusive: "
						+ (result ? "OK" : "Failed");

				trace(tr);
			}

		});

		_btnEndExclusive.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				_oneWireManager.endExclusive();

				String tr = "OneWire endExclusive: ...";

				trace(tr);
			}

		});
        
        
		_btnMasterSearch.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {

				if(_currentMaster != null) {
					
					OneWireSlaveID[] slaves = null;
					
					try {
						slaves = _oneWireManager.searchSlaves(_currentMaster);
						
					} catch (OneWireException e) {

						trace(e.toString());
						return;
					}
	
					String tr = "Search slaves: "
							+ (slaves == null ? "NULL" : Arrays.toString(slaves));
	
					trace(tr);
				}
			}

		});
	}
	
}
