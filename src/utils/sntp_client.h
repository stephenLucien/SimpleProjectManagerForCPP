#ifndef __SNTP_CLIENT_H__
#define __SNTP_CLIENT_H__

/**
 * @file sntp_client.h
 * @author LuckyKing (11510416@mail.sustech.edu.cn)
 * @brief ONLY work on Little-Endian, and element fields / bit fields are allocated from low-order to high-order
 * @version 0.1
 * @date 2025-01-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "cstring_proc.h"

#define SNTP_CLIENT_CHECK_TIMELAPSE  1
#define SNTP_CLIENT_MAX_TIMELAPSE_MS (3000)

#if defined(SNTP_CLIENT_CHECK_TIMELAPSE) && SNTP_CLIENT_CHECK_TIMELAPSE != 0
    #include "utils/os_tools_time.h"
#endif

/**
 * @brief In conformance with standard Internet practice,
 * NTP data are specified as integer or fixed-point quantities,
 * with bits numbered in big-endian fashion from 0 starting
 * at the left or most significant end. (bit0 is most significant bit)
 *
 * Unless specified otherwise, all quantities are unsigned and may occupy the full field width with an
 * implied 0 preceding bit 0.
 *
 * Because NTP timestamps are cherished data and, in fact, represent the
 * main product of the protocol, a special timestamp format has been
 * established.
 *
 * NTP timestamps are represented as a 64-bit unsigned
 * fixed-point number, in seconds relative to 0h on 1 January 1900.
 *
 * The integer part is in the first 32 bits, and the fraction part in the
 * last 32 bits.
 *
 * In the fraction part, the non-significant low-order
 * bits are not specified and are ordinarily set to 0
 *
 * UINT32MAX ~= \frac{2^{32}}{60\cdot60\cdot24\cdot365} = 136.1925195 years
 *
 */
typedef union __attribute__((packed))
{
    uint64_t ntp_ts;
    struct
    {
        uint32_t fraction;
        uint32_t integer;
    } part;
} NtpTimestamp;

//
#define NTP_TIMESTAMP_UNIX_DELTA (2'208'988'800)


static uint64_t ntp_ts_to_unix_time(NtpTimestamp ntp_ts, uint64_t reference_epoch_time)
{
    uint64_t reference_ntp_seconds = reference_epoch_time + NTP_TIMESTAMP_UNIX_DELTA;
    // only overflow bits were taken from reference_epoch_time
    uint64_t epoch_time_high32bits = reference_ntp_seconds & 0xFF'FF'FF'FF'00'00'00'00;
    // low 32bits were taken from ntp timestamp
    uint64_t epoch_time_low32bits = ((uint64_t)ntp_ts.part.integer + UINT32_MAX + 1 - NTP_TIMESTAMP_UNIX_DELTA) & 0x00'00'00'00'FF'FF'FF'FF;
    //
    return epoch_time_high32bits | epoch_time_low32bits;
}

/**
 * @brief
 * Leap Indicator (LI): This is a two-bit code warning of an impending
 * leap second to be inserted/deleted in the last minute of the current
 * day.  This field is significant only in server messages, where the
 * values are defined as follows:
 * - LI       Meaning
 * - ---------------------------------------------
 * - 0        no warning
 * - 1        last minute has 61 seconds
 * - 2        last minute has 59 seconds
 * - 3        alarm condition (clock not synchronized)
 *
 *
 * Version Number (VN): This is a three-bit integer indicating the
 * NTP/SNTP version number, currently 4.  If necessary to distinguish
 * between IPv4, IPv6, and OSI, the encapsulating context must be
 * inspected.
 * This is a three-bit integer indicating the NTP/SNTP version number, currently 4.
 * - SNTPv4 : \ref https://datatracker.ietf.org/doc/html/rfc4330
 * - NTPv4 : \ref https://datatracker.ietf.org/doc/html/rfc5905
 *
 *
 * Mode: This is a three-bit number indicating the protocol mode.  The values are defined as follows:
 * -  Mode     Meaning
 * -  ------------------------------------
 * -  0        reserved
 * -  1        symmetric active
 * -  2        symmetric passive
 * -  3        client
 * -  4        server
 * -  5        broadcast
 * -  6        reserved for NTP control message
 * -  7        reserved for private use
 *
 *  In unicast and manycast modes, the client sets this field to 3
 *  (client) in the request, and the server sets it to 4 (server) in the
 *  reply.  In broadcast mode, the server sets this field to 5
 *  (broadcast).  The other modes are not used by SNTP servers and
 *  clients.
 */
typedef struct __attribute__((packed))
{
    uint8_t Mode : 3;
    uint8_t VN   : 3;
    uint8_t LI   : 2;
} NtpLiVnMode;

