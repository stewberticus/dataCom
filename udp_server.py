__author__ = 'alex.stuart,Lauren.mills,Caleb Stevenson'
import socket               # Import socket module
import thread

# c is now serv_sock, addr is going to be the address of client
def clientconnection(c, addr):
    while True:
        #receive 1024 bytes for the filename
        #l = c.recv(1024)
        print "Trying to connect"
        l, addr = c.recvfrom(1024)
        print 'Connected to', addr       # Confirm correct client
        if not l:
            break
        #decode the raw byte using UTF8 
        filename = l.decode("utf-8")
        print filename
        awktidbits = []
        filemorsels = []
        c.settimeout(1.0)

        #open the file request 
        try:
            file = open(filename,'r')
            
        #read the file from disk in 1024 byte chunks 
            l = file.read(1022)
            ll = chr(0)
            l += ll
            l += ll 
            filemorsels.append(l)
        except Exception:
            #something went wrong with the file need to tell the client 
            c.sendto("*****", addr)
            l = False
            file = False
        #while bytes to send
        i = 1
        startwindow = 0
        endwindow = 5
        while(l):
            while(l and i < endwindow):
                c.sendto(l, addr)
                #read next 1024
                l = file.read(1022)
                if(l): 
                    print type(l)
                    ll = chr(i)
                    l += ll
                    l += ll
                    filemorsels[i] = l
                print "i", i
                i+=1
                if(i == 256):
                    i = 0
            while(1):
                try:
                    data, addr = c.recvfrom(l,addr)

                except Exception:
                    print "timeout!"
                 
        if file:
            file.close()
        
        print "finished sending file"
    print 'Client ' + str(addr) + ' is no longer connected.'

serv_sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# Create a socket object
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
#serv_sock.listen(1)                 # Now wait for client connection.

while True:
    try:
        #returns a socket and the adress we are connected to
        #c, addr = serv_sock.accept()    # Establish connection with client.
        #c, addr = serv_sock.recvfrom(1024)
        #c,addr = serv_sock.recvfrom(1024)
        #print 'Connected to', addr       # Confirm correct client
        #thread.start_new_thread(clientconnection, (c, addr))
        addr = "n/a"
        print "before connection"
        #thread.start_new_thread(clientconnection, (serv_sock, addr))
        clientconnection(serv_sock, addr)
        #thread.start_new_thread(clientconnection, (serv_sock, serv_sock))
    except Exception as e:
        print e
        print "Could not start thread"

c.close()
