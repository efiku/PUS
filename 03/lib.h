/*
 * Data:            2009-02-27
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 */

/* RFC 1071
        Depending upon the machine, it may be more efficient to defer
        adding end-around carries until the main summation loop is
        finished.

        One approach is to sum 16-bit words in a 32-bit accumulator, so
        the overflows build up in the high-order 16 bits.  This approach
        typically avoids a carry-sensing instruction but requires twice
        as many additions as would adding 32-bit segments; which is
        faster depends upon the detailed hardware architecture.
*/
unsigned short internet_checksum(unsigned short *addr, int count) {

    register int sum = 0;

    while (count > 1)   {
        sum += *addr++;
        count -= 2;
    }

    if (count > 0)
        sum += *(unsigned char *)addr;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return (unsigned short)(~sum);
}

/*
    alignment requirement: 4
*/
struct icmphdr {
    unsigned char     type;
    unsigned char     code;
    unsigned short    checksum;

    union {

        struct {
            unsigned short    id;
            unsigned short    sequence;
        } echo; /* echo datagram */

        unsigned int        gateway;    /* gateway address */

        struct {
            unsigned short    __unused;
            unsigned short    mtu;
        } frag; /* path mtu discovery */

    } un;
};

#define ICMP_ECHOREPLY      0       /* Echo Reply               */
#define ICMP_ECHO           8       /* Echo Request             */

/*
Structure members are stored sequentially in the order in which they are declared:
the first member has the lowest memory address and the last member the highest.

Bit fields are allocated within an integer from least-significant to most-significant bit.

For structures, unions, and arrays, the alignment-requirement is the largest alignment-requirement of its members.
Every object is allocated an offset so that

offset %  alignment-requirement == 0

Adjacent bit fields are packed into the same 1-, 2-, or 4-byte allocation unit if the integral types are the same size
and if the next bit field fits into the current allocation unit without crossing the boundary imposed by
the common alignment requirements of the bit fields.

__declspec(align(#)):
Without __declspec(align(#)), Visual C++ aligns data on natural boundaries based on the size of the data,
for example 4-byte integers on 4-byte boundaries and 8-byte doubles on 8-byte boundaries.
Data in classes or structures is aligned within the class or structure at the minimum of its natural alignment
and the current packing setting (from #pragma pack or the /Zp compiler option).
(...)
Thus, the offset of an object is based on the offset of the previous object and the current packing setting,
unless the object has a __declspec(align( # )) attribute, in which case the alignment
is based on the offset of the previous object and the __declspec(align( # )) value for the object.

Microsoft C/C++ allows bitfield structure members to be of any integral type.
Ale chcemy kompilowaæ kod jako C - z flag¹ /TC. Zatem u¿ywamy tylko unsigned int dla bitfields.
W przeciwnym wypadku: "warning C4214: nonstandard extension used : bit field types other than int".
Alternatywa:

Zamieniæ:
unsigned int    ip_hl:4;
unsigned int    ip_v:4;

na:

unsigned char   ip_v_hl;
ip_v_hl =   4<<4 | 6;

The sizeof value for any structure is the offset of the final member, plus that member's size,
rounded up to the nearest multiple of the largest member alignment value or the whole structure alignment value,
whichever is greater.

alignment requirement: 4
*/
struct ip {
    unsigned int    ip_hl:4;        /* header length */
    unsigned int    ip_v:4;         /* header version */
    unsigned int    ip_tos:8;       /* type of service */
    unsigned int    ip_len:16;      /* total length */
    unsigned short  ip_id;          /* identification */
    unsigned short  ip_off;         /* fragment offset field */
#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    unsigned char   ip_ttl;         /* time to live */
    unsigned char   ip_p;           /* protocol */
    unsigned short  ip_sum;         /* checksum */
    struct in_addr  ip_src, ip_dst; /* source and dest address */
};

/*
    alignment requirement: 4
*/
struct phdr {
    struct in_addr ip_src, ip_dst;
    unsigned char unused;
    unsigned char protocol;
    unsigned short length;
};

/*
alignment requirement: 4
*/
struct ipv6 {
    unsigned int    ip_v_tc_flow;   /* header version, traffic class, flow label */
    unsigned short  ip_pl;          /* payload length */
    unsigned char   ip_next;        /* next header */
    unsigned char   ip_hop;         /* hop limit */
    struct in6_addr ip_src, ip_dst; /* source and dest address */
};


/*
    alignment requirement: 4
*/
struct phdrv6 {
    struct in6_addr ip_src, ip_dst; /* source and dest address */
    unsigned int length;
    unsigned char unused[3];
    unsigned char next;
};


/*
    alignment requirement: 2
*/
struct udphdr {
    unsigned short uh_sport;          /* source port */
    unsigned short uh_dport;          /* destination port */
    unsigned short uh_ulen;           /* udp length */
    unsigned short uh_sum;            /* udp checksum */
};

/*
    alignment requirement: 4
*/
struct tcphdr {
    unsigned short  th_sport;       /* source port */
    unsigned short  th_dport;       /* destination port */
    unsigned int    th_seq;         /* sequence number */
    unsigned int    th_ack;         /* acknowledgement number */
    unsigned int    th_x2:4;        /* (unused) */
    unsigned int    th_off:4;       /* data offset */
    unsigned int    th_flags:8;
#  define TH_FIN    0x01
#  define TH_SYN    0x02
#  define TH_RST    0x04
#  define TH_PUSH   0x08
#  define TH_ACK    0x10
#  define TH_URG    0x20
    unsigned int    th_win:16;      /* window */
    unsigned short  th_sum;         /* checksum */
    unsigned short  th_urp;         /* urgent pointer */
};

/*
    alignment requirement: 2
*/
struct etherhdr {
    unsigned char     ether_dhost[6]; /* destination eth addr */
    unsigned char     ether_shost[6]; /* source ether addr    */
    unsigned short    ether_type;     /* packet type ID field */
};