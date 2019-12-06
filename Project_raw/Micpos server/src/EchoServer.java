import java.io.BufferedReader;
import java.util.ArrayList;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.net.*;


/**
 * EchoServer was built for an embedded systems project for ESE at Technikum Wien.
 * It listens on port 4444 and receives TCP-packets from its clients. 
 * The data it receives can be echoed back, hence "EchoServer". 
 * The main purpose for this server though is to receive timestamps from 3 TivaBoards with max4466 microphones
 * and calculate which timestamp has the highest value (meaning it's the earliest timestamp recorded).
 * It then opens a webpage in the default browser and sends the corresponding parameter in the URL.
 *
 * @author Daniel Schinewitz
 * @version 1.2
 * 
 */
public class EchoServer {
	private ServerSocket server = null;
	public volatile String[] clientValues = new String[3];
	public volatile ArrayList<Socket> clientSockets = new ArrayList<Socket>();
	public volatile int counter = 0;
	
	/**
	 * EchoServer() opens a socket for the server to listen on. 
	 *
	 * @param  host IP-Address of the server
	 * @param  port port of the server
	 * @param  backlogConnectionQueueLength how many connections from clients can be queued 
	 * @throws IOException IO caused an Exception
	 */
	public EchoServer(String host, int port, int backlogConnectionQueueLength) throws IOException {
		server = new ServerSocket(port, backlogConnectionQueueLength, InetAddress.getByName(host));
		System.out.println(Thread.currentThread() + " Created Server");
	}
	
	/**
	 * start() starts the Method that handels clients
	 */
	public void start() {
		System.out.println(Thread.currentThread() + " Server Ready: " + server);
		while (true) {
			acceptAndHandleClient(server);
		}
	}

	/**
	 * acceptAndHandleClient accepts new client-connections and forks a new thread for each to handle them.
	 *
	 * @param  server the server socket
	 */
	private void acceptAndHandleClient(ServerSocket server) {
		System.out.println(Thread.currentThread() + " Waiting for Incoming connections...");
		while(true) {
		try {
			Socket clientSocket = server.accept();
			System.out.println(clientSocket);
			Thread t = new ClientHandler(clientSocket);
			t.start();
		} catch (IOException e) {
			System.out.println("Error: new thread");
			e.printStackTrace();
		}
		}
	}

	/**
	 * ClientHandler handles the thread for each client-connection
	 *
	 * @author Daniel Schinewitz
	 * @version 1.2
	 * 
	 */	
	class ClientHandler extends Thread{
	public Socket clientSocket;
	
	/**
	 * Constructor for ClientHandler-class
	 * 
	 * @param clientSocket the client's socket
	 */
	public ClientHandler(Socket clientSocket)  
    { 
        this.clientSocket = clientSocket;
    } 
	
