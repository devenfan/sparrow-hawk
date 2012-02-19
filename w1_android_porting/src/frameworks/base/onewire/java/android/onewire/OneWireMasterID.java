/* * Copyright (C) 2007 The Android Open Source Project * * Licensed under the Apache License, Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *      http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */package android.onewire;public class OneWireMasterID  {	private int _id;		public OneWireMasterID() { }		public OneWireMasterID(int id) {		_id = id;	}		public void setId(int id) {		_id = id;	}		public int getId() {		return _id;	}		public String toString(){		return String.valueOf(_id);	}}