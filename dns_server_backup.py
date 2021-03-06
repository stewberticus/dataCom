__author__ = 'alex.stuart,Lauren Mills, Caleb Stevenson'
import socket               # Import socket module
import binascii
import time

lookings = []
ttl =0
looking = ""
cache = {}
recursion_desired = True
no_recursion_data = None
new_cache = {}
ttl_dict = {} 
ttl_list = []

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
                print "type: "+type
                if not (type == "0000000000000010"):
                    # this doesn't seem to work
                    print "Type is not A"
                    error_code = -88
                    #return ("type*incorrect*error", error_code)
                index +=2
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                ns_class = byte
                print "class: " + ns_class
                if not(ns_class == "0000000000000001"):
                    # this doesn't seem to work
                    print "Class is not IN"
                    error_code = -89
                    #return ("class*incorrect*error", error_code)
                index+=2
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                byte += bin(real_bin[index+2])[2:].rjust(8, '0')
                byte += bin(real_bin[index+3])[2:].rjust(8, '0')
                ttl = int(byte,2)
                #ttl_dict[real_bin] = ttl
                #ttl_list.append(("**", ttl, datetime.datetime.now()))
                ttl_list.append(("**", ttl, time.time()))
                index+=4
                print "ttl: "+str(ttl)
                byte = bin(real_bin[index])[2:].rjust(8, '0')
                byte += bin(real_bin[index+1])[2:].rjust(8, '0')
                rdata_len = int(byte,2)
                print "rdata length:" + str(rdata_len)
                index+=2
                rdata_words,not_needed= parse_name(real_bin,index,True)
                index+=rdata_len
                return (rdata_words,index)
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
            #print byte
            print "response  :"+ byte[0]
            print "OP code :" + byte[1:5]
            print "Auth Answer: "+ byte[5]
            if byte[5] == '1':
                return "Send to Client"
            print "Trun Resp: " +byte[6]
            print "Recusion Desired: "+ byte[7]
            if(byte[7] == '1'):
                print "Flip the recursion desired bit"
                real_bin[i] = 128
                global recursion_desired
                global no_recursion_data
                recursion_desired = False
                # TODO: figure out how to change data that's sent back
                data = real_bin
                no_recursion_data = data
            print "Recursion Available: " + byte[8]
            print "Z (unused): " +byte[9:12]
            print "R Code: " +byte[12:16]
            rcode = int(byte[12:16], 2)
            print "Integer R Code: ", rcode
            #TODO: send error to client
            #idea: send a code, then have these long messages in function
            if rcode == 0:
                print "No error"
            elif rcode == 1:
                print "Format error - Unable to interpret query"
                return "1*format*error"
            elif rcode == 2:
                print "Server failure - Problem with the name server"
                return "2*server*failure*"
            elif rcode == 3:
                print "Name error - specified domain name does not exist"
                return "3*name*error*"
            elif rcode == 4:
                print "Unimplemented query error - This query is not supported"
                return "4*unimplemented*query*error*"
            elif rcode == 5:
                print "Refused error - Name server refused operation"
                return "5*refused*error*"
        if i/2 == 2:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_DQ = int (byte,2)
            print 'DQcount: '+ str(num_DQ)
        if i/2 == 3: 
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_AN = int (byte,2)
            print 'AN count: '+ str(num_AN)
        if i/2 == 4:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_NS = int (byte,2)
            print 'NS count: '+ str(num_NS)
        if i/2 == 5: 
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')
            num_AR = int (byte,2)
            print 'AR count: '+ str(num_AR)
        if i/2 == 6:
            print 'Questions Start'
            for num in range(num_DQ):
                print "Question address: "
                words, i = parse_name(real_bin,i,False)
                #print "i:", i
                #if(i == 255):
                #    return words
                byte = bin(real_bin[i])[2:].rjust(8, '0')
                byte += bin(real_bin[i+1])[2:].rjust(8, '0')
                print "Qtype:" + byte
                i = i + 2
                byte = bin(real_bin[i])[2:].rjust(8, '0')
                byte += bin(real_bin[i+1])[2:].rjust(8, '0')
                print "QClass: " + byte
                i = i + 2
            #print "Answers start*******************************************:"
            places_to_ask_next = []
            for num in range(num_NS):
                print "Question: " + str(num+1)
                
                words, index = parse_name(real_bin,i,False)
                #print "index:", index
                #print "words ", words


               
                looking = ''.join(str(e) for e in words)
               
               #KEYYYYSSSSS
                lookings.append(looking)
                #if(index == 255):
                #    return "type*incorrect*error"
                #    #return words
                places_to_ask_next.append(words)
                if index>i:
                    i = index
        i = i + 2
     print "Places We Could Look:"
     print places_to_ask_next
     return places_to_ask_next


