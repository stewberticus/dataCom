#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>

#define BUF_SIZE 42


#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

void getmac(char * mac, char * interface)
{
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	printf("in get mac: %s",interface);
  strcpy(s.ifr_name, interface);
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
    for (i = 0; i < 6; ++i){
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
      mac[i] = s.ifr_addr.sa_data[i];
    }
    puts("\n");
    //return 0;
  }
  //return 1;
}
struct __attribute__((packed)) arp_header
{
    unsigned short arp_hd;
    unsigned short arp_pr;
    unsigned char arp_hdl;
    unsigned char arp_prl;
    unsigned short arp_op;
    unsigned char arp_sha[6];
    unsigned char arp_spa[4];
    unsigned char arp_dha[6];
    unsigned char arp_dpa[4];
};
//this method is meant to check if the packet is ARP
//and to process the request accordingly
//- forward to correct address?
//- use look up tables r1-table.txt and r2-table.txt
int process_arp_packet(struct sockaddr_ll * recvaddr, int * count) {
    printf("processing arp packet\n");
    int is_arp = 0;
    if(recvaddr->sll_protocol == 1544) {
        (*count) += 1;
        printf("ARP: YES\n");
        is_arp = 1;
    }else 
        printf("ARP: NO\n"); 
    return is_arp;
}

//this method is meant to check if the packet is ICMP 
//and to process the request accordingly
//- "router must correctly respond to ICMP echo request packets
//      with any of its own IP addresses as the destination"
//- ICMP echo response should be almost copied from ICMP echo request
//- "The IP source address in an ICMP Echo Reply MUST be the same as the 
// specific-destination address of the ICMP Echo Request message "
int process_icmp_packet(struct sockaddr_ll * recvaddr, int * count) {
    printf("processing icmp packet\n");
    int is_icmp = 0;
    if(recvaddr->sll_protocol == 8) {
        printf("ICMP: YES\n");
        (*count) += 1;
        is_icmp = 1;

        //type should be cleared to 0
        recvaddr->sll_pkttype = 0;
        
    } else {
        printf("ICMP: NO\n");
    } 
    return is_icmp;
}

