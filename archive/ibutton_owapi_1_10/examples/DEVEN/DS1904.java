
import java.util.Date;

import com.dalsemi.onewire.container.OneWireContainer24;
import com.dalsemi.onewire.utils.ConvertCodec;


public class DS1904 {
	
	static byte[][] states = new byte[][] {
		new byte[]{0x50, (byte) 0xE1, (byte) 0xC5, 0x2C, 0x4F},
		new byte[]{0x5C, 0x36, (byte) 0xF1, 0x2C, 0x4F},
		new byte[]{0x5C, 0x46, (byte) 0xF4, 0x2C, 0x4F},
		new byte[]{0x5C, 0x3D, 0x0D, 0x2D, 0x4F}
	};
	
	public static void main(String[] args){
		
		OneWireContainer24 container = new OneWireContainer24();
		
		for(byte[] state : states)
			printState(container, state);

		
		container.setClock(new Date().getTime(), states[0]);
		container.setClockRunEnable(true, states[0]);
		
		printState(container, states[0]);

	}
	
	
	private static void printState(OneWireContainer24 container, byte[] state){
		System.out.println("-----------------------------------------------");
		System.out.println("state: " + ConvertCodec.bytesToHexString(state));
		System.out.println("isClockRunning: " + container.isClockRunning(state));
		System.out.println("getClock: " + new Date(container.getClock(state)));
		
	}
}
