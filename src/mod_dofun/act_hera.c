/*
 * $Id: act_hera.c,v 1.11 1998-05-20 11:18:15 efdi Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *	
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  This file is a combination of:                                         *
 *		(H)unt.c, (E)nter.c, (R)epair.c and (A)uction.c            *
 *  Thus it is called ACT_HERA.C                                           *
 **************************************************************************/
 
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * $Id: act_hera.c,v 1.11 1998-05-20 11:18:15 efdi Exp $
 */
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "db.h"
#include "comm.h"
#include "interp.h"
#include "resource.h"

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( number_range( 0, 65535 ) );
        if ( room != NULL )
        if ( can_see_room(ch,room)
	&&   !room_is_private(room)
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE) 
	&&   (IS_NPC(ch) || IS_SET(ch->act,ACT_AGGRESSIVE) 
	||   !IS_SET(room->room_flags,ROOM_LAW | ROOM_SAFE)))
            break;
    }

    return room;
}

/* RT Enter portals */
void do_enter( CHAR_DATA *ch, char *argument)
{    
    ROOM_INDEX_DATA *location; 

    if ( ch->fighting != NULL ) 
	return;

    /* nifty portal stuff */
    if (argument[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next, *mount;

        old_room = ch->in_room;

	portal = get_obj_list( ch, argument,  ch->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_PORTAL 
        ||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,ANGEL)))
	{
	    send_to_char("You can't seem to find a way in.\n\r",ch);
	    return;
	}

	if (!IS_TRUSTED(ch,ANGEL) && !IS_SET(portal->value[2],GATE_NOCURSE)
	&&  (IS_AFFECTED(ch,AFF_CURSE) 
	||   IS_SET(old_room->room_flags,ROOM_NO_RECALL) 
	||   IS_RAFFECTED(old_room,AFF_ROOM_CURSE) ))
	{
	    send_to_char("Something prevents you from leaving...\n\r",ch);
	    return;
	}

	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
	{
	    location = get_random_room(ch);
	    portal->value[3] = location->vnum; /* keeps record */
	}
	else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
	    location = get_random_room(ch);
	else
	    location = get_room_index(portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	   act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR);
	   return;
	}

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags, ROOM_LAW | ROOM_SAFE))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

	act_printf(ch, portal, NULL, TO_ROOM, POS_RESTING,
		   MOUNTED(ch) ? HERA_STEPS_INTO_RIDING_ON :
				 HERA_STEPS_INTO,
		   MOUNTED(ch) ? MOUNTED(ch)->short_descr : NULL);
	
	act(IS_SET(portal->value[2], GATE_NORMAL_EXIT) ?
	    "You enter $p." :
	    "You walk through $p and find yourself somewhere else...",
	    ch,portal,NULL,TO_CHAR); 

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, location);

	if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
	{
	    obj_from_room(portal);
	    obj_to_room(portal,location);
	}

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act(mount ? "$n has arrived, riding $N" : "$n has arrived.",
		ch, portal, mount, TO_ROOM);
	else
	    act(mount ? "$n has arrived through $p, riding $N." :
	                "$n has arrived through $p.",
		ch, portal, mount, TO_ROOM);

	do_look(ch,"auto");

        if (mount) {
	  char_from_room( mount );
	  char_to_room( mount, location);
  	  ch->riding = TRUE;
  	  mount->riding = TRUE;
	 }

	/* charges */
	if (portal->value[0] > 0) {
	    portal->value[0]--;
	    if (portal->value[0] == 0)
		portal->value[0] = -1;
	}

	/* protect against circular follows */
	if (old_room == location)
	    return;

    	for ( fch = old_room->people; fch != NULL; fch = fch_next ) {
            fch_next = fch->next_in_room;

            if (portal == NULL || portal->value[0] == -1) 
	    /* no following through dead portals */
                continue;
 
            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
            	do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING)
            {
 
                if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
                &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE))) {
                    act("You can't bring $N into the city.",
                    	ch,NULL,fch,TO_CHAR);
                    act("You aren't allowed in the city.",
                    	fch,NULL,NULL,TO_CHAR);
                    continue;
            	}
 
            	act( "You follow $N.", fch, NULL, ch, TO_CHAR );
		do_enter(fch,argument);
            }
    	}

 	if (portal != NULL && portal->value[0] == -1)
	{
	    act("$p fades out of existence.",ch,portal,NULL,TO_CHAR);
	    if (ch->in_room == old_room)
		act("$p fades out of existence.",ch,portal,NULL,TO_ROOM);
	    else if (old_room->people != NULL)
	    {
		act("$p fades out of existence.", 
		    old_room->people,portal,NULL,TO_CHAR);
		act("$p fades out of existence.",
		    old_room->people,portal,NULL,TO_ROOM);
	    }
	    extract_obj(portal);
	}
	return;
    }

    send_to_char("Nope, can't do it.\n\r",ch);
    return;
}

void do_settraps( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_settraps].skill_level[ch->class] )
      {
	send_to_char("You don't know how to set traps.\n\r",ch);
	return;
      }

    if (!ch->in_room)	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_LAW) )
	{
	 send_to_char("A mystical power protects the room.\n\r",ch);
	 return;
	}

    WAIT_STATE( ch, skill_table[gsn_settraps].beats );

    if  (  IS_NPC(ch)
    ||   number_percent( ) <  ( get_skill(ch,gsn_settraps) * 0.7 ) )
    {
      AFFECT_DATA af,af2;

      check_improve(ch,gsn_settraps,TRUE,1);

      if ( is_affected_room( ch->in_room, gsn_settraps ))
      {
	send_to_char("This room has already trapped.\n\r",ch);
	return;
       }

      if ( is_affected(ch,gsn_settraps))
      {
	send_to_char("This skill is used too recently.\n\r",ch);
	return;
      }
   
      af.where     = TO_ROOM_AFFECTS;
      af.type      = gsn_settraps;
      af.level     = ch->level;
      af.duration  = ch->level / 40;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_ROOM_THIEF_TRAP;
      affect_to_room( ch->in_room, &af );

      af2.where     = TO_AFFECTS;
      af2.type      = gsn_settraps;
      af2.level	    = ch->level;
	
      if ( ch->last_fight_time != -1 && !IS_IMMORTAL(ch) &&
        	(current_time - ch->last_fight_time)<FIGHT_DELAY_TIME) 
         af2.duration  = 1;
      else af2.duration = ch->level / 10;

      af2.modifier  = 0;
      af2.location  = APPLY_NONE;
      af2.bitvector = 0;
      affect_to_char( ch, &af2 );
      send_to_char( "You set the room with your trap.\n\r", ch );
      act("$n set the room with $s trap.",ch,NULL,NULL,TO_ROOM);
      return;
    }
    else check_improve(ch,gsn_settraps,FALSE,1);

   return;
}

