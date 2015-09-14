__author__ = 'alex.stuart'
import socket               # Import socket module

s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 12345                 # Reserve a port for your service.

s.connect((host, port))
filename = input("Enter the file's name: ")
b=bytes(filename,'utf')
print 'Sending...'
b.send(b)
#while(True):
    
f.close()
print "Done Sending"
print s.recv(1024)
s.close        
