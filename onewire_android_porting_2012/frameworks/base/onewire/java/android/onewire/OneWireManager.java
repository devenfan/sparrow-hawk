/*
 * Copyright (C) 2012 deven.fan@gmail.com
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

package android.onewire;

import android.os.Looper;
import android.os.RemoteException;
import android.os.Handler;
import android.os.Message;
import android.util.Config;
import android.util.Log;

import java.util.HashMap;

/**
 * This class provides access to the system onewire services. 
 *
 * <p>You do not
 * instantiate this class directly; instead, retrieve it through
 * {@link android.content.Context#getSystemService
 * Context.getSystemService(Context.ONEWIRE_SERVICE)}.
 */
public class OneWireManager {
	
    private static final String TAG = "OneWireManager";
    
    private IOneWireService mService;
    
    
    
    // Map from OneWireListener to their associated OneWireListenerTransport objects
    private HashMap<OneWireListener, OneWireListenerTransport> mListeners =
        new HashMap<OneWireListener, OneWireListenerTransport>();
    
    

    // Constructors -----------------------------------------------------------
    
    
    /**
     * @hide - hide this constructor because it has a parameter
     * of type ILocationManager, which is a system private class. The
     * right way to create an instance of this class is using the
     * factory Context.getSystemService.
     */
    public OneWireManager(IOneWireService service) {
        if (Config.LOGD) {
            Log.d(TAG, "Constructor: service = " + service);
        }
        
        mService = service;
    }


    
    // Inner Classes -----------------------------------------------------------
    
    private class MasterSlaveWrapper {
		OneWireMasterID master;
		OneWireSlaveID slaveOfTheMaster;
		
		MasterSlaveWrapper(OneWireMasterID master, OneWireSlaveID slaveOfTheMaster){
			this.master = master;
			this.slaveOfTheMaster = slaveOfTheMaster;
		}
	}
    
    private class OneWireListenerTransport extends IOneWireListener.Stub {
    	
        private static final int TYPE_ONEWIRE_MASTER_ADDED = 1;
        private static final int TYPE_ONEWIRE_MASTER_REMOVED = 2;
        private static final int TYPE_ONEWIRE_SLAVE_ADDED = 3;
        private static final int TYPE_ONEWIRE_SLAVE_REMOVED = 4;
    	
    	private final OneWireListener mListener;
        private final Handler mListenerHandler;

    	public OneWireListenerTransport(OneWireListener listener, Looper looper){
    		mListener = listener;
    		
            if (looper == null) {
                mListenerHandler = new Handler() {
                    public void handleMessage(Message msg) {
                        _handleMessage(msg);
                    }
                };
            } else {
                mListenerHandler = new Handler(looper)  {
                    public void handleMessage(Message msg) {
                        _handleMessage(msg);
                    }
                };
            }
    	}
    	
        private void _handleMessage(Message msg) {
        	
        	OneWireMasterID master = null;
        	MasterSlaveWrapper wrapper = null;
        	
        	switch(msg.what) {
        	
        		case TYPE_ONEWIRE_MASTER_ADDED:
        			master = (OneWireMasterID)msg.obj;
        			mListener.onOneWireMasterAdded(master);
        			break;
        	
        		case TYPE_ONEWIRE_MASTER_REMOVED:
        			master = (OneWireMasterID)msg.obj;
        			mListener.onOneWireMasterRemoved(master);
        			break;
        	
        		case TYPE_ONEWIRE_SLAVE_ADDED:
        			wrapper = (MasterSlaveWrapper)msg.obj;
        			mListener.onOneWireSlaveAdded(wrapper.master, wrapper.slaveOfTheMaster);
        			break;
        	
        		case TYPE_ONEWIRE_SLAVE_REMOVED:
        			wrapper = (MasterSlaveWrapper)msg.obj;
        			mListener.onOneWireSlaveRemoved(wrapper.master, wrapper.slaveOfTheMaster);
        			break;
        			
    			default:
    				return;
        	}

            try {
                mService.oneWireCallbackFinished(this);
            } catch (RemoteException e) {
                Log.e(TAG, "locationCallbackFinished: RemoteException", e);
            }
        }
    	
		public void onOneWireMasterAdded(OneWireMasterID master)
				throws RemoteException {
			
			Message msg = Message.obtain();
            msg.what = TYPE_ONEWIRE_MASTER_ADDED;
            msg.obj = master;
            mListenerHandler.sendMessage(msg);
            
		}

		public void onOneWireMasterRemoved(OneWireMasterID master)
				throws RemoteException {
			
			Message msg = Message.obtain();
            msg.what = TYPE_ONEWIRE_MASTER_REMOVED;
            msg.obj = master;
            mListenerHandler.sendMessage(msg);
		}

		public void onOneWireSlaveAdded(OneWireMasterID master,
				OneWireSlaveID slaveOfTheMaster) throws RemoteException {

			Message msg = Message.obtain();
            msg.what = TYPE_ONEWIRE_SLAVE_ADDED;
            msg.obj = new MasterSlaveWrapper(master, slaveOfTheMaster);
            mListenerHandler.sendMessage(msg);
		}

