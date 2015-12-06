__author__ = 'alex.stuart,Lauren.mills,Caleb Stevenson'
import socket               # Import socket module
import thread
import struct

# c is now serv_sock, addr is going to be the address of client
def clientconnection(c):
    while True:
        #receive 1024 bytes for the filename
        #l = c.recv(1024)
        print "Trying to connect"
        l, addr = c.recvfrom(1024)
        print 'Connected to', addr       # Confirm correct client
        if not l:
            break
        #decode the raw byte using UTF8 
        filename = l.decode("utf-8")
        print "file:" + filename
        awktidbits = [0] * 256
        filemorsels = [0] * 256
        c.settimeout(1.0)

        #open the file request 
        try:
            file = open(filename,'r')
            
        #read the file from disk in 1024 byte chunks 
            l = file.read(1021)
            ll = "000"
            #print "awk num:" + ll
            
            ll += l 
            #print "whole packet:" + ll
            filemorsels.append(l)
        except Exception as e:
	    print e
            #something went wrong with the file need to tell the client 
            c.sendto("*****", addr)
            l = False
            file = False
        #while bytes to send
        i = 1
        startwindow = 0
        endwindow = 5
        no_timeout = True
        reset = False
        while(ll):
	    no_timeout = True
            if not l and reset:
                break
            while(l and i < endwindow):
                c.sendto(ll, addr)
                #read next 1024
                l = file.read(1021)
                if(ll): 
                    #print type(l)"%02d"%a
                    ll = "%03d"%i
                    #print "chr of i " + ll
                    ll += l
                    filemorsels[i] = l
                print "i", i
                i+=1
                if(i == 256):
                    i = 0
            while(no_timeout):
                try:
                    print "listening"
                    data, addr = c.recvfrom(1024)
                    print data
                    resp_num = int(data)
                    awktidbits[resp_num] = True
                    for g in range(5):
                        if awktidbits[startwindow + g]:
                            newstartwindow = startwindow+g
                        else:
                            break
                    startwindow = newstartwindow
                    endwindow = startwindow + 5
                    print startwindow
                    print endwindow
                    print "i - 2 is ", i -2
                    if i - 2 == startwindow:
                        print "i is ", i
                        reset = True
                        break

                except Exception:
                    print "timeout!"
                    no_timeout = False
                 
        if file:
            file.close()
        
        print "finished sending file"
        break
    print 'Client ' + str(addr) + ' is no longer connected.'

serv_sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip
#print this_machine_name

port = raw_input("Enter port number: ")
try:
    port = int(port)
    if(port < 0 or port > 65535):
        raise Exception
except Exception:
	print "not a valid port number"
	quit()                # Reserve the passed port for your service.
serv_sock.bind(("", port))        # Bind to the port
   # Open a temp file to store the data
#serv_sock.listen(1)                 # Now wait for client connection.


clientconnection(serv_sock)
 

#c.close()
