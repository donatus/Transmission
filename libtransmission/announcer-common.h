/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2. Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: announcer-common.h 12238 2011-03-26 12:06:04Z jordan $
 */

#ifndef __LIBTRANSMISSION_ANNOUNCER_MODULE___
 #error only the libtransmission announcer module should #include this header.
#endif

#ifndef _TR_ANNOUNCER_COMMON_H_
#define _TR_ANNOUNCER_COMMON_H_

#include "transmission.h" /* SHA_DIGEST_LENGTH */
#include "session.h" /* PEER_ID_LEN */

/***
****  SCRAPE
***/

enum
{
  /* pick a number small enough for common tracker software:
   *  - ocelot has no upper bound
   *  - opentracker has an upper bound of 64
   *  - udp protocol has an upper bound of 74
   *  - xbtt has no upper bound */
  TR_MULTISCRAPE_MAX = 64
};

typedef struct
{
    /* the scrape URL */
    char * url;

    /* the name to use when deep logging is enabled */
    char log_name[128];

    /* info hashes of the torrents to scrape */
    uint8_t info_hash[TR_MULTISCRAPE_MAX][SHA_DIGEST_LENGTH];

    /* how many hashes to use in the info_hash field */
    int info_hash_count;
}
tr_scrape_request;

struct tr_scrape_response_row
{
    /* the torrent's info_hash */
    uint8_t info_hash[SHA_DIGEST_LENGTH];

    /* how many peers are seeding this torrent */
    int seeders;

    /* how many peers are downloading this torrent */
    int leechers;

    /* how many times this torrent has been downloaded */
    int downloads;

    /* the number of active downloaders in the swarm.
     * this is a BEP 21 extension that some trackers won't support.
     * http://www.bittorrent.org/beps/bep_0021.html#tracker-scrapes  */
    int downloaders;
};

typedef struct
{
    /* whether or not we managed to connect to the tracker */
    bool did_connect;

    /* whether or not the scrape timed out */
    bool did_timeout;

    /* how many info hashes are in the 'rows' field */
    int row_count;

    /* the individual torrents' scrape results */
    struct tr_scrape_response_row rows[TR_MULTISCRAPE_MAX];

    /* the raw scrape url */
    char * url;

    /* human-readable error string on failure, or NULL */
    char * errmsg;

    /* minimum interval (in seconds) allowed between scrapes.
     * this is an unofficial extension that some trackers won't support. */
    int min_request_interval;
}
tr_scrape_response;

typedef void tr_scrape_response_func( const tr_scrape_response  * response,
                                      void                      * user_data );

void tr_tracker_http_scrape( tr_session               * session,
                             const tr_scrape_request  * req,
                             tr_scrape_response_func    response_func,
                             void                     * user_data );

void tr_tracker_udp_scrape( tr_session               * session,
                            const tr_scrape_request  * req,
                            tr_scrape_response_func    response_func,
                            void                     * user_data );

/***
 ****  FILE REPLICATION
 ***/

enum
{
    /* pick a number small enough for common tracker software:
     *  - ocelot has no upper bound
     *  - opentracker has an upper bound of 64
     *  - udp protocol has an upper bound of 74
     *  - xbtt has no upper bound */
    TR_FILEREPLICATION_REQUEST_MAX = 64
};

typedef struct
{
    /* the scrape URL */
    char * url;
    
    /* the name to use when deep logging is enabled */
    char log_name[128];
    
    /* size avaiable for new download */
    int avaiable_space;
}
tr_filereplicate_request;


typedef struct
{
    /* the current torrent tiers*/
    struct tr_torrent_tiers* torrent_tier;
    
    /* whether or not we managed to connect to the tracker */
    bool                did_connect;
    
    /* whether or not the filereplicate timed out */
    bool                did_timeout;
    
    /* the raw scrape url */
    char *              url;
    
    /* human-readable error string on failure, or NULL */
    char *              errmsg;
    
    /* minimum interval (in seconds) allowed between scrapes.
     * this is an unofficial extension that some trackers won't support. */
    int                 min_request_interval;
    
    /* the torrent's info_hash to download*/
    uint8_t             info_hash[SHA_DIGEST_LENGTH];
    
    /* Torrent info */
    uint32_t            pieceSize;
    tr_piece_index_t    pieceCount;
    uint64_t*           pieceToDownload;
    int                 pieceToDownloadSize;
    
    /* total size of the torrent, in bytes */
    uint64_t            totalSize;
    
    /* pieces raw sha */
    char*      piecesRawSign;
}tr_filereplicate_response;

