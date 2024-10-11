/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "defs.h"

int 
addsym (char *item, int id)
{

  strcpy (symstream + symstreamcount, item);
  symtab[symtabcount].id = id;
  symtab[symtabcount].len = strlen (symstream + symstreamcount);
  symtab[symtabcount].ptr = symstreamcount;
  symtab[symtabcount].res = 1;
  symtab[symtabcount].ins = 1;

  symstreamcount += symtab[symtabcount].len;
  symtabcount++;

  return 0;
}

int 
syminit ()
{

  /* the rest belongs here                    */

  symtabcount = 0;
  symstreamcount = 0;

/* this bit could be hardwired to make things more compact */
  addsym ("cd", LCD);
  addsym ("chdir", LCD);
  addsym ("dir", LDIR);
  addsym ("cls", LCLS);
  addsym ("copy", LCOP);
  addsym ("exit", LEXI);
  addsym ("logout", LEXI);
  addsym ("del", LDEL);
  addsym ("erase", LDEL);
  addsym ("echo", LECH);
  addsym ("prompt", LPRO);
  addsym ("type", LTYP);
  addsym ("map", LMAP);
  addsym ("alias", LALI);
  addsym ("md", LMD);
  addsym ("mkdir", LMD);
  addsym ("rd", LRD);
  addsym (";", LSEP);
  addsym (">", LTO);
  addsym ("<", LFRO);
  addsym ("|", LPIP);
  addsym ("@", LAT);
  addsym ("+", LPLU);
  addsym ("=", LASS);
/*  addsym("&",LAMP); hmmm, not yet... */
  addsym ("!", LBAN);
  addsym ("call", LCAL);
  addsym ("ctty", LCAL);
  addsym ("?", LQES);
  addsym ("rmdir", LRD);
  addsym ("set", LSET);
  addsym ("ren", LREN);
  addsym ("rename", LREN);
  addsym ("vol", LVOL);
  addsym ("ver", LVER);
  addsym ("path", LPAT);
  addsym ("date", LDAT);
  addsym ("time", LTIM);
  addsym ("zap", LZAP);
  addsym ("unalias", LUNA);
  addsym ("pause", LPAU);
  addsym ("bye", LEXI);
  addsym ("quit", LEXI);
  addsym ("break", LTRA);
  addsym ("rem", LREM);

#ifdef DIAGNOSE
  addsym ("dump", LDMP);
#endif

  symtabreserved = symtabcount;
  symstreamreserved = symstreamcount;

  return 0;
}

int 
stricmp (char *a, char *b, int i)
{
  i--;
  while (i >= 0)
    {
      if (tolower (a[i]) != b[i])
	{
	  return 1;
	}
      i--;
    }
  return 0;
}

