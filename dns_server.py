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
    #print 'typeof: ' + str(type(data))
    binary =  binascii.hexlify(data)
    #print data
    i = 0
    #print len(binary)

    #print binary
    i = 0
    while i < len(binary):
	
	if i/4 == 0:
	    print 'ID is:' 
	
	if i/4 == 1:
		#print 'OP code bit things'
		string = format(int(binary[i:i+4]),'016b')
		print "response  :"+ string[0]
		print "OP code :" + string[1:5]
		print "Auth Answer: "+ string[5]	
		print "Trun Resp: " +string[6]
		print "Recusion Desired: "+ string[7]
		print "Recursion Available: " + string[8]
		print "Z (unused): " +string[9:12]
		print "R Code: " +string[12:16]
	if i/4 == 2:
	    print 'DQcount'
	if i/4 == 3:
	    print 'AN Count' 
	
	if i/4 == 4:
	    print 'NS count'
		
	if i/4 == 5:
	    print 'AR count'
	
	if i/4 == 6:
	    print 'Questions Start'
	    words = []
	    while True:
		num_octets = int(binary[i:i+2],16)
		i = i + 2
		if num_octets == 0:
		    break
		word = ''		  
		for j in range(num_octets):

		    this_char = binary[i:i+2]
		    word = word + chr(int(this_char,16))
		    i = i + 2
		print word
		words.append(word)
	    print str(words)
		
        print(binary[i:i+4])
	i = i + 4
c.close()
