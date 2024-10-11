/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "defs.h"

int 
addalias (char *as, int al, char *ds, int dl)
{
  struct ali *nptr;

  char *aname, *adef;

#ifdef DEBUG
  fprintf (stderr, "Head is [%p]\n", head);
#endif

  nptr = (struct ali *) malloc (sizeof (struct ali));
  if (nptr == NULL)
    return 1;

  aname = (char *) malloc (sizeof (char) * CEM (al + 2, 16));
  if (aname == NULL)
    {
      free (nptr);
      return 1;
    }

  adef = (char *) malloc (sizeof (char) * CEM (dl + 2, 16));
  if (adef == NULL)
    {
      free (nptr);
      free (aname);
      return 1;
    }

#ifdef DEBUG
  fprintf (stderr, "mallocs worked\n");
#endif

  strncpy (aname, as, al);
  aname[al] = '\0';
  strncpy (adef, ds, dl);
  adef[dl] = '\0';

#ifdef DEBUG
  fprintf (stderr, "aname is <%s>, adef is <%s>, al is %d\n", aname, adef, al);
#endif

  nptr->len = al;
  nptr->aln = aname;
  nptr->dfn = adef;
  nptr->next = head;

#ifdef DEBUG
  fprintf (stderr, "assigned to nptr\n");
#endif

  head = nptr;

  return 0;
}

int 
zapalias (char *as, int al)
{
  struct ali *fptr, *nptr;

  fptr = head;
  nptr = NULL;

  while (fptr != NULL)
    {
      if ((!strncmp (as, fptr->aln, al)) && (al == fptr->len))
	{
#ifdef DEBUG
	  fprintf (stderr, "Found match with <%s> equal <%s> of len %d\n", as, fptr->aln, al);
#endif
	  if (nptr == NULL)
	    head = fptr->next;
	  else
	    nptr->next = fptr->next;
	  free (fptr);
	  return 0;
	}
      nptr = fptr;
      fptr = fptr->next;
    }

  return 1;
}

int 
cleanalias ()
{
  struct ali *fptr, *tptr;

  fptr = head;
  while (fptr != NULL)
    {
      tptr = fptr;
      fptr = fptr->next;
      free (tptr);
    }
  return 0;
}

int 
findalias (char *ss, int sc, int sl)
{
  struct ali *fptr;
  int i;

  fptr = head;
  while (fptr != NULL)
    {
      if ((!strncmp (ss, fptr->aln, sc)) && (sc == fptr->len))
	{
#ifdef DEBUG
	  fprintf (stderr, "Found match with <%s> equal <%s> of len %d\n", ss, fptr->aln, sc);
#endif
	  if (strlen (fptr->dfn) > (sl - 2))
	    return 1;
	  strcpy (ss, fptr->dfn);

#ifdef DEBUG
	  fprintf (stderr, "Substituted <%s>\n", ss);
#endif
	  i = 0;
	  while (ss[i] != '\0')
	    {
	      if (isspace (ss[i]) && i && (ss[i - 1] != '\0'))
		ss[i] = '\0';
	      i++;
	    }
	  ss[i + 1] = '\0';

	  return 0;
	}
      else
	{
#ifdef DEBUG
	  fprintf (stderr, "skipping <ss=%s>, <aln=%s>, <sc=%d> and <len=%d>\n", ss,
		   fptr->aln, sc, fptr->len);
#endif
	}
      fptr = fptr->next;
    }

  return 1;
}

int 
aliasdump (FILE * out)
{
  struct ali *tptr;
  tptr = head;
  while (tptr != NULL)
    {
      fprintf (out, "%s=%s\n", tptr->aln, tptr->dfn);
      tptr = tptr->next;
    }
  return 0;
}

int 
naliasdump (FILE * out)
{
  struct ali *tptr;
  tptr = head;
  while (tptr != NULL)
    {
      fprintf (out, "%s ", tptr->aln);
      tptr = tptr->next;
    }
  fprintf (out, "\n");
  return 0;
}

#ifdef OODEBUG

int 
main ()
{
  char a[40], b[40];
  int i;

  while (a[0] != 'q')
    {
      printf ("Enter alias: ");
      fgets (a, 40, stdin);

      i = 0;
      while (!iscntrl (a[i]))
	i++;
      a[i] = '\0';

      printf ("Enter definition: ");
      fgets (b, 40, stdin);

      i = 0;
      while (!iscntrl (b[i]))
	i++;
      b[i] = '\0';

      addalias (a, strlen (a), b, strlen (b));

      printf ("Enter alias to be found: ");
      fgets (b, 40, stdin);

      i = 0;
      while (!iscntrl (b[i]))
	i++;
      b[i] = '\0';

      if (findalias (b, strlen (b), 40))
	printf ("Findalias failed\n");
      else
	printf ("Its definition is <%s>\n", b);
    }



  return 0;
}

#endif
