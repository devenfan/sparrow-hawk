/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: F:\\ARM_BOARD_ANDROID\\GitHub\\android-ibutton-git\\android_onewire_jnitest\\frameworks-base\\onewire\\java\\android\\onewire\\IOneWireService.aidl
 */
package android.onewire;
/**
 * 
 * {@hide}
 */
public interface IOneWireService extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements android.onewire.IOneWireService
{
private static final java.lang.String DESCRIPTOR = "android.onewire.IOneWireService";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an android.onewire.IOneWireService interface,
 * generating a proxy if needed.
 */
public static android.onewire.IOneWireService asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof android.onewire.IOneWireService))) {
return ((android.onewire.IOneWireService)iin);
}
return new android.onewire.IOneWireService.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_addOneWireListener:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.IOneWireListener _arg0;
_arg0 = android.onewire.IOneWireListener.Stub.asInterface(data.readStrongBinder());
this.addOneWireListener(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_removeOneWireListener:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.IOneWireListener _arg0;
_arg0 = android.onewire.IOneWireListener.Stub.asInterface(data.readStrongBinder());
this.removeOneWireListener(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_oneWireCallbackFinished:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.IOneWireListener _arg0;
_arg0 = android.onewire.IOneWireListener.Stub.asInterface(data.readStrongBinder());
this.oneWireCallbackFinished(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_isDebugEnabled:
{
data.enforceInterface(DESCRIPTOR);
boolean _result = this.isDebugEnabled();
reply.writeNoException();
reply.writeInt(((_result)?(1):(0)));
return true;
}
case TRANSACTION_setDebugEnabled:
{
data.enforceInterface(DESCRIPTOR);
boolean _arg0;
_arg0 = (0!=data.readInt());
this.setDebugEnabled(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_beginExclusive:
{
data.enforceInterface(DESCRIPTOR);
boolean _result = this.beginExclusive();
reply.writeNoException();
reply.writeInt(((_result)?(1):(0)));
return true;
}
case TRANSACTION_endExclusive:
{
data.enforceInterface(DESCRIPTOR);
this.endExclusive();
reply.writeNoException();
return true;
}
case TRANSACTION_getCurrentMasters:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID[] _result = this.getCurrentMasters();
reply.writeNoException();
reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
return true;
}
case TRANSACTION_listMasters:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID[] _result = this.listMasters();
reply.writeNoException();
reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
return true;
}
case TRANSACTION_searchSlaves:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
android.onewire.OneWireSlaveID[] _result = this.searchSlaves(_arg0);
reply.writeNoException();
reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
return true;
}
case TRANSACTION_reset:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
boolean _result = this.reset(_arg0);
reply.writeNoException();
reply.writeInt(((_result)?(1):(0)));
return true;
}
case TRANSACTION_touch:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
byte[] _arg1;
_arg1 = data.createByteArray();
int _arg2;
_arg2 = data.readInt();
byte[] _result = this.touch(_arg0, _arg1, _arg2);
reply.writeNoException();
reply.writeByteArray(_result);
return true;
}
case TRANSACTION_read:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
int _arg1;
_arg1 = data.readInt();
byte[] _result = this.read(_arg0, _arg1);
reply.writeNoException();
reply.writeByteArray(_result);
return true;
}
case TRANSACTION_write:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
byte[] _arg1;
_arg1 = data.createByteArray();
boolean _result = this.write(_arg0, _arg1);
reply.writeNoException();
reply.writeInt(((_result)?(1):(0)));
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements android.onewire.IOneWireService
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
@Override public void addOneWireListener(android.onewire.IOneWireListener listener) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
mRemote.transact(Stub.TRANSACTION_addOneWireListener, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public void removeOneWireListener(android.onewire.IOneWireListener listener) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
mRemote.transact(Stub.TRANSACTION_removeOneWireListener, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public void oneWireCallbackFinished(android.onewire.IOneWireListener listener) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
mRemote.transact(Stub.TRANSACTION_oneWireCallbackFinished, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public boolean isDebugEnabled() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
boolean _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_isDebugEnabled, _data, _reply, 0);
_reply.readException();
_result = (0!=_reply.readInt());
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public void setDebugEnabled(boolean enabled) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeInt(((enabled)?(1):(0)));
mRemote.transact(Stub.TRANSACTION_setDebugEnabled, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public boolean beginExclusive() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
boolean _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_beginExclusive, _data, _reply, 0);
_reply.readException();
_result = (0!=_reply.readInt());
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public void endExclusive() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_endExclusive, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public android.onewire.OneWireMasterID[] getCurrentMasters() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.onewire.OneWireMasterID[] _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_getCurrentMasters, _data, _reply, 0);
_reply.readException();
_result = _reply.createTypedArray(android.onewire.OneWireMasterID.CREATOR);
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public android.onewire.OneWireMasterID[] listMasters() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.onewire.OneWireMasterID[] _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_listMasters, _data, _reply, 0);
_reply.readException();
_result = _reply.createTypedArray(android.onewire.OneWireMasterID.CREATOR);
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public android.onewire.OneWireSlaveID[] searchSlaves(android.onewire.OneWireMasterID masterId) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.onewire.OneWireSlaveID[] _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((masterId!=null)) {
_data.writeInt(1);
masterId.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_searchSlaves, _data, _reply, 0);
_reply.readException();
_result = _reply.createTypedArray(android.onewire.OneWireSlaveID.CREATOR);
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public boolean reset(android.onewire.OneWireMasterID masterId) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
boolean _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((masterId!=null)) {
_data.writeInt(1);
masterId.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_reset, _data, _reply, 0);
_reply.readException();
_result = (0!=_reply.readInt());
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
//dataOutLen is equal to dataInlen