/**
 * @brief
 * ------------------ CLIENT SIDE ------------
 *     Field Name               Unicast/Manycast            Broadcast
 *                              Request     Reply
 *     ---------------------------------------------------------------
 *     LI                       0           0-3            0-3
 *
 *     VN                       1-4         copied from    1-4
 *                                          request
 *
 *     Mode                     3           4              5
 *
 *     Stratum                  0           0-15           0-15
 *
 *     Poll                     0           ignore         ignore
 *
 *     Precision                0           ignore         ignore
 *
 *     Root Delay               0           ignore         ignore
 *
 *     Root Dispersion          0           ignore         ignore
 *
 *     Reference Identifier     0           ignore         ignore
 *
 *     Reference Timestamp      0           ignore         ignore
 *
 *     Originate Timestamp      0           (see text)     ignore
 *
 *     Receive Timestamp        0           (see text)     ignore
 *
 *     Transmit Timestamp       (see text)  nonzero        nonzero
 *
 *
 *
 * ------------------ SERVER SIDE ------------
 *  Field Name             Unicast/Manycast             Broadcast
 *                         Request     Reply
 *  ----------------------------------------------------------------
 *  LI                     ignore      as needed       as needed
 *
 *  VN                     1-4         copied from     4
 *                                     request
 *
 *  Mode                   3           4               5
 *
 *  Stratum                ignore      1               1
 *
 *  Poll                   ignore      copied from     log2 poll
 *                                     request         interval
 *
 *  Precision              ignore      -log2 server    -log2 server
 *                                     significant     significant
 *                                     bits            bits
 *
 *  Root Delay             ignore      0               0
 *
 *  Root Dispersion        ignore      0               0
 *
 *  Reference Identifier   ignore      source ident    source ident
 *
 *  Reference Timestamp    ignore      time of last    time of last
 *                                     source update   source update
 *
 *  Originate Timestamp    ignore      copied from     0
 *                                     transmit
 *                                     timestamp
 *
 *  Receive Timestamp      ignore      time of day     0
 *
 *  Transmit Timestamp     (see text)  time of day     time of day
 *
 *  Authenticator          optional    optional        optional
 *
 * @note From above table, we know that SNTP server only
 * cares about fields: VN / Mode / Transmit Timestamp
 *
 */
typedef struct __attribute__((packed))
{
    NtpLiVnMode leap_version_mode;
    /**
     * @brief Stratum: This is an eight-bit unsigned integer indicating the stratum.
     * This field is significant only in SNTP server messages, where the values
     * are defined as follows:
     * -  Stratum  Meaning
     * -  ----------------------------------------------
     * -  0        kiss-o'-death message (see below)
     * -  1        primary reference (e.g., synchronized by radio clock)
     * -  2-15     secondary reference (synchronized by NTP or SNTP)
     * -  16-255   reserved
     *
     */
    uint8_t Stratum;
    /**
     * @brief Poll Interval: This is an eight-bit unsigned integer used as an
     * exponent of two, where the resulting value is the maximum interval
     * between successive messages in seconds.  This field is significant
     * only in SNTP server messages, where the values range from 4 (16 s) to
     * 17 (131,072 s -- about 36 h).
     *
     */
    uint8_t Poll;
    /**
     * @brief Precision: This is an eight-bit signed integer used as an exponent of
     * two, where the resulting value is the precision of the system clock
     * in seconds.  This field is significant only in server messages, where
     * the values range from -6 for mains-frequency clocks to -20 for
     * microsecond clocks found in some workstations
     *
     */
    uint8_t Precision;
    /**
     * @brief  Root Delay: This is a 32-bit signed fixed-point number indicating the
     *  total roundtrip delay to the primary reference source, in seconds
     *  with the fraction point between bits 15 and 16.  Note that this
     *  variable can take on both positive and negative values, depending on
     *  the relative time and frequency offsets.  This field is significant
     *  only in server messages, where the values range from negative values
     *  of a few milliseconds to positive values of several hundred
     *  milliseconds.
     *
     */
    uint32_t RootDelay;
    /**
     * @brief Root Dispersion: This is a 32-bit unsigned fixed-point number
     * indicating the maximum error due to the clock frequency tolerance, in
     * seconds with the fraction point between bits 15 and 16.  This field
     * is significant only in server messages, where the values range from
     * zero to several hundred milliseconds.
     *
     */
    uint32_t RootDispersion;

    /**
     * @brief Reference Identifier: This is a 32-bit bitstring identifying the
     * particular reference source.
     *
     */
    uint32_t ReferenceID;

    /**
     * @brief Reference Timestamp: This field is the time the system clock was last
     * set or corrected, in 64-bit timestamp format.
     * If the server is synchronized, the Reference Timestamp is set to the
     * time the last update was received from the reference source.
     *
     */
    NtpTimestamp ReferenceTimestamp;

    /**
     * @brief Originate Timestamp: This is the time at which the request departed
     * the client for the server, in 64-bit timestamp format.
     *
     */
    NtpTimestamp OriginateTimestamp;

    /**
     * @brief Receive Timestamp: This is the time at which the request arrived at
     *  the server or the reply arrived at the client, in 64-bit timestamp
     *  format.
     *
     */
    NtpTimestamp ReceiveTimestamp;

    /**
     * @brief Transmit Timestamp: This is the time at which the request departed
     *  the client or the reply departed the server, in 64-bit timestamp
     *  format.
     *
     */
    NtpTimestamp TransmitTimestamp;

} NtpMinPacket;

