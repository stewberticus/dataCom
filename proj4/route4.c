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
    for (i = 1; i < 7; ++i){
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
      mac[i] = s.ifr_addr.sa_data[i-1];
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

  unsigned char mac_addrs[4][7]; 
  
  void* buffer = NULL;
  int packet_socket;
  fd_set sockets;
  FD_ZERO(&sockets);
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp;
    //gets all of the addresses of the network interfaces on your system
    //returns a list that contains roughly 3 addr per device / network interface
  //    > ipv4 addr, ipv6 addr, mac addr  
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }

  int q = 0;
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
      
      //creat res records from cache correctly after TTL    5
      //Socket timeouts implementedea packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
	  getmac(mac_addrs[q],tmp->ifa_name);
	  int j =1;
	  for(j;j<7;j++){
		printf("Mac: %02x\n",mac_addrs[q][j]);
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
	mac_addrs[q][0] = packet_socket;
       FD_SET(packet_socket,&sockets);
      }
    q++;
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

    fd_set tmp_set = sockets;
    select(FD_SETSIZE, &tmp_set, NULL,NULL,NULL);

    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        if(FD_ISSET(i,&tmp_set)){
           
                //we can use recv, since the addresses are in the packet, but we
                //use recvfrom because it gives us an easy way to determine if
                //this packet is incoming or outgoing (when using ETH_P_ALL, we
                //see packets in both directions. Only outgoing can be seen when
                //using a packet socket with some specific protocol)
                //
                //recvfrom - ensure only looking at incoming packet
                //skip all outgoing packets
                int n = recvfrom(i, buffer, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
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
                }elseah->arp_dha[0] = new_ah->arp_dha[0];
                    ah->arp_dha[1] = new_ah->arp_dha[1];
                    ah->arp_dha[2] = new_ah->arp_dha[2];
                    ah->arp_dha[3] = new_ah->arp_dha[3];
                    ah->arp_dha[4] = new_ah->arp_dha[4];
                    ah->arp_dha[5] 
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
                    icmp = (struct icmp_header *) ah->arp_dha[0] = new_ah->arp_dha[0];
                    ah->arp_dha[1] = new_ah->arp_dha[1];
                    ah->arp_dha[2] = new_ah->arp_dha[2];
                    ah->arp_dha[3] = new_ah->arp_dha[3];
                    ah->arp_dha[4] = new_ah->arp_dha[4];
                    ah->arp_dha[5] start_data;
                    
                    
                    
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
                    ah->arp_op = (unsigned short) htons(2) ;
                    int gogogo= 0;
		    int iface = 3;
		    for( iface; iface >= 0; iface --){
		     if(mac_addrs[iface][0] == i ) {
		       gogogo = 1;
		       break;
		     } 
		       
		    }
		    if(!gogogo){
		     // stop
		      continue;
		      
		    }
                    ah->arp_sha[0] = mac_addrs[iface][1];
                    ah->arp_sha[1] = mac_addrs[iface][2];
                    ah->arp_sha[2] = mac_addrs[iface][3];
                    ah->arp_sha[3] = mac_addrs[iface][4];
                    ah->arp_sha[4] = mac_addrs[iface][5];
                    ah->arp_sha[5] = mac_addrs[iface][6];
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
		    eh->h_dest[0] = eh->h_source[0];
		    eh->h_dest[1] = eh->h_source[1];
		    eh->h_dest[2] = eh->h_source[2];
		    eh->h_dest[3] = eh->h_source[3];
		    eh->h_dest[4] = eh->h_source[4];
		    eh->h_dest[5] = eh->h_source[5];
		    eh->h_source[0] = mac_addrs[iface][1];
		    eh->h_source[1] = mac_addrs[iface][2];
		    eh->h_source[2] = mac_addrs[iface][3];
		    eh->h_source[3] = mac_addrs[iface][4];
		    eh->h_source[4] = mac_addrs[iface][5];
		    eh->h_source[5] = mac_addrs[iface][6];
                    
                    
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

					int index;
					int a,b,c,d;
					for(index = 0; index < 4; index++) {
					} 

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
                    send(i ,buffer,n,0);
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
                    
                    int index;
		    int a,b,c,d;
		    
		    for(a = 0; a < 4; a++) {
			    for(b = 0; b < 6; b++) {
				    printf("~~~~~~~~~~\n");
				    printf("eha: %02X   mac_addrs: %02X\n", eh->h_dest[b],
				      mac_addrs[a][b]);
				    if(eh->h_dest[b] == mac_addrs[a][b]) {
					    printf("MATCH (a,b) = %d,%d\n", a,b);
				    }	
				    else {
					    printf("****NOT MATCHING MAC_ADDRS****\n");
					    break;
				    }
				    printf("~~~~~~~~~~\n");
			    }	
		    }  
            struct ipheader * iph;
            iph = (struct ipheader *) (etherhead + 14);

            printf("ip_header ip_ttl = %d\n", iph->ip_ttl);
            printf("ip_header ip_len  = %d\n", iph->ip_len);
            
            printf("ip_header ip_cheksum = %x\n", iph->ip_sum);
            
            unsigned short * checksumhead = etherhead + 14;
            unsigned short answer = 0;
            int chck_sum = 0;
            int p = 14;
            for(p; p<34; p+=2){
              if(!(p == 24 || p == 25)){
                printf("%x\n",*checksumhead);
                chck_sum = chck_sum + *checksumhead;
                checksumhead ++;
              }else
                checksumhead ++;
                
            }
            chck_sum = (chck_sum >> 16) + (chck_sum & 0xffff); 
            chck_sum += (chck_sum >> 16);
            /* add carry */
            answer = ~chck_sum;  
            /* truncate to 16 bits */
            printf("OUR ip_header ip_cheksum = %x\n", answer);
            if( answer != iph->ip_sum){
             continue ;
            }
            if( iph -> ip_ttl > 1)
                iph -> ip_ttl --; 
            else
                printf("send ttyl error");
            
            checksumhead = etherhead + 14;
            answer = 0;
            chck_sum = 0;
            p = 14;
            for(p; p<34; p+=2){
              if(!(p == 24 || p == 25)){
                printf("%x\n",*checksumhead);
                chck_sum = chck_sum + *checksumhead;
                checksumhead ++;
              }else
                checksumhead ++;  
            }
            chck_sum = (chck_sum >> 16) + (chck_sum & 0xffff); 
            chck_sum += (chck_sum >> 16);
            answer = ~chck_sum;  
            
            
            
            printf("OUR  NEW ip_header ip_cheksum = %x\n", answer);
            iph-> ip_sum = answer;
            
            // file IO stuff
             char dest_ip[20];
             char next_hop[20];
             char interface[20];
                     
		    printf("ip_dst = %d\n", iph->ip_dst);
		    printf("htons ip_dst = %d\n", htons(iph->ip_dst));


		  
		    char *  bytes_ip [4];
		    bytes_ip[0] = iph->ip_dst & 0xFF;
		    bytes_ip[1] = (iph->ip_dst >> 8) & 0xFF;
		    bytes_ip[2] = (iph->ip_dst >> 16) & 0xFF;
            char dst_ip_str[11]="";
            char tmp2[3];

            char binStart2_ip[8];
            //ip_dst to binary integer
            int binIP[32];
            int binStart = (htonl(iph->ip_dst) >> 8) << 8;
            int binStart2 = binStart;
            printf("START binStart = %d\n", binStart);
            int x = 1;
            int y;
            while(binStart != 0) {
                binIP[x++] = binStart %2 ;
                binStart = binStart / 2;
            }
            printf("x is %d\n", x);
            printf ("SECOND: ");
            for(y = x -1; y > 0; y--) {
                printf("%d", binIP[y]);
            }

            /*
            sprintf(tmp2, "%d", bytes_ip[0]);
            dst_ip_str += tmp;
            dst_ip+str += ".";
            sprintf(tmp2, "%d", bytes_ip[1]);
            dst_ip_str += tmp;
            dst_ip+str += ".";
            sprintf(tmp2, "%d", bytes_ip[2]);
            dst_ip_str += tmp;
            */
            //char dst_ip_str = itoa(bytes_ip[0]) + "." + itoa(bytes_ip[1])
             //   + itoa(bytes_ip[2]);
		    //bytes_ip[3] = (iph->ip_dst >> 24) & 0xFF;
		    printf("ip_dst as char[] = %d.%d.%d\n",
		      bytes_ip[0], bytes_ip[1], bytes_ip[2]); //, bytes_ip[3]);
		    //int sumof_bytes_ip = bytes_ip[0] + bytes_ip[1] + bytes_ip[2];	
		    //printf("sum of bytes_ip = %d\m", sumof_bytes_ip);
		    printf("ip_dst as char[] = %s\n", bytes_ip);
                    FILE *fp; 
                    fp = fopen(".//r1-table.txt","r");
		    printf("---Start reading routing table - r1 ---\n");
		    while (fscanf(fp, "%s %s %s", dest_ip, next_hop,
		      interface) != EOF) {
	
                /* START NEW */
                printf("file dest_ip %s\n", dest_ip);
                printf("next_hop = %s\n", next_hop);
                printf("interface = %s\n", interface);

                //printf("dest_ip[0] is %s\n", dest_ip[0]);
                int c = 0;
                while(c < 8) {
                    binStart2_ip[c] = dest_ip[c];
                    c++;
                }

                int matches = 1;
                int dst_ip0, dst_ip1, dst_ip2, dst_ip3;
                int dst_ip_sum;
                int z;
                dst_ip0 = 0;
                dst_ip0 = atoi(strtok(dest_ip, "."));
                printf("%d\n", dst_ip0);
                dst_ip1 = 0;
                dst_ip1 = atoi(strtok(NULL, "."));
                printf("%d\n", dst_ip1);
                dst_ip2 = 0;
                dst_ip2 = atoi(strtok(NULL, "."));
                printf("%d\n", dst_ip2);
                dst_ip3 = 0;
                dst_ip3 = atoi(strtok(NULL, "."));
                printf("%d\n", dst_ip3);

                dst_ip_sum = 0;
                dst_ip_sum = (dst_ip0 * 16777216) + (dst_ip1 * 65536) 
                    + (dst_ip2 * 256) + dst_ip3;
                printf("IP SUM: %d\n", dst_ip_sum);
                printf("binStart2: %d\n", binStart2);

                matches = (dst_ip_sum == binStart2) ? 1 : 0;

                /*
                int binIP2[32];
                //int binStart = (htonl(iph->ip_dst) >> 8) << 8;
                //printf("START binStart = %d\n", binStart);
                int x2 = 1;
                int y2;
                while(dst_ip_sum != 0) {
                    binIP2[x2++] = dst_ip_sum %2 ;
                    dst_ip_sum = dst_ip_sum / 2;
                }
                printf("x2 is %d\n", x2);
                printf ("IP SUM SECOND: ");
                for(y2 = x2 -1; y2 > 0; y2--) {
                    printf("%d", binIP2[y2]);
                }
                */

                //if(dst_ip_sum == 

                /* END NEW */
                /*
                char sub_dest_ip[7];
                memcpy(sub_dest_ip, &dest_ip[0], 6);
                sub_dest_ip[6] = '\0';
                //strncpy(dest_ip, dest_ip, 5);
                //printf("file new ip_dest %s\n", dest_ip);
                printf("file new sub_dest_ip %s\n", sub_dest_ip);

                //printf("sub_dest_ip[0] = %s\n", sub_dest_ip[0]);
                //printf("sub_dest_ip[1] = %s\n", sub_dest_ip[1]);
                //printf("sub_dest_ip[2] = %s\n", sub_dest_ip[2]);

                int dest_ip_int = 0;
                char * ip_strtok;
                //ip_strtok = strtok(sub_dest_ip,".");
                int strtok_id = 0;
                char scanned [3][3];
                
                char * ptr_sub;
                char * ptr_ip;
                ptr_sub = sub_dest_ip;
                ptr_ip = dst_ip_str; 


                while(ptr_sub != '\0' && ptr_ip != '\0') {
                    printf("ptr_sub: %c \t ptr_ip: %c", *ptr_sub, *ptr_ip);
                    if((*ptr_ip) != (*ptr_sub))  {
                        matches = 0;
                        break;
                    }
                    ptr_sub++;
                    ptr_ip++;
                }

                printf("dest_ip_int =  %d\n", dest_ip_int);
                printf("htons ip_dst = %d\n", htons(iph->ip_dst));
		        */ 

                //if(bytes_ip == sub_dest_ip) 
                //if(dest_ip_int == htons(iph->ip_dst))
                if(matches == 1) {
                    printf("ip dest MATCHEs - stop looking\n");	
                    break;
                } else
                    printf("ip dest NO MATCH - keep going\n");
                //if(dest_ip == iph->ip_dst) 
                //	printf("dest matches\n");
                //else
                //	printf("no match\n");
                printf("\n");
		    }
		    printf("---Done reading file---\n");
              if(maches == 0) {
                //still no matches, send an ICMP error message
                //TODO
              printf("ICMP destinaion unreachable (network unreachable\n");
              } else {
                //continue normally
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
                    send(i ,buffer,n,0);
                    
                    // send appropriate ICMP response
                    //int nsize = sendto(packet_socket, buffer, 1500,0,
                    //        (struct sockaddr*)&recvaddr, &recvaddrlen);
             
                    printf("ICMP response: sending something back?\n");
              } // ends else (matches) 
            } // ends (is_icmp ==1 )
                
            
        } // ends if(FD_ISSET(i,&tmp_set))
    } // ends for loop
    printf("ifindex = %d\n", recvaddr.sll_ifindex);
    printf("arp count = %d\n", arpcount);
    printf("icmp count = %d\n", icmpcount);

  } // ends while loop
  //exit
  return 0;
}
