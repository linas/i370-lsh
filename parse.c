/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "defs.h"

int 
stmts (int tc, int tk, int *tks, FILE * in, FILE * out)
{

#ifdef DEBUG
  fprintf (stderr, "stmts: received %d\n", tc);
#endif

  do
    {
      /*Always return the a pointer to the NEXT item */
      if (symtab[tks[tc]].id == LSEP)
	{
#ifdef DEBUG
	  fprintf (stderr, "Found a separator at location <%d>\n", tc);
#endif
	  tc++;
	}
      tc = stmt (tc, tk, tks, in, out);

    }
  while (symtab[tks[tc]].id == LSEP);

  return tc;
}

int 
stmt (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int isto = 0;
  int isfrom = 0;
  int isok = 1;
  int fromc = 0;
  int toc = 0;
  int i;
  FILE *inf;
  FILE *outf;

  inf = in;
  outf = out;

#ifdef DEBUG
  fprintf (stderr, "stmt received tc=%d\n", tc);
#endif

  for (i = tc; (symtab[tks[i]].id != LSEP) && (symtab[tks[i]].id != LEM); i++)
    {
      if (symtab[tks[i]].id == LTO)
	{
#ifdef DEBUG
	  fprintf (stderr, "Found TO >\n");
#endif
	  if ((symtab[tks[i + 1]].id == LND || symtab[tks[i + 1]].id == LELS) && (!isto))
	    {
	      strncpy (cmdv, symstream + symtab[tks[i + 1]].ptr, symtab[tks[i + 1]].len);
	      cmdv[symtab[tks[i + 1]].len] = '\0';
	      if (!fromlo (cmdv, cmdvc))
		{
		  outf = fopen (cmdv, "a");
#ifdef DEBUG
		  fprintf (stderr, "Will open file <%s>\n", cmdv);
#endif
		  if (outf == NULL)
		    {
		      isok = 0;
		    }
		  else
		    {
		      toc = 1;
		    }
		}
	      else
		{
		  isok = 0;
		}
	    }
	  else
	    {
	      isto++;
	    }
	  isto++;
	}
      else if (symtab[tks[i]].id == LFRO)
	{
#ifdef DEBUG
	  fprintf (stderr, "Found FROM <\n");
#endif
	  if ((symtab[tks[i + 1]].id == LND || symtab[tks[i + 1]].id == LELS) && (!isfrom))
	    {
	      strncpy (cmdv, symstream + symtab[tks[i + 1]].ptr, symtab[tks[i + 1]].len);
	      cmdv[symtab[tks[i + 1]].len] = '\0';
	      if (!fromlo (cmdv, cmdvc))
		{
#ifdef DEBUG
		  fprintf (stderr, "Opening file <%s>\n", cmdv);
#endif
		  inf = fopen (cmdv, "r");
		  if (inf == NULL)
		    {
		      isok = 0;
		    }
		  else
		    {
		      fromc = 1;
		    }
		}
	      else
		{
		  isok = 0;
		}
	    }
	  else
	    {
	      isfrom++;
	    }
	  isfrom++;
	}
    }

  doexec = 0;

#ifdef DEBUG
  fprintf (stderr, "i=%d\n", i);
#endif

  if (i > 1)
    {

#ifdef DEBUG
      fprintf (stderr, "i>1\n");
#endif

      if (symtab[tks[i - 1]].id == LAT)
	{
	  tks[i - 1] = tks[i];
	  doexec = 1;
#ifdef DEBUG
	  fprintf (stderr, "Replaced @ by ; and will do exec (@ found at %d)\n", i - 1);
#endif
	}
      else if (symtab[tks[i - 1]].id == LBAN)
	{
	  tks[i - 1] = tks[i];

	  if (!nobatch)
	    {

	      if (!fromc)
		inf = fopen ("/dev/null", "r");
	      if (!toc)
		outf = fopen (lshout, "a");

	      if (inf == NULL || outf == NULL)
		{
		  plerr (PLFNF);
		}
	      else
		{

		  dobatch = 1;
		  signal (SIGHUP, SIG_IGN);

		}
	    }
	}
    }

  if ((isfrom > 1) || (isto > 1))
    {
      plerr (PLSER);
    }
  else
    {
      if (isok)
	{
	  tc = leade (tc, tk, tks, inf, outf);
	}
      else
	{
	  plerr (PLFNF);
	}
    }

  if (fromc)
    fclose (inf);
  if (toc)
    fclose (outf);

  if (dobatch)
    {
      signal (SIGHUP, SIG_DFL);
      dobatch = 0;
    }

  if (junior)
    {
/* were were spawned from a pipe and have no reason to live anymore */
      exit (0);
    }

  return i;
}

