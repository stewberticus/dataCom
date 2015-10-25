import java.util.*;  

import org.jnetpcap.Pcap;  
import org.jnetpcap.packet.PcapPacket;  
import org.jnetpcap.packet.PcapPacketHandler;  
import org.jnetpcap.packet.PcapPacket;
import org.jnetpcap.protocol.network.Ip4;
import org.jnetpcap.protocol.lan.Ethernet;
import org.jnetpcap.packet.JPacket;

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
  static int countIP4 = 0;
  static double totalBytesTransferred = 0.0;
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
        //final String file = "tests/sample.pcap";
  
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
        
        Ethernet eth = new Ethernet();
        Ip4 ip = new Ip4();
        // make a HashMap mapping int type (in hex) to 
        // the # of packets (int) of that type
        HashMap<Integer, Integer> typeCounts =  new 
            HashMap<Integer, Integer>(); 
        //keep track of the total bytes transferred
        HashMap<Integer, Integer> bytesTransferred = new
            HashMap<Integer, Integer>();
        HashMap<byte[], HashMap<byte[], Integer>> mapSourceDest = new
            HashMap<byte[], HashMap<byte[], Integer>>();

        ArrayList<IP4Pair> ip4pairs = new ArrayList<IP4Pair>();
        
        PcapPacketHandler<String> percent = new PcapPacketHandler<String>() {  
            //if greater eth II else 802.3
            
            public void nextPacket(PcapPacket packet, String user) {  
                    //if (packet.getCaptureHeader().wirelen() > 1536){
                    //  count++;
                    //}

                    System.out.println("Packet:");
                    //add the size of the packet to totalBytes
                    totalBytesTransferred += packet.getTotalSize();

                    if(packet.hasHeader(eth)) {

                        //if the type is greater than 0x0600, it's E II
                        if(eth.type() > 1536) {
                            count++;

                            //if the type is already in the hashmap
                            if(typeCounts.keySet().contains(eth.type())) {
                                typeCounts.put(eth.type(),
                                  typeCounts.get(eth.type())+1); 
                            } else {
                            // else, it isn't already in the map, so init 1
                                typeCounts.put(eth.type(), 1);
                            }

                            //if the type is already in the bytes hashmap
                            if(bytesTransferred.keySet().contains(
                              eth.type())) {
                                bytesTransferred.put(eth.type(),
                                    bytesTransferred.get(eth.type())+
                                        packet.getTotalSize());
                            } else {
                            //else, it isn't there so init to packet size
                                bytesTransferred.put(eth.type(),
                                  packet.getTotalSize());
                            }

                        }

                        //print the (protocol?) type of this packet in hex
                        System.out.printf("ethernet.type=%X\n", eth.type());
                        // add 1 to the count of this type
                        
                    }
                    if(packet.hasHeader(ip)) {
                        //increment count of ip4 packets
                        countIP4++;
                        //print the ip version of this packet
                        System.out.printf("ip.version=%d\n", ip.version());
                        //System.out.printf("Destination=%d\nSource=%d\n",
                        //       ip.destinationToInt(),ip.sourceToInt());
                        //byte[] sourceArray = ip.source();
                        //byte[] destArray = ip.destination();
                        int sourceArray = ip.sourceToInt();
                        int destArray = ip.destinationToInt();
                        //if this source is already in the map
                        IP4Pair newpair = 
                           new IP4Pair(sourceArray, destArray); 
                        boolean addIt = true;
                      //  for(IP4Pair p: ip4pairs) {
                           // if(p.match(newpair))
                                addIt = false;
                              //  break;
                      //  } 
                       // if(addIt) {
                          //  newpair.count = 1;
                           // ip4pairs.add(newpair);    
                      //  }
                    //} 

           //}  
            
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
            System.out.println("Percentages of ethernet types:");
            for(int k: typeCounts.keySet()) {
                System.out.printf("Number of packets of type 0x%X is %d\n",
                        k, typeCounts.get(k));
                System.out.printf("Percent of type 0x%X is %f\n", k,
                       typeCounts.get(k) / 500.0 * 100); 
            }

            System.out.printf("Total bytes transferred %e\n",
              totalBytesTransferred);
            for(int k: bytesTransferred.keySet()) {
                System.out.printf("Bytes transferred of type 0x%X is %d\n",
                    k, bytesTransferred.get(k));
                System.out.printf("Percent of bytes transferred of type " + 
                    "0x%X is %f\n", k, bytesTransferred.get(k) /
                    totalBytesTransferred * 100.0);
            }
           System.out.printf("Number of IPv4 packets: %d\n", countIP4);

           for(IP4Pair p: ip4pairs) {
               if(p.count > 1)
              System.out.printf("Count %d\n", p.count);
           }
            pcap.loop(500, jpacketHandler, "jNetPcap rocks!"); 
        } 
        finally {  
        /*************************************************************************** 
         * Last thing to do is close the pcap handle 
         **************************************************************************/  
           pcap.close();  
        }  
            }
}};}  
