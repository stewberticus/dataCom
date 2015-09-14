__author__ = 'alex.stuart'
import socket               # Import socket module

s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 12345                 # Reserve a port for your service.

s.connect((host, port))
filename = raw_input("Enter the file's name: ")
b=bytes(filename)
print 'Sending...'
s.send(b)
l = s.recv(1024)
print "rec 1st"
while(l):
    
    newfile = open(filename, 'ab')
    newfile.write(l)
    print "wrote a chunk"
    l = s.recv(1024)
    print l

#f.close()
print "Done Sending"
#print s.recv(1024)
s.close        
