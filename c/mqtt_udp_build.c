/**
 *
 * MQTT/UDP project
 *
 * https://github.com/dzavalishin/mqtt_udp
 * Copyright (C) 2017-2018 Dmitry Zavalishin, dz@dz.ru
 *
 *
 * Generalized MQTT/UDP packet builder
 *
**/

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>

#include "mqtt_udp.h"

static int pack_len( char *buf, int *blen, int *used, int data_len );

//static size_t mqtt_udp_decode_size( char **pkt );
//static size_t mqtt_udp_decode_topic_len( const unsigned char *pkt );


// -----------------------------------------------------------------------
// Clear
// -----------------------------------------------------------------------

void mqtt_udp_clear_pkt( struct mqtt_udp_pkt *p )
{
    memset( p, 0, sizeof(struct mqtt_udp_pkt) );
}

// -----------------------------------------------------------------------
// Build
// -----------------------------------------------------------------------



//#define MQTT_UDP_PKT_HAS_ID(pkt)  ((pkt.pflags) & 0x6)


// sanity check
//#define MAX_SZ (4*1024)
//#define CHEWED (pkt - pstart)

//#define MQTT_UDP_PKT_HAS_ID(pkt)  ((pkt->pflags) & 0x6)

// out_len - length of build packet
int mqtt_udp_build_any_pkt( char *buf, size_t blen, struct mqtt_udp_pkt *p, size_t *out_len )
{
    // TODO check for consistency - if pkt has to have topic & value and has it

    int tlen = p->topic ? p->topic_len : 0;
    int dlen = p->value ? p->value_len : 0;

    unsigned char *bp = buf;

    if( out_len ) *out_len = 0;

    *bp++ = (p->ptype & 0xF0) | (p->pflags & 0x0F);
    blen--;

    // TODO incorrect
    int total = tlen + dlen + 2 + 2; // packet size
    //if( MQTT_UDP_PKT_HAS_ID(p) ) total += 2;
    if(MQTT_UDP_FLAGS_HAS_ID(p->pflags)) total += 2;

    if( total > blen )
        return ENOMEM;

    //int size = total+1;

    int used = 0;
    int rc = pack_len( bp, &blen, &used, total );
    if( rc ) return rc;

    bp += used;

    //if( MQTT_UDP_PKT_HAS_ID(p) )
    if(MQTT_UDP_FLAGS_HAS_ID(p->pflags))
    {
        *bp++ = (p->pkt_id >> 8) & 0xFF;
        *bp++ = p->pkt_id & 0xFF;
        blen -= 2;
    }

    if( tlen )
    {
        *bp++ = (tlen >>8) & 0xFF;
        *bp++ = tlen & 0xFF;
        blen -= 2;

        const char *topic = p->topic;
        //NB! Must be UTF-8
        while( tlen-- > 0 )
        {
            if( blen <= 0 ) return ENOMEM;
            *bp++ = *topic++;
            blen--;
        }

        const char *data = p->value;
        while( dlen-- > 0 )
        {
            if( blen <= 0 ) return ENOMEM;
            *bp++ = *data++;
            blen--;
        }

    }

    if( out_len ) *out_len = bp - (unsigned char *)buf;

    return 0;
}










// -----------------------------------------------------------------------
// Bits
// -----------------------------------------------------------------------



static int pack_len( char *buf, int *blen, int *used, int data_len )
{
    *used = 0;
    while( 1 )
    {
        if( *blen <= 0 ) return ENOMEM;

        int byte = data_len % 128;
        data_len /= 128;

        if( data_len > 0 )
            byte |= 0x80;

        *buf++ = byte;
        (*blen)--;
        (*used)++;

        if( data_len == 0 ) return 0;
    }
}




