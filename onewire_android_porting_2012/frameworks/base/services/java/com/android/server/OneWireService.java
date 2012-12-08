package com.android.server;



import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.Set;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.onewire.*;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.util.PrintWriterPrinter;


/**
 * The service class that manages OneWire resources
 * updates and alerts.
 *
 * {@hide}
 */
public class OneWireService extends IOneWireService.Stub {

    private static final String TAG = OneWireService.class.getSimpleName();

	private static final boolean LOCAL_LOGV = true;
	private static final boolean LOCAL_LOGD = true;


	// Variables --------------------------------------------------------------


    private final HashMap<Object, ListenerWrapper> mReceivers = new HashMap<Object, ListenerWrapper>();

	private final Context mContext;
		
    // wakelock variables
    private final static String WAKELOCK_KEY = "OneWireService";
    private PowerManager.WakeLock mWakeLock = null;
    private int mPendingBroadcasts = 0;
	
	/**
     * Object used internally for synchronization
     */
    private final Object mLock = new Object();



	// Constructors --------------------------------------------------------------


    /**
     * @param context  the context that the OneWireService runs in
     */
    public OneWireService(Context context) {
        super();
        mContext = context;
        
        if(!native_is_supported()) {
        	Log.e(TAG, "OneWire is not supported, OneWireService cannot be started!");
        	return;
        }
        
		if(!native_start()) {
			Log.i(TAG, "OneWire internal error during the starting, OneWireService cannot be started!");
			return;
		}
        
        new OneWireInnerThread().start();
		
        Log.v(TAG, "OneWireService started!");
    }

	
	private void initialize() {
		// Create a wake lock, needs to be done before calling loadProviders() below
		PowerManager powerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
		mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, WAKELOCK_KEY);