int 
lex (char *in, int tokens)
{
  int retval = 0, count = 0, found = 0, state = 0, iscmd = 1;

  int i;

  char *tptr;

  symtabcount = symtabreserved;
  symstreamcount = symstreamreserved;

  while ((in[count] != '\0') && (in[count] != '\n') && (in[count] != '\r'))
    {
      switch (state)
	{
	  /* strip leading ws */
	case 0:
	  if (!isspace (in[count]))
	    {
	      if (in[count] == '"')
		{
#ifdef DEBUG
		  fprintf (stderr, "Found a string in[count=%d]\n", count);
#endif
		  state = 2;
		  count++;
		}
	      else if (in[count] == '%')
		{
		  state = 3;
		  count++;
		}
	      else if (in[count] == '#')
		{
		  in[count] = '\0';
		}
	      else
		{
		  state = 1;
		}
	    }
	  else
	    count++;
	  break;


	  /* check if reserved command */
	case 1:
	  found = 0;

	  /* check if we find an existing match */
	  /* if(iscmd||(!isalnum(in[count]))){ hmmm... */
	  if (iscmd ||
	      (
		!isalnum (in[count]) &&
		in[count] != '?' &&
		!(in[count] == '@' && isalnum (in[count + 1]))
	      )
	    )
	    {
	      iscmd = 0;
	      for (i = 0; (i < symtabreserved) && (!found); i++)
		{
		  if (!((isalnum (in[count])) && isalnum (in[symtab[i].len + count])))
		    {
		      if (symtab[i].ins)
			{
			  if (!stricmp (in + count, symstream + symtab[i].ptr, symtab[i].len))
			    {
			      tokenstream[tokens++] = i;
			      count += symtab[i].len;
			      found = 1;
			      if (symtab[i].id == LSEP || symtab[i].id == LPIP || symtab[i].id == LAT || symtab[i].id == LQES)
				{
				  iscmd = 1;
				}
			    }
			}
		      else
			{
			  if (!strncmp (in + count, symstream + symtab[i].ptr, symtab[i].len))
			    {
			      tokenstream[tokens++] = i;
			      count += symtab[i].len;
			      found = 1;
			    }
			}
		    }
		}
	    }
	  /* we searched but did not find */

	  if (!found)
	    {
	      i = count;
#ifdef DEBUG
	      fprintf (stderr, "i=count=%d, in[i]=<%c> in+i=<%s>", i, in[i], in + i);
#endif
	      while (isalnum (in[i])
		     || (in[i] == '+')
		     || (in[i] == '-')
		     || (in[i] == '*')
		     || (in[i] == '%')
		     || (in[i] == '&')
		     || (in[i] == '?')
		     || (in[i] == '_')
		     || (in[i] == '/')
		     || (in[i] == ':')
		     || (in[i] == '@')
		     || (in[i] == '{')
		     || (in[i] == '}')
		     || (in[i] == ']')
		     || (in[i] == '[')
		     || (in[i] == '(')
		     || (in[i] == ')')
		     || (in[i] == '^')
		     || (in[i] == '!')
		     || (in[i] == ',')
		     || (in[i] == '.')
		     || (in[i] == '$')
		     || (in[i] == '~')
		     || (in[i] == '\\'))
		{
		  i++;
		}
#ifdef DEBUG
	      fprintf (stderr, "i=%d, in[i]=<%c> in+i=<%s>", i, in[i], in + i);
#endif
	      if (i > count)
		{
#ifdef DEBUG
		  fprintf (stderr, "Creating new symbol table entry ->%s of length %d\n", in + count, i - count);
#endif
		  symtab[symtabcount].ptr = symstreamcount;
		  strncpy (symstream + symstreamcount, in + count, i - count);
		  /* cosmetic  strcat(symstream+symstreamcount,"\0"); */
		  symstreamcount += (i - count);

		  symtab[symtabcount].len = (i - count);
		  symtab[symtabcount].res = 0;
		  symtab[symtabcount].ins = 0;

		  if (isalpha (in[count]) && (in[count + 1] == ':'))
		    {
#ifdef DEBUG
		      fprintf (stderr, "Found request for new drive - <%c%c%c>\n", in[count]
			       ,in[count + 1]
			       ,in[count + 2]);
#endif
		      symtab[symtabcount].id = LND;
		    }
		  else if (in[count] == ':')
		    {
		      symtab[symtabcount].id = LLAB;
		    }
		  else if (in[count] == '/')
		    {
		      symtab[symtabcount].id = LSWI;
		    }
		  else
		    {
		      symtab[symtabcount].id = LELS;
		    }

		  tokenstream[tokens++] = symtabcount;
		  symtabcount++;

		  count += (i - count);
		}
	      /* for chars which were not matched */
	      else
		{
		  if (in[count] != '\0')
		    {
#ifdef DEBUG
		      fprintf (stderr, "Ignorning unknown character val=%d\n", in[count]);
#endif
		      count++;
		      /* not serious - just to tell calling routine */
		      retval++;
		    }
		}
	    }
	  state = 0;
#ifdef DEBUG
	  fprintf (stderr, "In[Count=%d]=<%c>\n", count, in[count]);
#endif
	  break;
	case 2:
	  i = count;
	  /* no point in searching symbol table */

	  while ((in[i] != '"') &&
		 (in[i] != '\0') &&
		 (in[i] != '\n') &&
		 (in[i] != '\r'))
	    i++;

	  if (i > count)
	    {
#ifdef DEBUG
	      fprintf (stderr, "Creating new symbol table entry (string) <%s> of length <%d>\n", in + count, i - count);
#endif
	      symtab[symtabcount].ptr = symstreamcount;
	      strncpy (symstream + symstreamcount, in + count, i - count);
	      /* not cosmetic: strcat(symstream+symstreamcount,"\0"); */
	      symstreamcount += (i - count);

	      symtab[symtabcount].len = (i - count);
	      symtab[symtabcount].res = 0;
	      symtab[symtabcount].ins = 0;

	      symtab[symtabcount].id = LSTR;

	      tokenstream[tokens++] = symtabcount;
	      symtabcount++;

	      count += (i - count);
	    }
	  count++;
	  state = 0;
	  break;
	case 3:
	  i = 0;
	  while (isalnum (in[i + count]) || in[i + count] == '_')
	    i++;
	  if (i)
	    {
	      strncpy (symstream + symstreamcount + 1, in + count, i);
	      symstream[symstreamcount + 1 + i] = '\0';
#ifdef DEBUG
	      fprintf (stderr, "Added env <%s> of len <%d>\n", symstream + symstreamcount + 1, i);
#endif
	      if (NULL != (tptr = getenv (symstream + symstreamcount + 1)))
		{
#ifdef DEBUG
		  fprintf (stderr, "Got env value <%s>\n", tptr);
#endif
		  strcpy (symstream + symstreamcount, tptr);
		  symtab[symtabcount].len = strlen (tptr);

		  if (isalpha (tptr[0]) && (tptr[1] == ':'))
		    {
		      symtab[symtabcount].id = LND;
		    }
		  else if (tptr[0] == ':')
		    {
		      symtab[symtabcount].id = LLAB;
		    }
		  else if (tptr[0] == '/')
		    {
		      symtab[symtabcount].id = LSWI;
		    }
		  else
		    {
		      symtab[symtabcount].id = LELS;
		    }

		  symtab[symtabcount].ptr = symstreamcount;
		  symstreamcount += strlen (tptr);
		}
	      else
		{
		  symstream[symstreamcount] = '%';
		  symtab[symtabcount].len = i + 1;
		  symtab[symtabcount].id = LELS;
		  symtab[symtabcount].ptr = symstreamcount;
		  symstreamcount += (i + 1);
		}
	      symtab[symtabcount].res = 0;
	      symtab[symtabcount].ins = 0;
	      tokenstream[tokens++] = symtabcount;
	      symtabcount++;
	      count += i;
	    }
	  state = 0;
	  break;
	}
/* catch some errors */
      if (tokens == TOKENS - 1)
	{
	  return -1;
	}
      if (symtabcount == SYMBOLS - 2)
	{
	  return -1;
	}
      if (symstreamcount > (SYMBOLS - 2) * 8)
	{
	  return -1;
	}
    }

  symtab[symtabcount].id = LEM;

  tokenstream[tokens] = symtabcount;
  tokencount = tokens;

  return retval;
}