/* I seem to have a small problem with my pipes... if the final process
 * dies (like last | less and you exit less before reading all of last), 
 * then the processes writing to the pipe do NOT die... but they should,
 * they should die with a SIGPIPE... prolly my fault, but why ;-)
 */

/* Well, so I fixed that manually... keep track of processes and send
 * them a SIGPIPE if it is neccessary
 */

int 
leade (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int p[2];
  int count;
  int found;
  int quit;
  int pid;
  int rval;
  int exitstat;
  FILE *pin, *pout;

  found = 0;
  quit = 0;
  count = tc;

#ifdef DEBUG
  fprintf (stderr, " leade: received %d\n", tc);
#endif
  /* handle pipes here with the pipe() call (looks messy) */
  /* use fork and fileno and dup and fdopen and fclean */

  while (!quit)
    {
      switch (symtab[tks[count]].id)
	{
	case LPIP:
	  found = 1;
	case LEM:
	case LSEP:
	case LFRO:
	case LTO:
	case LAMP:
	  quit = 1;
	  break;
	}
      count++;
    }

  if (found)
    {
#ifdef DEBUG
      fprintf (stderr, "Found a pipe before %d\n", count);
#endif
      if (!pipe (p))
	{
	  if ((pid = fork ()) >= 0)
	    {
	      if (pid == 0)
		{

/* we are a younger process and should not hang around */
		  junior++;

		  close (p[1]);
		  dup2 (p[0], STDIN_FILENO);
		  pin = fdopen (STDIN_FILENO, "r");
		  if (pin != NULL)
		    {
#ifdef DEBUG
		      fprintf (stderr, "In child about to call leade(%d,...)\n", count);
		      fprintf (stderr, "Child reading from %d to %d\n", p[0], fileno (out));
#endif
		      running = 0;
		      tc = leade (count, tk, tks, pin, out);
		      if (fclose (pin))
			{
#ifdef DEBUG
			  fprintf (stderr, "In child -- Could not close stream\n");
#endif
			}
		      else
			{
#ifdef DEBUG
			  fprintf (stderr, "In child -- Managed to close stream\n");
#endif
			}
		    }
		  else
		    {
		      plerr (PLSRE);
#ifdef DEBUG
		      fprintf (stderr, "Could not associate stream with descriptor in child\n");
#endif
		    }
/* seems that we require more drastic measures to quit */
/*        exit(0);                                    */
		  running = 0;
		}
	      else
		{
		  senior = pid;
/* this bit did not work for some reason -- rats !    */
/*          close(p[0]);                              */
/*          dup2(p[1],STDOUT_FILENO);                 */
/*          pout=fdopen(STDOUT_FILENO,"w");           */
/* so this one does the job                           */
		  pout = fdopen (p[1], "w");

		  if (pout != NULL)
		    {
#ifdef DEBUG
		      fprintf (stderr, "In parent about to call cmd(%d,...) - child is %d\n", tc, pid);
		      fprintf (stderr, "Parent reading from %d to %d\n", fileno (in), p[1]);
#endif
		      tc = cmd (tc, tk, tks, in, pout);
		      if (fclose (pout))
			{
#ifdef DEBUG
			  fprintf (stderr, "In parent -- Could not close stream\n");
#endif
			}
		      else
			{

#ifdef DEBUG
			  fprintf (stderr, "In parent -- Managed to close stream\n");
#endif
			}
/* some problem here - lets resort to desperate measures */
/* hmmm - should we wait for the child here ? -- trouble */
		      close (p[0]);
		      while ((rval = wait (&exitstat)) != pid && rval != (-1));
/* let's try a different tack... wait for anything          */
/*            rval=wait(&exitstat);                         */
		    }
		  else
		    {
		      plerr (PLSRE);
#ifdef DEBUG
		      fprintf (stderr, "Could not associate stream with descriptor in parent\n");
#endif
		    }
		}
	    }
	  else
	    {
	      plerr (PLSRE);
	    }
	}
      else
	{
	  plerr (PLSRE);
	}
    }
  else
    {
      senior = 0;
#ifdef DEBUG
      fprintf (stderr, "Doing a vanilla cmd(%d,...)\n", tc);
#endif
      tc = cmd (tc, tk, tks, in, out);
    }

  return tc;
}