int main(){

  void* buffer = NULL;
  int packet_socket;
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp;
  //gets all of the addresses of the network interfaces on your system
  //returns a list that contains roughly 3 addr per device / network interface
  //    > ipv4 addr, ipv6 addr, mac addr  
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    //
    //tell which type it is by looking at family 
    //AF_INET = ipv4
    //AF_INET6(?) = ipv6
    //AF_PACKET = eth mac addr
    if(tmp->ifa_addr->sa_family==AF_PACKET){
	  char mac[6];
      printf("Interface: %s\n",tmp->ifa_name);
      
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
	  getmac(tmp->ifa_name,mac);
      printf("Mac: %s\n",mac);
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
	//create a packet socket
	//AF_PACKET makes it a packet socket
	//SOCK_RAW makes it so we get the entire packet
    //  > want packet raw, unchanged
    //  > could say SOCK_DGRAM - gives transport and network layer, but cuts off link layer header
	//could also use SOCK_DGRAM to cut off link layer header
	//ETH_P_ALL indicates we want all (upper layer) protocols
    //- protocol we want to receive messages / traffics for
    //- ETH_P_ALL = I WANT EVERYTHING, ALL YOUR PACKETS 
    //  > includes packets in both directions
	//we could specify just a specific one
	packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(packet_socket<0){
	  perror("socket");
	  return 2;
	}
	//Bind the socket to the address, so we only get packets
	//recieved on this specific interface. For packet sockets, the
	//address structure is a struct sockaddr_ll (see the man page
	//for "packet"), but of course bind takes a struct sockaddr.
	//Here, we can use the sockaddr we got from getifaddrs (which
	//we could convert to sockaddr_ll if we needed to)
    //
    //want a diff socket for each interface, so know which interface you are sending/recv on
    //bind each to a packet socket
	if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
    }
  }
  //free the interface list when we don't need it anymore
  freeifaddrs(ifaddr);

  buffer = (void*)malloc(BUF_SIZE);
  unsigned char* etherhead = buffer;
  struct ethhdr *eh = (struct ethhdr *)etherhead;
  struct arp_header *ah;

  //loop and recieve packets. We are only looking at one interface,
  //for the project you will probably want to look at more (to do so,
  //a good way is to have one socket per interface and use select to
  //see which ones have data)
  printf("Ready to recieve now\n");
  int arpcount = 0;
  int icmpcount = 0;
  while(1){
    
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);
    //we can use recv, since the addresses are in the packet, but we
    //use recvfrom because it gives us an easy way to determine if
    //this packet is incoming or outgoing (when using ETH_P_ALL, we
    //see packets in both directions. Only outgoing can be seen when
    //using a packet socket with some specific protocol)
    //
    //recvfrom - ensure only looking at incoming packet
    //skip all outgoing packets
    int n = recvfrom(packet_socket, buffer, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
    //ignore outgoing packets (we can't disable some from being sent
    //by the OS automatically, for example ICMP port unreachable
    //messages, so we will just ignore them here)
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
      continue;
    //start processing all others
    printf("Got a %d byte packet\n", n);
    
    //what else to do is up to you, you can send packets with send,
    //just like we used for TCP sockets (or you can use sendto, but it
    //is not necessary, since the headers, including all addresses,
    //need to be in the buffer you are sending)
    //
	
    if(htons(eh->h_proto) == 0x806) {
        printf("0x806! YAY\n");
    }

    /*
    if(recvaddr.sll_pkttype == 0) {
        printf("packet type == 0! i guess its ICMP?\n");
    }else if(recvaddr.sll_pkttype == 4) {
        pritnf("packet type == 4! i guess its ARP?\n");
    }else
        printf("not 0\n");
        */

    printf("sll_protocol: %x\n", recvaddr.sll_protocol);
    //printf("arp hardware type: %d\n", recvaddr.sll_hatype);
    
    int is_arp = process_arp_packet(&recvaddr, &arpcount);
    int is_icmp = process_icmp_packet(&recvaddr, &icmpcount);

    if(is_arp == 1) {
			ah =(struct arp_header *) (etherhead+14);
								printf("buffer is---------------- %s \n",(char*)ah);
                                printf("H/D TYPE : %x PROTO TYPE : %x \n",ah->arp_hd,ah->arp_pr);
                                printf("H/D leng : %x PROTO leng : %x \n",ah->arp_hdl,ah->arp_prl);
                                printf("OPERATION : %x \n", ah->arp_op);
                                printf("SENDER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_sha[0],
                                       ah->arp_sha[1],
                                       ah->arp_sha[2],
                                       ah->arp_sha[3],
                                       ah->arp_sha[4],
                                       ah->arp_sha[5]
                                       );
                                printf("SENDER IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_spa[0],
                                       ah->arp_spa[1],
                                       ah->arp_spa[2],
                                       ah->arp_spa[3]
                                       );
                                       
                                       printf("TARGET MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_dha[0],
                                       ah->arp_dha[1],
                                       ah->arp_dha[2],
                                       ah->arp_dha[3],
                                       ah->arp_dha[4],
                                       ah->arp_dha[5]
                                       );
                                printf("TARGET IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_dpa[0],
                                       ah->arp_dpa[1],
                                       ah->arp_dpa[2],
                                       ah->arp_dpa[3]
                                       );
 
                                printf("+++++++++++++++++++++++++++++++++++++++\n" );
                                printf("ETHER DST MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_dest[0],
                                       eh->h_dest[1],
                                       eh->h_dest[2],
                                       eh->h_dest[3],
                                       eh->h_dest[4],
                                       eh->h_dest[5]
                                       );
                                printf("ETHER SRC MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_source[0],
                                       eh->h_source[1],
                                       eh->h_source[2],
                                       eh->h_source[3],
                                       eh->h_source[4],
                                       eh->h_source[5]
                                       );
        // temp header struct for storing the fucking shit
        struct arp_header * new_ah;
        new_ah = ah;
        memcpy(&new_ah->arp_sha,&ah->arp_dha,sizeof(ah->arp_dha));
        memcpy(&new_ah->arp_dha,&ah->arp_sha,sizeof(ah->arp_sha));
        memcpy(&new_ah->arp_spa,&ah->arp_dpa,sizeof(ah->arp_dpa));
        memcpy(&new_ah->arp_dpa,&ah->arp_spa,sizeof(ah->arp_spa));
       // &new_ah->arp_spa = (ah->arp_dpa);
       // &new_ah->arp_dha = (ah->arp_sha);
       // &new_ah->arp_dpa = (ah->arp_spa);
        new_ah->arp_op = 2;
        
        ah->arp_sha[0] = new_ah->arp_sha[0];
        ah->arp_sha[1] = new_ah->arp_sha[1];
        ah->arp_sha[2] = new_ah->arp_sha[2];
        ah->arp_sha[3] = new_ah->arp_sha[3];
        ah->arp_sha[4] = new_ah->arp_sha[4];
        ah->arp_sha[5] = new_ah->arp_sha[5];
        ah->arp_dha[0] = new_ah->arp_dha[0];
        ah->arp_dha[1] = new_ah->arp_dha[1];
        ah->arp_dha[2] = new_ah->arp_dha[2];
        ah->arp_dha[3] = new_ah->arp_dha[3];
        ah->arp_dha[4] = new_ah->arp_dha[4];
        ah->arp_dha[5] = new_ah->arp_dha[5];
        ah->arp_spa[0] = new_ah->arp_spa[0];
        ah->arp_spa[1] = new_ah->arp_spa[1];
        ah->arp_spa[2] = new_ah->arp_spa[2];
        ah->arp_spa[3] = new_ah->arp_spa[3];
        ah->arp_dpa[0] = new_ah->arp_dpa[0];
        ah->arp_dpa[1] = new_ah->arp_dpa[1];
        ah->arp_dpa[2] = new_ah->arp_dpa[2];
        ah->arp_dpa[3] = new_ah->arp_dpa[3];
        
        
			
			
		printf("Setting up buffer to send...");
		send(packet_socket ,buffer,1500,0);
		printf("Sent the packet back.");	
		
       //send appropriate response, or forward it to others
       //
       //Just realized! we may get a ARP response back from other router
       //which we'd have to forward to the host who asked
       //example: h1 ping h3
       //   - we forward request from h1 to r2, r2 > h3
       //   - h3 sends response to r2, r2 >response> r1, r1 > h1 
    }
    if(is_icmp == 1) {
		
		void * start_data = etherhead + 26
		
		char * sip[4] = (char *) start_data;
		char * dip[4] = (char *) (start_data +4);
		short len = (short) (etherhead + 2)
		printf("packet size %hu", len);
		for(int i =0; i<4;i++){
			buffer[26+i] = dip[i];
			buffer[30+i] = sip[i];
			
		}
		
        // send appropriate ICMP response
        //send(packet_socket,(struct sockaddr*)&recvaddr,&recvaddrlen,0);
        //int nsize = sendto(packet_socket, buffer, 1500,0,
        //        (struct sockaddr*)&recvaddr, &recvaddrlen);
 
        printf("ICMP response: sending something back?\n");
    }

    printf("ifindex = %d\n", recvaddr.sll_ifindex);
    printf("arp count = %d\n", arpcount);
    printf("icmp count = %d\n", icmpcount);

  }
  //exit
  return 0;
}
