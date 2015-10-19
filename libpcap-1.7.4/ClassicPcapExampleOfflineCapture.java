import java.util.Date;  
  
import org.jnetpcap.Pcap;  
import org.jnetpcap.packet.PcapPacket;  
import org.jnetpcap.packet.PcapPacketHandler;  
  
/** 
 * This example is similar to the classic libpcap example shown in nearly every 
 * tutorial on libpcap. The main difference is that a file is opened instead of 
 * a live network interface. Using a packet handler it goes into a loop to read 
 * a few packets, say 10. Prints some simple info about the packets, and then 
 * closes the pcap handle and exits. 
 *  
 * Here is the output generated by this example : 
 *  
 * Opening file for reading: tests/test-l2tp.pcap 
 * Received at Tue Jan 27 16:17:17 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:17 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:18 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:18 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:19 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:19 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:20 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:20 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:21 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 * Received at Tue Jan 27 16:17:21 EST 2004 caplen=114  len=114  jNetPcap rocks! 
 *  
 * @author Mark Bednarczyk 
 * @author Sly Technologies, Inc. 
 */  
public class ClassicPcapExampleOfflineCapture {  
  static int count = 0;
    /** 
     * Main startup method 
     *  
     * @param args 
     *          ignored 
     */  
    public static void main(String[] args) {  
        /*************************************************************************** 
         * First we setup error buffer and name for our file 
         **************************************************************************/  
        final StringBuilder errbuf = new StringBuilder(); // For any error msgs  
        //final String file = "tests/test-l2tp.pcap";  
        final String file = "tests/test_wireshark.pcap";
  
        System.out.printf("Opening file for reading: %s%n", file);  
  
        /*************************************************************************** 
         * Second we open up the selected file using openOffline call 
         **************************************************************************/  
        Pcap pcap = Pcap.openOffline(file, errbuf);  
  
        if (pcap == null) {  
            System.err.printf("Error while opening device for capture: "  
                + errbuf.toString());  
            return;  
        }  
  
        /*************************************************************************** 
         * Third we create a packet handler which will receive packets from the 
         * libpcap loop. 
         **************************************************************************/  
        PcapPacketHandler<String> jpacketHandler = new PcapPacketHandler<String>() {  
  
            public void nextPacket(PcapPacket packet, String user) {  
  
                System.out.printf("Received at %s caplen=%-4d len=%-4d %s\n",   
                    new Date(packet.getCaptureHeader().timestampInMillis()),   
                    packet.getCaptureHeader().caplen(), // Length actually captured  
                    packet.getCaptureHeader().wirelen(), // Original length  
                    user // User supplied object  
                    );  
            }  
        };  
        
        
         PcapPacketHandler<String> percent = new PcapPacketHandler<String>() {  
            //if greater eth II else 802.3
            
            public void nextPacket(PcapPacket packet, String user) {  
                    if (packet.getCaptureHeader().wirelen() > 1536){
                      count++;
                    }
                    
            }  
            
            
        };
        
        
  
        /*************************************************************************** 
         * Fourth we enter the loop and tell it to capture 10 packets. The loop 
         * method does a mapping of pcap.datalink() DLT value to JProtocol ID, which 
         * is needed by JScanner. The scanner scans the packet buffer and decodes 
         * the headers. The mapping is done automatically, although a variation on 
         * the loop method exists that allows the programmer to sepecify exactly 
         * which protocol ID to use as the data link type for this pcap interface. 
         **************************************************************************/  
        try {  
            
            pcap.loop(500, percent, "jNetPcap rocks!"); 
            System.out.println("percent of Ethernet II: " + (count/500.0)*100);
            //pcap.loop(500, jpacketHandler, "jNetPcap rocks!"); 
        } finally {  
        /*************************************************************************** 
         * Last thing to do is close the pcap handle 
         **************************************************************************/  
           pcap.close();  
        }  
    }  
}  
