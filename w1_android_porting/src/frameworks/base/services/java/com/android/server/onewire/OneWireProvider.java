/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server.onewire;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.location.Criteria;
import android.location.IGpsStatusListener;
import android.location.IGpsStatusProvider;
import android.location.ILocationManager;
import android.location.INetInitiatedListener;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.SntpClient;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.WorkSource;
import android.provider.Settings;
import android.provider.Telephony.Carriers;
import android.provider.Telephony.Sms.Intents;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.telephony.gsm.GsmCellLocation;
import android.util.Log;
import android.util.SparseIntArray;

import com.android.internal.app.IBatteryStats;
import com.android.internal.location.GpsNetInitiatedHandler;
import com.android.internal.location.GpsNetInitiatedHandler.GpsNiNotification;
import com.android.internal.telephony.Phone;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.StringBufferInputStream;
import java.util.ArrayList;
import java.util.Date;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;

/**
 * The implementation of OneWireProvider used by OneWireManager.
 *
 * {@hide}
 */
public class OneWireProvider {

    private static final String TAG = "OneWireProvider";

    private static final boolean DEBUG = false;
    private static final boolean VERBOSE = false;


    static { 
    	class_init_native(); 
    }
    
    
    
    //Callbacks (Invoked by JNI native codes...) ---------------------------------
    
    
    /**
     * masterId: 4 bytes
     * */
    private void masterAdded(int masterId){
    	//TODO
    }

    /**
     * masterId: 4 bytes
     * */
    private native void masterRemoved(int masterId){
    	//TODO
    }

    /**
     * slaveRN: 8 bytes
     * */
    private native void slaveAdded(long slaveRN){
    	//TODO
    }

    /**
     * slaveRN: 8 bytes
     * */
    private native void slaveRemoved(long slaveRN){
    	//TODO
    }
    
    private OneWireMasterID getMasterIDFromInt(int masterId){
    	return new OneWireMasterID(masterId);
    }
    
    private OneWireSlaveID getSlaveIDFromLong(long slaveRN){
    	//TODO slaveRN.
    	return new OneWireSlaveID();
    }
    

    //native (JNI mapped functions) ----------------------------------------
	
    private static native void class_init_native();
    
    private static native boolean native_is_supported();
    
    
    
    private static boolean native_start();
    
    private static void native_stop();
    
    //return masterId by integer...
    private static int native_get_current_master(); 
    
    //return the slave count...
    private static int native_get_current_slaves(long[] slaveRNs);
    
    //TODO return status...
    private static void native_begin_exclusive(); 
    
    //TODO return status...
    private static void native_end_exclusive();
    
    //return the master count...
    private static int native_list_masters(int[] masterIDs);
    
    //return the slave count...
    private static int native_search_slaves(
            int masterId, boolean isSearchAlarm, long[] slaveRNs);
    
    private static boolean native_master_reset(int masterId);
    
    private static boolean native_master_touch(
            int masterId, byte[] dataIn, int dataInLen, byte[] dataOut);
    
    private static boolean native_master_read(
            int masterId, int readLen, byte[] dataReadOut);
    
    private static boolean native_master_write(
            int masterId, int writeLen, byte[] dataWriteIn);
    
}
