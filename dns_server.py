__author__ = 'alex.stuart'
import socket               # Import socket module
import binascii
def parse_name(data,index,rdata):
    real_bin = bytearray(data)
    words = []
    while True:
        num_octets = real_bin[index]
        index += 1
        if num_octets == 0:
            break
        if num_octets == 192:
            byte = bin(real_bin[index-1])[2:].rjust(8, '0')
            byte += bin(real_bin[index])[2:].rjust(8, '0')
            pointer_to = int(byte[2:],2)
            for a_word in parse_name(data,pointer_to,False)[0]:
                words.append(a_word)
            index+=1
            if not rdata:
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                type = byte
                print "type:"+type
                index +=2
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                ns_class = byte
                print "class" + ns_class
                index+=2
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                byte += bin(real_bin[index+2])[2:].rjust(8, '0')
                byte += bin(real_bin[index+3])[2:].rjust(8, '0')
                ttl = int(byte,2)
                index+=4
                print "ttl:"+str(ttl)
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                rdata_len = int(byte,2)
                print "rdata len:" + str(rdata_len)
                index+=2
                rdata_words,not_needed= parse_name(real_bin,index,True)
                index+=rdata_len
                return (rdata_words,index)
                break
            else:
                break
        word = ''
        if not num_octets == 192:
            for j in range(num_octets):
                this_char = real_bin[index]
                word = word + chr(this_char)
                index += 1
        if len(word)> 0:
            words.append(word)
    print words
    return (words,index)
def parse_data(data):
     i = 0
     real_bin  = bytearray(data)
     raw_bytes = []
     for k in real_bin:
        raw_bytes.append(bin(k)[2:].rjust(8, '0'))
     while i < len(real_bin):
        if i/2 == 0:
            print 'ID is:'
        if i/2 == 1:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            print byte
            print "response  :"+ byte[0]
            print "OP code :" + byte[1:5]
            print "Auth Answer: "+ byte[5]
            if byte[5] == '1':
                return "Send to Client"
            print "Trun Resp: " +byte[6]
            # TODO: flip recursion desired
            print "Recusion Desired: "+ byte[7]
            if(byte[7] == '1'):
                # if the recursion is desired, unset that bit
                # 128 = 0b10000000
                real_bin[i] = 128
            print "Recursion Available: " + byte[8]
            print "Z (unused): " +byte[9:12]
            print "R Code: " +byte[12:16]
        if i/2 == 2:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_DQ = int (byte,2)
            print 'DQcount: '+ str(num_DQ)
        if i/2 == 3:
            print 'AN Count'
        if i/2 == 4:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_NS = int (byte,2)
            print 'NS count: '+ str(num_NS)
        if i/2 == 5:
            print 'AR count'
        if i/2 == 6:
            print 'Questions Start'
            for num in range(num_DQ):
                words, i = parse_name(real_bin,i,False)
                byte = bin(real_bin[i])[2:].rjust(8, '0')
                byte += bin(real_bin[i+1])[2:].rjust(8, '0')
                print "Qtype:" + byte
                i = i + 2
                byte = bin(real_bin[i])[2:].rjust(8, '0')
                byte += bin(real_bin[i+1])[2:].rjust(8, '0')
                print "QClass: " + byte
                i = i + 2
            print "Answers start:"
            places_to_ask_next = []
            for num in range(num_NS):
                words, index = parse_name(real_bin,i,False)
                places_to_ask_next.append(words)
                if index>i:
                    i = index
        i = i + 2
     print places_to_ask_next
     return places_to_ask_next
serv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)         # Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip
print this_machine_name
port = raw_input("Enter port number: ")
try:
        port = int(port)
except Exception:
        print "not a number"
        quit()                # Reserve the passed port for your service.
serv_sock.bind((this_machine_name, 9999))        # Bind to the port
while True:
    data, client_addr = serv_sock.recvfrom(1024)     # Establish connection with client.
    print 'connected to', str(client_addr)       # Confirm correct client
    real_bin  = bytearray(data)
    dns_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dns_sock.sendto(real_bin,("192.112.36.4",53))
    data, addr = dns_sock.recvfrom(1024)
    next = parse_data(data)
    next_ip = ''
    for i in next[0]:
        next_ip+=i+'.'
    next_ip = next_ip[0:-1]
    while True:
        dns_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        dns_sock.sendto(real_bin,(next_ip,53))
        origdata, dns_addr = dns_sock.recvfrom(1024)
        data = origdata
        next = parse_data(data)
        if(next=="Send to Client"):
            break
        else:
            next_ip = ''
            for i in next[0]:
                next_ip+=i+'.'
            next_ip = next_ip[0:-1]
    to_send = bytearray(origdata)
    serv_sock.sendto(to_send,client_addr)
