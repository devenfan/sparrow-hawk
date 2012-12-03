/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: F:\\ARM_BOARD_ANDROID\\GitHub\\android-ibutton-git\\android_onewire_jnitest\\frameworks-base\\onewire\\java\\android\\onewire\\IOneWireListener.aidl
 */
package android.onewire;
/**
 * Note that this is a one-way interface so the server
 * does not block waiting for the client.
 * 
 * {@hide}
 */
public interface IOneWireListener extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements android.onewire.IOneWireListener
{
private static final java.lang.String DESCRIPTOR = "android.onewire.IOneWireListener";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an android.onewire.IOneWireListener interface,
 * generating a proxy if needed.
 */
public static android.onewire.IOneWireListener asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof android.onewire.IOneWireListener))) {
return ((android.onewire.IOneWireListener)iin);
}
return new android.onewire.IOneWireListener.Stub.Proxy(obj);
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
case TRANSACTION_onOneWireMasterAdded:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.onOneWireMasterAdded(_arg0);
return true;
}
case TRANSACTION_onOneWireMasterRemoved:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.onOneWireMasterRemoved(_arg0);
return true;
}
case TRANSACTION_onOneWireSlaveAdded:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
android.onewire.OneWireSlaveID _arg1;
if ((0!=data.readInt())) {
_arg1 = android.onewire.OneWireSlaveID.CREATOR.createFromParcel(data);
}
else {
_arg1 = null;
}
this.onOneWireSlaveAdded(_arg0, _arg1);
return true;
}
case TRANSACTION_onOneWireSlaveRemoved:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
android.onewire.OneWireSlaveID _arg1;
if ((0!=data.readInt())) {
_arg1 = android.onewire.OneWireSlaveID.CREATOR.createFromParcel(data);
}
else {
_arg1 = null;
}
this.onOneWireSlaveRemoved(_arg0, _arg1);
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements android.onewire.IOneWireListener
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
@Override public void onOneWireMasterAdded(android.onewire.OneWireMasterID master) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((master!=null)) {
_data.writeInt(1);
master.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_onOneWireMasterAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
@Override public void onOneWireMasterRemoved(android.onewire.OneWireMasterID master) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((master!=null)) {
_data.writeInt(1);
master.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_onOneWireMasterRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
@Override public void onOneWireSlaveAdded(android.onewire.OneWireMasterID master, android.onewire.OneWireSlaveID slaveOfTheMaster) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((master!=null)) {
_data.writeInt(1);
master.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
if ((slaveOfTheMaster!=null)) {
_data.writeInt(1);
slaveOfTheMaster.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_onOneWireSlaveAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
@Override public void onOneWireSlaveRemoved(android.onewire.OneWireMasterID master, android.onewire.OneWireSlaveID slaveOfTheMaster) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((master!=null)) {
_data.writeInt(1);
master.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
if ((slaveOfTheMaster!=null)) {
_data.writeInt(1);
slaveOfTheMaster.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_onOneWireSlaveRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
}
static final int TRANSACTION_onOneWireMasterAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_onOneWireMasterRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_onOneWireSlaveAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_onOneWireSlaveRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
}
public void onOneWireMasterAdded(android.onewire.OneWireMasterID master) throws android.os.RemoteException;
public void onOneWireMasterRemoved(android.onewire.OneWireMasterID master) throws android.os.RemoteException;
public void onOneWireSlaveAdded(android.onewire.OneWireMasterID master, android.onewire.OneWireSlaveID slaveOfTheMaster) throws android.os.RemoteException;
public void onOneWireSlaveRemoved(android.onewire.OneWireMasterID master, android.onewire.OneWireSlaveID slaveOfTheMaster) throws android.os.RemoteException;
}