/***************************************************************************
 ************************       hunt.c        ******************************
 ***************************************************************************/


/***************************************************************************
 *  Original idea from SillyMUD v1.1b (C)1993.                             *
 *  Modified to merc2.1 by Rip.                                            *
 *  Modified by Turtle for Merc22 (07-Nov-94).                             *
 *  Adopted to ANATOLIA by Chronos.                                        *
 ***************************************************************************/

#if	defined(linux)
void bcopy(const void *src,void *dest,int n);
void bzero(void *s,int n);
#elif	defined(BSD44)
#include <string.h>
#else
void bcopy(char *s1,char* s2,int len);
void bzero(char *sp,int len);
#endif

extern const char * dir_name[];

struct hash_link
{
  int			key;
  struct hash_link	*next;
  void			*data;
};

struct hash_header
{
  int			rec_size;
  int			table_size;
  int			*keylist, klistsize, klistlen; /* this is really lame,
							  AMAZINGLY lame */
  struct hash_link	**buckets;
};

#define WORLD_SIZE	32700
#define	HASH_KEY(ht,key)((((unsigned int)(key))*17)%(ht)->table_size)



struct hunting_data
{
  char			*name;
  struct char_data	**victim;
};

struct room_q
{
  int		room_nr;
  struct room_q	*next_q;
};

struct nodes
{
  int	visited;
  int	ancestor;
};

#define IS_DIR		(get_room_index(q_head->room_nr)->exit[i])
#define GO_OK		(!IS_SET( IS_DIR->exit_info, EX_CLOSED ))
#define GO_OK_SMARTER	1



void init_hash_table(struct hash_header	*ht,int rec_size,int table_size)
{
  ht->rec_size	= rec_size;
  ht->table_size= table_size;
  ht->buckets	= (void*)calloc(sizeof(struct hash_link**),table_size);
  ht->keylist	= (void*)malloc(sizeof(ht->keylist)*(ht->klistsize=128));
  ht->klistlen	= 0;
}

void init_world(ROOM_INDEX_DATA *room_db[])
{
  /* zero out the world */
  bzero((char *)room_db,sizeof(ROOM_INDEX_DATA *)*WORLD_SIZE);
}

CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *ach;
   int number;
   int count;

   if( argument[0] == '\0' )
     return NULL;

   number = number_argument( argument, arg );
/*    if (arg[0] == NULL) return NULL; */
   if (arg[0] == '\0') return NULL;
   count = 0;     

   if( ( ach = get_char_room( ch, argument ) ) != NULL )
     return ach;



   for( ach = char_list; ach != NULL; ach = ach->next )
   { 
     if( ach->in_room && (ach->in_room->area != ch->in_room->area
	|| !can_see( ch, ach ) || !is_name( arg, ach->name )) )
	continue;
     if( ++count == number )
       return ach;
   }
   return NULL;
}


void destroy_hash_table(struct hash_header *ht,void (*gman)())
{
  int			i;
  struct hash_link	*scan,*temp;

  for(i=0;i<ht->table_size;i++)
    for(scan=ht->buckets[i];scan;)
      {
	temp = scan->next;
	(*gman)(scan->data);
	free(scan);
	scan = temp;
      }
  free(ht->buckets);
  free(ht->keylist);
}

void _hash_enter(struct hash_header *ht,int key,void *data)
{
  /* precondition: there is no entry for <key> yet */
  struct hash_link	*temp;
  int			i;

  temp		= (struct hash_link *)malloc(sizeof(struct hash_link));
  temp->key	= key;
  temp->next	= ht->buckets[HASH_KEY(ht,key)];
  temp->data	= data;
  ht->buckets[HASH_KEY(ht,key)] = temp;
  if(ht->klistlen>=ht->klistsize)
    {
      ht->keylist = (void*)realloc(ht->keylist,sizeof(*ht->keylist)*
				   (ht->klistsize*=2));
    }
  for(i=ht->klistlen;i>=0;i--)
    {
      if(ht->keylist[i-1]<key)
	{
	  ht->keylist[i] = key;
	  break;
	}
      ht->keylist[i] = ht->keylist[i-1];
    }
  ht->klistlen++;
}

ROOM_INDEX_DATA *room_find(ROOM_INDEX_DATA *room_db[],int key)
{
  return((key<WORLD_SIZE&&key>-1)?room_db[key]:0);
}

