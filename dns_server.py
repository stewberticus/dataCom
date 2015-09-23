__author__ = 'alex.stuart'
import socket               # Import socket module
import binascii

serv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)         # Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip

port = raw_input("Enter port number: ")
try:
	port = int(port)
except Exception:
	print "not a number"
	quit()                # Reserve the passed port for your service.
serv_sock.bind((this_machine_name, port))        # Bind to the port
   # Open a temp file to store the data
#serv_sock.listen(1)                 # Now wait for client connection.
while True:
#returns a socket and the adress we are connected to
    data, addr = serv_sock.recvfrom(1024)     # Establish connection with client.

    print 'connected to', str(addr)       # Confirm correct client
    print 'typeof: ' + str(type(data))
    binary =  binascii.hexlify(data)
    i = 0
    print len(binary)

    print binary
    i = 0
    while i < len(binary):
	
	if i/4 == 0:
	    print 'ID is:' 
	
	if i/4 == 1:
	    print 'OP code bit things'
		
	if i/4 == 2:
	    print 'DQcount'
	if i/4 == 3:
	    print 'AN Count' 
	
	if i/4 == 4:
	    print 'NS count'
		
	if i/4 == 5:
	    print 'AR count'


        print(binary[i:i+4])
	i = i + 4
c.close()
