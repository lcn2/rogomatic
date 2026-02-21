/*
 * learn.c: Rog-O-Matic XIV (CMU) Sat Jul  5 23:55:06 1986 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Genetic learning component.
 *
 * EDITLOG
 *	LastEditDate = Sat Jul  5 23:55:06 1986 - Michael Mauldin
 *	LastFileName = /usre3/mlm/src/rog/ver14/learn.c
 *
 * HISTORY
 *  5-Jul-86  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 */

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <sys/types.h>
# include <math.h>

# include "types.h"
# include "globals.h"

# define TRIALS(g)		((g)->score.count)
# define NONE		(-1)
# define MAXM		100
# define ALLELE		100
# define ZEROSTAT	{0, 0, 0, 0, 0}

typedef struct
{ int   id, creation, father, mother, dna[MAXKNOB];
        statistic score, level;
}               genotype;

extern int knob[];

static time_t inittime=0;
static int trialno=0, lastid=0;
static int crosses=0, shifts=0, mutations=0;
static statistic g_score = ZEROSTAT;
static statistic g_level = ZEROSTAT;
static genotype *genes[MAXM];
static int length = 0;
static int mindiff = 10, pmutate = 4, pshift = 2, mintrials = 1;
static double step = 0.33; /* standard deviations from the mean */
static FILE *glog=NULL;

/*
 * static declarations
 */
static int compgene (const void *, const void *);
static void parsegene (char *, genotype *);
static void summgene (FILE *, genotype *);
static void birth (FILE *, genotype *);
static void printdna (FILE *, genotype *);
static void cross (int, int, int);
static void mutate (int, int);
static void shift (int, int);
static void randompool (int);
static int selectgene (int, int);
static int unique (int);
static int untested (void);
static int youngest (void);
static void makeunique (int);
static int triangle (int);
static int badgene (int, int);
static void writegene (FILE *, genotype *);
static void initgene (genotype *);

/*
 * Start a new gene pool
 */

void
initpool (int k, int m)
{ inittime = time (0);

  if (glog) fprintf (glog, "Gene pool initalized, k %d, m %d, %s",
                     k, m, ctime (&inittime));

  randompool (m);
}

/*
 * Summarize the current gene pool
 */

void
analyzepool (int full)
{ int g;

  qsort (genes, length, sizeof (*genes), compgene);

  printf ("Gene pool size %d, started %s", length, ctime (&inittime));
  printf ("Trials %d, births %d (crosses %d, mutations %d, shifts %d)\n",
          trialno, lastid, crosses, mutations, shifts);
  printf ("Mean score %1.0lf+%1.0lf, Mean level %3.1lf+%3.1lf\n\n",
	  mean (&g_score), stdev (&g_score),
	  mean (&g_level), stdev (&g_level));

  /* Give average of each gene */
  if (full == 2)
  { statistic gs;
    int k;
    extern char *knob_name[];

    for (k=0; k<MAXKNOB; k++)
    { clearstat (&gs);

      for (g=0; g<length; g++)
      { addstat (&gs, genes[g]->dna[k]); }

      printf ("%s%5.2lf+%1.2lf\n", knob_name[k], mean (&gs), stdev (&gs));
    }
  }

  /* List detail of gene pool */
  else
  {
    for (g=0; g<length; g++)
    { printf ("Living: "); summgene (stdout, genes[g]);
      if (full)
      { if (genes[g]->mother)
	  printf ("  Parents: %3d,%-3d", genes[g]->father, genes[g]->mother);
	else
	  printf ("  Parent:  %3d,   ", genes[g]->father);
	printf ("  best %4.0lf/%-2.0lf",
		  genes[g]->score.high, genes[g]->level.high);
	printf ("    DNA  "); printdna (stdout, genes[g]); printf ("\n\n");
      }
    }
  }
}

/*
 * setknobs: Read gene pool, pick genotype, and set knobs accordingly.
 */