void *hash_find(struct hash_header *ht,int key)
{
  struct hash_link *scan;

  scan = ht->buckets[HASH_KEY(ht,key)];

  while(scan && scan->key!=key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

int room_enter(ROOM_INDEX_DATA *rb[],int key,ROOM_INDEX_DATA *rm)
{
  ROOM_INDEX_DATA *temp;
   
  temp = room_find(rb,key);
  if(temp) return(0);

  rb[key] = rm;
  return(1);
}

int hash_enter(struct hash_header *ht,int key,void *data)
{
  void *temp;

  temp = hash_find(ht,key);
  if(temp) return 0;

  _hash_enter(ht,key,data);
  return 1;
}

ROOM_INDEX_DATA *room_find_or_create(ROOM_INDEX_DATA *rb[],int key)
{
  ROOM_INDEX_DATA *rv;

  rv = room_find(rb,key);
  if(rv) return rv;

  rv = (ROOM_INDEX_DATA *)malloc(sizeof(ROOM_INDEX_DATA));
  rb[key] = rv;
    
  return rv;
}

void *hash_find_or_create(struct hash_header *ht,int key)
{
  void *rval;

  rval = hash_find(ht, key);
  if(rval) return rval;

  rval = (void*)malloc(ht->rec_size);
  _hash_enter(ht,key,rval);

  return rval;
}

int room_remove(ROOM_INDEX_DATA *rb[],int key)
{
  ROOM_INDEX_DATA *tmp;

  tmp = room_find(rb,key);
  if(tmp)
    {
      rb[key] = 0;
      free(tmp);
    }
  return(0);
}

void *hash_remove(struct hash_header *ht,int key)
{
  struct hash_link **scan;

  scan = ht->buckets+HASH_KEY(ht,key);

  while(*scan && (*scan)->key!=key)
    scan = &(*scan)->next;

  if(*scan)
    {
      int		i;
      struct hash_link	*temp, *aux;

      temp	= (*scan)->data;
      aux	= *scan;
      *scan	= aux->next;
      free(aux);

      for(i=0;i<ht->klistlen;i++)
	if(ht->keylist[i]==key)
	  break;

      if(i<ht->klistlen)
	{
	  bcopy((char *)ht->keylist+i+1,(char *)ht->keylist+i,(ht->klistlen-i)
		*sizeof(*ht->keylist));
	  ht->klistlen--;
	}

      return temp;
    }

  return NULL;
}

void room_iterate(ROOM_INDEX_DATA *rb[],void (*func)(),void *cdata)
{
  register int i;

  for(i=0;i<WORLD_SIZE;i++)
    {
      ROOM_INDEX_DATA *temp;
  
      temp = room_find(rb,i);
      if(temp) (*func)(i,temp,cdata);
    }
}

void hash_iterate(struct hash_header *ht,void (*func)(),void *cdata)
{
  int i;

  for(i=0;i<ht->klistlen;i++)
    {
      void		*temp;
      register int	key;

      key = ht->keylist[i];
      temp = hash_find(ht,key);
      (*func)(key,temp,cdata);
      if(ht->keylist[i]!=key) /* They must have deleted this room */
	i--;		      /* Hit this slot again. */
    }
}



int exit_ok( EXIT_DATA *pexit )
{
  ROOM_INDEX_DATA *to_room;

  if ( ( pexit == NULL )
  ||   ( to_room = pexit->u1.to_room ) == NULL )
    return 0;

  return 1;
}

void donothing()
{
  return;
}

int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
	       int depth, int in_zone )
{
  struct room_q		*tmp_q, *q_head, *q_tail;
  struct hash_header	x_room;
  int			i, tmp_room, count=0, thru_doors;
  ROOM_INDEX_DATA	*herep;
  ROOM_INDEX_DATA	*startp;
  EXIT_DATA		*exitp;

  if ( depth <0 )
    {
      thru_doors = TRUE;
      depth = -depth;
    }
  else
    {
      thru_doors = FALSE;
    }

  startp = get_room_index( in_room_vnum );

  init_hash_table( &x_room, sizeof(int), 2048 );
  hash_enter( &x_room, in_room_vnum, (void *) - 1 );

  /* initialize queue */
  q_head = (struct room_q *) malloc(sizeof(struct room_q));
  q_tail = q_head;
  q_tail->room_nr = in_room_vnum;
  q_tail->next_q = 0;

  while(q_head)
    {
      herep = get_room_index( q_head->room_nr );
      /* for each room test all directions */
      if (herep==NULL) fprintf(stderr,"BUG:  Null herep in hunt.c, room #%d",q_head->room_nr);
      if( herep && (herep->area == startp->area || !in_zone) )
		{
	  /* only look in this zone...
	     saves cpu time and  makes world safer for players  */
	  for( i = 0; i <= 5; i++ )
	    {
	      exitp = herep->exit[i];
	      if( exit_ok(exitp) && ( thru_doors ? GO_OK_SMARTER : GO_OK ) )
		{
		  /* next room */
		  tmp_room = herep->exit[i]->u1.to_room->vnum;
		  if( tmp_room != out_room_vnum )
		    {
		      /* shall we add room to queue ?
			 count determines total breadth and depth */
		      if( !hash_find( &x_room, tmp_room )
			 && ( count < depth ) )
			/* && !IS_SET( RM_FLAGS(tmp_room), DEATH ) ) */
			{
			  count++;
			  /* mark room as visted and put on queue */
			  
			  tmp_q = (struct room_q *)
			    malloc(sizeof(struct room_q));
			  tmp_q->room_nr = tmp_room;
			  tmp_q->next_q = 0;
			  q_tail->next_q = tmp_q;
			  q_tail = tmp_q;
	      
			  /* ancestor for first layer is the direction */
			  hash_enter( &x_room, tmp_room,
				     ((int)hash_find(&x_room,q_head->room_nr)
				      == -1) ? (void*)(i+1)
				     : hash_find(&x_room,q_head->room_nr));
			}
		    }
		  else
		    {
		      /* have reached our goal so free queue */
		      tmp_room = q_head->room_nr;
		      for(;q_head;q_head = tmp_q)
			{
			  tmp_q = q_head->next_q;
			  free(q_head);
			}
		      /* return direction if first layer */
		      if ((int)hash_find(&x_room,tmp_room)==-1)
			{
			  if (x_room.buckets)
			    {
			      /* junk left over from a previous track */
			      destroy_hash_table(&x_room, donothing);
			    }
			  return(i);
			}
		      else
			{
			  /* else return the ancestor */
			  int i;
			  
			  i = (int)hash_find(&x_room,tmp_room);
			  if (x_room.buckets)
			    {
			      /* junk left over from a previous track */
			      destroy_hash_table(&x_room, donothing);
			    }
			  return( -1+i);
			}
		    }
		}
	    }
	}
      
      /* free queue head and point to next entry */
      tmp_q = q_head->next_q;
      free(q_head);
      q_head = tmp_q;
    }

  /* couldn't find path */
  if( x_room.buckets )
    {
      /* junk left over from a previous track */
      destroy_hash_table( &x_room, donothing );
    }
  return -1;
}



void do_hunt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int direction,i;
  bool fArea,ok;
  

    if ( ch_skill_nok(ch,gsn_hunt) )
	return;

    if (!cabal_ok(ch,gsn_hunt)) return;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_hunt].skill_level[ch->class] )
    {
	send_to_char("Huh?\n\r", ch );
	return;
    }

  one_argument( argument, arg );

  if( arg[0] == '\0' )
    {
      send_to_char( "Whom are you trying to hunt?\n\r", ch );
      return;
    }