def check_errors(error_message, real_bin):
    have_errors = "no errors"
    #print "loooooking before true******:" , looking
    if error_message == "1*format*error":
        print "Format error - Unable to interpret query"
        have_errors = "Format error - Unable to interpret query"
        #return "Format error - Unable to interpret query"
        #quit()
    elif error_message == "2*server*failure":
        print "Server failure - Problem with the name server"
        have_errors =  "Server failure - Problem with the name server"
        #return "Server failure - Problem with the name server"
        #quit()
    elif error_message == "3*name*error*": 
        print "Name error - specified domain name does not exist"
        have_errors =  "Name error - specified domain name does not exist"
        #return "Name error - specified domain name does not exist"
        #quit()
    elif error_message == "4*unimplemeted*query*error*":
        print "Unimplemented query error - This query is not supported"
        have_errors = "Unimplemented query error - This query is not supported"
        #return "Unimplemented query error - This query is not supported"
        #quit()
    elif error_message == "5*refused*error":
        print "Refused error - Name server refused operation"
        have_errors = "Refused error - Name server refused operation"
        #return "Refused error - Name server refused operation"
    elif error_message == "type*incorrect*error":
        print "Incorrect Type - Type is not A"
        have_errors = "Incorrect Type - Type is not A"
    elif error_message == "class*incorrect*error":
        print "Incorrect Class - Class is not IN"
        have_errors = "Incorrect Class - Class is not IN"

    #print "checking errors"
    #print have_errors
    if (not (have_errors == "no errors")):
        # there was some error message. send it to the client
        #serv_sock.sendto(have_errors,client_addr)
        copy_bin = real_bin
        print "There were some errors!"
        #print real_bin
        #return (True, have_errors)  # errors

    return (False, "No error") # no errors

def get_byte_array(data, qdcount, ancount, nscount, arcount):
    i = 0

    new_bin = "";

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

            # need to turn AA-Authoritative Answer on?
            new_bin += (byte);
            #print "new_bin AuthAnswer: " + new_bin[5]
            #new_bin[5] = '1'
            #print new_bin

            if byte[5] == '1':
                return "Send to Client"
            if(byte[7] == '1'):
                real_bin[i] = 128
                global recursion_desired
                global no_recursion_data
                recursion_desired = False
                # TODO: figure out how to change data that's sent back
                data = real_bin
                no_recursion_data = data
            rcode = int(byte[12:16], 2)
            #TODO: send error to client
            #idea: send a code, then have these long messages in function
            if rcode == 0:
                print "No error"
            elif rcode == 1:
                print "Format error - Unable to interpret query"
                return "1*format*error"
            elif rcode == 2:
                print "Server failure - Problem with the name server"
                return "2*server*failure*"
            elif rcode == 3:
                print "Name error - specified domain name does not exist"
                return "3*name*error*"
            elif rcode == 4:
                print "Unimplemented query error - This query is not supported"
                return "4*unimplemented*query*error*"
            elif rcode == 5:
                print "Refused error - Name server refused operation"
                return "5*refused*error*"
        if i/2 == 2:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')


            #new_bin += bin(qdcount)[2:].rjust(8, '0')
            new_bin += byte

            num_DQ = int (byte,2)
        if i/2 == 3: 
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')

            #new_bin += bin(ancount)[2:].rjust(8, '0')
            new_bin += byte

            num_AN = int (byte,2)
        if i/2 == 4:
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')

            #new_bin += bin(nscount)[2:].rjust(8, '0')
            new_bin += byte

            num_NS = int (byte,2)
        if i/2 == 5: 
            byte = bin(real_bin[i])[2:].rjust(8, '0')
            byte += bin(real_bin[i+1])[2:].rjust(8, '0')

            #new_bin += bin(1)[2:].rjust(8, '0')
            new_bin += byte

            num_AR = int (byte,2)
        i = i + 2
    return new_bin 


serv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)         # Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip
print this_machine_name
port = raw_input("Enter port number: ")
try:
        port = int(port)
except Exception:
        print "not a number"
        quit()                # Reserve the passed port for your service.
