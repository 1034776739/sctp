#include "sctp.h"

sctp::sctp()
{
    Connected.store(0);

    if((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1)
        throw "Client socket create error";


    //enable some notifications
    if(enable_notifications() != 0)
        throw "Error enabling notifications.";


    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(server_ip);
    local_addr.sin_port = htons(sctp_port);
    int bind_errno = bind(client_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (bind_errno != 0)
        throw strerror(bind_errno);



    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(sctp_port);
    if(inet_pton(AF_INET, server_ip, &(peer_addr.sin_addr)) != 1)
        throw "Error converting IP address to sockaddr_in structure";



    if(connect(client_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == -1)
        throw "Error to Connect";


    printf("Connected\n");

    char buf[1024];
    while(1) {
        if(recv(client_fd, &buf, sizeof(buf), 0) == -1) {
            perror("recv");
        }

        printf("%s\n", buf);
    }


}

sctp::~sctp()
{
    if (client_fd!=0)
      close(client_fd);
    if (server_fd!=0)
      close(server_fd);
}

bool sctp::isReady()
{
    return Connected.load();
}


int sctp::get_reply()
{
    struct sockaddr_in dest_addr;

    char payload[1024];
    int buffer_len = sizeof(payload) - 1;
    memset(&payload, 0, sizeof(payload));

    struct iovec io_buf;
    memset(&payload, 0, sizeof(payload));
    io_buf.iov_base = payload;
    io_buf.iov_len = buffer_len;

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_iov = &io_buf;
    msg.msg_iovlen = 1;
    msg.msg_name = &dest_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);

    while(1) {
        int recv_size = 0;
        if((recv_size = recvmsg(server_fd, &msg, 0)) == -1)
        {
            printf("recvmsg() error\n");
            return 1;
        }

        if(msg.msg_flags & MSG_NOTIFICATION)
        {
            if(!(msg.msg_flags & MSG_EOR))
            {
                printf("Notification received, but the buffer is not big enough.\n");
                continue;
            }

            handle_notification((union sctp_notification*)payload, recv_size);
        }
        else if(msg.msg_flags & MSG_EOR)
        {
            printf("%s\n", payload);
            break;
        }
        else
        {
            printf("%s", payload); //if EOR flag is not set, the buffer is not big enough for the whole message
        }
    }

    return 0;
}



int sctp::enable_notifications()
{
    struct sctp_event_subscribe events_subscr;
    memset(&events_subscr, 0, sizeof(events_subscr));
    events_subscr.sctp_association_event = 1;
    events_subscr.sctp_shutdown_event = 1;
    return setsockopt(client_fd, IPPROTO_SCTP, SCTP_EVENTS, &events_subscr, sizeof(events_subscr));
}

int sctp::handle_notification(union sctp_notification *notif, size_t notif_len)
{
    // http://stackoverflow.com/questions/20679070/how-does-one-determine-the-size-of-an-unnamed-struct
    int notif_header_size = sizeof( ((union sctp_notification*)NULL)->sn_header );

    if(notif_header_size > notif_len) {
        printf("Error: Notification msg size is smaller than notification header size!\n");
        return 1;
    }

    switch(notif->sn_header.sn_type) {
    case SCTP_ASSOC_CHANGE: {
        if(sizeof(struct sctp_assoc_change) > notif_len) {
            printf("Error notification msg size is smaller than struct sctp_assoc_change size\n");
            return 2;
        }

        char* state = NULL;
        struct sctp_assoc_change* n = &notif->sn_assoc_change;

        switch(n->sac_state) {
        case SCTP_COMM_UP:
            state = "COMM UP";
            break;

        case SCTP_COMM_LOST:
            state = "COMM_LOST";
            break;

        case SCTP_RESTART:
            state = "RESTART";
            break;

        case SCTP_SHUTDOWN_COMP:
            state = "SHUTDOWN_COMP";
            break;

        case SCTP_CANT_STR_ASSOC:
            state = "CAN'T START ASSOC";
            break;
        }

        printf("SCTP_ASSOC_CHANGE notif: state: %s, error code: %d, out streams: %d, in streams: %d, assoc id: %d\n",
               state, n->sac_error, n->sac_outbound_streams, n->sac_inbound_streams, n->sac_assoc_id);

        break;
    }

    case SCTP_SHUTDOWN_EVENT: {
        if(sizeof(struct sctp_shutdown_event) > notif_len) {
            printf("Error notification msg size is smaller than struct sctp_assoc_change size\n");
            return 3;
        }

        struct sctp_shutdown_event* n = &notif->sn_shutdown_event;

        printf("SCTP_SHUTDOWN_EVENT notif: assoc id: %d\n", n->sse_assoc_id);
        break;
    }

    default:
        printf("Unhandled notification type %d\n", notif->sn_header.sn_type);
        break;
    }

    return 0;
}
