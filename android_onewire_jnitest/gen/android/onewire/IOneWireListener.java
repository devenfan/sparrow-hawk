/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: F:\\ARM_BOARD_ANDROID\\Workspace\\android-ibutton-git\\android_onewire_jnitest\\src\\android\\onewire\\IOneWireListener.aidl
 */
package android.onewire;
/**
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
android.os.IInterface iin = (android.os.IInterface)obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof android.onewire.IOneWireListener))) {
return ((android.onewire.IOneWireListener)iin);
}
return new android.onewire.IOneWireListener.Stub.Proxy(obj);
}
public android.os.IBinder asBinder()
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
case TRANSACTION_oneWireMasterAdded:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.oneWireMasterAdded(_arg0);
return true;
}
case TRANSACTION_oneWireMasterRemoved:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireMasterID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireMasterID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.oneWireMasterRemoved(_arg0);
return true;
}
case TRANSACTION_oneWireSlaveAdded:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireSlaveID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireSlaveID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.oneWireSlaveAdded(_arg0);
return true;
}
case TRANSACTION_oneWireSlaveRemoved:
{
data.enforceInterface(DESCRIPTOR);
android.onewire.OneWireSlaveID _arg0;
if ((0!=data.readInt())) {
_arg0 = android.onewire.OneWireSlaveID.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.oneWireSlaveRemoved(_arg0);
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
public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
public void oneWireMasterAdded(android.onewire.OneWireMasterID master) throws android.os.RemoteException
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
mRemote.transact(Stub.TRANSACTION_oneWireMasterAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
public void oneWireMasterRemoved(android.onewire.OneWireMasterID master) throws android.os.RemoteException
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
mRemote.transact(Stub.TRANSACTION_oneWireMasterRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
public void oneWireSlaveAdded(android.onewire.OneWireSlaveID slave) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((slave!=null)) {
_data.writeInt(1);
slave.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_oneWireSlaveAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
public void oneWireSlaveRemoved(android.onewire.OneWireSlaveID slave) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((slave!=null)) {
_data.writeInt(1);
slave.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_oneWireSlaveRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
}
finally {
_data.recycle();
}
}
}
static final int TRANSACTION_oneWireMasterAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_oneWireMasterRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_oneWireSlaveAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_oneWireSlaveRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
}
public void oneWireMasterAdded(android.onewire.OneWireMasterID master) throws android.os.RemoteException;
public void oneWireMasterRemoved(android.onewire.OneWireMasterID master) throws android.os.RemoteException;
public void oneWireSlaveAdded(android.onewire.OneWireSlaveID slave) throws android.os.RemoteException;
public void oneWireSlaveRemoved(android.onewire.OneWireSlaveID slave) throws android.os.RemoteException;
}
