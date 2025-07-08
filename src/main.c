#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>

#include "checksum.h"
#include "rkudp.h"

// Pseudo header needed for UDP checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <src_ip> <src_port> <dst_ip> <dst_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source_ip = argv[1];
    int source_port = atoi(argv[2]);
    const char *dest_ip = argv[3];
    int dest_port = atoi(argv[4]);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char packet[4096];
    memset(packet, 0, sizeof(packet));

    // IP header pointer
    struct iphdr *ip_header = (struct iphdr *)packet;

    // UDP header pointer
    struct udphdr *udp_header = (struct udphdr *)(packet + sizeof(struct iphdr));

    // Data payload
    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    const char *msg = "Raw UDP Packet Test";
    int data_len = strlen(msg);
    memcpy(data, msg, data_len);

    // Fill in IP header
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
    ip_header->id = htons(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    ip_header->saddr = inet_addr(source_ip);
    ip_header->daddr = inet_addr(dest_ip);

    ip_header->check = checksum((unsigned short *)ip_header, sizeof(struct iphdr));

    // Fill in UDP header
    udp_header->source = htons(source_port);
    udp_header->dest = htons(dest_port);
    udp_header->len = htons(sizeof(struct udphdr) + data_len);
    udp_header->check = 0;

    // Pseudo header for checksum
    struct pseudo_header psh;
    psh.source_address = inet_addr(source_ip);
    psh.dest_address = inet_addr(dest_ip);
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + data_len);

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + data_len;
    char *pseudogram = malloc(psize);

    memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udp_header, sizeof(struct udphdr) + data_len);

    udp_header->check = checksum((unsigned short *)pseudogram, psize);
    free(pseudogram);

    // Destination address struct
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = udp_header->dest;
    dest_addr.sin_addr.s_addr = ip_header->daddr;

    // Send the packet
    if (sendto(sockfd, packet, ntohs(ip_header->tot_len), 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Packet sent successfully");

    close(sockfd);
    return 0;
}