		/*
		// Load providers
		loadProviders();

		// Register for Network (Wifi or Mobile) updates
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

		// Register for Package Manager updates
		intentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
		intentFilter.addAction(Intent.ACTION_PACKAGE_RESTARTED);
		mContext.registerReceiver(mBroadcastReceiver, intentFilter);

		// listen for settings changes
		ContentResolver resolver = mContext.getContentResolver();
		Cursor settingsCursor = resolver.query(Settings.Secure.CONTENT_URI, null,
				"(" + Settings.System.NAME + "=?)",
				new String[]{Settings.Secure.LOCATION_PROVIDERS_ALLOWED},
				null);
		mSettings = new ContentQueryMap(settingsCursor, Settings.System.NAME, true, mLocationHandler);
		SettingsObserver settingsObserver = new SettingsObserver();
		mSettings.addObserver(settingsObserver);
		*/
	}
	

	// ListenerWrapper(Receiver) -------------------------------------------------------

	//Consider ListenerWrapper as Receiver
	private class ListenerWrapper implements IBinder.DeathRecipient {

		final IOneWireListener mListener;
		final Object mKey;
		int mPendingBroadcasts;

		ListenerWrapper(IOneWireListener listener) {
            mListener = listener;
            mKey = listener.asBinder();
        }

		public IOneWireListener getListener() {
            if (mListener != null) {
                return mListener;
            }
            throw new IllegalStateException("Request for non-existent listener");
        }

		@Override
        public boolean equals(Object otherObj) {
            if (otherObj instanceof ListenerWrapper) {
                return mKey.equals(
					((ListenerWrapper)otherObj).mKey);
            }
            return false;
        }

		@Override
        public int hashCode() {
            return mKey.hashCode();
        }

		@Override
        public String toString() {
            if (mListener != null) {
                return "ListenerWrapper{"
                        + Integer.toHexString(System.identityHashCode(this))
                        + " Listener " + mKey + "}";
            } else {
                return "ListenerWrapper{"
                        + Integer.toHexString(System.identityHashCode(this))
                        + " Intent " + mKey + "}";
            }
        }

		public void binderDied() {

            logV(TAG, "IOneWireListener died");
			
            synchronized (mLock) {
                OneWireService.this.removeReceiver(this);
            }
            synchronized (this) {
                if (mPendingBroadcasts > 0) {
                    OneWireService.this.decrementPendingBroadcasts();
                    mPendingBroadcasts = 0;
                }
            }
        }

		
        // this must be called while synchronized by caller in a synchronized block
        // containing the sending of the broadcaset
        private void incrementPendingBroadcastsLocked() {
            if (mPendingBroadcasts++ == 0) {
                OneWireService.this.incrementPendingBroadcasts();
            }
        }

        private void decrementPendingBroadcastsLocked() {
            if (--mPendingBroadcasts == 0) {
                OneWireService.this.decrementPendingBroadcasts();
            }
        }


		public boolean callOneWireMasterAddedOrRemoved(OneWireMasterID master, boolean isAddedOrRemoved) {
            if (mListener != null) {
                try {
					synchronized (this) {
                        // synchronize to ensure incrementPendingBroadcastsLocked()
                        // is called before decrementPendingBroadcasts()

						if(isAddedOrRemoved) {
							mListener.onOneWireMasterAdded(master);
						} else {
							mListener.onOneWireMasterRemoved(master);
						}
						
                        // call this after broadcasting so we do not increment
                        // if we throw an exeption.
                        incrementPendingBroadcastsLocked();
                    }
                } catch (RemoteException e) {
                    return false;
                }
            } 
			
            return true;
        }


		public boolean callOneWireSlaveAddedOrRemoved(OneWireMasterID master, 
				OneWireSlaveID slaveOfTheMaster, boolean isAddedOrRemoved) {
            if (mListener != null) {
                try {
					synchronized (this) {
                        // synchronize to ensure incrementPendingBroadcastsLocked()
                        // is called before decrementPendingBroadcasts()
						
						if(isAddedOrRemoved) {
							mListener.onOneWireSlaveAdded(master, slaveOfTheMaster);
						} else {
							mListener.onOneWireSlaveRemoved(master, slaveOfTheMaster);
						}
						
                        // call this after broadcasting so we do not increment
                        // if we throw an exeption.
                        incrementPendingBroadcastsLocked();
                    }
                } catch (RemoteException e) {
                    return false;
                }
            } 
			
            return true;
        }

		
	}



	//It will be invoked by upper layer...
	public void oneWireCallbackFinished(IOneWireListener listener) {
		
        //Do not use getReceiver here as that will add the IOneWireListener to
        //the receiver list if it is not found.  
        //If it is not found then the ListenerWrapper was removed when 
        // it had a pending broadcast and should not be added back.
        IBinder binder = listener.asBinder();
        ListenerWrapper receiver = mReceivers.get(binder);
        if (receiver != null) {
            synchronized (receiver) {
                // so wakelock calls will succeed
                long identity = Binder.clearCallingIdentity();
                receiver.decrementPendingBroadcastsLocked();
                Binder.restoreCallingIdentity(identity);
           }
        }
    }
	
	
	private void addReceiver(ListenerWrapper receiver) {
		
        logV(TAG, "addReceiver: receiver = " + receiver);
        
		if(mReceivers.get(receiver.mKey) == null) {

			mReceivers.put(receiver.mKey, receiver);
			
			try {
	        	receiver.getListener().asBinder().linkToDeath(receiver, 0);
	        } catch (RemoteException e) {
	            Log.e(TAG, "linkToDeath failed:", e);
	        }
			
		} else {
			
        	logV("receiver already in the list...");
        }
        
	}
	
	private void removeReceiver(ListenerWrapper receiver) {

		logV(TAG, "removeReceiver: receiver = " + receiver);
        
        // so wakelock calls will succeed
        //final int callingUid = Binder.getCallingUid();
        long identity = Binder.clearCallingIdentity();
        try {
            if (mReceivers.remove(receiver.mKey) != null) {
                receiver.getListener().asBinder().unlinkToDeath(receiver, 0);
                synchronized(receiver) {
                    if(receiver.mPendingBroadcasts > 0) {
                        decrementPendingBroadcasts();
                        receiver.mPendingBroadcasts = 0;
                    }
                }
            } else {
            	logV("receiver not in the list...");
            }
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }

	// OneWireMessageHandler ----------------------------------------------

	private static final int MESSAGE_ONEWIRE_MASTER_ADDED = 1;
	private static final int MESSAGE_ONEWIRE_MASTER_REMOVED = 2;
	private static final int MESSAGE_ONEWIRE_SLAVE_ADDED = 3;
	private static final int MESSAGE_ONEWIRE_SLAVE_REMOVED = 4;

	private class OneWireMessageHandler extends Handler {

		OneWireMasterID master = null;
		OneWireSlaveID slave = null;
		MasterSlaveWrapper wrapper = null;
		
        @Override
        public void handleMessage(Message msg) {
            try {
            	
            	//Log.i(TAG, "Begin in OneWireMessageHandler.handleMessage: " + msg.what + "|" + msg.obj);
            	
				switch(msg.what){
					case MESSAGE_ONEWIRE_MASTER_ADDED:
						master = (OneWireMasterID) msg.obj;						
						handleOneWireMasterAddedOrRemoved(master, true);						
						break;
					case MESSAGE_ONEWIRE_MASTER_REMOVED:
						master = (OneWireMasterID) msg.obj;						
						handleOneWireMasterAddedOrRemoved(master, false);	
						break;
					case MESSAGE_ONEWIRE_SLAVE_ADDED:
						wrapper = (MasterSlaveWrapper) msg.obj;						
						handleOneWireSlaveAddedOrRemoved(wrapper.master, wrapper.slaveOfTheMaster, true);
						break;
					case MESSAGE_ONEWIRE_SLAVE_REMOVED:
						wrapper = (MasterSlaveWrapper) msg.obj;						
						handleOneWireSlaveAddedOrRemoved(wrapper.master, wrapper.slaveOfTheMaster, false);
						break;
				};

            	//Log.i(TAG, "End in OneWireMessageHandler.handleMessage: " + msg.what + "|" + msg.obj);
            	
            } catch (Exception e) {
                // Log, don't crash!
                Log.e(TAG, "Exception in OneWireMessageHandler.handleMessage:", e);
            }
        }
    }
	
	
	private class MasterSlaveWrapper {
		OneWireMasterID master;
		OneWireSlaveID slaveOfTheMaster;
		
		MasterSlaveWrapper(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster){
			this.master = master;
			this.slaveOfTheMaster = slaveOfTheMaster;
		}
	}

	
	private void handleOneWireMasterAddedOrRemoved(OneWireMasterID master, boolean isAddedOrRemoved) {
	
		ArrayList<ListenerWrapper> deadReceivers = null;

		for(Object receiverObj : mReceivers.values().toArray()) {
			
			ListenerWrapper receiver = (ListenerWrapper)receiverObj;
			
			if (!receiver.callOneWireMasterAddedOrRemoved(master, isAddedOrRemoved)) {
				
				Log.w(TAG, "RemoteException callOneWireMasterAddedOrRemoved on " + receiver);
				if (deadReceivers == null) {
					deadReceivers = new ArrayList<ListenerWrapper>();
				}
				deadReceivers.add(receiver);
			}
		}

		if (deadReceivers != null) {
			for (int i = deadReceivers.size() - 1; i >= 0; i--) {
				removeReceiver(deadReceivers.get(i));
			}
		}
			
	}

	private void handleOneWireSlaveAddedOrRemoved(OneWireMasterID master, 
			OneWireSlaveID slaveOfTheMaster, boolean isAddedOrRemoved) {
	
		ArrayList<ListenerWrapper> deadReceivers = null;

		for(Object receiverObj : mReceivers.values().toArray()) {
			
			ListenerWrapper receiver = (ListenerWrapper)receiverObj;
			
			if (!receiver.callOneWireSlaveAddedOrRemoved(master, slaveOfTheMaster, isAddedOrRemoved)) {
				
				Log.w(TAG, "RemoteException callOneWireSlaveAddedOrRemoved on " + receiver);
				if (deadReceivers == null) {
					deadReceivers = new ArrayList<ListenerWrapper>();
				}
				deadReceivers.add(receiver);
			}
		}

		if (deadReceivers != null) {
			for (int i = deadReceivers.size() - 1; i >= 0; i--) {
				removeReceiver(deadReceivers.get(i));
			}
		}
			
	}
	
	
    private void addListener(IOneWireListener listener) {
    	
        IBinder binder = listener.asBinder();
        
        ListenerWrapper receiver = mReceivers.get(binder);

        if(receiver == null)
        	addReceiver(new ListenerWrapper(listener));
    }
    
    private void removeListener(IOneWireListener listener) {

        IBinder binder = listener.asBinder();
        
        ListenerWrapper receiver = mReceivers.get(binder);

        if(receiver != null)
        	removeReceiver(receiver);
    }
	

	// Inner Thread --------------------------------------------------------
	
	private OneWireMessageHandler oneWireMessageHandler;

	private class OneWireInnerThread extends Thread {

		public OneWireInnerThread(){
			super("OneWireInnerThread");
		}

 		public void run() {
	        Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
	        Looper.prepare();
	        oneWireMessageHandler = new OneWireMessageHandler();
	        initialize();
	        Looper.loop();
	    }
		
	}


	// Wake locks -------------------------------------------------------------

    private void incrementPendingBroadcasts() {
        synchronized (mWakeLock) {
            if (mPendingBroadcasts++ == 0) {
                try {
                    mWakeLock.acquire();
                    logD("Acquired wakelock: " + WAKELOCK_KEY);
                } catch (Exception e) {
                    // This is to catch a runtime exception thrown when we try to release an
                    // already released lock.
                    Log.e(TAG, "exception in acquireWakeLock()", e);
                }
            }
        }
    }

    private void decrementPendingBroadcasts() {
        synchronized (mWakeLock) {
            if (--mPendingBroadcasts == 0) {
                try {
                    // Release wake lock
                    if (mWakeLock.isHeld()) {
                        mWakeLock.release();
                        logD("Released wakelock: " + WAKELOCK_KEY);
                    } else {
                        logD("Can't release wakelock again!");
                    }
                } catch (Exception e) {
                    // This is to catch a runtime exception thrown when we try to release an
                    // already released lock.
                    Log.e(TAG, "exception in releaseWakeLock()", e);
                }
            }
        }
    }


	private void logV(String... log) {
    	String logs = "";
    	for(String s : log) {
    		logs += s;
    	}
        //if (Log.isLoggable(TAG, Log.VERBOSE)) {
		if(LOCAL_LOGV) {
            Log.d(TAG, logs);
        }
    }

    private void logD(String... log) {
    	String logs = "";
    	for(String s : log) {
    		logs += s;
    	}
		if(LOCAL_LOGD) {
            Log.d(TAG, logs);
        }
    }

    // public --------------------------------------------------------------

	

	static {

        //android_servers.so will be loaded by SystemServer class
    	//System.loadLibrary("android_servers");

    	class_init_native();

    	Log.i(TAG, "class_init_native... ");
    }

	
	
    public static boolean isSupported(){
    	return native_is_supported();
    }
    
    
    
	public void addOneWireListener(IOneWireListener listener)
			throws RemoteException {
		
		try {
            synchronized (mLock) {
            	addListener(listener);
            }
        } catch (SecurityException se) {
            throw se;
        } catch (Exception e) {
            Log.e(TAG, "addOneWireListener got exception:", e);
        }
		
	}


	public void removeOneWireListener(IOneWireListener listener)
			throws RemoteException {
		
		try {
            synchronized (mLock) {
            	removeListener(listener);
            }
        } catch (SecurityException se) {
            throw se;
        } catch (Exception e) {
            Log.e(TAG, "removeOneWireListener got exception:", e);
        }
	}


	public boolean begnExclusive() throws RemoteException {
		 synchronized (mLock) {
			 return native_begin_exclusive();
		 }
	}


	public void endExclusive() throws RemoteException {
		 synchronized (mLock) {
			 native_end_exclusive();
		 }
	}


	public OneWireMasterID[] listMasters() throws RemoteException {

		//it's impossible that to have more than 10 masters on one system
		int[] masterIDs = new int[10];	
		int masterCount = 0;
		OneWireMasterID[] result = null;
		
		logD("listMasters begin...");
		
		synchronized (mLock) {
			
			if(native_begin_exclusive()) {
				
				logD("begin_exclusive...");
				
				masterCount = native_list_masters(masterIDs);
				
				logD("native_list_masters... masterCount is " + masterCount);
				
				native_end_exclusive();
				
				logD("end_exclusive...");
			}
		}
		
		logD("listMasters end...");
		
		if(masterCount > 0) {
			result = new OneWireMasterID[masterCount];
			for(int i = 0; i < masterCount; i++){
				result[i] = new OneWireMasterID(masterIDs[i]);
			}
		}
		
		return result;
	}


	public OneWireSlaveID[] searchSlaves(OneWireMasterID masterId)
			throws RemoteException {
		
		//it's impossible that to have more than 100 salves on one master
		long[] slaveIDs = new long[100];	
		int slaveCount = 0;
		OneWireSlaveID[] result = null;

		synchronized (mLock) {
			slaveCount = native_search_slaves(masterId.getId(), slaveIDs);
		}
		
		if(slaveCount > 0) {
			result = new OneWireSlaveID[slaveCount];
			for(int i = 0; i < slaveCount; i++){
				result[i] = new OneWireSlaveID(slaveIDs[i]);
			}
		}
		
		return result;
	}


	public boolean reset(OneWireMasterID masterId) throws RemoteException {
		
		synchronized (mLock) {
			return native_master_reset(masterId.getId());
		}
	}


	public byte[] touch(OneWireMasterID masterId, byte[] dataIn,
			int dataInLen) throws RemoteException {
		
		boolean success = false;
		
		byte[] dataOut = new byte[dataInLen];
		
		synchronized (mLock) {
			success = native_master_touch(masterId.getId(), dataIn, dataInLen, dataOut);
		}
		
		return success ? dataOut : null;
	}


	public byte[] read(OneWireMasterID masterId, int readLen) throws RemoteException {

		boolean success = false;
		
		byte[] dataReadOut = new byte[readLen];
		
		synchronized (mLock) {
			success = native_master_read(masterId.getId(), readLen, dataReadOut);
		}
		
		return success ? dataReadOut : null;
	}


	public boolean write(OneWireMasterID masterId, byte[] dataWriteIn) throws RemoteException {

		synchronized (mLock) {
			return native_master_write(masterId.getId(), dataWriteIn.length, dataWriteIn);
		}
	}




    // Callbacks (Invoked by JNI native codes...) --------------------------


    /**
     * masterId: 4 bytes
     * */
    private void masterAdded(int masterId){

    	Log.i(TAG, "OneWire Event [masterAdded] raised from navice code!");
		
    	OneWireMasterID master = new OneWireMasterID(masterId);

		oneWireMessageHandler.removeMessages(MESSAGE_ONEWIRE_MASTER_ADDED, master);
		
        Message m = Message.obtain(oneWireMessageHandler, MESSAGE_ONEWIRE_MASTER_ADDED, master);
		
        oneWireMessageHandler.sendMessageAtFrontOfQueue(m);
    }

    /**
     * masterId: 4 bytes
     * */
    private void masterRemoved(int masterId){

    	Log.i(TAG, "OneWire Event [masterRemoved] raised from navice code!");
		
    	OneWireMasterID master = new OneWireMasterID(masterId);

		oneWireMessageHandler.removeMessages(MESSAGE_ONEWIRE_MASTER_REMOVED, master);
		
        Message m = Message.obtain(oneWireMessageHandler, MESSAGE_ONEWIRE_MASTER_REMOVED, master);
		
        oneWireMessageHandler.sendMessageAtFrontOfQueue(m);
    }

    /**
     * masterId: 4 bytes, slaveRN: 8 bytes
     * */
    private void slaveAdded(int masterId, long slaveRN){

    	Log.i(TAG, "OneWire Event [slaveAdded] raised from navice code!");

    	OneWireMasterID master = new OneWireMasterID(masterId);
    	OneWireSlaveID slaveOfTheMaster = new OneWireSlaveID(slaveRN);
    	
    	MasterSlaveWrapper wrapper = new MasterSlaveWrapper(master, slaveOfTheMaster);

		//oneWireMessageHandler.removeMessages(MESSAGE_ONEWIRE_SLAVE_ADDED, wrapper);
		
        Message m = Message.obtain(oneWireMessageHandler, MESSAGE_ONEWIRE_SLAVE_ADDED, wrapper);

        oneWireMessageHandler.sendMessage(m);
    }

    /**
     * masterId: 4 bytes, slaveRN: 8 bytes
     * */
    private void slaveRemoved(int masterId, long slaveRN){

    	Log.i(TAG, "OneWire Event [slaveRemoved] raised from navice code!");

    	OneWireMasterID master = new OneWireMasterID(masterId);
    	OneWireSlaveID slaveOfTheMaster = new OneWireSlaveID(slaveRN);
    	
    	MasterSlaveWrapper wrapper = new MasterSlaveWrapper(master, slaveOfTheMaster);

		//oneWireMessageHandler.removeMessages(MESSAGE_ONEWIRE_SLAVE_REMOVED, wrapper);
		
        Message m = Message.obtain(oneWireMessageHandler, MESSAGE_ONEWIRE_SLAVE_REMOVED, wrapper);
		
        oneWireMessageHandler.sendMessage(m);
    }


    // Native (JNI mapped functions) -------------------------------------

    private static native void class_init_native();

    private static native boolean native_is_supported();

    private native boolean native_start();

    private native void native_stop();

    private native boolean native_begin_exclusive();

    private native void native_end_exclusive();

    //return the master count...
    private native int native_list_masters(int[] masterIDs);

    //return the slave count...
    private native int native_search_slaves(int masterId, long[] slaveRNs);

    private native boolean native_master_reset(int masterId);

    private native boolean native_master_touch(
            int masterId, byte[] dataIn, int dataInLen, byte[] dataOut);

    private native boolean native_master_read(
            int masterId, int readLen, byte[] dataReadOut);

    private native boolean native_master_write(
            int masterId, int writeLen, byte[] dataWriteIn);


}