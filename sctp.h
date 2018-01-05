#ifndef SCTP_H
#define SCTP_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>		//for close()
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>	//for inet_ntop()
#include <atomic>
#include <errno.h>
#include <stdexcept>


class sctp
{
public:
    sctp();
    ~sctp();
    bool isReady();
private:
    int client_fd = 0;
    int server_fd = 0;
    const char* server_ip = "185.46.88.2";
    int sctp_port = 14015;
    std::atomic_uint_fast8_t Connected;
    int get_reply();
    int enable_notifications();
    int handle_notification(union sctp_notification *notif, size_t notif_len);
};

#endif // SCTP_H