int 
cmd (int tc, int tk, int *tks, FILE * in, FILE * out)
{
#ifdef DEBUG
  fprintf (stderr, " cmd  : received %d\n", tc);
#endif

  if (symtab[tks[tc]].res || (symtab[tks[tc]].id == LND && symtab[tks[tc]].len == 2))
    {
      tc = reser (tc, tk, tks, in, out);
    }
  else
    {
      tc = norma (tc, tk, tks, in, out);
    }
  return tc;
}

int 
norma (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int pid, retval, exitstat, quit;
  int i, j, done;
  int aa;
  char **tt;

#ifdef DEBUG
  fprintf (stderr, " norma: received %d\n", tc);
#endif

  if (symtab[tks[tc]].id == LELS || symtab[tks[tc]].id == LND ||
   ((!nopath) && (symtab[tks[tc]].id == LSTR || symtab[tks[tc]].id == LSWI))
    )
    {

      if (doexec)
	pid = 0;
      else
	pid = fork ();

      if (pid >= 0)
	{
	  if (pid == 0)
	    {

	      if (dobatch)
		{
		  dup2 (fileno (out), STDERR_FILENO);
#ifdef DEBUG
		  fprintf (stderr, "Changing nice level by %d\n", niceval);
#endif
		  nice (niceval);
		  setsid ();
		  setpriority (PRIO_PROCESS, 0, getpriority (PRIO_PROCESS, 0) + niceval);
		}

	      aa = fileno (in);
	      if (aa != (-1))
		{
		  dup2 (aa, STDIN_FILENO);
		}

	      aa = fileno (out);
	      if (aa != (-1))
		{
		  dup2 (aa, STDOUT_FILENO);
		}

#ifdef DEBUG
	      fprintf (stderr, "In child : Going to exec soon\n");
#endif

/* Modify and then copy args into command vector */

	      done = 0;
	      i = 0;
	      while (!done)
		{
		  switch (symtab[tks[tc]].id)
		    {
		    case LAMP:
		    case LEM:
		    case LSEP:
		    case LPIP:
		    case LTO:
		    case LFRO:
		      done = 1;
		      break;

		    default:
		      strncpy (cmdv + i, symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len);

#ifdef DEBUG
		      fprintf (stderr, "Added %s to vector at %d\n", cmdv + i, i);
#endif

		      /* do a fromlo on paths here and a / to - on flag  */
		      if (symtab[tks[tc]].id == LND ||
			  symtab[tks[tc]].id == LELS ||
			  symtab[tks[tc]].id == LENV)
			{

			  cmdv[i + symtab[tks[tc]].len] = '\0';

			  if (fromlo (cmdv + i, cmdvc - i))
			    {
			      plerr (PLBAD);
			      exit (127);
			    }
			  else
			    {
#ifdef DEBUG
			      fprintf (stderr, "Fromlo changed this to %s\n", cmdv + i);
#endif
			      /* we do NOT expand the command itself */
			      if (i && expwild (symstream + symtab[tks[tc]].ptr, symtab[tks[tc]].len - 1))
				{
#ifdef DEBUG
				  fprintf (stderr, "expanding %s\n", cmdv + i);
#endif
				  if (expr (cmdv + i, cmdv + i, cmdvc - i, OOACC))
				    {
				      plerr (PLMAE);
				      exit (127);
				    }
				  i = expcount (cmdv, i);
				}
			      else
				{

				  cmdv[i + strlen (cmdv + i) + 1] = '\0';
				  findalias (cmdv + i, strlen (cmdv + i), cmdvc - i);
#ifdef DEBUG
				  fprintf (stderr, "findalias returned <%s>\n", cmdv + i);
#endif
				  i = expcount (cmdv, i);
				}
			    }
			}
		      else if (symtab[tks[tc]].id == LSWI)
			{
/* not doing this might be a security snag        */
			  if (nopath)
			    cmdv[i] = '-';
			  i += symtab[tks[tc]].len;
			}
		      else if (symtab[tks[tc]].id == LSTR)
			{
			  if (!nopath)
			    {
			      i += symtab[tks[tc]].len;
			    }
			  else
			    {
/* does this work !?                              */
			      i--;
			    }
			}
		      else
			{
/* is that a good idea... having any argument you like                */
			  i += symtab[tks[tc]].len;
			}


		      cmdv[i] = '\0';
		      i++;
		      tc++;
		      break;
		    }
		}

/* now set up the correct pointers */
	      done = 1;
	      cmdc[0] = cmdv;
	      for (j = 0; (j < i) && (done < cmdcc); j++)
		{
		  if (cmdv[j] == '\0')
		    {
		      cmdc[done++] = cmdv + (j + 1);
		    }
		  if (done == cmdcc - 1)
		    {
		      if (cmdcc < CMDCM)
			{
			  tt = (char **) realloc (cmdc, sizeof (char *) * cmdcc * 2);
			  if (tt != NULL)
			    {
			      cmdc = tt;
			      cmdcc *= 2;
			    }
			}
		    }
		}
#ifdef DEBUG
	      fprintf (stderr, "Will execute <%s>\n", cmdv);
#endif

	      cmdc[done - 1] = NULL;

/* now go and execute the external */

	      signal (SIGINT, SIG_DFL);

/* this is a problem... if the process happens  */
/* to write to a nonexistant pipe it should die */
/* but somehow it does not...                   */
/*      signal(SIGPIPE,exit);                   */
/* maybe I need to fool around with fcntl() !?! */

	      execvp (cmdv, cmdc);
	      plerr (PLBAD);
	      exit (127);
	    }
	  else
	    {
#ifdef DEBUG
	      fprintf (stderr, "In parent : my child is %d\n", pid);
#endif
	      if (dobatch)
		{
/*           is there anything I can to here to prevent zombies ? */
/*           maybe send a SIG_HUP to the child ?                  */
/*           ask init to adopt this one                           */
		  kill (pid, SIGHUP);
/*           make sure it goes on running                         */
		  kill (pid, SIGCONT);
		}
	      else
		{
/*          while((retval=wait(&exitstat))!=pid&&retval!=(-1)); */

/*          if(senior)do{}whilewait(&exitstat);                 */
/*          else      while((retval=wait(&exitstat))!=pid&&retval!=(-1)); */

/* while the just spawned process has not died loop                    */
		  while ((retval = wait (&exitstat)) != pid && retval != (-1))
		    {
/* if a process downstream in the pipeline has finished, kill this one */
		      if (retval > 0 && senior == retval)
			{
			  kill (pid, SIGPIPE);
			}
		    }
		  if (WIFEXITED (exitstat))
		    {
		      quitstat = WEXITSTATUS (exitstat);
		    }
		  else
		    {
		      quitstat = (-1);
		    }
		}
	    }
	}
      else
	{
	  plerr (PLINK);
	}
    }
  else
    {
      if (symtab[tks[tc]].id != LEM && symtab[tks[tc]].id != LSEP)
	{
	  plerr (PLBAD);
	}
    }

  quit = 0;
  while (!quit)
    {
      switch (symtab[tks[tc]].id)
	{
	case LSEP:
	case LPIP:
	case LEM:
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
reser (int tc, int tk, int *tks, FILE * in, FILE * out)
{
  int i;
  int help = 0;

#ifdef DEBUG
  fprintf (stderr, " reser: received %d\n", tc);
#endif

  i = tc + 1;

  /* might as well handle /? here */
  while ((symtab[tks[i]].id != LEM) &&
	 (symtab[tks[i]].id != LTO) &&
	 (symtab[tks[i]].id != LFRO) &&
	 (symtab[tks[i]].id != LPIP) &&
	 (symtab[tks[i]].id != LSEP))
    {

      if (symtab[tks[i]].id == LSWI)
	{
	  if (symstream[symtab[tks[i]].ptr + 1] == '?')
	    {
#ifdef DEBUG
	      fprintf (out, "Going to print some help");
#endif
	      help = 1;
	    }
	}
      i++;
    }

  if (help && (symtab[tks[tc]].id != LND))
    {
      plerr (PLRTF);
/*  switch(symtab[tks[tc]].id){       */
/* here goes the help for each option */
/*  }                                 */
      tc = i;
    }
  else
    {
      /* check for excess parameters here before we call the functions */
      switch (symtab[tks[tc]].id)
	{
	case LALI:
	  tc = lali (tc, tk, tks, in, out);
	  break;
	case LND:
	  tc = lnd (tc, tk, tks, in, out);
	  break;
	case LCD:
	  tc = lcd (tc, tk, tks, in, out);
	  break;
	case LCLS:
	  tc = lcls (tc, tk, tks, in, out);
	  break;
	case LCOP:
	  tc = lcop (tc, tk, tks, in, out);
	  break;
	case LAT:
	case LQES:
	case LCAL:
	  tc = lcal (tc, tk, tks, in, out);
	  break;
	case LDAT:
	  tc = ldat (tc, tk, tks, in, out);
	  break;
	case LDIR:
	  tc = ldir (tc, tk, tks, in, out);
	  break;
	case LDEL:
	  tc = ldel (tc, tk, tks, in, out);
	  break;
	case LECH:
	  tc = lech (tc, tk, tks, in, out);
	  break;
	case LEXI:
	  tc = lexi (tc, tk, tks, in, out);
	  break;
	case LMAP:
	  tc = lmap (tc, tk, tks, in, out);
	  break;
	case LMD:
	  tc = lmd (tc, tk, tks, in, out);
	  break;
	case LPRO:
	  tc = lpro (tc, tk, tks, in, out);
	  break;
	case LPAT:
	  tc = lpat (tc, tk, tks, in, out);
	  break;
	case LPAU:
	  tc = lpau (tc, tk, tks, in, out);
	  break;
	case LRD:
	  tc = lrd (tc, tk, tks, in, out);
	  break;
	case LREN:
	  tc = lren (tc, tk, tks, in, out);
	  break;
	case LREM:
	  tc = lrem (tc, tk, tks, in, out);
	  break;
	case LSET:
	  tc = lset (tc, tk, tks, in, out);
	  break;
	case LTIM:
	  tc = ltim (tc, tk, tks, in, out);
	  break;
	case LTRA:
	  tc = ltra (tc, tk, tks, in, out);
	  break;
	case LTYP:
	  tc = ltyp (tc, tk, tks, in, out);
	  break;
	case LUNA:
	  tc = luna (tc, tk, tks, in, out);
	  break;
	case LVER:
	  tc = lver (tc, tk, tks, in, out);
	  break;
	case LVOL:
	  tc = lvol (tc, tk, tks, in, out);
	  break;
	case LZAP:
	  tc = lzap (tc, tk, tks, in, out);
	  break;
#ifdef DIAGNOSE
	case LDMP:
	  tc = ldmp (tc, tk, tks, in, out);
	  break;
#endif
	}
    }

  return tc;
}