void
setknobs (int *newid, int *knb, int *best, int *avg)
{ int i, g;

  ++trialno;

  g = pickgenotype ();	/* Pick one genotype */
  *newid = genes[g]->id;

  for (i=0; i<MAXKNOB; i++)	/* Set the knobs for that genotype */
    knb[i] = genes[g]->dna[i];

  *best = genes[g]->score.high;
  *avg = (int) mean (&(genes[g]->score));
}

/*
 * evalknobs: Add a data point to the gene pool
 */

void
evalknobs (int gid, int score, int level)
{ int g;

  /* Find out which gene has the correct id */
  for (g=0; g<length; g++)
    if (gid == genes[g]->id) break;

  /* If he got deleted by someone else, blow it off */
  if (g >= length) return;

  /* Add information about performance */
  addstat (&(genes[g]->score), score);
  addstat (&g_score, score);
  addstat (&(genes[g]->level), level);
  addstat (&g_level, level);

  if (glog)
  { fprintf (glog, "Trial %4d, Id %3d -> %4d/%-2d  ",
      trialno, genes[g]->id, score, level);

    fprintf (glog, "age %2d, %4.0lf+%-4.0lf  %4.1lf+%3.1lf\n",
	     TRIALS(genes[g]),
	     mean (&(genes[g]->score)), stdev (&(genes[g]->score)),
	     mean (&(genes[g]->level)), stdev (&(genes[g]->level)));
  }
}

/*
 * openlog: Open the gene log file
 */

FILE *
openlog (char *genelog)
{ glog = wopen (genelog, "a");
  return (glog);
}

/*
 * closelog: Close the log file
 */

void
closelog (void)
{ if (glog) fclose (glog);
}

/*
 * pickgenotype: Run one trial, record performance, and do some learning
 */

