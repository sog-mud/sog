/*
 * IMC2 version 0.10 - an inter-mud communications protocol
 * Copyright (C) 1996 & 1997 Oliver Jowett <oliver@randomly.org>
 *
 * IMC2 Gold versions 1.00 though 2.00 are developed by MudWorld.
 * Copyright (C) 1999 - 2002 Haslage Net Electronics (Anthony R. Haslage)
 *
 * IMC2 MUD-Net version 3.10 is developed by Alsherok and Crimson Oracles
 * Copyright (C) 2002 Roger Libiez ( Samson )
 * Additional code Copyright (C) 2002 Orion Elder
 * Registered with the United States Copyright Office
 * TX 5-555-584
 *
 * IMC2 Continuum version 4.00 is developed by Rogel
 * Copyright (C) 2003 by Rogel, WhoStyles Copyright (C) 2003 by Kris Craig
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: imc.h,v 1.1.2.3 2003-10-02 07:24:27 matrim Exp $
 */

#ifndef _IMC_H_
#define _IMC_H_

#include "imccfg.h"
#include "typedef.h"

/* This is the protocol version */
#define IMC_VERSION	2
#define IMC_VERSION_ID	"IMC2 4.00 Continuum"

#define IMC_PATH		"imc"
#define IMC_CHANNEL_FILE	"imc2.channels"
#define IMC_CONFIG_FILE		"imc2.config"
#define IMC_BLACKLIST_FILE	"imc2.blacklist"

/*
 * Memory allocation macros.
 */
#define IMCCREATE(result, type, number) do {				\
	(result) = (type *)calloc((number), sizeof(type));		\
	if ((result) == NULL) {						\
		perror("malloc failure");				\
		fprintf(stderr, "Malloc failure @ %s:%d\n",		\
		    __FILE__, __LINE__ );				\
		abort();						\
	}								\
} while(0)

#define IMCRECREATE(result, type, number) do {				\
	(result) = (type *)realloc((result), sizeof(type)*(number));	\
	if ((result) == NULL) {						\
		perror("realloc failure");				\
		fprintf(stderr, "Realloc failure @ %s:%d\n",		\
			__FILE__, __LINE__ );				\
		abort();						\
	}								\
} while(0)

#define IMCDISPOSE(point) do {						\
	if ((point) == NULL) {						\
		imcbug("Freeing null pointer %s:%d",			\
		    __FILE__, __LINE__);				\
		fprintf(stderr, "DISPOSEing NULL in %s, line %d\n",	\
		    __FILE__, __LINE__);				\
	} else {							\
		free((point));						\
		(point) = NULL;						\
	}								\
} while(0)

/*
 * double-linked list handling macros -Thoric (From the Smaug codebase)
 * Updated by Scion 8/6/1999
 */
#define IMCLINK(link, first, last, next, prev) do {			\
	if ((first) == NULL) {						\
		(first) = (link);					\
		(last) = (link);					\
	} else								\
		(last)->next = (link);					\
	(link)->next = NULL;						\
	if (first == link)						\
		(link)->prev = NULL;					\
	else								\
		(link)->prev = (last);					\
	(last) = (link);						\
} while(0)

#define IMCINSERT(link, insert, first, next, prev) do {			\
	(link)->prev = (insert)->prev;					\
	if ((insert)->prev == NULL)					\
		(first) = (link);					\
	else								\
		(insert)->prev->next = (link);				\
	(insert)->prev = (link);					\
	(link)->next = (insert);					\
} while(0)

#define IMCUNLINK(link, first, last, next, prev) do {                   \
	if ((link)->prev == NULL) {					\
		(first) = (link)->next;			                \
		if ((first) != NULL)					\
			(first)->prev = NULL;				\
	} else								\
		(link)->prev->next = (link)->next;			\
	if ((link)->next == NULL) {					\
		(last) = (link)->prev;					\
		if ((last) != NULL)					\
			(last)->next = NULL;				\
	} else								\
		(link)->next->prev = (link)->prev;			\
} while(0)

/* Less tweakable parameters - only change these if you know what they do */

