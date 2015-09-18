__author__ = 'alex.stuart'
import socket               # Import socket module
import thread

def clientconnection(c, addr):
    print "we here"
    while True:
        #recu=ive 1024 bytes for the filename
        l = c.recv(1024)
        print 'where is l!'
        print l
        if not l:
            break
        #decode the raw byte using UTF8 
        filename = l.decode("utf-8")
        if filename == 'exit':
           print 'wanto to exit' 
     
        print filename
        
        #open the file request 
        #TODO Error checking
        try:
            file = open(filename,'r')
        
        #read the file from disk in 1024 byte chunks 
            l = file.read(1024)
        except Exception:
            #print "404 mutha fucka"
            c.send("404 motha fucka")
        l = False
        file = False
        #print l 
        #print "read"
        #while bytes to send
        while(l):
            c.send(l)
        #print "sent chunk"
        l = file.read(1024)
        #print "read another chunk"
        #print l
        #read the next bytes
        if file:
        #print file
            file.close()
        print "closed connection"
            #file.write(l)
            #l = c.recv(1024)
        #file.close()
        print "finished sending file"

serv_sock = socket.socket()         # Create a socket object
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
serv_sock.bind((this_machine_name, port))        # Bind to the port
   # Open a temp file to store the data
serv_sock.listen(1)                 # Now wait for client connection.

while True:
    try:
        #returns a socket and the adress we are connected to
        c, addr = serv_sock.accept()     # Establish connection with client.
        print 'connected to', addr       # Confirm correct client
        print c
        thread.start_new_thread(clientconnection, (c, addr))
    except Exception as e:
        print e
        print "Could not start thread"

c.close()
