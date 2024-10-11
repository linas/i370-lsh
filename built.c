/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include "defs.h"

int 
lren (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int maskbase, destbase, srcbase;
  char optconf, all = 0, quit = 0, semiall = 0, unique = 0;
  int ok, pcount, temptc;
  struct stat fst;

  if (nochange)
    {
      plerr (PLACD);
    }
  else
    {
      optconf = goptconf;
      temptc = tc;
      pcount = 0;

      do
	{
/* if we have not hit a parameter then shift */
	  if (!pcount)
	    tc++;
/* we search the entire thing for a switch  */
	  temptc++;
	  ok = checkopt (temptc, tk, tks);
	  switch (ok)
	    {
/* something neither a switch or terminator */
	    case -1:
	      pcount++;
	      break;
/* terminator of command sequence           */
	    case 0:		/* no point in setting ok=0 */
	      break;
/* Valid switch                             */
	    case 'C':
	    case 'I':
	    case 'P':
	    case 'c':
	    case 'i':
	    case 'p':
	      optconf = 1;
	      break;
/* Invalid switch                           */
	    default:
	      ok = 0;
	      pcount = 10;
	      break;
	    }
	}
      while (ok);

      if ((symtab[tks[tc]].id == LND || symtab[tks[tc]].id == LELS) && (pcount == 2))
	{
	  if (symtab[tks[tc]].len >= cmdvc)
	    {
	      plerr (PLMAE);
	    }
	  else
	    {
	      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      cmdv[symtab[tks[tc]].len] = '\0';
	      tc++;
#ifdef DEBUG
	      fprintf (stderr, "First parameter is LND or LELS <%s>\n", cmdv);
#endif
	      if (!fromlo (cmdv, cmdvc))
		{
		  maskbase = strlen (cmdv);
#ifdef DEBUG
		  fprintf (stderr, "First parameter converted to <%s>\n", cmdv);
#endif
		  if (expwild (cmdv, cmdvc))
		    {
#ifdef DEBUG
		      fprintf (stderr, "First parameter is a wild card\n");
#endif
		      if (expr (cmdv, cmdv, cmdvc, OOPLA))
			{
			  plerr (PLMAE);
			}
		      maskbase = expcount (cmdv, 0);
		    }
		  if (maskbase)
		    {
		      maskbase++;
#ifdef DEBUG
		      fprintf (stderr, "We have <%s> as first file : total length is %d\n", cmdv, maskbase);
#endif
		      if (symtab[tks[tc]].len < cmdvc - maskbase)
			{
			  strncpy (cmdv + maskbase, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
			  cmdv[maskbase + symtab[tks[tc]].len] = '\0';
			  tc++;

			  if (!fromlo (cmdv + maskbase, cmdvc - maskbase))
			    {
#ifdef DEBUG
			      fprintf (stderr, "Managed to aquire the destination <%s>\n", cmdv + maskbase);
#endif

/*here we have a list of source files at cmdv[0] and their destination at 
   cmdv[maskbase] -- the source has been wild card expanded, but the destination
   is still possibly a wildcard */

/* WARNING - potential for Heisenbug if we run out of space on cmdv -
   maybe have a conditional realloc here ? 
 */

#ifdef DEBUG
			      fprintf (stderr, "Source starts at <%s> while destination is <%s>\n", cmdv, cmdv + maskbase);
#endif
			      destbase = maskbase + strlen (cmdv + maskbase) + 1;
			      srcbase = 0;
			      while (srcbase < maskbase)
				{
#ifdef DEBUG
				  fprintf (stderr, "Working with source <%s> at <%d>\n", cmdv + srcbase, srcbase);
#endif

				  if (mangle (cmdv + srcbase, cmdv + maskbase, cmdv + destbase, cmdvc - destbase))
				    {
				      plerr (PLMAE);
				    }
				  else
				    {
#ifdef DEBUG
				      fprintf (stderr, "###Will move #<%s># to #<%s>###\n", cmdv + srcbase, cmdv + destbase);
#endif
				      if (optconf)
					{
					  fprintf (out, "Move %s to %s ", cmdv + srcbase, cmdv + destbase);
					  if (!stat (cmdv + destbase, &fst))
					    {
					      fprintf (out, "which exits ");
					      unique = 1;
					    }
					  fprintf (out, "[ynaqu] ");
					  if (all)
					    {
					      if (!(semiall && unique))
						{
						  if (rename (cmdv + srcbase, cmdv + destbase))
						    {
						      plerr (PLACD);
						    }
						  else
						    {
						      fprintf (out, "OK\n");
						    }
						}
					      else
						{
						  fprintf (out, "NO\n");
						}
					    }
					  else if (quit)
					    {
					      fprintf (out, "NO\n");
					    }
					  else
					    {
					      switch (readc (in))
						{
						case 'u':
						case 'U':
						  semiall = 1;
						case 'a':
						case 'A':
						  all = 1;
						  if (!(semiall && unique))
						    {
						      if (rename (cmdv + srcbase, cmdv + destbase))
							{
							  plerr (PLACD);
							}
						      else
							{
							  fprintf (out, "OK\n");
							}
						    }
						  else
						    {
						      fprintf (out, "NO\n");
						    }
						  break;
						case 'y':
						case 'Y':
						  if (rename (cmdv + srcbase, cmdv + destbase))
						    {
						      plerr (PLACD);
						    }
						  else
						    {
						      fprintf (out, "OK\n");
						    }
						  break;
						case 'q':
						case 'Q':
						  quit = 1;
						default:
						  fprintf (out, "NO\n");
						  break;
						}
					    }
					}
				      else
					{
#ifdef DEBUG
					  fprintf (stderr, "Doing things without confirmation\n");
#endif
					  if (rename (cmdv + srcbase, cmdv + destbase))
					    plerr (PLACD);
					}
				    }

				  srcbase += (strlen (cmdv + srcbase) + 1);
				}

			    }
			  else
			    {
			      plerr (PLFNF);
			    }
			}
		      else
			{
			  plerr (PLMAE);
			}
		    }
		  else
		    {
		      plerr (PLFNF);
		    }
		}
	      else
		{
		  plerr (PLFNF);
		}
	    }
	}
      else
	{
	  plerr (PLPFN);
	}
    }
  return tc;
}

int 
lcal (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int success = 0;
  int isctty = 0;
  FILE *fp;

  if (toupper (symstream[symtab[tks[tc]].ptr + 1]) == 'T')
    {
#ifdef DEBUG
      fprintf (stderr, "Working with ctty\n");
#endif
      isctty = 1;
    }

  if (symtab[tks[tc]].id != LQES || (quitstat == testtrue))
    {
#ifdef DEBUG
      fprintf (stderr, "Will execute command file\n");
#endif
      tc++;

      if (calldep < CALLDEP)
	{
	  switch (symtab[tks[tc]].id)
	    {
	    case LELS:
	    case LND:
	      if (symtab[tks[tc]].len < cmdvc)
		{
		  strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
		  cmdv[symtab[tks[tc]].len] = '\0';
		  if (!fromlo (cmdv, cmdvc))
		    {
		      findalias (cmdv, strlen (cmdv), cmdvc);
		      if (isctty)
			{
			  success = 1;
			  fp = fopen (cmdv, "a");
			  if (fp != NULL)
			    {
			      if (dobatch)
				{
				  dup2 (fileno (fp), STDERR_FILENO);
				  setsid ();
				}
			      if (!cmdrun (cmdv, fp))
				{
/* buggy... should we save tks[] since cmdrun is liable to zap it ? */
				}
			      fclose (fp);
			    }
			  else
			    {
			      plerr (PLACD);
			    }
/* hmm... how do I let go of my old terminal ?                      */
/*                          vhangup();                              */
			}
		      else
			{
			  if (!cmdrun (cmdv, out))
			    {
/* buggy... should we save tks[] since cmdrun is liable to zap it ? */
			      success = 1;
			    }
			}
		    }
		}
	      break;
	    case LSEP:
	    case LEM:
	      success = 1;
	      break;
	    }
	  if (!success)
	    {
#ifdef DEBUG
	      fprintf (stderr, "Could not find file... running command instead\n");
#endif
	      while (symtab[tks[tc]].id == LAT || (symtab[tks[tc]].id == LQES && testtrue == quitstat))
		tc++;
	      tc = cmd (tc, tk, tks, in, out);
	    }
	}
      else
	{
	  plerr (PLSRE);
	}
    }
  else
    {
#ifdef DEBUG
      fprintf (stderr, "Will not execute command file\n");
#endif
      if (junior)
	{
	  char cpbuf[CPBUF];
	  int count;
	  while ((count = fread (cpbuf, sizeof (char), CPBUF, in)) > 0)
	    {
	      fwrite (cpbuf, sizeof (char), count, out);
	    }
#ifdef DEBUG
	  fprintf (stderr, "Downstream of a pipe: echoed stdin to stdout\n");
#endif

	}
    }
  return tc;
}

int 
lech (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int quit = 0;
  int i = 0;
  int na = 1;

  tc++;
  while (!quit)
    {
      switch (symtab[tks[tc]].id)
	{
	case LAMP:
	case LEM:
	case LSEP:
	case LTO:
	case LFRO:
	case LPIP:
	  quit = 1;
	  break;
	default:
	  for (i = 0; i < symtab[tks[tc]].len; i++)
	    {
	      fputc (symstream[symtab[tks[tc]].ptr + i], out);
	    }
	  fputc (' ', out);
	  tc++;
	  na = 0;
	  break;
	}
    }

  if (na)
    {
      fprintf (out, "Echo is ON");
    }
  fputc ('\n', out);

  return tc;
}

int 
lrem (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int quit = 0;

  while (!quit)
    {
      switch (symtab[tks[tc]].id)
	{
	case LEM:
	case LSEP:
	  quit = 1;
	  break;
	default:
	  tc++;
	  break;
	}
    }

  return tc;
}

int 
lcop (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int track = 0;
  int app = 0;

  int max = 0;
  int offs = 0;
  int prevz = 0;
  int drive;
  int isdir = 0;
  int extra;
  int count = 0;

  int srcbase = 0;
  int destbase = 0;

  char *tstr;
  struct stat fst;

  drive = drivecur;
  tc++;
  if (symtab[tks[tc]].id == LND || symtab[tks[tc]].id == LELS)
    {
      if (symtab[tks[tc]].len >= cmdvc)
	{
	  plerr (PLMAE);
	}
      else
	{
	  strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	  cmdv[symtab[tks[tc]].len] = '\0';
#ifdef DEBUG
	  fprintf (stderr, "First parameter is LND or LELS <%s>\n", cmdv);
#endif
	  if (!fromlo (cmdv, cmdvc))
	    {
#ifdef DEBUG
	      fprintf (stderr, "First parameter converted to <%s>\n", cmdv);
#endif
	      if (expwild (cmdv, cmdvc))
		{
#ifdef DEBUG
		  fprintf (stderr, "First parameter is a wild card\n");
#endif
		  if (expr (cmdv, cmdv, cmdvc, OOREA | OOPLA))
		    {
		      plerr (PLMAE);
		    }
		  tc++;
		}
	      else
		{
		  track = (strlen (cmdv) + 1);
		  tc++;
		  while ((symtab[tks[tc]].id == LPLU) && (cmdvc > track))
		    {
		      app = 1;
#ifdef DEBUG
		      fprintf (stderr, "Found a plus\n");
#endif
		      tc++;
		      if (symtab[tks[tc]].len >= cmdvc - track)
			{
			  plerr (PLMAE);
			}
		      else
			{
			  if (symtab[tks[tc]].id == LND || symtab[tks[tc]].id == LELS)
			    {
			      strncpy (cmdv + track, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
			      cmdv[track + symtab[tks[tc]].len] = '\0';
			      if (!fromlo (cmdv + track, cmdvc - track))
				{
#ifdef DEBUG
				  fprintf (stderr, "Added and converted <%s>\n", cmdv + track);
#endif
				  track += (strlen (cmdv + track) + 1);
				}
			      else
				{
				  plerr (PLFNF);
				}
			    }
			  else
			    {
			      plerr (PLPFN);
			      tc--;
			    }
			}
		      tc++;
		    }
		  cmdv[track] = '\0';
		}
	      if (cmdv[0] == '\0')
		{
		  plerr (PLFNF);
		}
	      else
		{
#ifdef DEBUG
		  fprintf (stderr, "The source file(s) : <%s>\n", cmdv);
#endif
		  do
		    {
		      if (cmdv[offs] == '\0')
			{
			  if ((offs - prevz) > max)
			    {
#ifdef DEBUG
			      fprintf (stderr, "Updating max from %d to", max);
#endif
			      max = offs - prevz;
#ifdef DEBUG
			      fprintf (stderr, " %d\n", max);
#endif
			    }
			  prevz = offs + 1;
#ifdef DEBUG
			  fprintf (stderr, "Looking at next item <%s>\n", cmdv + prevz);
#endif
			}
		      offs++;
		    }
		  while (!(cmdv[offs] == '\0' && cmdv[offs - 1] == '\0'));
		  max++;
#ifdef DEBUG
		  fprintf (stderr, "Final maximum is %d\n", max);
#endif

		  switch (symtab[tks[tc]].id)
		    {
		    case LND:
		      drive = toupper (symstream[symtab[tks[tc]].ptr]) - 'A';
		    case LELS:
		      max += (2 + drivetab[drive].plen + symtab[tks[tc]].len);
#ifdef DEBUG
		      fprintf (stderr, "Will allocate %d bytes \n", max);
#endif
		      tstr = (char *) malloc (sizeof (char) * CEM (max, 16));
		      if (tstr != NULL)
			{
			  strncpy (tstr, symstream + symtab[tks[tc]].ptr,
				   symtab[tks[tc]].len);
			  tstr[symtab[tks[tc]].len] = '\0';
			  if (!fromlo (tstr, max))
			    {
#ifdef DEBUG
			      fprintf (stderr, "fromlo converted destination to <%s>\n", tstr);
#endif
			      if (expwild (tstr, strlen (tstr)))
				{
#ifdef DEBUG
				  fprintf (stderr, "<%s> is a wild expression\n", tstr);
#endif

/* we have the source files (wild card expanded) at cmdv[0], and the 
   destination, which we know is an unexpanded wild card at tstr[0]
 */
#ifdef DEBUG
				  fprintf (stderr, "source starts at <%s>, mask is <%s>\n", cmdv, tstr);
#endif
				  while (!(cmdv[destbase] == '\0' && cmdv[destbase - 1] == '\0') || (!destbase && cmdv[destbase] != '\0'))
				    {
				      destbase++;
				    }

#ifdef DEBUG
				  fprintf (stderr, "Destination is at <%d> of length <%d>\n", destbase, cmdvc - destbase);
#endif

				  while (srcbase < destbase)
				    {
				      if (mangle (cmdv + srcbase, tstr, cmdv + destbase, cmdvc - destbase))
					{
					  plerr (PLMAE);
					}
				      else
					{
#ifdef DEBUG
					  fprintf (stderr, "will copy <%s> to <%s>\n", cmdv + srcbase, cmdv + destbase);
#endif
					  /* do copy here */
					  fprintf (out, "%s\n", cmdv + srcbase);
					  if (dcp (cmdv + destbase, cmdv + srcbase, "w"))
					    {
					      plerr (PLACD);
					    }
					  else
					    {
					      count++;
					    }
					}
				      srcbase += (strlen (cmdv + srcbase) + 1);
				    }
				}
			      else
				{
#ifdef DEBUG
				  fprintf (stderr, "<%s> is NOT a wild expression\n", tstr);
#endif
				  if (!stat (tstr, &fst))
				    {
				      if ((fst.st_mode & S_IFMT) == S_IFDIR)
					{
					  isdir = 1;
					}
				    }

				  offs = 0;
				  if (isdir)
				    {
				      if (app)
					plerr (PLSER);
				      else
					{
					  prevz = strlen (tstr);
#ifdef DEBUG
					  fprintf (stderr, "Dir <%s> has length %d\n", tstr, prevz);
#endif
					  if (tstr[prevz - 1] != '/')
					    {
					      tstr[prevz] = '/';
					      prevz++;
					      tstr[prevz] = '\0';
#ifdef DEBUG
					      fprintf (stderr, "Added a slash : now dir <%s> has length %d\n", tstr, prevz);
#endif
					    }
					  while (!(cmdv[offs] == '\0' && cmdv[offs + 1] == '\0'))
					    {
					      if (cmdv[offs - 1] == '\0' || (!offs))
						{
						  fprintf (out, "%s\n", cmdv + offs);
						  extra = strlen (cmdv + offs);
						  while (extra != (-1) && cmdv[offs + extra] != '/')
						    {
						      extra--;
						    }
						  extra++;
						  strcpy (tstr + prevz, cmdv + offs + extra);
#ifdef DEBUG
						  fprintf (stderr, "Going to copy <%s> to <%s>\n", cmdv + offs, tstr);
#endif
						  if (dcp (tstr, cmdv + offs, "w"))
						    plerr (PLACD);
						  else
						    count++;
						}
					      offs++;
					    }
					}
				    }
				  else
				    {
				      while (!(cmdv[offs] == '\0' && cmdv[offs + 1] == '\0'))
					{
					  if (cmdv[offs - 1] == '\0' || (!offs))
					    {
					      fprintf (out, "%s\n", cmdv + offs);
					      if (dcp (tstr, cmdv + offs, "a"))
						plerr (PLACD);
					      else
						count++;
					    }
					  offs++;
					}
				    }
				}
			      fprintf (out, " %8d file(s) copied\n", count);
			    }
			  else
			    {
			      plerr (PLPNF);
			    }
			  free (tstr);
			}
		      else
			{
			  plerr (PLMAE);
			}
		      break;
		    case LTO:
		    case LFRO:
		    case LPIP:
		    case LSEP:
		    case LEM:
		      tstr = (char *) malloc (sizeof (char) * CEM (max, 16));
		      if (tstr != NULL)
			{
			  offs = 0;
			  if (app)
			    {
			      while (cmdv[offs] != '\0')
				offs++;
			      while (!(cmdv[offs] == '\0' && cmdv[offs + 1] == '\0'))
				{
				  if (cmdv[offs] == '\0')
				    {
				      fprintf (out, "%s\n", cmdv + offs + 1);
				      if (dcp (cmdv, cmdv + offs + 1, "a"))
					plerr (PLACD);
				      else
					count++;
				    }
				  offs++;
				}
			    }
			  else
			    {
			      while (!(cmdv[offs] == '\0' && cmdv[offs + 1] == '\0'))
				{
				  if (cmdv[offs] == '\0' || (!offs))
				    {
				      if (offs)
					offs++;
				      strcpy (tstr, cmdv + offs);
				      prevz = strlen (tstr);
				      while ((prevz >= 0) && (tstr[prevz] != '/'))
					prevz--;
				      prevz++;
				      fprintf (out, "%s\n", tstr + prevz);
				      if (dcp (tstr + prevz, cmdv + offs, "w"))
					plerr (PLACD);
				      else
					count++;
				      if (offs)
					offs--;
				    }
				  offs++;
				}
			    }
			  fprintf (out, " %8d file(s) copied\n", count);
			  free (tstr);
			}
		      else
			{
			  plerr (PLMAE);
			}
		      break;
		    default:
		      plerr (PLPFN);
		      break;
		    }
		}
	    }
	  else
	    {
	      plerr (PLFNF);
	    }
	}
    }
  else
    {
      plerr (PLPFN);
    }
  return tc;
}

int 
lpau (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int count = 0;

  tc++;
  switch (symtab[tks[tc]].id)
    {
    case LSTR:
      while (count < symtab[tks[tc]].len)
	{
	  fputc (symstream[symtab[tks[tc]].ptr + count], out);
	  count++;
	}
      break;
    default:
      plerr (PLPAK);
      break;
    }
  readc (in);
  return tc;
}

int 
lpat (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  char *tptr;
  int count, quit, temp;

  count = 0;
  quit = 0;
  temp = 0;
  tc++;
  cmdv[0] = '\0';

  while (!quit)
    {
      switch (symtab[tks[tc]].id)
	{
	case LPIP:
	case LEM:
	case LTO:
	case LFRO:
	  if (!count)
	    {
	      fprintf (out, "PATH=");
	      if (NULL != (tptr = getenv ("PATH")))
		fprintf (out, "%s\n", tptr);
	      else
		fprintf (out, "\n");
	    }
	  quit = 1;
	  break;
	case LSTR:
	  strncpy (cmdv + count, symstream + symtab[tks[tc]].ptr,
		   symtab[tks[tc]].len);
	  cmdv[symtab[tks[tc]].len] = '\0';
	  count = strlen (cmdv);
	  quit = 1;
	  break;
	case LSEP:
	  if (!count)
	    {
	      cmdv[0] = '.';
	      count++;
	    }
	  cmdv[count] = ':';
	  count++;
	  cmdv[count] = '\0';
	  while (symtab[tks[temp]].id != LEM)
	    temp++;
	  tks[tc] = temp;
	  tc++;
	  break;
	case LASS:
	  if (!count)
	    tc++;
	  else
	    quit = 1;
	  break;
	default:
	  strncpy (cmdv + count, symstream + symtab[tks[tc]].ptr,
		   symtab[tks[tc]].len);
	  cmdv[count + symtab[tks[tc]].len] = '\0';
	  if (fromlo (cmdv + count, cmdvc - count))
	    quit = 1;
	  count = strlen (cmdv);
#ifdef DEBUG
	  fprintf (stderr, "Now cmdv is <%s> - len now is %d\n", cmdv, count);
#endif
	  tc++;
	  break;
	}
    }
  if (count)
    {
      if (noset)
	plerr (PLACD);
      else
	{
	  if (count == 2)
	    cmdv[1] = '\0';
	  setenv ("PATH", cmdv, 1);
#ifdef DEBUG
	  fprintf (stderr, "set PATH to <%s>\n", cmdv);
#endif
	}
    }
  return tc;
}

int 
lali (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int defnptr;

  tc++;
  if (symtab[tks[tc]].id == LELS && (!noalias))
    {
      defnptr = tc;
      tc++;
      if (symtab[tks[tc]].id == LASS)
	tc++;
      switch (symtab[tks[tc]].id)
	{
	case LELS:
	case LND:
	  strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	  cmdv[symtab[tks[tc]].len] = '\0';
	  if (!fromlo (cmdv, cmdvc))
	    {
	      if (addalias (symstream + symtab[tks[defnptr]].ptr,
			    symtab[tks[defnptr]].len,
			    cmdv,
			    strlen (cmdv)
		  ))
		plerr (PLMAE);
	    }
	  else
	    {
	      plerr (PLFNF);
	    }
	  break;
	case LSWI:
	case LSTR:
	  if (addalias (symstream + symtab[tks[defnptr]].ptr,
			symtab[tks[defnptr]].len,
			symstream + symtab[tks[tc]].ptr,
			symtab[tks[tc]].len))
	    plerr (PLMAE);
	  break;
	default:
	  plerr (PLPFN);
	  break;
	}
    }
  else
    {
      if (symtab[tks[tc]].id == LEM
	  || symtab[tks[tc]].id == LPIP
	  || symtab[tks[tc]].id == LSEP
	  || symtab[tks[tc]].id == LFRO
	  || symtab[tks[tc]].id == LTO
	  || symtab[tks[tc]].id == LAMP)
	{
	  if (noalias)
	    naliasdump (out);
	  else
	    aliasdump (out);
	}
      else
	{
	  if (noalias)
	    plerr (PLACD);
	  else
	    plerr (PLPFN);
	}
      tc++;
    }
  return tc;
}

int 
luna (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;
  if (noalias)
    {
      plerr (PLACD);
    }
  else
    {
      if (symtab[tks[tc]].id == LELS)
	{
#ifdef DEBUG
	  fprintf (stderr, "Intent on zapping <%s>\n", symstream + symtab[tks[tc]].ptr);
#endif
	  if (zapalias (symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len))
	    {
	      plerr (PLFNF);
	    }
	  tc++;
	}
      else
	{
	  plerr (PLFNF);
	}
    }
  return tc;
}

#ifdef DIAGNOSE

int 
ldmp (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;
  symdump ();
  tokendump ();
  pathdump (out);
  aliasdump (out);
  return tc;
}
#endif

int 
lcd (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int mz;
  int nd;
  char *tmp;

  switch (symtab[tks[tc + 1]].id)
    {
    case LELS:
      mz = symtab[tks[tc + 1]].len;
      mz += strlen (drivetab[drivecur].path);
      mz += 4;
      mz = CEM (mz, 16);
      tmp = (char *) malloc (mz * sizeof (char));
      if (tmp != NULL)
	{
	  strncpy (tmp, symstream + symtab[tks[tc + 1]].ptr, symtab[tks[tc + 1]].len);
	  tmp[symtab[tks[tc + 1]].len] = '\0';
	  if (!fromlo (tmp, mz))
	    {
	      if (!chdir (tmp))
		{
		  if (getcwd (tmp, mz) != NULL)
		    {
		      if (setlo (drivecur, tmp))
			{
			  chdir (drivetab[drivecur].path);
			  plerr (PLIND);
			}
		    }
		  else
		    {
		      chdir (drivetab[drivecur].path);
		      plerr (PLIND);
		    }
		}
	      else
		{
		  plerr (PLIND);
		}
	    }
	  else
	    {
	      plerr (PLIND);
	    }
	  free (tmp);
	}
      else
	{
	  plerr (PLMAE);
	}
      break;
    case LND:
      mz = symtab[tks[tc + 1]].len + 4;
      nd = toupper (symstream[symtab[tks[tc + 1]].ptr]) - 'A';
      if (drivetab[nd % DRIVES].valid)
	{
	  mz += strlen (drivetab[nd].path);
	  mz = CEM (mz, 16);
	  tmp = (char *) malloc (mz * sizeof (char));
	  if (tmp != NULL)
	    {
	      strncpy (tmp, symstream + symtab[tks[tc + 1]].ptr, symtab[tks[tc + 1]].len);
	      tmp[symtab[tks[tc + 1]].len] = '\0';
	      if (!fromlo (tmp, mz))
		{
		  if (!chdir (tmp))
		    {
		      if (getcwd (tmp, mz) != NULL)
			{
			  if (setlo (nd, tmp))
			    {
			      chdir (drivetab[drivecur].path);
			      plerr (PLIND);
			    }
			}
		      else
			{
			  plerr (PLIND);
			}
		      chdir (drivetab[drivecur].path);
		    }
		  else
		    {
		      plerr (PLIDS);
		    }
		}
	      else
		{
		  plerr (PLIDS);
		}
	      free (tmp);
	    }
	  else
	    {
	      plerr (PLMAE);
	    }
	}
      else
	{
	  plerr (PLIDS);
	}
      break;
    case LEM:
    case LTO:
    case LFRO:
    case LPIP:
    case LSEP:
      mz = 4 + drivetab[drivecur].plen;
      mz = CEM (mz, 16);
      tmp = (char *) malloc (mz * sizeof (char));
      if (tmp != NULL)
	{
	  if (getcwd (tmp, mz) != NULL)
	    {
	      if (!tolo (drivecur, tmp, mz))
		fprintf (out, "%s\n", tmp);
	    }
	  free (tmp);
	}
      else
	{
	  plerr (PLMAE);
	}
      tc--;
      break;
    default:
      plerr (PLPNF);
      break;
    }
  tc += 2;
  return tc;
}

int 
lnd (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int nd;
  nd = toupper (symstream[symtab[tks[tc]].ptr]) - 'A';

#ifdef DEBUG
  fprintf (stderr, "Now in lnd with drive <%d>\n", nd);
#endif

  if (drivetab[nd].valid)
    {
      if (!chdir (drivetab[nd].path))
	{
	  drivecur = nd;
#ifdef DEBUG
	  fprintf (stderr, "Changed to drive <%d> path <%s> \n", nd, drivetab[nd].path);
#endif
	}
      else
	{
	  plerr (PLIDS);
	}
    }
  else
    {
      plerr (PLIDS);
    }
  tc++;
  return tc;
}

int 
lcls (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  doodle = 0;

#ifdef USE_TERMCAP
  if (rsstr != NULL)
    fprintf (out, "%s", rsstr);
#else
/* This resets the console (I think).                              */
  fprintf (out, "%c%c%c", 22, 27, 'c');

/* Is this the ansi seqence for clearing the screen ?              */
  fprintf (out, "%c%c%c%c%c%c", 27, '[', 'H', 27, '[', 'J');
#endif

  tc++;
  return tc;
}

int 
ldat (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  char day[4];
  struct tm *tstr;
  time_t tt;

#ifdef DEBUG
  fprintf (stderr, "In ldat\n");
#endif

  tc++;

  if (checkopt (tc, tk, tks))
    {
      plerr (PLTMP);
    }
  else
    {
      time (&tt);
      tstr = localtime (&tt);

      switch (tstr->tm_wday)
	{
	case 0:
	  strcpy (day, "Sun");
	  break;
	case 1:
	  strcpy (day, "Mon");
	  break;
	case 2:
	  strcpy (day, "Tue");
	  break;
	case 3:
	  strcpy (day, "Wed");
	  break;
	case 4:
	  strcpy (day, "Thu");
	  break;
	case 5:
	  strcpy (day, "Fri");
	  break;
	case 6:
	  strcpy (day, "Sat");
	  break;
	}

      fprintf (out, "Current date is ");

      if (tstr->tm_year < 100)
	fprintf (out, "%s %02d-%02d-19%02d\n", day, (tstr->tm_mon) + 1, tstr->tm_mday, tstr->tm_year);

      else
	{
	  fprintf (out, "%s %02d-%02d-20%02d\n", day, (tstr->tm_mon) + 1, tstr->tm_mday, tstr->tm_year);
	}

    }
  return tc;
}

/* cbf  :
   returns 1 if we were passed part of a 
   directory and it managed to convert (shorten) and change to it
   returns 0 if it was not part of a dir
   returns -1 if it failed to change dirs   
 */

int 
cbf (char *ptr)
{
  int retval = 0;
  int j = 0, k;
  struct stat fst;

  j = strlen (ptr);

  while (j >= 0 && ptr[j] != '/')
    j--;
  if (j == (-1))
    {
#ifdef DEBUG
      fprintf (stderr, "<%s> does NOT contain a dir part\n", ptr);
#endif
      k = stat (ptr, &fst);
#ifdef DEBUG
      if (!k)
	fprintf (stderr, "But the entire <%s> is a dir\n", ptr);
#endif
    }
#ifdef DEBUG
  else
    fprintf (stderr, "<%s> does contain a dir part\n", ptr);
#endif

  if (j != (-1) || ((k == 0) && (j == (-1)) && (S_IFDIR & fst.st_mode)))
    {

#ifdef DEBUG
      fprintf (stderr, "<%s> is either a dir or contains a dir\n", ptr);
#endif

      retval = 1;
      ptr[j] = '\0';

#ifdef DEBUG
      fprintf (stderr, "We will change to this dir <%s>\n", ptr);
#endif
      if (j)
	{
	  if (chdir (ptr))
	    {
	      retval = (-1);
#ifdef DEBUG
	      perror ("In cbf() chdir failed");
#endif
	    }
	}
      else
	{
	  if (chdir ("/"))
	    {
	      retval = (-1);
#ifdef DEBUG
	      perror ("In cbf() chdir failed");
#endif
	    }
	}
      k = 0;
      j++;
      while (ptr[j] != '\0')
	{
	  ptr[k] = ptr[j];
	  j++;
	  k++;
	}
      ptr[k] = '\0';
#ifdef DEBUG
      fprintf (stderr, "The part that remains is <%s>\n", ptr);
#endif
    }
#ifdef DEBUG
  fprintf (stderr, "We will return %d\n", retval);
#endif
  return retval;
}

int 
ldir (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  static m = 12;
  int i = 0;
  int ltot = 0;
  int lcn = 1;
  int ff = 0;
  uid_t myid, mygid;
  struct stat fst;
  int j, changeback = 0;
  int donlist, statok, dcw;
  struct tm *ts;
  struct statfs sfs;
  unsigned long count = 0;
  unsigned long bliks = 0;
  char optbrie;
  char optwide;
  char optpage;
  char optcomp;
  char optall;
  int temptc;
  int pcount;
  int ok;

  myid = geteuid ();
  mygid = getegid ();

  optall = goptall;
  optbrie = goptbrie;
  optwide = goptwide;
  optpage = goptpage;
  optcomp = goptcomp;

  temptc = tc;
  pcount = 0;

  do
    {
/* if we have not hit a parameter then shift */
      if (!pcount)
	tc++;
/* we search the entire thing for a switch  */
      temptc++;
      ok = checkopt (temptc, tk, tks);
      switch (ok)
	{
/* something neither a switch or terminator */
	case -1:
	  pcount++;
	  break;
/* terminator of command sequence           */
	case 0:		/* no point in setting ok=0 */
	  break;
/* Valid switch                             */
	case 'A':
	case 'a':
	  optall = 1;
	  break;
	case 'C':
	case 'c':
	  optcomp = 1;
	  break;
	case 'P':
	case 'p':
	  optpage = 1;
	  break;
	case 'W':
	case 'w':
	  optwide = 1;
	  break;
	case 'B':
	case 'b':
	  optbrie = 1;
	  break;
/* Invalid switch                           */
	default:
	  ok = 0;
	  pcount = 10;
	  break;
	}
    }
  while (ok);

  if (pcount > 1)
    {
      if (pcount == 10)
	plerr (PLISW);
      else
	plerr (PLTMP);
    }
  else
    {
      dcw = drivecur;
      if (symtab[tks[tc]].id == LEM
	  || symtab[tks[tc]].id == LPIP
	  || symtab[tks[tc]].id == LFRO
	  || symtab[tks[tc]].id == LTO
	  || symtab[tks[tc]].id == LAMP
	  || symtab[tks[tc]].id == LSEP)
	{
	  strcpy (cmdv, "*.*.*");
	}
      else
	{
	  strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	  cmdv[symtab[tks[tc]].len] = '\0';
	  cmdv[symtab[tks[tc]].len + 1] = '\0';
	  if (symtab[tks[tc]].id == LND)
	    dcw = toupper (symstream[symtab[tks[tc]].ptr]) - 'A';
	}

      if (!fromlo (cmdv, cmdvc))
	{

	  if (expwild (cmdv, strlen (cmdv)))
	    {
#ifdef DEBUG
	      fprintf (stderr, "this is a WILD expression\n");
#endif
	      if ((changeback = cbf (cmdv)) != (-1))
		{
		  if (optall)
		    donlist = expr (cmdv, cmdv, cmdvc, OOACC);
		  else
		    donlist = expr (cmdv, cmdv, cmdvc, OOACC | OOVIE);
		}
	      else
		{
		  donlist = 1;
		}
	    }
	  else
	    {
#ifdef DEBUG
	      fprintf (stderr, "this is NOT a wild expression\n");
#endif
	      donlist = stat (cmdv, &fst);
	      if (((07 & fst.st_mode) ||
		   ((070 & fst.st_mode) && (fst.st_gid == mygid)) ||
		   ((0700 & fst.st_mode) && (fst.st_uid == myid))) &&
		  (S_IFDIR == (fst.st_mode & S_IFMT)))
		{
#ifdef DEBUG
		  fprintf (stderr, "But we seem to have a directory\n");
#endif
		  if (cmdv[strlen (cmdv) - 1] != '/')
		    strcat (cmdv, "/*.*.*");
		  else
		    strcat (cmdv, "*.*.*");
		  if ((changeback = cbf (cmdv)) != (-1))
		    {
		      if (optall)
			donlist = expr (cmdv, cmdv, cmdvc, OOACC);
		      else
			donlist = expr (cmdv, cmdv, cmdvc, OOACC | OOVIE);
		    }
		  else
		    {
		      donlist = 1;
		    }
		}
	    }

#ifdef DEBUG
	  fprintf (stderr, "Will try to list <%s>\n", cmdv);
#endif

	  statok = statfs (".", &sfs);
	  if (!statok && !optbrie)
	    {
	      fprintf (out, "\n Volume in drive %c is too loud\n", dcw + 'A');
	      fprintf (out, " Volume Serial Number is %04lX-%04lX\n", sfs.f_type,
		       sfs.f_namelen);
	      fprintf (out, " Directory List\n\n");
	    }
	  if (!donlist)
	    {
	      while (cmdv[i] != '\0')
		{
		  if (!lstat (cmdv + i, &fst))
		    {
		      if ((S_IFREG == (S_IFMT & fst.st_mode)) || (S_IFDIR == (S_IFMT & fst.st_mode)))
			{
			  ff++;
			  ts = localtime (&fst.st_ctime);
			  j = strlen (cmdv + i);
			  if (optwide || optbrie)
			    {
			      if (S_IFREG & fst.st_mode)
				{
				  fprintf (out, " %s ", cmdv + i);
				}
			      else
				{
				  fprintf (out, "[%s]", cmdv + i);
				}
			      if (optbrie || (ltot + m + m + 2 > ccols))
				{
				  fputc ('\n', out);
				  ltot = 0;
				  lcn++;
				  if (j > m)
				    m = j;
				}
			      else
				{
				  fputc (' ', out);
				  while (j < m)
				    {
				      fputc (' ', out);
				      j++;
				    }
				  m = j;
				  ltot += (m + 4);
				  count += fst.st_size;
				}
			    }
			  else
			    {
			      fprintf (out, "%s ", cmdv + i);
			      while (j < m)
				{
				  fputc (' ', out);
				  j++;
				}
			      m = j;
			      if (S_IFREG & fst.st_mode)
				{
				  fprintf (out, "%9ld", fst.st_size);
				  count += fst.st_size;
				}
			      else
				fprintf (out, "<DIR>    ");
			      fprintf (out, " %02d-%02d-%02d  %2d:%02d%c", (ts->tm_mon) + 1,
				       ts->tm_mday,
				       ts->tm_year,
				       (ts->tm_hour) % 12,
				       ts->tm_min,
				       ((ts->tm_hour) / 12) * 15 + 'a');
			      if (optcomp)
				{
				  fputc (' ', out);
				  if ((04 & fst.st_mode) ||
				      ((040 & fst.st_mode) && (fst.st_gid == mygid)) ||
				      ((0400 & fst.st_mode) && (fst.st_uid == myid))
				    )
				    {
				      fputc ('R', out);
				    }
				  else
				    {
				      fputc (' ', out);
				    }
				  if ((02 & fst.st_mode) ||
				      ((020 & fst.st_mode) && (fst.st_gid == mygid)) ||
				      ((0200 & fst.st_mode) && (fst.st_uid == myid))
				    )
				    {
				      fputc ('W', out);
				    }
				  else
				    {
				      fputc (' ', out);
				    }
				  if ((01 & fst.st_mode) ||
				      ((010 & fst.st_mode) && (fst.st_gid == mygid)) ||
				      ((0100 & fst.st_mode) && (fst.st_uid == myid))
				    )
				    {
				      fputc ('X', out);
				    }
				  else
				    {
				      fputc (' ', out);
				    }
				  ts = localtime (&fst.st_atime);
				  fprintf (out, " %02d-%02d-%02d  %2d:%02d%c", (ts->tm_mon) + 1,
					   ts->tm_mday,
					   ts->tm_year,
					   (ts->tm_hour) % 12,
					   ts->tm_min,
					   ((ts->tm_hour) / 12) * 15 + 'a');
				  fprintf (out, "  %ld", fst.st_blocks);
				  bliks += (fst.st_blocks);
				}
			      lcn++;
			      fputc ('\n', out);
			    }
			}
#ifdef DEBUG
		      else
			{
			  fprintf (stderr, "file <%s> neither dir nor plain\n", cmdv + i);
			}
#endif
		    }
		  if (optpage)
		    {
		      if (llines > 4)
			{
			  if (!((lcn % (llines - 1)) || ltot))
			    {
#ifdef DEBUG
			      fprintf (stderr, "Will wait with linecount=<%d> and files=<%d>\n", lcn, ff);
#endif
			      plerr (PLPAK);
			      switch (readc (in))
				{
				case 'q':
				case 'Q':
				  return tc;
				  break;
				}
#ifdef USE_TERMCAP
			      if (hostr != NULL)
				fputs (hostr, out);
#endif
			      lcn++;
			    }
			}
		    }
		  i += strlen (cmdv + i) + 1;
		}
	      if (optwide)
		fputc ('\n', out);
	      if (i && !optbrie)
		{
		  if (ff == 1)
		    fprintf (out, " %8u file    %10lu bytes\n", ff, count);
		  else
		    fprintf (out, " %8u files   %10lu bytes\n", ff, count);

/* I anticipate problems with *huge* partitions... what is a long*512 ? */
		  if (optcomp && (!optwide))
		    {
		      fprintf (out, " %8lu blocks  %10lu bytes allocated\n", bliks, bliks * 512);
		    }
		  if (!statok)
		    {
		      fprintf (out, "                  %10lu bytes free\n", sfs.f_bavail * fst.st_blksize);
		      if (optcomp)
			fprintf (out, "                  %10ld file nodes free\n", sfs.f_ffree);

		    }
		}
	    }
	  if (!i)
	    {
	      plerr (PLFNF);
#ifdef DEBUG
	      fprintf (stderr, "expr() failed with <%s>\n", cmdv);
#endif
	    }

	  if (changeback == 1)
	    chdir (drivetab[drivecur].path);
	}
      else
	{
	  plerr (PLFNF);
#ifdef DEBUG
	  fprintf (stderr, "fromlo() failed with <%s>\n", cmdv);
#endif
	}
    }

  return tc;
}

int 
ldel (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int count = 0;
  int all = 0;
  int quit = 0;
  int temptc;
  int pcount;
  int ok;
  char optprom;

  optprom = goptprom;

  if (nochange)
    {
      plerr (PLACD);
    }
  else
    {
#ifdef DEBUG
      fprintf (stderr, "We have permission to delete stuff\n");
#endif
      temptc = tc;
      pcount = 0;

      do
	{
/* if we have not hit a parameter then shift */
	  if (!pcount)
	    tc++;
/* we search the entire thing for a switch  */
	  temptc++;
	  ok = checkopt (temptc, tk, tks);
	  switch (ok)
	    {
/* something neither a switch or terminator */
	    case -1:
	      pcount++;
	      break;
/* terminator of command sequence           */
	    case 0:		/* no point in setting ok=0 */
	      break;
/* Valid switch                             */
	    case 'C':
	    case 'I':
	    case 'P':
	    case 'c':
	    case 'i':
	    case 'p':
	      optprom = 1;
	      break;
/* Invalid switch                           */
	    default:
	      ok = 0;
	      pcount = 10;
	      break;
	    }
	}
      while (ok);

      if (pcount > 1)
	{
	  if (pcount == 10)
	    plerr (PLISW);
	  else
	    plerr (PLTMP);
	}
      else
	{
	  switch (symtab[tks[tc]].id)
	    {
	    case LEM:
	    case LTO:
	    case LFRO:
	    case LPIP:
	    case LSEP:
	      plerr (PLRPM);
	      break;
/*        case LSTR : if(nopath)break; */
	    case LND:
	    case LELS:
	      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      cmdv[symtab[tks[tc]].len] = '\0';

	      if (fromlo (cmdv, cmdvc))
		{
		  plerr (PLFNF);
		}
	      else
		{
		  if (expwild (cmdv, strlen (cmdv)))
		    {
		      if (expr (cmdv, cmdv, cmdvc, OOPLA))
			{
			  plerr (PLMAE);
			}
		      else
			{
			  while ((cmdv[count] != '\0') && (count < cmdvc))
			    {
#ifdef DEBUG
			      fprintf (stderr, "about to delete %s\n", cmdv + count);
#endif
			      if (optprom)
				{
				  fprintf (out, "Delete %s [ynaq] ", cmdv + count);
				  if (all)
				    {
				      if (unlink (cmdv + count))
					{
					  plerr (PLACD);
					}
				      else
					{
					  fprintf (out, "OK\n");
					}
				    }
				  else if (quit)
				    {
				      fprintf (out, "NO\n");
				    }
				  else
				    switch (readc (in))
				      {
				      case 'A':
				      case 'a':
					all = 1;
				      case 'Y':
				      case 'y':
					if (unlink (cmdv + count))
					  {
					    plerr (PLACD);
					  }
					else
					  {
					    fprintf (out, "OK\n");
					  }
					break;
				      case 'Q':
				      case 'q':
					quit = 1;
				      default:
					fprintf (out, "NO\n");
					break;
				      }
				}
			      else
				{
				  if (unlink (cmdv + count))
				    {
				      plerr (PLACD);
				    }
				}
			      count += (strlen (cmdv + count) + 1);
			    }
			  if (!count)
			    plerr (PLFNF);
			  /* there was no file to zap */
			}
		    }
		  else
		    {
		      if (unlink (cmdv))
			{
			  plerr (PLACD);
			}
		      /* managed to zap the single file */
		    }
		}
	      tc++;
	      break;
	    default:
	      plerr (PLPFN);
	      break;
	    }
	}
    }
  return tc;
}

int 
lexi (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int temp;

  tc++;
  /* A last check for mail */
  temp = countmail ();
  if (temp > mailbytes)
    {
      mailbytes = temp;
      fprintf (out, "You have Mail [%d bytes]\n", mailbytes);
    }
  else
    {
      if (mailbytes)
	{
	  fprintf (out, "You have Mail [%d bytes]\n", mailbytes);
	}
      running = 0;
      if (symtab[tks[tc]].id == LELS && isdigit (symstream[symtab[tks[tc]].ptr]))
	{
	  quitstat = atoi (symstream + symtab[tks[tc]].ptr);
	}
    }
  return tc;
}

int 
lmd (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;
  if (!nochange)
    {
      if (checkopt (tc + 1, tk, tks))
	{
	  if (checkopt (tc, tk, tks))
	    plerr (PLTMP);
	  else
	    plerr (PLRPM);
	}
      else
	{

	  if (symtab[tks[tc]].id == LELS || symtab[tks[tc]].id == LND || symtab[tks[tc]].id == LSTR)
	    {
	      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      cmdv[symtab[tks[tc]].len] = '\0';
	      if (fromlo (cmdv, cmdvc))
		{
		  plerr (PLUCD);
		}
	      else
		{
		  if (mkdir (cmdv, 0777))
		    {
		      plerr (PLUCD);
		    }
		}
	    }
	  else
	    {
	      plerr (PLPFN);
	    }
	}
    }
  else
    {
      plerr (PLACD);
    }
  return tc;
}

int 
lmap (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int newd;
  tc++;

  if (nomap)
    {
      plerr (PLACD);
    }
  else
    {
      if (symtab[tks[tc]].id == LND)
	{
#ifdef DEBUG
	  fprintf (stderr, "Found a drive \n");
#endif
	  if (DRIVES > (newd = (toupper (symstream[symtab[tks[tc]].ptr]) - 'A')))
	    {
#ifdef DEBUG
	      fprintf (stderr, "Drive is %c\n", 'A' + newd);
#endif
	      if (newd != drivecur)
		{
		  tc++;
		  if (symtab[tks[tc]].id == LASS)
		    tc++;
		  if (symtab[tks[tc]].id == LSWI)
		    {
		      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
		      cmdv[symtab[tks[tc]].len] = '\0';
#ifdef DEBUG
		      fprintf (stderr, "Path to be assigned is %s\n", cmdv);
#endif
		      if (chdir (cmdv))
			{
			  plerr (PLPNF);
			}
		      else
			{
			  chdir (drivetab[drivecur].path);
			  if (map (newd, cmdv))
			    {
			      plerr (PLMAE);
			    }
			}
		    }
		  else
		    {
		      plerr (PLPFN);
		    }
		}
	      else
		{
		  plerr (PLATR);
		}
	    }
	  else
	    {
	      plerr (PLIDS);
	    }
	}
      else
	{
	  if (symtab[tks[tc]].id == LEM
	      || symtab[tks[tc]].id == LPIP
	      || symtab[tks[tc]].id == LSEP
	      || symtab[tks[tc]].id == LFRO
	      || symtab[tks[tc]].id == LTO
	      || symtab[tks[tc]].id == LAMP)
	    {
	      pathdump (out);
	    }
	  else
	    {
	      plerr (PLPFN);
	    }
	  tc++;
	}
    }
  return tc;
}

int 
lpro (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;
  if (symtab[tks[tc]].id == LASS)
    tc++;
  switch (symtab[tks[tc]].id)
    {
    case LEM:
    case LSEP:
    case LPIP:
    case LTO:
    case LFRO:
      strcpy (proz, DEFPROMPT);
      break;
    case LELS:
    case LND:
    case LSTR:
      if (symtab[tks[tc]].len < PROZ - 1)
	{
	  strncpy (proz, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	  proz[symtab[tks[tc]].len] = '\0';
	}
      else
	{
	  strncpy (proz, symstream + symtab[tks[tc]].ptr, PROZ - 1);
	  proz[PROZ - 1] = '\0';
	}
      tc++;
      break;
    default:
      plerr (PLPFN);
      break;
    }
  return tc;
}

int 
lrd (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;
  if (nochange)
    {
      plerr (PLACD);
    }
  else
    {
      if (checkopt (tc + 1, tk, tks))
	{
	  if (checkopt (tc, tk, tks))
	    plerr (PLTMP);
	  else
	    plerr (PLRPM);
	}
      else
	{
	  if (symtab[tks[tc]].id == LELS || symtab[tks[tc]].id == LND || symtab[tks[tc]].id == LSTR)
	    {
	      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      cmdv[symtab[tks[tc]].len] = '\0';
	      tc++;
	      if (fromlo (cmdv, cmdvc))
		{
		  plerr (PLIDN);
		}
	      else
		{
		  if (rmdir (cmdv))
		    {
		      plerr (PLACD);
		    }
		  /* otherwise ok */
		}
	    }
	  else
	    {
	      plerr (PLPFN);
	    }
	}
    }
  return tc;
}

int 
lset (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  char *tptr;
  int i = 0;

  tc++;
  switch (symtab[tks[tc]].id)
    {
    case LPRO:
    case LPAT:
    case LELS:
      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
      tptr = cmdv + symtab[tks[tc]].len;
      tptr[0] = '\0';
      tptr = tptr + 1;
      tc++;
      if (symtab[tks[tc]].id == LASS)
	tc++;
      switch (symtab[tks[tc]].id)
	{
	case LELS:
	case LSTR:
	case LND:
	case LSWI:
	  if (!noset)
	    {
	      strncpy (tptr, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      tptr[symtab[tks[tc]].len] = '\0';
	      if (setenv (cmdv, tptr, 1))
		{
		  plerr (PLOES);
		}
	      introspect ();
	      tc++;
	    }
	  else
	    {
	      plerr (PLACD);
	    }
	  break;
	case LEM:
	case LSEP:
	  if (!noset)
	    {
	      unsetenv (cmdv);
	      introspect ();
	    }
	  else
	    {
	      plerr (PLACD);
	    }
	  break;
	}
      break;
    case LPIP:
    case LFRO:
    case LTO:
    case LEM:
    case LSEP:
      if (noset)
	plerr (PLACD);
      else
	while (environ[i] != NULL)
	  fprintf (out, "%s\n", environ[i++]);
      break;
    default:
#ifdef DEBUG
      fprintf (stderr, "Parameter was <%2X>\n", symtab[tks[tc]].id);
#endif
      plerr (PLPFN);
      break;
    }
  return tc;
}

int 
ltim (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  struct tm *tstr;
  time_t tt;

  tc++;

  if (checkopt (tc, tk, tks))
    {
      plerr (PLTMP);
    }
  else
    {
      time (&tt);
      tstr = localtime (&tt);

/* Am in 2 minds here - should I prompt for time ? */

      if (tstr->tm_hour > 12)
	{
	  fprintf (out, "Current time is %02d:%02d:%02d.00p\n", tstr->tm_hour - 12, tstr->tm_min, tstr->tm_sec);
	}
      else
	{
	  fprintf (out, "Current time is %02d:%02d:%02d.00a\n", tstr->tm_hour, tstr->tm_min, tstr->tm_sec);
	}
    }

  return tc;
}

int 
ltyp (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  char tbuf[CPBUF];		/*for loslike performance ;-) */
  char *tstr;
  FILE *fp;

  int trive;
  int mlen;
  int i;
  trive = drivecur;
  tc++;

  if (checkopt (tc + 1, tk, tks))
    {
      if (checkopt (tc, tk, tks))
	plerr (PLTMP);
      else
	plerr (PLRPM);
    }
  else
    {
      switch (symtab[tks[tc]].id)
	{
	case LFRO:
	case LTO:
	case LSEP:
	case LPIP:
	case LEM:
	  plerr (PLRPM);
	  break;
	case LND:
	  trive = toupper (symstream[symtab[tks[tc]].ptr]) - 'A';
	  if (!((trive >= 0) && (trive < DRIVES) && drivetab[trive].valid))
	    {
	      plerr (PLFNF);
	      break;
	    }
	default:
	  mlen = strlen (drivetab[trive].path) + symtab[tks[tc]].len + 4;
	  mlen = CEM (mlen, 16);
	  tstr = (char *) malloc (mlen * sizeof (char));
	  if (tstr != NULL)
	    {
	      strncpy (tstr, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      tstr[symtab[tks[tc]].len] = '\0';
	      if (!fromlo (tstr, mlen))
		{
		  fp = fopen (tstr, "r");
		  if (fp != NULL)
		    {
#ifdef DEBUG
		      fprintf (stderr, "Managed to open file %s\n", tstr);
#endif
		      do
			{
			  i = fread (tbuf, 1, 128, fp);
#ifdef DEBUG
			  fprintf (stderr, "Read %d characters\n", i);
#endif
			  fwrite (tbuf, 1, i, out);
			}
		      while (i == 128);
		      fclose (fp);
		    }
		  else
		    {
		      plerr (PLFNF);
		    }
		}
	      else
		{
		  plerr (PLFNF);
		}
	      free (tstr);
	    }
	  else
	    {
	      plerr (PLMAE);
	    }
	  tc++;
	  break;
	}
    }
  return tc;
}

int 
lver (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  tc++;

  if (checkopt (tc, tk, tks))
    {
      plerr (PLTMP);
    }
  else
    {
      fprintf (out, "\n%s\n\n", LVERSION);
    }

  return tc;
}

int 
lvol (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int trive;

  tc++;
  switch (symtab[tks[tc]].id)
    {
    case LFRO:
    case LTO:
    case LSEP:
    case LPIP:
    case LEM:
      trive = drivecur;
      break;
    case LND:
      if (symtab[tks[tc]].len == 2)
	{
	  trive = toupper (symstream[symtab[tks[tc]].ptr]) - 'A';
	  if (trive > 0 && trive < DRIVES)
	    {
	      if (!drivetab[trive].valid)
		trive = (-2);
	    }
	  else
	    {
	      trive = (-2);
	    }
	}
      else
	{
	  trive = (-1);
	}
      tc++;
      break;
    default:
      trive = (-1);
#ifdef DEBUG
      fprintf (stderr, "We have invalid argument id = %d (token %d)\n", symtab[tks[tc]].id, tc);
#endif
      tc++;
      break;
    }

  if (trive == -1)
    {
      plerr (PLPFN);
    }
  else if (trive == -2)
    {
      plerr (PLIDS);
    }
  else
    {
      fprintf (out, "\n Volume in drive %c is %s\n Volume Serial Number is MGW2-1575\n", trive + 'A', drivetab[trive].path);

    }

  return tc;
}

int 
lzap (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int newd;
  tc++;
  if (symtab[tks[tc]].id == LND)
    {
      if (DRIVES > (newd = (toupper (symstream[symtab[tks[tc]].ptr]) - 'A')))
	{
	  if (newd != drivecur)
	    {
#ifdef DEBUG
	      fprintf (stderr, "Planning to zap drive %c\n", newd + 'A');
#endif
	      zap (newd);
	    }
	  else
	    {
	      plerr (PLATR);
	    }
	}
      else
	{
	  plerr (PLIDS);
	}
    }
  else
    {
      plerr (PLPFN);
    }
  return 0;
}

int 
ltra (int tc, int tk, int *tks, FILE * in, FILE * out)
{

  tc++;
  if (notrap)
    {
      plerr (PLACD);
    }
  else
    {
      char setsomething = 0;
      char caught = 0;

      while (symtab[tks[tc]].id == LELS)
	{
	  if (isdigit (symstream[symtab[tks[tc]].ptr]))
	    {

	      strncpy (cmdv, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);
	      cmdv[symtab[tks[tc]].len] = '\0';

	      setsomething = 1;
	      if (caught)
		{
#ifdef DEBUG
		  fprintf (stderr, "Planning on ignoring signal %d\n", atoi (cmdv));
#endif
		  if (signal (atoi (cmdv), SIG_IGN) < 0)
		    {
		      plerr (PLACD);
		    }
		}
	      else
		{
		  if (signal (atoi (cmdv), SIG_DFL) < 0)
		    {
		      plerr (PLACD);
		    }
		}
	    }
	  else
	    {
	      if (toupper (symstream[symtab[tks[tc]].ptr + 1]) == 'F')
		caught = 1;
	      else
		caught = 0;
	    }
	  tc++;
	}
      if (!setsomething)
	{
	  if (caught)
	    {
	      if (signal (SIGINT, SIG_IGN) < 0)
		{
		  plerr (PLACD);
		}
	    }
	  else
	    {
	      if (signal (SIGINT, SIG_DFL) < 0)
		{
		  plerr (PLACD);
		}
	    }
	}
    }
  return 0;
}