/* number of packets to remember at a time */
#define IMC_MEMORY 256

/* start dropping really old packets based on this figure */
#define IMC_PACKET_LIFETIME 60

/* min input/output buffer size */
#define IMC_MINBUF        256

/* max input/output buffer size */
#define IMC_MAXBUF        16384

/*
 * Changing these defines impacts the protocol itself - other muds may drop your
 * packets if you get this wrong
 */
/* max length of any packet */
#define IMC_PACKET_LENGTH 16300

/* max length of any mud name */
#define IMC_MNAME_LENGTH  20

/* max length of any player name */
#define IMC_PNAME_LENGTH  40

/* max length of any player@mud name */
#define IMC_NAME_LENGTH   (IMC_MNAME_LENGTH+IMC_PNAME_LENGTH+1)

/* max length of a path */
#define IMC_PATH_LENGTH   200

/* max length of a packet type */
#define IMC_TYPE_LENGTH   20

/* max length of a password */
#define IMC_PW_LENGTH     20

/*
 * max length of a data type (estimate)
 * (max length equal to 15,938 by default)
 */
#define IMC_DATA_LENGTH   (IMC_PACKET_LENGTH-2*IMC_NAME_LENGTH	\
			   -IMC_PATH_LENGTH-IMC_TYPE_LENGTH-20)

/* max number of data keys in a packet */
#define IMC_MAX_KEYS      20

/* channel policies */
enum {
	CHAN_OPEN = 1,
	CHAN_CLOSED,
	CHAN_PRIVATE,
	CHAN_COPEN,
	CHAN_CPRIVATE
} channel_policies;

/* connection states */
enum {
	CONN_NONE,
	CONN_SENDCLIENTPWD,
	CONN_WAITSERVERPWD,
	CONN_COMPLETE
} connection_states;

/* Permission Levels */
enum {
	PERM_NONE,
	PERM_PLAYER,
	PERM_IMMORTAL,
	PERM_ADMIN
} permission_levels;

/* Player Flags */
/* For things that are unreasonable */
#define ALLOW_NOTSUPPORTED       (1 << 0)
#define DENY_NOTSUPPORTED        (1 << 1)
#define PSET_NOTSUPPORTED        (1 << 2)
/* Network Function Player Flags */
#define ALLOW_IMCSTATUS          (1 << 3)
#define DENY_IMCSTATUS           (1 << 4)
#define PSET_IMCSTATUS           (1 << 5)
#define ALLOW_IMCCHANFYI         (1 << 6)
#define DENY_IMCCHANFYI          (1 << 6)
#define PSET_IMCCHANFYI          (1 << 7)
#define ALLOW_IMCPVISIBLE        (1 << 8)
#define DENY_IMCPVISIBLE         (1 << 9)
#define PSET_IMCPVISIBLE         (1 << 10)
#define ALLOW_IMCPBEEP           (1 << 11)	/* Not supported at this time */
						/* because of command status */
#define DENY_IMCPBEEP            (1 << 12)
#define PSET_IMCPBEEP            (1 << 13)
#define ALLOW_IMCPTELL           (1 << 14)	/* Not supported at this time */
						/* because of command status */
#define DENY_IMCPTELL            (1 << 15)
#define PSET_IMCPTELL            (1 << 16)
/* Miscellaneous Player Flags */
#define PSET_IMCPAFK             (1 << 17)
#define PSET_IMCPCOLOR           (1 << 18)

/* String Defines - Used to set lengths of numerous string arrays in code */
/* Should be no need to change these. */

/* Large String Size */
#define LSS 4096
/* Medium String Size */
#define MSS 1024
/* Small String Size */
#define SSS 256

/*
 * Macro taken from DOTD codebase. Fcloses a file, then nulls its pointer
 * for safety.
 */
#define SFCLOSE(fp)  fclose(fp); fp=NULL;

/* Number of entries to keep in the channel histories */
#define MAX_IMCHISTORY 20