static inline NtpMinPacket ntp_min_packet_revert_byteorder(NtpMinPacket pkt)
{
#define NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(pobj, field) revert_byte_array((void*)&((pobj)->field), sizeof((pobj)->field))
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, leap_version_mode);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, Stratum);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, Poll);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, Precision);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, RootDelay);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, RootDispersion);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, ReferenceID);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, ReferenceTimestamp);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, OriginateTimestamp);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, ReceiveTimestamp);
    NTP_PACKET_REVERT_SINGLE_FIELD_BYTE_ORDER(&pkt, TransmitTimestamp);
    return pkt;
}

static inline NtpMinPacket ntp_min_packet_gen_for_client()
{
    NtpMinPacket pkt;
    //
    memset(&pkt, 0, sizeof(pkt));
    //
    pkt.leap_version_mode.VN   = 4;
    pkt.leap_version_mode.Mode = 3;

#if defined(SNTP_CLIENT_CHECK_TIMELAPSE) && SNTP_CLIENT_CHECK_TIMELAPSE != 0
    /**
     * @brief The server copies this field to the Originate Timestamp in the reply, so
     * feel free to set this data field to any kind of data.
     *
     * here, we set it to MONOTONIC clock in local system, so timelapse could be estimated.
     *
     */
    pkt.TransmitTimestamp = {.ntp_ts = os_get_timestamp_ms()};
#endif
    //
    return pkt;
}

static inline int ntp_min_packet_to_client_netdata(NtpMinPacket pkt, uint8_t* buf, size_t bufsz)
{
    //
    if (!buf || bufsz < sizeof(pkt))
    {
        return -1;
    }
    //

    pkt = ntp_min_packet_revert_byteorder(pkt);
    memcpy(buf, &pkt, sizeof(pkt));

    //
    return (int)sizeof(pkt);
}


static inline NtpMinPacket ntp_min_packet_from_server_netdata(const uint8_t* netdata, size_t datalen)
{
    NtpMinPacket pkt;
    //
    memset(&pkt, 0, sizeof(pkt));
    if (!netdata || datalen < sizeof(pkt))
    {
        return pkt;
    }
    memcpy(&pkt, netdata, sizeof(pkt));
    //
    pkt = ntp_min_packet_revert_byteorder(pkt);

    return pkt;
}


static inline int ntp_min_packet_from_server_is_valid(NtpMinPacket pkt)
{
    if (!(pkt.leap_version_mode.LI >= 0 && pkt.leap_version_mode.LI <= 3))
    {
        return 0;
    }
    if (!(pkt.leap_version_mode.VN >= 1 && pkt.leap_version_mode.VN <= 4))
    {
        return 0;
    }
    if (!(pkt.leap_version_mode.Mode == 4 || pkt.leap_version_mode.Mode == 5))
    {
        return 0;
    }
    if (!(pkt.Stratum >= 0 && pkt.Stratum <= 15))
    {
        return 0;
    }
    if (pkt.TransmitTimestamp.ntp_ts == 0)
    {
        return 0;
    }
    return 1;
}

static inline int ntp_min_packet_from_server_is_good(NtpMinPacket pkt)
{
    if (!ntp_min_packet_from_server_is_valid(pkt))
    {
        return 0;
    }
    if (pkt.leap_version_mode.LI == 3)
    {
        // server clock not sync
        return 0;
    }
    if (pkt.Stratum == 0)
    {
        // server informs client to stop request!
        return 0;
    }

#if defined(SNTP_CLIENT_CHECK_TIMELAPSE) && SNTP_CLIENT_CHECK_TIMELAPSE != 0
    //
    uint64_t timelapse_ms = os_get_timelapse_ms(pkt.OriginateTimestamp.ntp_ts);
    if (timelapse_ms > SNTP_CLIENT_MAX_TIMELAPSE_MS)
    {
        return 0;
    }
    return (int)timelapse_ms;
#endif
    return 1;
}


#endif  // __SNTP_CLIENT_H__