		public void onOneWireSlaveRemoved(OneWireMasterID master,
				OneWireSlaveID slaveOfTheMaster) throws RemoteException {

			Message msg = Message.obtain();
            msg.what = TYPE_ONEWIRE_SLAVE_REMOVED;
            msg.obj = new MasterSlaveWrapper(master, slaveOfTheMaster);
            mListenerHandler.sendMessage(msg);
		}
    }

    
    
    // Public Methods ----------------------------------------------------------

    public void addOneWireListener(OneWireListener listener) {
    	
    	if (listener == null) {
            throw new IllegalArgumentException("OneWireListener==null");
        }

        if (Config.LOGD) {
            Log.d(TAG, "addOneWireListener: listener = " + listener);
        }
        
    	try {
            synchronized (mListeners) {
                OneWireListenerTransport transport = mListeners.get(listener);
                if (transport == null) {
                    transport = new OneWireListenerTransport(listener, null);
                }
                mListeners.put(listener, transport);
                mService.addOneWireListener(transport);
            }
        } catch (RemoteException ex) {
            Log.e(TAG, "requestLocationUpdates: DeadObjectException", ex);
        }
    }
    
    
    public void removeOneWireListener(OneWireListener listener) {
    	
    	if (listener == null) {
            throw new IllegalArgumentException("OneWireListener==null");
        }

        if (Config.LOGD) {
            Log.d(TAG, "removeOneWireListener: listener = " + listener);
        }
        
    	try {
    		OneWireListenerTransport transport = mListeners.remove(listener);
            if (transport != null) {
                mService.removeOneWireListener(transport);
            }
        } catch (RemoteException ex) {
            Log.e(TAG, "removeUpdates: DeadObjectException", ex);
        }
    }



	public boolean isDebugEnabled() {
		
		try {
            return mService.isDebugEnabled();
        } catch (RemoteException ex) {
            Log.e(TAG, "isDebugEnabled: RemoteException", ex);
            return false;
        }
	}

    public void setDebugEnabled(boolean enabled) {

		try {
            mService.setDebugEnabled(enabled);
        } catch (RemoteException ex) {
            Log.e(TAG, "setDebugEnabled: RemoteException", ex);
        }
    }

    
	public boolean beginExclusive() {
		
		try {
            return mService.beginExclusive();
        } catch (RemoteException ex) {
            Log.e(TAG, "beginExclusive: RemoteException", ex);
            return false;
        }
	}
    
    public void endExclusive() {

		try {
            mService.endExclusive();
        } catch (RemoteException ex) {
            Log.e(TAG, "endExclusive: RemoteException", ex);
        }
    }
    
    public OneWireMasterID[] getCurrentMasters() {
    	
    	try {
            return mService.getCurrentMasters();
        } catch (RemoteException ex) {
            Log.e(TAG, "getCurrentMasters: RemoteException", ex);
            return null;
        }
    }
    
    public OneWireMasterID[] listMasters() {
    	
    	try {
            return mService.listMasters();
        } catch (RemoteException ex) {
            Log.e(TAG, "listMasters: RemoteException", ex);
            return null;
        }
    }
    
    
    public OneWireSlaveID[] searchSlaves(OneWireMasterID masterId) {
    	
    	if(masterId == null)
    		throw new IllegalArgumentException("OneWireMasterID cannot be null!");

    	try {
            return mService.searchSlaves(masterId);
        } catch (RemoteException ex) {
            Log.e(TAG, "searchSlaves: RemoteException", ex);
            return null;
        }
    }
    
    
    public boolean reset(OneWireMasterID masterId) {
    	
    	if(masterId == null)
    		throw new IllegalArgumentException("OneWireMasterID cannot be null!");

    	try {
            return mService.reset(masterId);
        } catch (RemoteException ex) {
            Log.e(TAG, "reset: RemoteException", ex);
            return false;
        }
    }
    
    
    public byte[] touch(OneWireMasterID masterId, byte[] dataIn) {
    	
    	if(masterId == null)
    		throw new IllegalArgumentException("masterId cannot be null!");

    	if(dataIn == null || dataIn.length == 0)
    		throw new IllegalArgumentException("dataIn cannot be null or empty!");
    	
    	try {
            return mService.touch(masterId, dataIn, dataIn.length);
        } catch (RemoteException ex) {
            Log.e(TAG, "touch: RemoteException", ex);
            return null;
        }
    }
    

    
    
    public byte[] read(OneWireMasterID masterId, int readLen) {
    	
    	if(masterId == null)
    		throw new IllegalArgumentException("masterId cannot be null!");

    	if(readLen <= 0)
    		throw new IllegalArgumentException("readLen must be a positive integer!");
    	
    	try {
            return mService.read(masterId, readLen);
        } catch (RemoteException ex) {
            Log.e(TAG, "read: RemoteException", ex);
            return null;
        }
    }
    
    
    public boolean write(OneWireMasterID masterId, byte[] dataWriteIn) {
    	
    	if(masterId == null)
    		throw new IllegalArgumentException("masterId cannot be null!");

    	if(dataWriteIn == null || dataWriteIn.length == 0)
    		throw new IllegalArgumentException("dataWriteIn cannot be null or empty!");
    	
    	try {
            return mService.write(masterId, dataWriteIn);
        } catch (RemoteException ex) {
            Log.e(TAG, "searchSlaves: RemoteException", ex);
            return false;
        }
    }
    
    

    
    
}
