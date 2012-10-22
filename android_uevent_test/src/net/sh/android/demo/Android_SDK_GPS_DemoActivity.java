package net.sh.android.demo;


import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class Android_SDK_GPS_DemoActivity extends Activity {
	
	static final String TAG = Android_SDK_GPS_DemoActivity.class.getSimpleName();
	
	LocationManager _locationManager;
	LocationListener _locationListener;
	
	long _minTimeInterval = 1000;	//by millisecond
	float _minDistanceChange = 0;
	
	TextView _txtView;
	
	ListView _listView;
	
	String[] mStrings;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gps_demo);
        
        _txtView = (TextView) findViewById(R.id.textView1);
        
        _listView = (ListView) findViewById(R.id.listView1);
        
        // Use an existing ListAdapter that will map an array
        // of strings to TextViews
        ArrayAdapter arrayAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mStrings);
        
        _listView.setTextFilterEnabled(true);
        
        // Acquire a reference to the system Location Manager
        _locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        
        // Define a listener that responds to location updates
        _locationListener = new LocationListener() {
    		
    		public void onLocationChanged(Location location) {
    			// Called when a new location is found by the network location provider.
    			
    			debug("onLocationChanged: location[" + location + "].");
    		}

    		public void onStatusChanged(String provider, int status, Bundle extras) {
    			debug("onStatusChanged: provider[" + provider + 
    					"], status[" + status + "], extras[" + extras + "].");
    		}

    		public void onProviderEnabled(String provider) {
    			debug("onProviderEnabled: provider[" + provider + "].");
    		}

    		public void onProviderDisabled(String provider) {
    			debug("onProviderDisabled: provider[" + provider + "].");
    		}
    	};
        
        
        // Register the listener with the Location Manager to receive location updates
    	// To request location updates from the GPS provider, substitute GPS_PROVIDER for NETWORK_PROVIDER. 
    	// You can also request location updates from both the GPS and the Network Location Provider by calling 
    	// requestLocationUpdates() twice—once for NETWORK_PROVIDER and once for GPS_PROVIDER.
        _locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, _minTimeInterval, _minDistanceChange, _locationListener);
        _locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, _minTimeInterval, _minDistanceChange, _locationListener);
        
        
    }


	@Override
	protected void onDestroy() {
		
		_locationManager.removeUpdates(_locationListener);
		
		super.onDestroy();
	}
    
    
    void debug(String text){
    	Log.e(TAG, text);
    	_txtView.setText(text);
    }
	
}