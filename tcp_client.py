__author__ = 'alex.stuart'
import socket               # Import socket module
import select
s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 12345                 # Reserve a port for your service.

s.connect((host, port))
filename = raw_input("Enter the file's name: ")
b=bytes(filename)
print 'Sending...'
s.send(b)
l = s.recv(1024)

s.setblocking(0)
print "rec 1st"
newfile = open(filename, 'ab')
while(l):
    
    
    newfile.write(l)
    print "wrote a chunk"
    print l
    try:
        l = s.recv(1024)
    except Exception: 
        l = False
	newfile.write(l)
	pass
#f.close()
print "Done Recieving"
#print s.recv(1024)
s.close        