#define CHECKIMC(ch) do {						\
	if (IS_NPC(ch)) {						\
		imc_to_char("NPCs cannot use IMC.\n\r", (ch));		\
		return;							\
	}								\
	if (!this_imcmud || this_imcmud->state < CONN_COMPLETE) {	\
		imc_to_char("IMC is not active.\n\r", (ch));		\
		return;							\
	}								\
} while(0)

#define CHECKMUD(ch, m) do {						\
	if (strcasecmp((m), imc_name) && !imc_find_reminfo((m),1)) {	\
		char mbuf[MSS];						\
		snprintf(mbuf, MSS, "Warning: '%s' doesn't seem to be "	\
		    "active on IMC.\n\r", (m));				\
		imc_to_char(mbuf, (ch));				\
	}								\
} while(0)

#define CHECKMUDOF(ch, n) CHECKMUD((ch), imc_mudof((n)))

/*
 * No real functional difference in alot of this, but double linked lists DO
 * seem to handle better, and they look alot neater too.
 * Yes, readability IS important! - Samson
 */
typedef struct imc_channel IMC_CHANNEL;	/* Channels, both local and non-local */
typedef struct imc_hubinfo HUBINFO;	/* The given mud :) */
typedef struct imc_packet PACKET;	/* It's a packet! */
typedef struct imc_remoteinfo REMOTEINFO;	/* Information on a mud */
						/* connected to IMC	*/
typedef struct imc_blacklist IMC_BLACKLIST;	/* Player-Level and	*/
						/* Mud-Level Blacklists	*/

extern IMC_CHANNEL *first_imc_channel;
extern IMC_CHANNEL *last_imc_channel;
extern REMOTEINFO *first_rinfo;
extern REMOTEINFO *last_rinfo;
extern IMC_BLACKLIST *first_imc_mudblacklist;
extern IMC_BLACKLIST *last_imc_mudblacklist;

struct imc_blacklist {
	IMC_BLACKLIST	*next;
	IMC_BLACKLIST	*prev;
	const char	*name;
};

struct imcchar_data {
	long    imc_pflags;	/* IMC network function settings	*/
				/* and privilege flags			*/
	const char *imc_csubscribed;	/* IMC subscribed channels */
	const char *rreply;	/* IMC reply-to */
	IMC_BLACKLIST *imcfirst_blacklist;	/* Blacklisted people */
	IMC_BLACKLIST *imclast_blacklist;
};

/* a player on IMC */
typedef struct {
	char    name[IMC_NAME_LENGTH];	/* name of character */
	int     perm_level;	/* trust level */
} imc_char_data;

struct imc_channel {
	IMC_CHANNEL *next;
	IMC_CHANNEL *prev;
	const char *name;	/* name of channel */
	const char *owner;	/* owner (singular) of channel */
	const char *operators;	/* current operators of channel */

	unsigned char policy;

	const char *invited;
	const char *excluded;

	const char *local_name;
	unsigned char perm_level;
	const char *regformat;
	const char *emoteformat;
	const char *socformat;
	const char *history[MAX_IMCHISTORY];
	bool refreshed;
};

/* an IMC packet, as seen by the high-level code */
struct imc_packet {
	char    to[IMC_NAME_LENGTH];	/* destination of packet */
	char    from[IMC_NAME_LENGTH];	/* source of packet      */
	char    type[IMC_TYPE_LENGTH];	/* type of packet        */
	const char *key[IMC_MAX_KEYS];
	const char *value[IMC_MAX_KEYS];

	/* internal things which only the low-level code needs to know about */
	struct {
		char    to[IMC_NAME_LENGTH];
		char    from[IMC_NAME_LENGTH];
		char    path[IMC_PATH_LENGTH];
		unsigned long sequence;
	} i;
};

/* The mud's connection data for the hub */
struct imc_hubinfo {
	const char *hubname;	/* name of hub */
	const char *host;	/* hostname of hub */
	unsigned short port;	/* remote port of hub */
	const char *serverpw;	/* server password */
	const char *clientpw;	/* client password */
	bool    autoconnect;	/* Do we autoconnect on bootup or not? -Samson*/

