__author__ = 'alex.stuart,Lauren.mills,Caleb Stevenson'
import socket               # Import socket module
import thread
import struct


def checkchecksum(checksum, filedata):
    if checksum != str(calcchecksum(filedata)):            
        return False
    else:
        return True
def calcchecksum(filedata):
    sum = 0 
    for i in filedata:
        sum += ord(i)
    sum = sum % 1000
    print "sum  %03d"%sum
    return "%03d"%sum 


# c is , addr is going to be the address of client
def clientconnection(c):
    while True:
        #receive 1024 bytes for the filename
        #l = c.recv(1024)
        print "Trying to connect"
        fileAckMatch = False
        while not fileAckMatch:
            l, addr = c.recvfrom(1024)
            print l
            print l[-3:]
            print l[:-3]
            if not checkchecksum(l[-3:], l[:-3]):
                print "checksum does not match"
                continue
            else:
                print "checksum does match"
                print "Sending ack for filename"
                fileAck = "888"
                fileAck += calcchecksum(fileAck)
                print "set fileAckMatch to true"
                fileAckMatch = True
                c.sendto(fileAck, addr) 
        print 'Connected to', addr       # Confirm correct client
        if not l:
            break
        #decode the raw byte using UTF8 
        filename = l[:-3].decode("utf-8")
        print "file:" + filename
        awktidbits = [0] * 256
        filemorsels = [0] * 256
        c.settimeout(1.0)

        #open the file request 
        try:
            file = open(filename,'r')
            
        #read the file from disk in 1024 byte chunks 
            l = file.read(1018)
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
        # l is the data from the file
        # ll is the ack number + the file data
        # lll is the acknumber + filedata + checksum
        while(ll):
	    no_timeout = True
            if not l and reset:
                break
            checksum = calcchecksum(ll)
            lll = ll + str(checksum)
            while(l and i < endwindow):
                c.sendto(lll, addr)
                if(filemorsels[i]):
                    l = filemorsels[i]    
                else:
                    #read next 1024
                    l = file.read(1018)
                #print type(l)"%02d"%a
                ll = "%03d"%i
                print "i is ", ll
                #print "chr of i " + ll
                ll += l
                filemorsels[i] = l
                print "i", i
                i+=1
                if(i == 256):
                    i = 0
                checksum = calcchecksum(ll)
                lll = ll + checksum
            while(no_timeout):
                try:
                    print "Listening for Acks"
                    data, addr = c.recvfrom(1024)
                    print data
                    awkchecksum = checkchecksum(data[-3:], data[:-3])
                    print "awkchecksum = ", awkchecksum
                    if awkchecksum:
                        #print "if awkchecksum"
                        resp_num = int(data[:-3])
                        awktidbits[resp_num] = True

                    #print "after if awkchecksum"
                    for g in range(5):
                        #print "g is ", g
                        #print "startwindow is ", startwindow
                        #print "awktidbits[startwindow + g]"
                        #print "\t", awktidbits[startwindow + g]
                        if awktidbits[startwindow + g]:
                            newstartwindow = startwindow+g
                        else:
                            break
                    startwindow = newstartwindow
                    i = startwindow
                    endwindow = startwindow + 5
                    print startwindow
                    print endwindow
                    print "i - 2 is ", i -2
                    #if i - 2 == startwindow:
                    #    print "i is ", i
                    #    reset = True
                    #    break


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