#ifdef DIAGNOSE

int 
symdump ()
{
#ifdef DEBUG
  int i, j;
#endif
  fprintf (stderr, "DUMPING SYMBOL TABLE:\n");
#ifdef DEBUG
  for (i = 0; i < symtabcount; i++)
    {
      fprintf (stderr, "String ");
      for (j = 0; j < symtab[i].len; j++)
	{
	  fprintf (stderr, "%c", symstream[symtab[i].ptr + j]);
	}
      fprintf (stderr, " (id=%d) at %d(+%d) ", symtab[i].id
	       ,symtab[i].ptr
	       ,symtab[i].len);
      if (symtab[i].ins)
	fprintf (stderr, "is case insensitive ");
      if (symtab[i].res)
	fprintf (stderr, "is reserved");
      fprintf (stderr, "\n");
    }
#endif
  fprintf (stderr, "RAW : <%s>\n", symstream);
  fprintf (stderr, "---------END--------\n");
  return 0;
}


int 
tokendump ()
{
  int i = 0, j = 0;
  fprintf (stderr, "DUMPING TOKEN STREAM:\n");
  while (i < tokencount)
    {
      fprintf (stderr, "Token %d (pos=%d) is associated with string <",
	       symtab[tokenstream[i]].id, tokenstream[i]);
      for (j = 0; j < symtab[tokenstream[i]].len; j++)
	fprintf (stderr, "%c", symstream[symtab[tokenstream[i]].ptr + j]);
      fprintf (stderr, ">\n");
      i++;
    }
  fprintf (stderr, "RAW stream : ");
  for (i = 0; i <= tokencount; i++)
    fprintf (stderr, "%d ", tokenstream[i]);
  fprintf (stderr, "\n");
  fprintf (stderr, "---------END--------\n");
  return 0;
}

#endif