/*  fArea = ( get_trust(ch) < MAX_LEVEL ); */
  fArea = !(IS_IMMORTAL(ch));

  if ( ch->level >= skill_table[gsn_world_find].skill_level[ch->class] )
	{
	 if (number_percent() < get_skill(ch,gsn_world_find) )
	  {
	   fArea = 0;
	   check_improve(ch,gsn_world_find,TRUE,1);
	  }
	 check_improve(ch,gsn_world_find,FALSE,1);
	}

  if( fArea )
    victim = get_char_area( ch, arg);
  else
    victim = get_char_world( ch, arg);

  if( victim == NULL )
    {
      send_to_char("No-one around by that name.\n\r", ch );
      return;
    }

  if( ch->in_room == victim->in_room )
    {
      act( "$N is here!", ch, NULL, victim, TO_CHAR );
      return;
    }

  if( IS_NPC( ch  ) )
  {
    ch->hunting = victim;
    hunt_victim( ch );
    return;
  }


  /*
   * Deduct some movement.
   */
  if (!IS_IMMORTAL(ch))
{
  if( ch->endur > 2 )
    ch->endur -= 3;
  else
    {
      send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
      return;
    }
}

  act( "$n stares intently at the ground.", ch, NULL, NULL, TO_ROOM );

  WAIT_STATE( ch, skill_table[gsn_hunt].beats );
  direction = find_path( ch->in_room->vnum, victim->in_room->vnum,
			ch, -40000, fArea );

  if( direction == -1 )
    {
      act( "You couldn't find a path to $N from here.",
	  ch, NULL, victim, TO_CHAR );
      return;
    }

  if( direction < 0 || direction > 5 )
    {
      send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
      return;
    }

  /*
   * Give a random direction if the player misses the die roll.
   */
  if ( IS_NPC (ch) && number_percent () > 75)        /* NPC @ 25% */
    {
    log_string("Do PC hunt");
    ok=FALSE;
    for(i=0;i<6;i++) {
    	if (ch->in_room->exit[direction]!=NULL) {
    		ok=TRUE;
    		break;
    		}
    	}
    if (ok)	
    {
      do
	{
	  direction = number_door();
	}
      while( ( ch->in_room->exit[direction] == NULL )
	    || ( ch->in_room->exit[direction]->u1.to_room == NULL) );
    }
else {
	  log_string("Do hunt, player hunt, no exits from room!");
  	  ch->hunting=NULL;
  	  send_to_char("Your room has not exits!!!!\n\r",ch);
  	  return;
  	}
  /*
   * Display the results of the search.
   */
  }
	act_printf(ch, NULL, victim, TO_CHAR, POS_RESTING,
		   HERA_IS_FROM_HERE, dir_name[direction]);
  return;
}


/*
 * revised by chronos.
 */
