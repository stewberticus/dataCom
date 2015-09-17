import java.io.*;
import java.net.*;
class tcpclient{
	public static viod main (String args[]) throws Exception{
	Socket ClientSocket = new Socket("127.0.0.1",9876);
	DataOutputStream outToServer = new DataOutputStream(clientSocket.getOutputStream());
	BufferedReader inFromServer = new BufferedReader(
				new InputStreamReader(clientsocket.getInputStream()));
	BufferedReader inFromUser = 
		new BufferedReader(new InputStreamReader(System.in));
	System.out.println("ENTER:");
	String message = inFromUSer.readLine();
	outToServer.writeBytes(message+"\n");
	System.out.println("got from server






	}
}
