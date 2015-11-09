#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/ip.h>
#define BUF_SIZE 100
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
struct __attribute__((packed)) icmp_header
{
    unsigned char src[4];
    unsigned char dst[4];
};
struct __attribute__((packed)) ipheader {
    /* 0 */ unsigned char ip_hl:4, ip_v:4;
    /* 1 */ unsigned char ip_tos;
    /* 2 */ unsigned short int ip_len;
    /* 3 */ unsigned short int ip_id;
    /* 4 */ unsigned short int ip_off;
    /* 5 */ unsigned char ip_ttl;
    /* 6 */ unsigned char ip_p;
    /* 7 */ unsigned short int ip_sum;
    /* 8 */ unsigned int ip_src;
    /* 9 */ unsigned int ip_dst;
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
  unsigned char mac[6];

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
	  getmac(mac,tmp->ifa_name);
	  int j =0;
	  for(j;j<6;j++){
		printf("Mac: %02x\n",mac[j]);
	  }
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
    
    int is_arp = 0;
    is_arp = process_arp_packet(&recvaddr, &arpcount);
    int is_icmp = 0;
    is_icmp = process_icmp_packet(&recvaddr, &icmpcount);

    //trying to check the ip header checksum
    //do we check the IP checksum, or the ICMP checksum
    // +28 or +9 or ...
    void * ttl_location = etherhead  + 28;
    //void * hdr_checksum = etherhead + 11;
    // +32 or +11 or ...
    void * hdr_checksum = ttl_location + 32;
    char * ttl = (char * ) ttl_location;
    char * checksum = (char *) hdr_checksum;

    //printf("ip ttl is ---------%s \n", ttl); 
    //printf("ip header checksum is ---------%s \n", checksum);

    /* Trying to use the below code as a basis for the ttl and checksum
     *
		void * start_data = etherhead + 26;
		struct icmp_header * icmp; 
		icmp = (struct icmp_header *) start_data;
		
		
		
			char tmp_pnt[4];
			memcpy(&tmp_pnt,icmp->src,sizeof(tmp_pnt));
			//icmp->src = icmp->dst;
			memcpy(&icmp->src , &icmp->dst, sizeof(icmp->src));
			memcpy(&icmp->dst,&tmp_pnt,sizeof(tmp_pnt));
			//icmp->dst =  (char *) tmp_pnt;
		void * icmp_type = etherhead + 34;
		char * k = (char *) icmp_type;
		*k = 0;
    *
    */

    

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
	
  	new_ah = (void*)malloc(sizeof(struct arp_header));
	printf ("***  1  ***\n");
        memcpy(new_ah->arp_sha,ah->arp_dha,sizeof(ah->arp_dha));
	printf ("***  2  ***\n");
        memcpy(new_ah->arp_dha,ah->arp_sha,sizeof(ah->arp_sha));
	printf ("***  3  ***\n");
        memcpy(new_ah->arp_spa,ah->arp_dpa,sizeof(ah->arp_dpa));
	printf ("***  4  ***\n");
        memcpy(new_ah->arp_dpa,ah->arp_spa,sizeof(ah->arp_spa));
       // &new_ah->arp_spa = (ah->arp_dpa);
	printf ("***  after  ***\n");
       // &new_ah->arp_dha = (ah->arp_sha);
       // &new_ah->arp_dpa = (ah->arp_spa);
        //ah->arp_op = 2;
        
        ah->arp_sha[0] = mac[0];
        ah->arp_sha[1] = mac[1];
        ah->arp_sha[2] = mac[2];
        ah->arp_sha[3] = mac[3];
        ah->arp_sha[4] = mac[4];
        ah->arp_sha[5] = mac[5];
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
			
			
		printf("Setting up buffer to send...");
		sendto(packet_socket ,buffer,n,0,(struct sockaddr *) &recvaddr, sizeof(recvaddr));
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
		
		char tmp[6];
		
		memcpy(&tmp,etherhead,sizeof(tmp));
		
		memcpy(etherhead,etherhead+6,sizeof(tmp));

		memcpy(etherhead +6,&tmp,sizeof(tmp));
		
		
		struct ipheader * iph;
		iph = (struct ipheader *) (etherhead + 14);

		printf("ip_header ip_ttl = %d\n", iph->ip_ttl);
		printf("ip_header ip_len  = %d\n", iph->ip_len);
		
		printf("ip_header ip_cheksum = %d\n", iph->ip_sum);
		
		unsigned char* checksumhead = etherhead + 14;
		unsigned short answer = 0;
		int chck_sum = 0;
		int i = 14;
		for(i; i<36; i++){
		  if(!(i == 26 || i == 27)){
		    printf("%x\n",*checksumhead);
		    chck_sum = chck_sum + *checksumhead;
		    checksumhead ++;
		  }else
		    printf("wtf");
		}
		chck_sum = (chck_sum >> 16) + (chck_sum & 0xffff); 
		printf("OUR ip_header ip_cheksum before adding carry = %d\n", chck_sum);/* add hi 16 to low 16 */
		chck_sum += (chck_sum >> 16);
		/* add carry */
		answer = ~chck_sum;              /* truncate to 16 bits */
		printf("OUR ip_header ip_cheksum = %d\n", answer);
		void * start_data = etherhead + 26;
		struct icmp_header * icmp; 
		icmp = (struct icmp_header *) start_data;
		
		
		
			char tmp_pnt[4];
			memcpy(&tmp_pnt,icmp->src,sizeof(tmp_pnt));
			//icmp->src = icmp->dst;
			memcpy(&icmp->src , &icmp->dst, sizeof(icmp->src));
			memcpy(&icmp->dst,&tmp_pnt,sizeof(tmp_pnt));
			//icmp->dst =  (char *) tmp_pnt;
		void * icmp_type = etherhead + 34;
		char * k = (char *) icmp_type;
		*k = 0;
		sendto(packet_socket ,buffer,n,0,(struct sockaddr *) &recvaddr, sizeof(recvaddr));
		 	
		
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
