__author__ = 'alex.stuart'
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
except Exception:
        print "Not a port number"
        quit()                     # Reserve a port for your service.
try:
	s.connect((host, port))
except Exception:
	print "problem connecting to that IP address"
	quit()
filename = raw_input("Enter the file's name: ")
b=bytes(filename)
print 'Sending...'
s.send(b)
l = s.recv(1024)
if l=="404 motha fucka":
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
s.close        