	/**
	 * run() reads to client's datastream and stores those values according to its ID (msg: "'id' 'value'").
	 * Once 3 clients are registered in clientSockets, run() sends a start-msg to clients so that they start synchronized.
	 * run() waits for all the clients to send their values and then calls calculate().
	 * 
	 */
  public void run(){
		try {
	  System.out.println(Thread.currentThread() + " Received Connection from " + clientSocket);
		System.out.println(clientSocket);
		BufferedReader is = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
		PrintStream os = new PrintStream(clientSocket.getOutputStream());

		os.println("12go\0");                                      // after testing put this under while-loop at 91
				
		clientSockets.add(clientSocket);
		

		while(clientSockets.size() != 2) {        
		}
		/*
		try{
			Thread.sleep(5000);
		} catch(Exception e) {
			System.out.println(e);
		}

		if(clientSockets.size() == 1) {   // only for testing ( change to 3 ! )
			sendMulticastMessage();
		}
		*/
		
		// echo that data back to the client, except for QUIT.
		String line = null;
		while ((line = is.readLine()) != null) {
			line = line.trim();
			System.out.println(Thread.currentThread() + " Server Got => " + line); 
			if((line.charAt(0) == 'A') | (line.charAt(1) == 'A'))
			{
				System.out.println(" Tiva1");
				String value = line.substring(2, line.length());
				System.out.println(value);
				clientValues[0] = value;
				counter++;
			}
			else if((line.charAt(0) == 'B') | (line.charAt(1) == 'B'))
			{
				System.out.println(" Tiva2");
				String value = line.substring(2, line.length());
				System.out.println(value);
				clientValues[1] = value;
				counter++;
			}
			else if((line.charAt(0) == 'C') | (line.charAt(1) == 'C'))
			{
				System.out.println(" Tiva3");
				String value = line.substring(2, line.length());
				System.out.println(value);
				clientValues[2] = value;
				counter++;
			}
		
			System.out.println("check for arrayList length");

			while(counter%2 != 0) {}            // for testing only ( change to 3 )
			
			calculatePosition(); 
			
			counter = 0;
		}
		
		
		System.out.println(Thread.currentThread() + " Server Closing Connection by Sending => Ok");
		os.println("Ok");
		os.flush();
		is.close();
		os.close();
		} catch(Exception e) {
			System.out.println(e);
		}
  }

/**
 * calculatePosition() calculates the client's timestamps and opens a website in the default browser with
 * the according parameter in the URL.
 */
	public void calculatePosition() {
		System.out.println("calculate");
		int tiva1 = Integer.parseInt(clientValues[0]);
		int tiva2 = Integer.parseInt(clientValues[1]);
	//	int tiva3 = Integer.parseInt(clientValues[2]);
		
		int result = 0;
		
		// only for testing
	//int	tiva1 = 79999;
	//int	tiva2 = 77777;
	int	tiva3 = 78888;
		
		if(tiva1 < tiva2 && tiva1 < tiva3) {
			System.out.println("tiva1");
			result = 1;
		}
		else if(tiva2 < tiva1 && tiva2 < tiva3) {
			System.out.println("tiva2");
			result = 2;
		}
		else if(tiva3 < tiva1 && tiva3 < tiva2) {
			System.out.println("tiva3");
			result = 3;
		}
		else {
			return;
		}
		String request = "http://ieproject.dx.am/micpos.html?input=";
		request = request.concat(Integer.toString(result));
		sendRequest(request);
	}
	
	/**
	 * sendRequest opens the default Browser to display the output of calculatePosition().
	 * 
	 * @param request the URL containing the output of calculatePosition()
	 */
	void sendRequest(String request)
	{
		try {
			if(java.awt.Desktop.isDesktopSupported() ) {
		        java.awt.Desktop desktop = java.awt.Desktop.getDesktop();

		        if(desktop.isSupported(java.awt.Desktop.Action.BROWSE) ) {
		          java.net.URI uri = new java.net.URI(request);
		              desktop.browse(uri);
		        }
		      } 
		
	} catch (Exception e) {
		System.out.println(e);
	}
	}
}
/*		
	public static void sendMulticastMessage() {

		    try {
		    	InetAddress multicastAddress = InetAddress.getByName("238.255.255.240"); 
		        final int port = 5000; 
		        MulticastSocket socket = new MulticastSocket(port); 
		        socket.setReuseAddress(true);
		        socket.setSoTimeout(15000);
		        socket.joinGroup(multicastAddress);
		        
		        byte[] txbuf = "test1".getBytes("UTF-8");
		        DatagramPacket hi = new DatagramPacket(txbuf, txbuf.length,
		            multicastAddress, port);
		        socket.send(hi);
		        System.out.println("Multicast sent");
		        socket.close();
		    }
		    catch(Exception e) {
		    	System.out.println(e);
		    }
	}

	public void close() throws IOException {
		server.close();
	}
	*/	

/**
 * main method to start the server
 * 
 * @param args usually empty
 */
	public static void main(String[] args) {
		// sendMulticastMessage();
		try {
			EchoServer server = new EchoServer("192.168.1.103", 4444, 50);
			server.start();
		} catch (Exception e) {
			System.out.println(e);
		}
	}
}