void hunt_victim( CHAR_DATA *ch )
{
  int		dir;
  bool		found;
  CHAR_DATA	*tmp;
  char		tBuf[MAX_INPUT_LENGTH];

  /*
   * Make sure the victim still exists.
   */
  for( found = 0, tmp = char_list; tmp && !found; tmp = tmp->next )
    if( ch->hunting == tmp )
      found = 1;

  if( !found || !can_see( ch, ch->hunting ) )
    {
     if( get_char_area( ch, ch->hunting->name) != NULL )
        {
           sprintf( tBuf, "portal %s", ch->hunting->name );
           log_string("mob portal");
           do_cast( ch, tBuf );
           log_string("do_enter1");
           do_enter( ch, "portal" );
	   if (ch->in_room==NULL || ch->hunting==NULL) return;
	   if( ch->in_room == ch->hunting->in_room )
	    {
	      act( "$n glares at $N and says, 'Ye shall DIE!'",
		  ch, NULL, ch->hunting, TO_NOTVICT );
	      act( "$n glares at you and says, 'Ye shall DIE!'",
		  ch, NULL, ch->hunting, TO_VICT );
	      act( "You glare at $N and say, 'Ye shall DIE!",
		  ch, NULL, ch->hunting, TO_CHAR);
	      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
	      ch->hunting = NULL;
	      return;
	    }  
	   log_string("done1");  
	   return;
        } 
       else 
	{ 
         do_say( ch, "Ahhhh!  My prey is gone!!" );
         ch->hunting = NULL;
         return;
        }  
    }   /* end if !found or !can_see */ 



  dir = find_path( ch->in_room->vnum, ch->hunting->in_room->vnum,
		  ch, -40000, TRUE );

  if( dir < 0 || dir > 5 )
  {
/* 1 */ 
    if( get_char_area( ch, ch->hunting->name) != NULL  
        && ch-> level > 35 )
    {
      sprintf( tBuf, "portal %s", ch->hunting->name );
      log_string("mob portal");
      do_cast( ch, tBuf );
      log_string("do_enter2");
      do_enter( ch, "portal" );
      if (ch->in_room==NULL || ch->hunting==NULL) return;
      if( ch->in_room == ch->hunting->in_room )
       {
      	act( "$n glares at $N and says, 'Ye shall DIE!'",
		  ch, NULL, ch->hunting, TO_NOTVICT );
	act( "$n glares at you and says, 'Ye shall DIE!'",
		  ch, NULL, ch->hunting, TO_VICT );
        act( "You glare at $N and say, 'Ye shall DIE!",
		  ch, NULL, ch->hunting, TO_CHAR);
        multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
        ch->hunting = NULL;
        return;
       }  
      log_string("done2"); 
      return;
    }
    else
    { 
      act( "$n says 'I have lost $M!'", ch, NULL, ch->hunting, TO_ROOM );
      ch->hunting = NULL;
      return;
    }
   } /* if dir < 0 or > 5 */  


  if( ch->in_room->exit[dir] && IS_SET( ch->in_room->exit[dir]->exit_info, EX_CLOSED ) )
    {
      do_open( ch,(char *)dir_name[dir]);
      return;
    }
  if (!ch->in_room->exit[dir]) {
		log_string("BUG:  hunt through null door");
		ch->hunting = NULL;
		return;
		}
  move_char( ch, dir, FALSE );
  if (ch->in_room==NULL || ch->hunting==NULL) return;
  if( ch->in_room == ch->hunting->in_room )
    {
      act( "$n glares at $N and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_NOTVICT );
      act( "$n glares at you and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_VICT );
      act( "You glare at $N and say, 'Ye shall DIE!",
	  ch, NULL, ch->hunting, TO_CHAR);
      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
      ch->hunting = NULL;
      return;
    }  
  return;
}

void hunt_victim_old( CHAR_DATA *ch )
{
  int		dir,i;
  bool		found,ok;
  CHAR_DATA	*tmp;
  char		tBuf[MAX_INPUT_LENGTH];

  if( ch == NULL || ch->hunting == NULL || !IS_NPC(ch) ) 
   {
    if (IS_NPC(ch))
      {
    	if ((ROOM_INDEX_DATA*)ch->logon!=ch->in_room)
    	log_string("HUNT: Return creature to original home!");
       	act("\n\rA glowing portal appears.",ch,NULL,NULL,TO_ROOM);	    	
    	act("$n steps through a glowing portal.\n\r",ch,NULL,NULL,TO_ROOM);
       	char_from_room(ch);
    	char_to_room(ch,(ROOM_INDEX_DATA*)ch->logon);
        }
    return;
   }

  /*
   * Make sure the victim still exists.
   */
  for( found = 0, tmp = char_list; tmp && !found; tmp = tmp->next )
    if( ch->hunting == tmp )
      found = 1;

  if( !found || !can_see( ch, ch->hunting ) )
    {
/*1 */  if( get_char_world( ch, ch->hunting->name) != NULL  
            && ch-> level > 35 )
        {
           sprintf( tBuf, "portal %s", ch->hunting->name );
           log_string("mob portal");
           do_cast( ch, tBuf );
           log_string("do_enter1");
           do_enter( ch, "portal" );
  /* Deth...this shouldn't have to be here..but it got
  here in a core file with ch->hunting==null.. */
  if (ch->in_room==NULL || ch->hunting==NULL) return;
  if( ch->in_room == ch->hunting->in_room )
    {
      act( "$n glares at $N and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_NOTVICT );
      act( "$n glares at you and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_VICT );
      act( "You glare at $N and say, 'Ye shall DIE!",
	  ch, NULL, ch->hunting, TO_CHAR);
      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
      ch->hunting = NULL;
      return;
    }  
	   log_string("done1");  
	   	return;
        } 
       else { 
       if (IS_NPC(ch)) 
	 {
          if ( (ROOM_INDEX_DATA*)ch->logon!=ch->in_room) 
          {
     	   log_string("HUNT: Send mob home");
    	   act("\n\rA glowing portal appears.",ch,NULL,NULL,TO_ROOM);	    	
    	   act("$n steps through a glowing portal.\n\r",ch,NULL,NULL,TO_ROOM); 
       	   char_from_room(ch);
    	   char_to_room(ch,(ROOM_INDEX_DATA*)ch->logon);
    	  }
         }

         do_say( ch, "Ahhhh!  My prey is gone!!" );
         ch->hunting = NULL;
         return;
        }  
    }   /* end if !found or !can_see */ 



  dir = find_path( ch->in_room->vnum, ch->hunting->in_room->vnum,
		  ch, -40000, TRUE );

  if( dir < 0 || dir > 5 )
  {
/* 1 */ 
    if( get_char_area( ch, ch->hunting->name) != NULL  
        && ch-> level > 35 )
    {
      sprintf( tBuf, "portal %s", ch->hunting->name );
      log_string("mob portal");
      do_cast( ch, tBuf );
      log_string("do_enter2");
      do_enter( ch, "portal" );
  /* Deth...this shouldn't have to be here..but it got
  here in a core file with ch->hunting==null.. */
  if (ch->in_room==NULL || ch->hunting==NULL) return;
  if( ch->in_room == ch->hunting->in_room )
    {
      act( "$n glares at $N and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_NOTVICT );
      act( "$n glares at you and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_VICT );
      act( "You glare at $N and say, 'Ye shall DIE!",
	  ch, NULL, ch->hunting, TO_CHAR);
      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
      ch->hunting = NULL;
      return;
    }  
      log_string("done2"); 
      return;
    }
    else
    { 
     if (IS_NPC(ch)) 
       {
    	if ((ROOM_INDEX_DATA*)ch->logon!=ch->in_room)
        {  
          log_string("HUNT: return creature to original room");
	  act("\n\rA glowing portal appears.",ch,NULL,NULL,TO_ROOM);	    	
	  act("$n steps through a glowing portal.\n\r",ch,NULL,NULL,TO_ROOM);
	  char_from_room(ch);
    	  char_to_room(ch,(ROOM_INDEX_DATA*)ch->logon);
    	}
      }   

      act( "$n says 'I have lost $M!'", ch, NULL, ch->hunting, TO_ROOM );
      ch->hunting = NULL;
      return;
    }
   } /* if dir < 0 or > 5 */  

  /*
   * Give a random direction if the mob misses the die roll.
   */
  if( number_percent () > 75 )        /* @ 25% */
    {
    	ok=FALSE;
        for(i=0;i<6;i++) {
    	if (ch->in_room->exit[dir]!=NULL) {
    		ok=TRUE;
    		break;
    		}
    	}
	if(ok) {
      do
        {
	  dir = number_door();
        }
      while( ( ch->in_room->exit[dir] == NULL )
	    || ( ch->in_room->exit[dir]->u1.to_room == NULL ) );
	   }
	  else {
	  log_string("Do hunt, player hunt, no exits from room!");
  	  ch->hunting=NULL;
  	  send_to_char("Your room has not exits!!!!\n\r",ch);
  	  return;
  	}	    
    }
    


  if( ch->in_room->exit[dir] && IS_SET( ch->in_room->exit[dir]->exit_info, EX_CLOSED ) )
    {
      do_open( ch,(char *)dir_name[dir]);
      return;
    }
	if (!ch->in_room->exit[dir]) {
		log_string("BUG:  hunt through null door");
		return;
		}
  move_char( ch, dir, FALSE );
  /* Deth...this shouldn't have to be here..but it got
  here in a core file with ch->hunting==null.. */
  if (ch->in_room==NULL || ch->hunting==NULL) return;
  if( ch->in_room == ch->hunting->in_room )
    {
      act( "$n glares at $N and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_NOTVICT );
      act( "$n glares at you and says, 'Ye shall DIE!'",
	  ch, NULL, ch->hunting, TO_VICT );
      act( "You glare at $N and say, 'Ye shall DIE!",
	  ch, NULL, ch->hunting, TO_CHAR);
      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
      ch->hunting = NULL;
      return;
    }  
  return;
}


/***************************************************************************
 ************************      repair.c       ******************************
 ***************************************************************************/

void damage_to_obj(CHAR_DATA *ch,OBJ_DATA *wield, OBJ_DATA *worn, int damage) 
{

 	if ( damage == 0) return;
 		worn->condition -= damage;

	act_puts("{gThe $p inflicts damage on {r$P{g.{x",
		ch,wield,worn,TO_ROOM,POS_RESTING);

	if (worn->condition < 1) {
		act_puts("{gThe {r$P{g breaks into pieces.{x",
			ch,wield,worn,TO_ROOM,POS_RESTING);
		extract_obj(worn);
		return;
	}
 
	if (IS_SET(wield->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(wield->extra_flags,ITEM_ANTI_NEUTRAL)
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_NEUTRAL)) {
		act_puts("$p doesn't want to fight against $P.", ch, wield, worn, TO_ROOM, POS_RESTING);
		act_puts("$p removes itself from you!", ch, wield, worn, TO_CHAR, POS_RESTING);
		act_puts("$p removes itself from $n.", ch, wield, worn, TO_ROOM, POS_RESTING);
		unequip_char( ch, wield );
		return;
 	}

	if (IS_SET(wield->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_EVIL)) {
		act_puts("The $p worries for the damage to $P.", ch, wield, worn, TO_ROOM, POS_RESTING);
		return;
	}
}


void check_weapon_destroy(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn,i;

 if (IS_NPC(victim) || number_percent() < 94 )  return;

 if (!second)
	{
	 if ( (wield = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch);
 	 skill = get_skill(ch, sn );
	}
  else  {
	 if ( (wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_second_sn(ch);
 	 skill = get_skill(ch, sn );
	}
 if (is_metal(wield))
      {
	for (i=0;i < MAX_WEAR; i++) 
	{
	 if ( ( destroy = get_eq_char(victim,i) ) == NULL 
		|| number_percent() > 95
		|| number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1
		|| (i == WEAR_WIELD || i== WEAR_SECOND_WIELD 
			|| i == WEAR_TATTOO || i == WEAR_STUCK_IN) )
	 continue;
	
	 chance += 20;
	 if ( check_material(wield, "platinium") ||
	      check_material(wield, "titanium") )
	 chance += 5;

	 if ( is_metal(destroy) )  chance -= 20;
	 else 			chance += 20; 

	 chance += ( (ch->level - victim->level) / 5);

	 chance += ( (wield->level - destroy->level) / 2 );

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;
	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 50)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 5) );
		 break;
		}
	}
      }
 else {
	for (i=0;i < MAX_WEAR;i++) 
	{
	 if ( ( destroy = get_eq_char(victim,i) ) == NULL 
		|| number_percent() > 95
		|| number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1
		|| (i == WEAR_WIELD || i== WEAR_SECOND_WIELD 
			|| i == WEAR_TATTOO || i == WEAR_STUCK_IN ) )
	 continue;
	
	 chance += 10;

	 if ( is_metal(destroy) )  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;

	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 50)
		{
		 damage_to_obj(ch,wield,destroy, chance / 5 );
		 break;
		}
	}
      }

 return;
}


void do_repair(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int cost;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if (!IS_NPC(mob)) continue;
	if (mob->spec_fun == spec_lookup("spec_repairman") )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_say(mob,"I will repair a weapon for you, for a price.");
	send_to_char("Type estimate <weapon> to be assessed for damage.\n\r",ch);
	return;
    }
    if (( obj = get_obj_carry(ch, arg)) == NULL)
    {
	do_say(mob,"You don't have that item");
	return;
    }

    if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER)
    {
     do_say(mob,"That hammer is beyond my power.");
     return;
    }

    if (obj->condition >= 100)
    {
	do_say(mob,"But that item is not broken.");
        return;
    }

    if (obj->cost == 0) {
	do_say(mob, "%s is beyond repair.\n\r", obj->short_descr);
   	return;
    }

    cost = ( (obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)    );
    cost /= 100;

    if (cost > ch->gold)
    {
	do_say(mob,"You do not have enough gold for my services.");
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    ch->gold -= cost;
    mob->gold += cost;
    sprintf(buf, "$N takes %s from $n, repairs it, and returns it to $n", obj->short_descr); 
    act(buf,ch,NULL,mob,TO_ROOM);
    sprintf(buf, "%s takes %s, repairs it, and returns it\n\r", mob->short_descr, obj->short_descr);
    send_to_char(buf, ch);
    obj->condition = 100;
}

void do_estimate(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *mob; 
    char arg[MAX_INPUT_LENGTH];
    int cost;
    
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if (!IS_NPC(mob)) continue;
	if (mob->spec_fun == spec_lookup("spec_repairman") )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }
    
    one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
	do_say(mob,"Try estimate <item>");
   	return; 
    } 
    if ((obj = (get_obj_carry(ch, arg))) == NULL)
    {
	do_say(mob,"You don't have that item");
	return;
    }
    if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER)
	{
	    do_say(mob,"That hammer is beyond my power.");
	    return;
	}
    if (obj->condition >= 100)
    {
	do_say(mob,"But that item's not broken");
	return;
    }
    if (obj->cost == 0)
    {
	do_say(mob,"That item is beyond repair");
    	return;
    } 
    
    cost = ( (obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)    );
    cost /= 100;

    sprintf(buf, "It will cost %d to fix that item", cost);
    do_say(mob,buf);
}

void do_restring( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_INPUT_LENGTH];
    char arg  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    int cost = 2000;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

	if ( arg[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  restring <obj> <field> <string>\n\r",ch);
		send_to_char("    fields: name short long\n\r",ch);
		return;
	}

    if ((obj = (get_obj_carry(ch, arg))) == NULL)
    {
		send_to_char("The Stringer says '`sYou don't have that item``.'\n\r", ch);
		return;
    }

	cost += (obj->level * 1500);

    if (cost > ch->gold)
    {
		act("$N says 'You do not have enough gold for my services.'",
		  ch,NULL,mob,TO_CHAR);
		return;
    }
    
	if ( !str_prefix( arg1, "name" ) )
	{
		free_string( obj->name );
		obj->name = str_dup( arg2 );
	}
	else
	if ( !str_prefix( arg1, "short" ) )
	{
		free_string( obj->short_descr );
        obj->short_descr = str_dup(arg2);
	}
	else
	if ( !str_prefix( arg1, "long" ) )
	{
		free_string( obj->description );
		obj->description = str_dup( arg2 );
	}
	else
	{
		send_to_char("That's not a valid Field.\n\r",ch);
		return;
	}
	
    WAIT_STATE(ch,PULSE_VIOLENCE);

    ch->gold -= cost;
    mob->gold += cost;
    sprintf(buf, "$N takes $n's item, tinkers with it, and returns it to $n.");
	act(buf,ch,NULL,mob,TO_ROOM);
  sprintf(buf,"%s takes your item, tinkers with it, and returns %s to you.\n\r", mob->short_descr, obj->short_descr);
  send_to_char(buf,ch);
  send_to_char("Remember, if we find your new string offensive, we will not be happy.\n\r", ch);
  send_to_char(" This is your ONE AND ONLY Warning.\n\r", ch);
}

void check_shield_destroyed(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn;

 if (IS_NPC(victim) || number_percent() < 94 )  return;

 if (!second)
	{
	 if ( (wield = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch);
 	 skill = get_skill(ch, sn );
	}
  else  {
	 if ( (wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_second_sn(ch);
 	 skill = get_skill(ch, sn );
	}

 destroy = get_eq_char(victim,WEAR_SHIELD);
 if (destroy == NULL) return; 

 if (is_metal(wield))
      {
	 if (   number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1 )
	 return;
	
	 chance += 20;
	 if ( check_material(wield, "platinium") ||
	      check_material(wield, "titanium") )
	 chance += 5;

	 if ( is_metal(destroy) )  chance -= 20;
	 else 			chance += 20; 

	 chance += ( (ch->level - victim->level) / 5);

	 chance += ( (wield->level - destroy->level) / 2 );

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;
	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/* 	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 20 )
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4) );
		 return;
		}
      }
 else {
	 if (   number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1 )
	 return;
	
	 chance += 10;

	 if ( is_metal(destroy) )  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;

	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 20)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4) );
		 return;
		}
      }
 return;
}

void check_weapon_destroyed(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn;

 if (IS_NPC(victim) || number_percent() < 94 )  return;

 if (!second)
	{
	 if ( (wield = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch);
 	 skill = get_skill(ch, sn );
	}
  else  {
	 if ( (wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_second_sn(ch);
 	 skill = get_skill(ch, sn );
	}

  destroy = get_eq_char(victim,WEAR_WIELD);
  if (destroy == NULL ) return;

 if (is_metal(wield))
      {
	 if (   number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1 )
	 return;
	
	 chance += 20;
	 if ( check_material(wield, "platinium") ||
	      check_material(wield, "titanium") )
	 chance += 5;

	 if ( is_metal(destroy) )  chance -= 20;
	 else 			chance += 20; 

	 chance += ( (ch->level - victim->level) / 5);

	 chance += ( (wield->level - destroy->level) / 2 );

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;
	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < (chance / 2) && chance > 20 )
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4) );
		 return;
		}
      }
 else {
	 if (   number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1 )
	 return;
	
	 chance += 10;

	 if ( is_metal(destroy) )  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if ( IS_WEAPON_STAT(wield,WEAPON_SHARP) )
		chance += 10;

	 if ( sn == gsn_axe ) chance += 10;

	/* spell affects */
	 if ( IS_OBJ_STAT( destroy, ITEM_BLESS) ) chance -= 10;
	 if ( IS_OBJ_STAT( destroy, ITEM_MAGIC) ) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat( ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < (chance / 2) && chance > 20 )
		{
		 damage_to_obj(ch,wield,destroy, chance / 4 );
		 return;
		}
      }
 return;
}


void do_smithing(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *hammer;

    if ( IS_NPC(ch)
    ||   ch->level < skill_table[gsn_smithing].skill_level[ch->class] )
    {
	send_to_char("Huh?\n\r", ch );
	return;
    }


    if ( ch->fighting )
    {
        send_to_char( "Wait until the fight finishes.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Which object do you want to repair.\n\r",ch);
	return;
    }

    if (( obj = get_obj_carry(ch, arg)) == NULL)
    {
	send_to_char("You are not carrying that.\n\r",ch);
	return;
    }

   if (obj->condition >= 100)
    {
	send_to_char("But that item is not broken.\n\r",ch);
	return;
    }

    if (( hammer = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
	send_to_char("You are not holding a hammer.\n\r",ch);
	return;
    }

    if ( hammer->pIndexData->vnum != OBJ_VNUM_HAMMER )
    {
	send_to_char("That is not the correct hammer.\n\r",ch);
	return;
    }

    WAIT_STATE(ch,2 * PULSE_VIOLENCE);
    if ( number_percent() > get_skill(ch,gsn_smithing) )    
     {
      check_improve(ch,gsn_smithing,FALSE,8);
    sprintf(buf, "$n try to repair %s with the hammer.But $n fails.", obj->short_descr); 
    act(buf,ch,NULL,obj,TO_ROOM);
    sprintf(buf, "You failed to repair %s\n\r", obj->short_descr);
    send_to_char(buf, ch);
    hammer->condition -= 25;
     }
    else
     {
    check_improve(ch,gsn_smithing,TRUE,4);
    sprintf(buf, "$n repairs %s with the hammer.", obj->short_descr); 
    act(buf,ch,NULL,NULL,TO_ROOM);
    sprintf(buf, "You repair %s\n\r", obj->short_descr);
    send_to_char(buf, ch);
    obj->condition = UMAX( 100 , 
	 obj->condition + ( get_skill(ch,gsn_smithing) / 2) );
    hammer->condition -= 25;
     }
   if (hammer->condition < 1)  extract_obj( hammer );
   return;
}

/***************************************************************************
 ************************      auction.c      ******************************
 ***************************************************************************/

/***************************************************************************
 *  This snippet was orginally written by Erwin S. Andreasen.              *
 *	erwin@pip.dknet.dk, http://pip.dknet.dk/~pip1773/                  *
 *  Adopted to Anatolia MUD by chronos.                                    *
 ***************************************************************************/

DECLARE_SPELL_FUN(	spell_identify	);

void talk_auction(char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"AUCTION: %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->comm,COMM_NOAUCTION) )
            act (buf, original, NULL, NULL, TO_CHAR);

    }
}


/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/

int advatoi (const char *s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi (tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{

  int newbet = 0;                /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
  char *stringptr = string;      /* a pointer we can move around */
  char buf2[MAX_STRING_LENGTH];

  strcpy (string,argument);      /* make a work copy of argument */


  if (*stringptr)               /* check for an empty string */
  {

    if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
      newbet = advatoi (stringptr); /* parse and set newbet to that value */

    else
      if (*stringptr == '+') /* add ?? percent */
      {
        if (strlen (stringptr) == 1) /* only + specified, assume default */
          newbet = (currentbet * 125) / 100; /* default: add 25% */
        else
          newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
      }
      else
        {
        sprintf (buf2,"considering: * x \n\r");
        if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
          if (strlen (stringptr) == 1) /* only x specified, assume default */
            newbet = currentbet * 2 ; /* default: twice */
          else /* user specified a number */
            newbet = currentbet * atoi (++stringptr); /* cut off the first char */
        }
  }

  return newbet;        /* return the calculated bet */
}



void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0)
                sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
            else
                sprintf (buf, "%s: going %s (not bet received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));
            talk_auction (buf);
            break;

            case 3 : /* SOLD! */

            if (auction->bet > 0)
            {
                sprintf (buf, "%s sold to %s for %d.",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
                    auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */

                auction->item = NULL; /* reset item */

            }
            else /* not sold */
            {
                sprintf (buf, "No bets received for %s - object has been removed.",auction->item->short_descr);
                talk_auction(buf);
                talk_auction("The auctioneer puts the unsold item to his pit.");
                extract_obj(auction->item);
                auction->item = NULL; /* clear auction */

            } /* else */

            } /* switch */
        } /* if */
} /* func */


void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char betbuf[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

	if (IS_NPC(ch))    /* NPC extracted can't auction! */
		return;

    if (IS_SET(ch->comm,COMM_NOAUCTION))
	{
    	 if (!str_cmp(arg1,"on") )
	 {
	  send_to_char("Auction channel is now ON.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_NOAUCTION);
	  return;
	 }
	 else
	 {
	  send_to_char("Your auction channel is OFF.\n\r",ch);
	  send_to_char("You must first change auction channel ON.\n\r",ch);
	  return;
	 }
	}

	if (arg1[0] == '\0')
		if (auction->item != NULL) {
			/* show item data here */
			if (auction->bet > 0)
				char_printf(ch, "Current bet on this item is "
						"%d gold.\n\r",auction->bet);
			else
				send_to_char ("No bets on this item have been "
					      "received.\n\r", ch);
			spell_identify(0, 0, ch, auction->item,0);
			return;
		}
		else {	
			send_to_char("Auction WHAT?%s\n\r", ch);
			return;
		}

    if (!str_cmp(arg1,"off") )
	{
	 send_to_char("Auction channel is now OFF.\n\r",ch);
	 SET_BIT(ch->comm,COMM_NOAUCTION);
	 return;
	}

    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    if (auction->item == NULL)
    {
        send_to_char ("There is no auction going on you can stop.\n\r",ch);
        return;
    }
    else /* stop the auction */
    {
        sprintf(buf,"Sale of %s has been stopped by God. Item confiscated.",
                        auction->item->short_descr);
        talk_auction(buf);
        obj_to_char(auction->item, auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
        return;
    }

    if  (!str_cmp(arg1,"bet") ) 
	if (auction->item != NULL)
        {
            int newbet;

	if ( ch == auction->seller )
		{
	send_to_char("You cannot bet on your own selling equipment...:)\n\r",ch);
	return;
		}
            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("Bet how much?\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
            sprintf (betbuf,"Bet: %d\n\r",newbet);

            if (newbet < (auction->bet + 1))
            {
                send_to_char ("You must at least bid 1 gold over the current bet.\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("You don't have that much money!\n\r",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"A bet of %d gold has been received on %s.\n\r",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;


        }
        else
        {
            send_to_char ("There isn't anything being auctioned right now.\n\r",ch);
            return;
        }

    /* finally... */

    obj = get_obj_carry (ch, arg1 ); /* does char have the item ? */ 

    if (obj == NULL)
    {
        send_to_char ("You aren't carrying that.\n\r",ch);
        return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act_puts ("You cannot auction $Ts.",ch, NULL, item_type_name(obj), 
		TO_CHAR,POS_SLEEPING);
        return;

    case ITEM_LIGHT:
    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_GEM:
    case ITEM_TREASURE:
        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0; 	/* obj->cost / 100 */
        auction->buyer = NULL;
        auction->seller = ch;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf(buf, "A new item has been received: {Y%s{x.", obj->short_descr);
        talk_auction(buf);

        return;

    } /* switch */
    else
    {
        act ("Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
        return;
    }
}