serv_sock.bind((this_machine_name, port))        # Bind to the port
while True:
    data, client_addr = serv_sock.recvfrom(1024)     # Establish connection with client.
    print 'connected to', str(client_addr)

    new_byte_array = get_byte_array(data, 1, 2, 3, 1)

    # Confirm correct client
    real_bin  = bytearray(data)
    dns_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dns_sock.settimeout(10)
    dns_sock.sendto(real_bin,("192.112.36.4",53))
    dns_sock.settimeout(None)
    data, addr = dns_sock.recvfrom(1024)

    old_data = data
    new_data_from_array = get_byte_array(data, 0, 0, 0, 0)

    #timer = time.time()

    recursion_data = data
    next = parse_data(data)
    next_ip = ''

    if(new_data_from_array in new_cache):
        print "This was in the cache before!"
        curr_time = time.time()
        #print "curr_time: ", curr_time
        #print "ttl set time: ", ttl_dict[new_data_from_array][2]
        #print "ttl time: ", ttl_dict[new_data_from_array][1]
        #print "curr - set: ", curr_time - ttl_dict[new_data_from_array][2] 
        
        #dns_sock.settimeout(1.0)        

        if(new_data_from_array in ttl_dict):
            if( curr_time - ttl_dict[new_data_from_array][2] > 
                    ttl_dict[new_data_from_array][1]):
                print "You are out of time! Get updated stuff."
                #TODO: remove the value from the dictionary
                del ttl_dict[new_data_from_array]
                #print ttl_dict
            else:
                print "Just in time! Use previous data"
                next = "Send to Client"
                to_send = new_cache[new_data_from_array] 
        else:
            print "This entry is not found in the ttl_dictionary. Sorry."
    else:
        print "Not in the cache. Keep searching"
    
    
    error, error_message = check_errors(next, real_bin)
    if(next=="Send to Client"):
        to_send = bytearray(data) 
        #new_cache[bin(bytearray(old_data))] = bin(bytearray(data))
        new_cache[new_data_from_array] = bytearray(data)
        print "This is an answer! Store it in the cache."
    elif(error):
        to_send = bytearray(data)
    else:
        for i in next[0]:
          next_ip+=i+'.'
        next_ip = next_ip[0:-1]
        error = False
        error_message = "No error"
        while True:
            dns_sock  = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            dns_sock.sendto(real_bin,(next_ip,53))
            origdata, dns_addr = dns_sock.recvfrom(1024)
            
            #print "Check values in the ttl list"
            #print ttl_list

#sets cache################################################           
            cache[lookings[len(lookings)-1]]=str(dns_addr)
            #print 'connected to*********', str(dns_addr)       # Confirm correct client
            if recursion_desired:
                print "recursion is on"
                data = origdata
            else:
                print "recursion is off"
                #data = no_recursion_data
            data = origdata
            next = parse_data(data)
            error, error_message = check_errors(next, real_bin) 
            if(next=="Send to Client"):
                if(not new_data_from_array in new_cache):
                    #new_cache[bin(bytearray(old_data))] = bin(bytearray(origdata))
                    new_cache[new_data_from_array] = bytearray(origdata)
                    print "This is an answer! Store it in the cache"
                else:
                    print "Found this before in the cache."
                break
            elif(error):
                break
            else:
                next_ip = ''
                for i in next[0]:
                    next_ip+=i+'.'
                next_ip = next_ip[0:-1]
        if(error):
            to_send = bytearray(origdata)
        elif (new_data_from_array in new_cache):
            #to_send = new_cache[bin(bytearray(origdata))]
            to_send = new_cache[new_data_from_array]
            print "You just added this to the cache"
            #print to_send
            print "update the ttl list for the ones from data_from_array"
            for ttl in ttl_list:
                #print ttl
                if ttl[0] == "**":
                    #ttl[0] = new_data_from_array
                    ttl_dict[new_data_from_array] = ttl
            #ttl_dict[new_data_from_array] = ("**", 15, time.time())
            del ttl_list[:]
            #ttl_dict[new_data_from_array] = ttl_list[
            #byte = bin(real_bin[i])[2:].rjust(8, '0')
        else:
            to_send = bytearray(origdata) 

    new_timer = time.time()
    #print "Timer difference = ", new_timer - timer
    print 'Sending response to: ', str(client_addr)       # Confirm correct client
    serv_sock.sendto(to_send,client_addr)