@Override public byte[] touch(android.onewire.OneWireMasterID masterId, byte[] dataIn, int dataInLen) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
byte[] _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((masterId!=null)) {
_data.writeInt(1);
masterId.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
_data.writeByteArray(dataIn);
_data.writeInt(dataInLen);
mRemote.transact(Stub.TRANSACTION_touch, _data, _reply, 0);
_reply.readException();
_result = _reply.createByteArray();
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public byte[] read(android.onewire.OneWireMasterID masterId, int readLen) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
byte[] _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((masterId!=null)) {
_data.writeInt(1);
masterId.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
_data.writeInt(readLen);
mRemote.transact(Stub.TRANSACTION_read, _data, _reply, 0);
_reply.readException();
_result = _reply.createByteArray();
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public boolean write(android.onewire.OneWireMasterID masterId, byte[] dataWriteIn) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
boolean _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((masterId!=null)) {
_data.writeInt(1);
masterId.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
_data.writeByteArray(dataWriteIn);
mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
_reply.readException();
_result = (0!=_reply.readInt());
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
}
static final int TRANSACTION_addOneWireListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_removeOneWireListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_oneWireCallbackFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_isDebugEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
static final int TRANSACTION_setDebugEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
static final int TRANSACTION_beginExclusive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
static final int TRANSACTION_endExclusive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
static final int TRANSACTION_getCurrentMasters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
static final int TRANSACTION_listMasters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
static final int TRANSACTION_searchSlaves = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
static final int TRANSACTION_reset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
static final int TRANSACTION_touch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
static final int TRANSACTION_read = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
}
public void addOneWireListener(android.onewire.IOneWireListener listener) throws android.os.RemoteException;
public void removeOneWireListener(android.onewire.IOneWireListener listener) throws android.os.RemoteException;
public void oneWireCallbackFinished(android.onewire.IOneWireListener listener) throws android.os.RemoteException;
public boolean isDebugEnabled() throws android.os.RemoteException;
public void setDebugEnabled(boolean enabled) throws android.os.RemoteException;
public boolean beginExclusive() throws android.os.RemoteException;
public void endExclusive() throws android.os.RemoteException;
public android.onewire.OneWireMasterID[] getCurrentMasters() throws android.os.RemoteException;
public android.onewire.OneWireMasterID[] listMasters() throws android.os.RemoteException;
public android.onewire.OneWireSlaveID[] searchSlaves(android.onewire.OneWireMasterID masterId) throws android.os.RemoteException;
public boolean reset(android.onewire.OneWireMasterID masterId) throws android.os.RemoteException;
//dataOutLen is equal to dataInlen

public byte[] touch(android.onewire.OneWireMasterID masterId, byte[] dataIn, int dataInLen) throws android.os.RemoteException;
public byte[] read(android.onewire.OneWireMasterID masterId, int readLen) throws android.os.RemoteException;
public boolean write(android.onewire.OneWireMasterID masterId, byte[] dataWriteIn) throws android.os.RemoteException;
}