	/* Conection parameters - These don't save in the config file */
	int     desc;		/* descriptor */
	unsigned short state;	/* IMC_xxxx state */
	short   newoutput;	/* try to write at end of cycle */
				/* regardless of fd_set state?	*/
	char   *inbuf;		/* input buffer */
	int     insize;
	char   *outbuf;		/* output buffer */
	int     outsize;
	bool    in_progress;
};

/* IMC statistics */
typedef struct {
	time_t  start;		/* when statistics started               */

	long    rx_pkts;	/* Received packets                      */
	long    tx_pkts;	/* Transmitted packets                   */
	long    rx_bytes;	/* Received bytes                        */
	long    tx_bytes;	/* Transmitted bytes                     */

	int     max_pkt;	/* Max. size packet processed            */
	int     sequence_drops;	/* Dropped packets due to age            */
} imc_statistics;

/* info about another mud on IMC */
struct imc_remoteinfo {
	REMOTEINFO *next;
	REMOTEINFO *prev;
	const char *name;
	const char *netname;
	const char *version;
	bool    expired;
	const char *path;
	unsigned long top_sequence;
};

/* an event */
typedef struct _imc_event {
	time_t	when;
	void	(*callback) (void *data);
	void	*data;
	int	timed;
	struct	_imc_event *next;
} imc_event;

/* for the versioning table */
typedef struct {
	int     version;
	char   *(*generate) (PACKET *);
	PACKET *(*interpret) (char *);
} _imc_vinfo;

/* an entry in the memory table */
typedef struct {
	const char *from;
	unsigned long sequence;
} _imc_memory;

/* site information */
typedef struct {
	const char *name;	/* FULL name of mud */
	const char *netname;	/* Name of the mud's IMC2 network */
	const char *host;	/* hostname or ip address */
	const char *email;	/* contact address (email) */
	const char *www;	/* homepage */
	const char *details;	/* BRIEF description of mud */
	const char *base;	/* Name of the mud's codebase */
	int     port;		/* The port the mud itself is on */
} imc_siteinfo_struct;

/* command table structure */
typedef struct {
	const char *name;
	void    (*ComFunction) (CHAR_DATA * ch, const char *argument);
	int     permlevel;
} command_struct;

extern command_struct command_table[];

extern bool imc_channel_handler(CHAR_DATA * ch, const char *command,
    const char *argument);

typedef struct {
	const char *mud;
	const char *ansi;
	const char *imc;
} imccolor_struct;

extern imccolor_struct imccolor_table[];

extern imc_siteinfo_struct imc_siteinfo;

/* the packet memory table */
extern _imc_memory imc_memory[IMC_MEMORY];

/* the version lookup table */
extern _imc_vinfo imc_vinfo[];

/* global stats struct */
extern imc_statistics imc_stats;

/* the local IMC name */
extern const char *imc_name;

/* the event list, and recycle list */
extern imc_event *imc_event_list, *imc_event_free;

/* the current time */
extern time_t imc_now;

/* next sequence number to use */
extern unsigned long imc_sequencenumber;

/* Version ID plus the base name from IMC Statistics */
extern const char *imc_versionid;

/* External functions hooked to the mud */
bool    imc_command_hook(CHAR_DATA * ch, const char *command,
    const char *argument);
void    imc_startup(bool forced);
void    imc_shutdown(bool reconnect);
void    imc_initchar(CHAR_DATA * ch);
bool    imc_loadchar(CHAR_DATA * ch, FILE * fp, const char *word);
void    imc_savechar(CHAR_DATA * ch, FILE * fp);
void    imc_freechardata(CHAR_DATA * ch);
void    imc_loop(void);
extern int imc_minplayerlevel;
extern int imc_minimmlevel;
extern int imc_minadminlevel;

/* Internal functions */
void    imc_initchannels(void);
void    do_imcsend(char *line);
void    imc_send(PACKET * p);
char *	color_mtoi(const char *);
char *	color_itom(const char *);

extern unsigned char imc_whostyle; /* Sets the color pattern for the imcplist */

#endif