typedef void tr_filereplicate_response_func( const tr_scrape_response  * response,
                                     void                      * user_data );

void tr_tracker_http_filereplicate( tr_session               * session,
                            const tr_filereplicate_request  * req,
                            tr_filereplicate_response_func    response_func,
                            void                     * user_data );

void tr_tracker_udp_filereplicate( tr_session               * session,
                           const tr_filereplicate_request  * req,
                           tr_filereplicate_response_func    response_func,
                           void                     * user_data );

/***
****  ANNOUNCE
***/

typedef enum
{
    TR_ANNOUNCE_EVENT_NONE,
    TR_ANNOUNCE_EVENT_COMPLETED,
    TR_ANNOUNCE_EVENT_STARTED,
    TR_ANNOUNCE_EVENT_STOPPED
}
tr_announce_event;

const char * tr_announce_event_get_string( tr_announce_event );

typedef struct
{
    tr_announce_event event;
    bool partial_seed;

    /* the port we listen for incoming peers on */
    int port;

    /* per-session key */
    int key;

    /* the number of peers we'd like to get back in the response */
    int numwant;

    /* the number of bytes we uploaded since the last 'started' event */
    uint64_t up;

    /* the number of good bytes we downloaded since the last 'started' event */
    uint64_t down;

    /* the number of bad bytes we downloaded since the last 'started' event */
    uint64_t corrupt;

    /* the total size of the torrent minus the number of bytes completed */
    uint64_t left;

    /* the tracker's announce URL */
    char * url;

    /* key generated by and returned from an http tracker.
     * see tr_announce_response.tracker_id_str */
    char * tracker_id_str;

    /* the torrent's peer id.
     * this changes when a torrent is stopped -> restarted. */
    char peer_id[PEER_ID_LEN];

    /* the torrent's info_hash */
    uint8_t info_hash[SHA_DIGEST_LENGTH];

    /* the name to use when deep logging is enabled */
    char log_name[128];
    
    /* ADDED fro File Replication */
    
    /* Torrent info */
    uint32_t            pieceSize;
    tr_piece_index_t    pieceCount;
    uint64_t            totalSize;
    
    /* pieces raw sha */
    char*      piecesRawSign;
    
}
tr_announce_request;

struct tr_pex;

typedef struct
{
    /* the torrent's info hash */
    uint8_t info_hash[SHA_DIGEST_LENGTH];

    /* whether or not we managed to connect to the tracker */
    bool did_connect;

    /* whether or not the scrape timed out */
    bool did_timeout;

    /* preferred interval between announces.
     * transmission treats this as the interval for periodic announces */
    int interval;

    /* minimum interval between announces. (optional)
     * transmission treats this as the min interval for manual announces */
    int min_interval;

    /* how many peers are seeding this torrent */
    int seeders;

    /* how many peers are downloading this torrent */
    int leechers;

    /* how many times this torrent has been downloaded */
    int downloads;

    /* number of items in the 'pex' field */
    size_t pex_count;

    /* IPv4 peers that we acquired from the tracker */
    struct tr_pex * pex;

    /* number of items in the 'pex6' field */
    size_t pex6_count;

    /* IPv6 peers that we acquired from the tracker */
    struct tr_pex * pex6;

    /* human-readable error string on failure, or NULL */
    char * errmsg;

    /* human-readable warning string or NULL */
    char * warning;

    /* key generated by and returned from an http tracker.
     * if this is provided, subsequent http announces must include this. */
    char * tracker_id_str;
}
tr_announce_response;

typedef void tr_announce_response_func( const tr_announce_response * response,
                                        void                       * userdata );

void tr_tracker_http_announce( tr_session                 * session,
                               const tr_announce_request  * req,
                               tr_announce_response_func    response_func,
                               void                       * user_data );

void tr_tracker_udp_announce( tr_session                 * session,
                              const tr_announce_request  * req,
                              tr_announce_response_func    response_func,
                              void                       * user_data );

void tr_tracker_udp_start_shutdown( tr_session * session );

#endif /* _TR_ANNOUNCER_COMMON_H_ */
