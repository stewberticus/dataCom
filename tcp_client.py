__author__ = 'alex.stuart,lauren.mills'
import socket               # Import socket module
import select
s = socket.socket()         # Create a socket object

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
    s.send(b)
    l = s.recv(1024)
    if l=="*****":
        print("file not available")
        quit()
    s.settimeout(1.0)
    #print "rec 1st"
    newfile = open(filename, 'ab')
    while(l):
        
        
        newfile.write(l)
        #print "wrote a chunk"
        #print l
        try:
            l = s.recv(1024)
        except Exception: 
           
    #	newfile.write(l)
    #	print "inexpection"
    #	print l
            l = False
            pass
#f.close()
print "Done Recieving"
#print s.recv(1024)
       
