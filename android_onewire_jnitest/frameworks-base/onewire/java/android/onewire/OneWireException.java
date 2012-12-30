package android.onewire;

public class OneWireException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	
	public OneWireException() {
		super();
	}
	
	
	public OneWireException(String detailMessage) { 
		super(detailMessage); 
	}
	

	public OneWireException(String detailMessage, Throwable throwable) { 
		super(detailMessage, throwable); 
	}

	
	public OneWireException(Throwable throwable) { 
		super(throwable); 
	}


}
