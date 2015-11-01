#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#define BUF_SIZE 42

//this method is meant to check if the packet is ARP
//and to process the request accordingly
//- forward to correct address?
//- use look up tables r1-table.txt and r2-table.txt
bool process_arp_packet(struct sockaddr_ll * recvaddr, int * count) {
    printf("processing arp packet\n");
    bool is_arp = false;
    if(recvaddr->sll_protocol == 1544) {
        (*count) += 1;
        printf("ARP: YES\n");
        is_arp = true;
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
bool process_icmp_packet(struct sockaddr_ll * recvaddr, int * count) {
    printf("processing icmp packet\n");
    bool is_icmp = false;
    if(recvaddr->sll_protocol == 8) {
        printf("ICMP: YES\n");
        (*count) += 1;
        is_icmp = true;

        //type should be cleared to 0
        recvaddr->type = 0;
        
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
      printf("Interface: %s\n",tmp->ifa_name);
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
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

  //loop and recieve packets. We are only looking at one interface,
  //for the project you will probably want to look at more (to do so,
  //a good way is to have one socket per interface and use select to
  //see which ones have data)
  printf("Ready to recieve now\n");
  int arpcount = 0;
  int icmpcount = 0;
  while(1){
    char buf[1500];
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
    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
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
    
    bool is_arp = process_arp_packet(&recvaddr, &arpcount);
    bool is_icmp = process_icmp_packet(&recvaddr, &icmpcount);

    if(is_arp) {
       //send appropriate response, or forward it to others
       //
       //Just realized! we may get a ARP response back from other router
       //which we'd have to forward to the host who asked
       //example: h1 ping h3
       //   - we forward request from h1 to r2, r2 > h3
       //   - h3 sends response to r2, r2 >response> r1, r1 > h1 
    }
    if(is_icmp) {
        // send appropriate ICMP response
    }

    printf("arp count = %d\n", arpcount);
    printf("icmp count = %d\n", icmpcount);

  }
  //exit
  return 0;
}