int
pickgenotype (void)
{ int youth, father, mother, new;

  /* Find genotype with fewer trials than needed to measure its performance */
  youth = untested ();
  if (youth >= 0) return (youth);

  /*
   * Have a good measure of all genotypes, pick a father, a mother, and
   * a loser and create a new genotype using genetic operators.
   */

  father = selectgene (NONE, NONE);
  mother = selectgene (father, NONE);
  new = badgene (father, mother);

  /* If no losers yet, return the youngest */
  if (new < 0) return (youngest ());

  /* Shift a single genotype with probability pshift */
  if (randint (100) < pshift)
  { if (glog)
    { fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    shift (father, new);
  }

  /* Mutate a single genotype with probability pmutate */
  else if (randint (100-pshift) < pmutate)
  { if (glog)
    { fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    mutate (father, new);
  }

  /* Cross two genotypes with probability 1-pshift-pmutate */
  else
  { if (glog)
    { fprintf (glog, "Select: "); summgene (glog, genes[father]);
      fprintf (glog, "Select: "); summgene (glog, genes[mother]);
      fprintf (glog, "Death:  "); summgene (glog, genes[new]);
    }

    cross (father, mother, new);
  }

  /* Log the birth */
  if (glog) birth (glog, genes[new]);

  return (new);			/* Evaluate the new genotype */
}

/*
 * readgenes: Open the genepool for reading, and fill the current gene pool.
 * Returns true if the file was read, and 0 if there was no fail.  Exits
 * if the file exists and cannot be read.
 */

int
readgenes (char *genepool)
{ char buf[BUFSIZ];
  char *b;
  int g=0;
  FILE *gfil;

  if ((gfil = fopen (genepool, "r")) == NULL)
  { if (fexists (genepool))
      quit (1, "Cannot open file '%s'\n", genepool);
    else
      return (0);
  }

  /* Read the header line */
  b = buf;
  fgets (b, BUFSIZ, gfil);
  sscanf (b, "%ld %d %d %d %d %d",
	&inittime, &trialno, &lastid, &crosses, &shifts, &mutations);
  SKIPTO ('|', b);
  parsestat (b, &g_score);
  SKIPTO ('|', b);
  parsestat (b, &g_level);

  /* Now read in each genotype */
  while (fgets (buf, BUFSIZ, gfil) && length < MAXM-1)
  { if (g >= length)
    { genes[g] = (genotype *) malloc (sizeof (**genes));
      length++;
    }
    initgene (genes[g]);
    parsegene (buf, genes[g++]);
  }

  fclose (gfil);
  return (1);
}

/*
 * parsegene: Given a string representing a genotype and a genotype
 * structure, fill the structure according to the string.
 */

static void
parsegene (char *buf, genotype *gene)
{ int i;

  /* Get genotype specific info */
  sscanf (buf, "%d %d %d %d", &gene->id, &gene->creation,
	  &gene->father, &gene->mother);

  /* Read each DNA gene */
  SKIPTO ('|', buf);
  SKIPCHAR (' ', buf);
  for (i=0; ISDIGIT (*buf); i++)
  { if (i < MAXKNOB) gene->dna[i] = atoi (buf);
    SKIPDIG (buf);
    SKIPCHAR (' ', buf);
  }

  /* Read the score and level performance stats */
  SKIPTO ('|', buf);
  parsestat (buf, &(gene->score));
  SKIPTO ('|', buf);
  parsestat (buf, &(gene->level));
}

/*
 * writegenes: Write the gene pool 'genes' out to file 'genepool'
 */

void
writegenes (char *genepool)
{ FILE *gfil;
  int g;

  /* Open the gene file */
  if ((gfil = wopen (genepool, "w")) == NULL)
    quit (1, "Cannot open file '%s'\n", genepool);

  /* Write the header line */
  fprintf (gfil, "%ld %d %d %d %d %d",
	   inittime, trialno, lastid, crosses, shifts, mutations);
  fprintf (gfil, "|");
  writestat (gfil, &g_score);
  fprintf (gfil, "|");
  writestat (gfil, &g_level);
  fprintf (gfil, "|\n");

  /* Loop through each genotype */
  for (g=0; g<length; g++)
    writegene (gfil, genes[g]);

  fclose (gfil);
}

/*
 * Write out one line representing the gene.
 */

static void
writegene (FILE *gfil, genotype *g)
{ int i;

  /* Print genotype specific info */
  fprintf (gfil, "%3d %4d %3d %3d|", g->id, g->creation,
	  g->father, g->mother);

  /* Write out dna */
  for (i=0; i<MAXKNOB; i++)
  { fprintf (gfil, "%2d", g->dna[i]);
    if (i < MAXKNOB-1) fprintf (gfil, " ");
  }
  fprintf (gfil, "|");

  /* Write out statistics */
  writestat (gfil, &(g->score));
  fprintf (gfil, "|");
  writestat (gfil, &(g->level));
  fprintf (gfil, "|\n");
}

/*
 * initgene: Allocate a new genotype structure, set everything to 0.
 */

static void
initgene (genotype *gene)
{ int i;

  /* Clear genoptye specific info */
  gene->id = gene->creation = gene->father = gene->mother = 0;

  /* Clear the dna */
  for (i = 0; i < MAXKNOB; i++) gene->dna[i] = 0;

  /* Clear the statictics */
  clearstat (&(gene->score));
  clearstat (&(gene->level));
}

/*
 * compgene: Compare two genotypes in terms of score.
 */

static int
compgene (const void *arg_a, const void *arg_b)
{ int result;
  genotype **a = (genotype **) arg_a;
  genotype **b = (genotype **) arg_b;

  result = (int) mean (&((*b)->score)) -
           (int) mean (&((*a)->score));

  if (result) return (result);
  else return ((*a)->id - (*b)->id);
}

/*
 * summgene: Summarize a single genotype
 */

static void
summgene (FILE *f, genotype *gene)
{ fprintf (f, "%3d age %2d, created %4d, ",
	   gene->id, TRIALS(gene), gene->creation);
  fprintf (f, "score %5.0lf+%-4.0lf level %4.1lf+%-3.1lf\n",
           mean (&(gene->score)), stdev (&(gene->score)),
           mean (&(gene->level)), stdev (&(gene->level)));
}

/*
 * Birth: Summarize Record the birth of a genotype.
 */

static void
birth (FILE *f, genotype *gene)
{
  if (!glog) return;

  fprintf (f, "Birth:  %d ", gene->id);

  if (gene->mother)
    fprintf (f, "(%d,%d)", gene->father, gene->mother);
  else
      fprintf (f, "(%d)", gene->father);

  fprintf (f, " created %d, DNA ", gene->creation);
  printdna (f, gene);
  fprintf (f, "\n");
}

/*
 * printdna: Print the genotype of a gene
 */

static void
printdna (FILE *f, genotype *gene)
{ int i;

  fprintf (f, "(");
  for (i=0; i < MAXKNOB; i++)
  { fprintf (f, "%02d", gene->dna[i]);
    if (i < MAXKNOB-1) fprintf (f, " ");
  }
  fprintf (f, ")");
}

/*
 * cross: Cross two genotypes producing a new genotype
 */

static void
cross (int father, int mother, int new)
{ int cpoint, i;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = genes[mother]->id;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Pick a crossover point and dominant parent */
  cpoint = randint (MAXKNOB-1) + 1;

  /* Fifty/fifty chance we swap father and mother */
  if (randint (100) < 50)
  { father ^= mother; mother ^= father; father ^= mother; }

  /* Copy the dna over */
  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = (i<cpoint) ?
      genes[father]->dna[i] : genes[mother]->dna[i];

  makeunique (new);

  /* Log the crossover */
  if (glog)
  { fprintf (glog, "Crossing %d and %d produces %d\n",
		genes[father]->id,  genes[mother]->id, genes[new]->id);
  }

  crosses++;
}

/*
 * mutate: mutate a genes producing a new gene
 */

static void
mutate (int father, int new)
{ int i;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = 0;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Copy the dna over */
  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = genes[father]->dna[i];

  /* Randomly change genes until the new genotype is unique */
  do
  { i=randint (MAXKNOB);
    genes[new]->dna[i] = (genes[new]->dna[i] +
			  triangle (20) + ALLELE) % ALLELE;
  } while (!unique (new));

  /* Log the mutation */
  if (glog)
  { fprintf (glog, "Mutating %d produces %d\n",
		genes[father]->id, genes[new]->id);
  }

  mutations++;
}

/*
 * shift: shift a gene producing a new gene
 */

static void
shift (int father, int new)
{ int i, offset;

  /* Set the new genotypes info */
  genes[new]->id = ++lastid;
  genes[new]->creation = trialno;
  genes[new]->father = genes[father]->id;
  genes[new]->mother = 0;
  clearstat (&(genes[new]->score));
  clearstat (&(genes[new]->level));

  /* Pick an offset, triangularly distributed around 0, until unique */
  offset = triangle (20);
  for (i=0; i<MAXKNOB; i++)
    genes[new]->dna[i] = (genes[father]->dna[i] +
			  offset + ALLELE) % ALLELE;

  makeunique (new);

  /* Now log the shift */
  if (glog)
  { fprintf (glog, "Shifting %d by %d produces %d\n",
		genes[father]->id, offset, genes[new]->id);
  }

  shifts++;
}

/*
 * randompool: Initialize the pool to a random starting point
 */

static void
randompool (int m)
{ int i, g;

  for (g=0; g<m; g++)
  { if (g >= length)
    { genes[g] = (genotype *) malloc (sizeof (**genes));
      length++;
    }
    initgene (genes[g]);
    genes[g]->id = ++lastid;
    for (i=0; i<MAXKNOB; i++) genes[g]->dna[i] = randint (ALLELE);
    birth (glog, genes[g]);
  }

  length = m;
}

/*
 * selectgene: Select a random gene, weighted by mean score.
 */

static int
selectgene (int e1, int e2)
{ int total=0;
  int g;

  /* Find the total worth */
  for (g=0; g<length; g++)
  { if (g==e1 || g==e2) continue;
    /* total += (int) mean (&(genes[g]->score)); */
    total += genes[g]->score.high;
  }

  /* Pick a random number and find the corresponding gene */
  if (total > 0)
  { for (g=0, total=randint (total); g<length; g++)
    { if (g==e1 || g==e2) continue;
      /* total -= (int) mean (&(genes[g]->score)); */
      total -= genes[g]->score.high;
      if (total < 0) return (g);
    }
  }

  /* Total worth zero, pick any gene at random */
  while ((g = randint (length))==e1 || g==e2) ;
  return (g);
}

/*
 * unique: Return false if gene is an exact copy of another gene.
 */

static int
unique (int new)
{ int g, i, delta, sumsquares;

  for (g=0; g<length; g++)
  { if (g != new)
    { sumsquares = 0;
      for (i=0; i<MAXKNOB; i++)
      { delta = genes[g]->dna[i] - genes[new]->dna[i];
	sumsquares += delta * delta;
      }
      if (sumsquares < mindiff) return (0);
    }
  }

  return (1);
}

/*
 * untested: Return the index of the youngest genotype with too few
 * trials to have an accurate measure.  The number of trials is
 * greater for older genotypes.
 */

static int
untested (void)
{ int g, y= -1, trials=1e9, newtrials, count=length;

  for (g = randint (length); count-- > 0; g = (g+1) % length)
  { if (TRIALS (genes[g]) >= trials) continue;

    newtrials = trialno - genes[g]->creation;	/* Turns since creation */

    if (TRIALS (genes[g]) < newtrials / (4 * mintrials * length) + mintrials)
    { y = g; trials = TRIALS (genes[g]); }
  }

  return (y);
}

/*
 * youngest: Return the index of the youngest genotype
 */

static int
youngest (void)
{ int g, y=0, trials=1e9, newtrials, count=length;

  for (g = randint (length); count-- > 0; g = (g+1) % length)
  { newtrials = TRIALS (genes[g]);
    if (newtrials < trials) { y=g; trials=newtrials; }
  }

  return (y);
}

/*
 * makeunique: Mutate a genotype until it is unique
 */

static void
makeunique (int new)
{ int i;

  while (!unique (new))
  { i=randint (MAXKNOB);
    genes[new]->dna[i] = (genes[new]->dna[i] +
		          triangle (20) + ALLELE) % ALLELE;
  }
}

/*
 * triangle: Return a non-zero triangularly distributed number from -n to n.
 */

static int
triangle (int n)
{ int val;

  do
  { val = randint (n) - randint (n);
  } while (val==0);

  return (val);
}

/*
 * badgene: Find the worst performer so far (with the lowest id).
 * only consider genotypes dominated by other genotypes.
 */

static int
badgene (int e1, int e2)
{ int g, worst, trials;
  double worstval, bestval, avg, dev, value;

  worst = -1; worstval = 1.0e9;
  bestval = -1.0e9;

  for (g=0; g<length; g++)
  { if ((trials = TRIALS (genes[g])) < mintrials) continue;
    avg = mean (&(genes[g]->score));
    dev = stdev (&(genes[g]->score)) / sqrt ((double) trials);
    value = avg - step * dev;
    if (value > bestval) { bestval=value; }
    if (g==e1 || g==e2) continue;
    value = avg + step * dev;
    if (value < worstval) { worst=g; worstval=value; }
  }

  if (worstval < bestval)	return (worst);
  else				return (-1);
}
