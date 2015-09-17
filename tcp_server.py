__author__ = 'alex.stuart'
import socket               # Import socket module


serv_sock = socket.socket()         # Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip

port = raw_input("Enter port number: ")
try:
	port = int(port)
except Exception:
	print "not a number"
	quit()                # Reserve the passed port for your service.
serv_sock.bind((this_machine_name, port))        # Bind to the port
   # Open a temp file to store the data
serv_sock.listen(1)                 # Now wait for client connection.
while True:
#returns a socket and the adress we are connected to
    c, addr = serv_sock.accept()     # Establish connection with client.
    print 'connected to', addr       # Confirm correct client
#recu=ive 1024 bytes for the filename
    l = c.recv(1024)
#decode the raw byte using UTF8 
    filename = l.decode("utf-8")
 
    print filename
	
#open the file request 
#TODO Error checking
    try:
        file = open(filename,'r')
    
	
#reda the file from disk in 1024 byte chunks 
        l = file.read(1024)
    except Exception:
        print "404 mutha fucka"
        c.send("404 motha fucka")
	l = False
	file = False
    print l 
    print "read"
#while bytes to send
    while(l):
        c.send(l)
	print "sent chunk"
	l = file.read(1024)
	print "read another chunk"
	print l
    #read the next bytes
    if file:
	print file
        file.close()
    print "closed"
        #file.write(l)
        #l = c.recv(1024)
    #file.close()
print "finished"
c.close()
