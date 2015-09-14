__author__ = 'alex.stuart'
import socket               # Import socket module

serv_sock = socket.socket()         # Create a socket object
this_machine_name = socket.gethostname() # Get local machine ip
port = 12345                 # Reserve the passed port for your service.
serv_sock.bind((this_machine_name, port))        # Bind to the port
   # Open a temp file to store the data
serv_sock.listen()                 # Now wait for client connection.
while True:
    c, addr = serv_sock.accept()     # Establish connection with client.
    print 'connected to', addr       # Confirm correct client
    l = c.recv(1024)
    filename = l.decode("utf-8") 
    print filename
    #file = open('/tmp/tempfile','wb') 
    #while (l):
        #file.write(l)
        #l = c.recv(1024)
    #file.close()
    print "finished"
    c.close()
