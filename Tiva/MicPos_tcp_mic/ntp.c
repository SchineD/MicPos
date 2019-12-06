/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

 /* NDK Header files */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"

#include "ntp.h"

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define NTPSERVER "147.125.80.35"

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0


void error(char* msg)
{
    perror(msg); // Print the error message to stderr.

    Task_exit(); // Quit the process.
}

void ntp()
{
  int n;
  int errornr = 0;
  SOCKET sockfd;
  int portno = 123; // NTP UDP port number.
  char* host_name = "at.pool.ntp.org"; // NTP server host-name.

  // Structure that defines the 48 byte NTP packet protocol.
  typedef struct
  {

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

  } ntp_packet;              // Total: 384 bits or 48 bytes.


  // Create and zero out the packet. All 48 bytes worth.
  ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  memset( &packet, 0, sizeof( ntp_packet ) );

  // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
  *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.
  System_printf("packet: %s\n", (char*)&packet);
  System_flush();

  // Create a UDP socket, convert the host-name to an IP address, set the port number,
  // connect to the server, send the packet, and then read in the return packet.

  struct sockaddr_in serv_addr; // Server address data structure.^
  char* server[512];// Server data structure.

  memset((char *)&serv_addr, 0, sizeof(serv_addr));

  sockfd = NDK_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.
  System_printf("udp-socket: %d\n", (int)sockfd);
  System_flush();

  if (sockfd < 0)
    error( "ERROR opening socket" );
/*
  DNSGetHostByName(host_name, &server, sizeof(server)); // Convert URL to IP.
  System_printf("URL to IP - Server: %s\n", (char*)server);
  System_flush();
*/

  if(inet_aton(NTPSERVER, &serv_addr.sin_addr.s_addr) == 0) {
          System_printf("error: inet_aton\n");//htonl(INADDR_ANY);
          System_flush();
      }

/*
  // Zero out the server address structure.
  bzero((char*) &serv_addr, sizeof(serv_addr));

  // Copy the server's IP address to the server address structure.
  memmove((char*)&serv_addr.sin_addr.s_addr, (char*)server, sizeof(*server));
  System_printf("NTP-Server: %s\n", (int)serv_addr.sin_addr.s_addr);
  System_flush();
*/

  // Convert the port number integer to network big-endian style and save it to the server address structure.
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_family = AF_INET;
  System_printf("NTP-Server: %d\n", serv_addr.sin_addr.s_addr);
  System_flush();

/*
  // Call up the server using its IP address and port number.
  if (NDK_connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error( "ERROR connecting" );
*/

  // Send it the NTP packet it wants. If n == -1, it failed.
  // n = NDK_send(sockfd,(char*)&packet, sizeof(ntp_packet),0);
  n = NDK_sendto(sockfd,(char*)&packet, sizeof(ntp_packet),0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  System_printf("n: %d\n", n);
    System_flush();
  if (n < 0){
      errornr = fdError();
  System_printf("error: %d\n", errornr);
  System_flush();
    error( "ERROR writing to socket" );
  }

  // Wait and receive the packet back from the server. If n == -1, it failed.
  // n = NDK_recv(sockfd,(char*)&packet, sizeof(packet), MSG_WAITALL);
    int size = sizeof(serv_addr);
    n = NDK_recvfrom(sockfd,(char*)&packet, sizeof(ntp_packet), 0,(struct sockaddr *)&serv_addr, &size);
    System_printf("error: %d\n", errornr);
    System_flush();
    if (n < 0)
        if (n < 0){
              errornr = fdError();
          System_printf("error: %d\n", errornr);
          System_flush();
            error( "ERROR writing to socket" );
          }

  // These two fields contain the time-stamp seconds as the packet left the NTP server.
  // The number of seconds correspond to the seconds passed since 1900.
  // ntohl() converts the bit/byte order from the network's to host's "endianness".

  packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
  packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.

  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
  // Subtract 70 years worth of seconds from the seconds since 1900.
  // This leaves the seconds since the UNIX epoch of 1970.
  // (1900)------------------(1970)**************************************(Time Packet Left the Server)

  time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

  // Print the time we got from the server, accounting for local timezone and conversion from UTC time.

  System_printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );
  System_flush();

}

