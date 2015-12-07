__author__ = 'alex.stuart,lauren.mills,Caleb.Stevenson'
import socket               # Import socket module
import select
import struct 
s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

def checkchecksum(checksum, filedata):
    print "checksum = ", checksum
    print "calc sum = ", calcchecksum(filedata)
    if checksum != str(calcchecksum(filedata)):        
        return False
    else:
        return True
def calcchecksum(filedata):
    sum = 0
    for i in filedata:
        sum += ord(i)
    sum = sum % 1000
    return "%03d"%sum

try:
	host = raw_input("Enter IP address: ")
	#host = socket.gethostname()
	#print host
except Exception:
        print "not an IP address"
        quit()                     # Reserve a port for your service.
#host = socket.gethostname() # Get local machine name

port = raw_input("Enter port number: ")
try:
    port = int(port)
    # throw Exception if port < 0 or > 352983752 whatever
    if(port < 0 or port > 65535):
        raise Exception;
except Exception:
    print "Not a port number"
    quit()                     # Reserve a port for your service.
try:
	s.connect((host, port))
except Exception:
	print "problem connecting to that IP address"
	quit()
while True:
    filename = raw_input("Enter the file's name or press ENTER to exit: ")
    b=bytes(filename)
    if not b:
        print "Exiting"
        quit()
    print 'Sending...'
    fileAckMatch = False
    b += calcchecksum(b)
    s.settimeout(1.0)
    while not fileAckMatch:
        s.send(b)
        try:
            l,a = s.recvfrom(1024)
        except Exception as e:
            continue
        fileAck = l[0:3]
        if not checkchecksum(l[-3:], l[:-3]):
            #do stuff
            print "Checksum's don't match!"
        else:
            print "checksum's match"
            #print l
            print fileAck
            if fileAck == "888":
                print "file ack match"
                fileAckMatch = True
            else:
                print "file ack does not match"
    l,a = s.recvfrom(1024)
    #print l
    if l=="*****":
        print("file not available")
        quit()
    s.settimeout(1.0)
    #print "rec 1st"
    newfile = open(filename, 'ab')
    skip_write = False
    done = False 
    skip_count = 0
    filemorsels = [0] * 256
    lastwrite_awk = 0
    while(not done):
        print skip_write
        if not skip_write:

            if not checkchecksum(l[-3:], l[:-3]):
                #do stuff
                print "Checksum's don't match!"
            else:
                print "checksum's match"
                print len(l[3:])
    
                awk_num = l[0:3]
                print " awk:" + awk_num
                filemorsels[int(awk_num)] = l[3:-3]
                if filemorsels[lastwrite_awk]:
                    i = lastwrite_awk
                    while filemorsels[i]:
                        newfile.write(filemorsels[i])
                        i+= 1 
                    lastwrite_awk = i
                 
                #we need to wait to write all the data
                #newfile.write(l[3:-3])

                s.send(awk_num)
                #print "wrote a chunk"
                #print l
        try:
            if skip_write:
                skip_count += 1
            skip_write = False
            l = False
            l,a = s.recvfrom(1024)
            if l:
                skip_count = 0
            
        except Exception as e: 
            skip_write = True
            print skip_count
            if skip_count > 5:
                l = False
                done = True
    #	newfile.write(l)
    #	print "inexpection"
    #	print l
    #print len(newfile)
            
#f.close()
print "Done Recieving"
#print s.recv(1024)
       
