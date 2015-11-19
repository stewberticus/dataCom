__author__ = 'alex.stuart,Lauren.mills,Caleb Stevenson'
import socket               # Import socket module
import thread


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

while True:
    try:
        data, addr = sock.recvfrom(1024)
        
     
        #decode the raw byte using UTF8 
        filename = data.decode("utf-8")
        print filename
        
        #open the file request 
        try:
            file = open(filename,'r')
        
        #read the file from disk in 1024 byte chunks 
            l = file.read(1024)
        except Exception:
            #something went wrong with the file need to tell the client 
            c.send("*****")
            l = False
            file = False
        #while bytes to send
        while(l):
            c.send(l)
            #read next 1024
            l = file.read(1024)
        if file:
            file.close()
        #returns a socket and the adress we are connected to
        #print 'Connected to', addr       # Confirm correct client
    except Exception as e:
        print e
        print "Could not start thread"

c.close()
