#include <stdio.h>
#define EEN 0x001
#define TWEE 0x002
#define DRIE 0x004
#define VIER 0x008
#define VIJF 0x010
#define ZES 0x020
#define ZEVEN 0x040
#define ACHT 0x080
#define NEGEN 0x100

#define DEBUG 3
#define HYPO 10;

typedef struct
{
  int grid[81];
  int *row[9][9];
  int *col[9][9];
  int *box[9][9];
} sudoku;

int hash[257];

int hypo=HYPO;
int maxlevel=0;
int debug=DEBUG;
int hintno=1;
char sudokusolver_string[89];
char numstring[32];
char numstring2[32];

void init(sudoku *mySud);
void fill_in(sudoku *mySud, char *filename);
int unsetbit(int level, int **set, int idx, int value, char *str);
int countbits(int value);
int possible(sudoku *mySud);
int solved(sudoku *mySud);
void print_puzzle(sudoku *mySud,char *string);
void copy(sudoku *p1, sudoku *p2);
int findsoleoption(int level, int **set, char *str);
int compulsaryblockpart(int level, int **box,int boxpart,int **set,int setpart,char *comm,char *comm2);
int stragglerelim(int level, int **row, char *str);
int findchain(int level, int **set,char *s);
int handleopts(int argc, char **argv);
char *digitstr(char *chararr, int nums);
int getSet(sudoku *mySud, int *cell, int *rcb);
int xwing(int level,sudoku *mySud,int order,char *str);
int checkIt(int c,int setnum, int cellnum, int *sets, int *pos, int size);

int main (int argc, char **argv)
{

  sudokusolver_string[0] = '\0';

  if (argc < 2 || !handleopts(argc,argv))
  {fprintf(stderr,"Usage:\n\tsudoku <inputfile>\n\n"); exit(1);}

  sudoku *baseSud;
  baseSud = malloc(sizeof(sudoku));
  
  hash[1] = 1;
  hash[2] = 2;
  hash[4] = 3;
  hash[8] = 4;
  hash[16] = 5;
  hash[32] = 6;
  hash[64] = 7;
  hash[128] = 8;
  hash[256] = 9;
 
  init(baseSud);
  
  
  if (sudokusolver_string[0])
    fill_in_string(baseSud);
  else
    fill_in(baseSud,argv[argc-1]);
  
  if (debug>=1 && debug!=3)
    print_puzzle(baseSud,"INITIAL");

  counter_hypothesize(baseSud, 0);
  
  print_puzzle(baseSud,"FINAL");

  if (!possible(baseSud))
    printf("\nPUZZLE IS IMPOSSIBLE\n");
  else if (solved(baseSud))
    printf("\nPUZZLE IS SOLVED\n");
  else if (hypo > 0)
    printf("\nPUZZLE HAS MULTIPLE SOLUTIONS\n");  
  else
    printf("\nPUZZLE MAY BE POSSIBLE\n"); 
  printf("maximum search depth: %d\n", maxlevel);  

  return 0;
}

int xwing(int level,sudoku *mySud,int order,char *str)
{
  int i,j,k,cand,prog=0;
  int cnt,cnt2,cnt_check,idxs[3];
  int p1,p2,q1,q2,r1,r2;
  int found,ok_cellsinset;
  int pos[5],sets[5];
  
  cand=EEN;  
  for (cand=EEN;cand<=NEGEN;cand=cand<<1)
  {
    found = 0;
    for (i=0;!found && i<8;i++)
    {
      cnt = 0;
      for (j=0;j<9;j++)
      {
        if (*mySud->row[i][j] & cand)
        {
          getSet(mySud, mySud->row[i][j], idxs);
          pos[cnt] = idxs[1];
          cnt++;
        }
      }
      
      if (cnt==order)  // i indicates set with exactly <order> candidates
      {
        sets[0] = i;
        cnt_check = cnt;
        cnt2 = 1;  // first set with <order> cells counts for 1
        for (k=i+1;k<9;k++)
        {
          cnt = 0;
          ok_cellsinset = 1;
          for (j=0;j<9;j++)
          {
            if (*mySud->row[k][j] & cand)
            {
              getSet(mySud, mySud->row[k][j], idxs);
              ok_cellsinset = ok_cellsinset && (cnt < cnt_check) && (pos[cnt] == idxs[1]);
              cnt++;
            }
          }
          if (ok_cellsinset && cnt==cnt_check)
          {
            sets[cnt2] = k;
            cnt2++;
          }
        }

        if (cnt2==order)
        {
          cnt2 = 0;
          for (i=0;i<9;i++)
          {
            cnt = 0;
            for(j=0;j<9;j++)
            {
              if (checkIt(cand,i,j,sets,pos,order))
              {
                if (*mySud->row[i][j] & cand)
                {
                  if (level == 0 && debug >= 3)
                    printf("%03d. X wing for cand %s; dismissed as candidate from (%d,%d)\n", hintno++, digitstr(numstring,cand), i+1, j+1);
                  *mySud->row[i][j] = *mySud->row[i][j] & ~cand;
                  prog=1;
                }
              }
            }
          }
        }
      }
    }
  }
  return prog;

}

int checkIt(int cand, int setnum, int cellnum, int *sets, int *pos, int size)
{
  int i,ok=1;
  for (i=0;i<size;i++)
  {
    ok = ok && (setnum != sets[i]);
  }
  if (!ok)
    return 0;
  ok = 0;
  for (i=0;i<size;i++)
    ok = ok || (cellnum == pos[i]);
  return ok;
}


int getSet(sudoku *mySud, int *cell, int *rcb)
{
  int i,j;
  for(i=0;i<9;i++)
    for(j=0;j<9;j++)
    {
      if (mySud->row[i][j] == cell)
        rcb[0] = i;
      if (mySud->col[i][j] == cell)
        rcb[1] = i;
      if (mySud->box[i][j] == cell)
        rcb[2] = i;
    }
}

int handleopts(int argc, char **argv)
{
  char *opt;
  int p;
  for(opt=argv[p=1];p<argc-1;opt=argv[++p])
  {
    if (*opt != '-' || strlen(opt)==1)
      return 0;
    switch(*(opt+1))
    {
      case 'd':
        debug = atoi(argv[++p]);
        break;
      case 'h':
        hypo = atoi(argv[++p]);
        break;
      case 's':
        if (strlen(argv[++p]) != 89)
        {
          fprintf(stderr, "Invalid length for sudoku solverstring!\n");
          exit(1);
        }
        else
          strcpy(sudokusolver_string, argv[p]);
        break;
      default:
        fprintf(stderr, "Invalid option: %s\n", opt);
        return 0;
        break;
    }
  }
}

int findchain(int level, int **set,char *str)
{
  int prog=0;
  int s,cnt,i,c,or,cc,ccc,a,found=0;
  for(a=2;!found && a<=3;a++)
  {
    for (c=1;!found && c<=511;c++)
    {
      cnt=countbits(c);
      if (cnt==a)
      {
        or = 0x0;
        for (i=0;i<9;i++)
          if ((0x1 << i) & c)
            if (countbits(*set[i])>=2)
              or = or | *set[i];
            else
              or = 0x1FF;
        if (countbits(or) == cnt)
        {
          cc = c;
          found=1;
        }
      }
    }
  }
  ccc=cc;
  if (found)
  {
    // printf("%s: samengestelde waarden=%03X rijindexen=%03X\n", str,or, cc);
    for (i=0;i<9;i++)
    {
      if (cc & EEN)
        ; // nix
      else
      {
        if (*set[i] & or)
        {
          prog = 1;
          *set[i] = *set[i] & ~or; 
          if (level == 0 && debug >= 3)
            printf("%03d. Found chain in %s (cells %s open for candidates %s), hence cell %d cannot contain these values.\n", hintno++, str, digitstr(numstring,ccc), digitstr(numstring2,or), i+1);
	}
      }
      cc = cc >> 1;
    }  
  }
  return prog;
}


int stragglerelim(int level, int **set, char *str)
{
  int prog=0;
  int i,j,k,l;
  int candset;
  
  for(i=0;i<8;i++)
    for(j=i+1;j<9;j++)
    {
      candset = *set[i] & *set[j];
      if (countbits(candset)>=2)
      {
        for (k=0;k<9;k++)
          if (k!=i && k!=j)
            candset = candset & ~*set[k];
        if (countbits(candset)==2 && (countbits(*set[i])>2 || countbits(*set[j])>2))
        {
          *set[i] = candset;
          *set[j] = candset;
          prog=1;
          if (level == 0 && debug >= 3)
            printf("%03d. Digit pair %s only possible in cells %d and %d of %s. Removing any other candidate.\n", hintno++, digitstr(numstring,candset), i+1, j+1, str);
        }        
      }
    }
  return prog;
}

int compulsaryblockpart(int level, int **abox,int boxpart,int **set,int setpart,char *comm,char *comm2)
{
  int i,j,k,l;
  int bmode,smode,prog=0;
  int within,inboxpart;
  int cand;
  int verbose=0;
  
  if (debug>=6)
    printf("compblockpt(level=%d,abox,boxpart=%d,set,setpart=%d,comm=%s,comm2=%s)\n", level, boxpart,setpart,comm,comm2);
  
  bmode=(boxpart<3) ? 0 : 1;  /* 0: row  1: column */
  boxpart = boxpart%3;
  smode=(setpart<3) ? 0 : 1;  /* 0: row  1: column */
  setpart = setpart%3;

  cand=EEN;
  for (i=0;i<9;i++)  /* each candidate digit */
  {
    inboxpart=1;
    for (j=0; j<3;j++) /* each row/column part */
      for (k=0; k<3;k++) /* each element of part j */
      {
        within = (smode==0 && j==setpart || smode==1 && k==setpart);  /* smode==1 <=>  */
        inboxpart = inboxpart && cand != *set[j*3+k] && (within || !within && ((*set[j*3+k] & cand) == 0) );
      }
    /* inboxpart <=> cand must be inside column/row part that overlaps with block */

    if (inboxpart)
    {
      for (j=0; j<3;j++) /* each block row*/
        for (k=0; k<3;k++) /* each element of block row */
        {
          within = (bmode==0 && j==boxpart || bmode==1 && k==boxpart);
          if (!within && (*abox[j*3+k] & cand) && countbits(*abox[j*3+k]) > 1)
          {
            if (level == 0 && debug >= 3)
              printf("%03d. Digit %s in %s must be in overlap with %s! Dismissed as candidate for cell %d of %s.", hintno++, digitstr(numstring,cand), comm2, comm, j*3+k+1, comm);
            *abox[j*3+k] &= ~cand;
            if (level == 0 && debug >= 3)
              if (countbits(*abox[j*3+k])==1)
                printf(" Single possibility left is digit %s.\n", digitstr(numstring,*abox[j*3+k]));
              else
                printf("\n");
            prog = 1;
          }
        }
    }
    
    cand = cand << 1;
  }
  return prog;
}

int findsoleoption(int level, int **set, char *str)
{
  
  int cand,i,j;
  int prog=0,cnt,ind;
  for (i=0;i<9;i++)
  {
    cand = pow(2,i);
    cnt=0; ind=-1;
    for (j=0;j<9;j++)
      if(cand & *set[j])
      {
        cnt++;
        ind = j;
      }
    if (cnt==1 && countbits(*set[ind])>1)
    {
      if (level == 0 && debug >= 3)
        printf("%03d. Digit %s in %s can only be in cell %d.\n", hintno++, digitstr(numstring, cand), str, ind+1);
      *set[ind] = cand;
      prog=1;
    }
  }
  return prog;
}

void print_puzzle(sudoku *mySud,char *string)
{
  int i;
  printf("%s\n",string);
  if (solved(mySud))
    for(i=0;i<81;i++)
      printf("%3d%s", hash[mySud->grid[i]], (!((i+1)%9)) ? "\n" : " ");
  else  
    for(i=0;i<81;i++)
    {
      printf("%03X%s%s", mySud->grid[i], (countbits(mySud->grid[i])==1) ? "*" : " ", (!((i+1)%9)) ? "\n" : ((i+1)%3) ? " " : " | ");
      if (!((i+1)%27) && i!=80) printf("------------------------------------------------\n");
    }
  printf("\n");
}


int rule_out(int level,sudoku *mySud)
{
  int i,j,k,l;
  int c,prog=0;
  char string[16];
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      for (k=0;k<3;k++)
        for (l=0;l<3;l++)
        {
          c = i*27+j*9+k*3+l;
          if (countbits(mySud->grid[c])==1)
          {
            sprintf(string,"row %d", i*3+j+1);
            prog += unsetbit(level,mySud->row[i*3+j],k*3+l,mySud->grid[c],string);
            sprintf(string,"column %d", k*3+l+1);
            prog += unsetbit(level,mySud->col[k*3+l],i*3+j,mySud->grid[c],string);
            sprintf(string,"block %d", i*3+k+1);
            prog += unsetbit(level,mySud->box[i*3+k],j*3+l,mySud->grid[c],string);
          }
        }
  return prog;
}

char *digitstr(char *chararr, int nums)
{
  char chararr2[32];
  int n,i,cnt=0,cand=EEN;
  n = countbits(nums);
  strcpy(chararr,"");
  for (i=0;i<9;i++)
  {
    if (cand & nums)
    {
      cnt++;
      sprintf(chararr2, "%s%s%d", chararr, (cnt==1) ? "" : (cnt<=n-1) ? ", " : " and ", i+1);
      strcpy(chararr,chararr2);
    }
    cand = cand << 1;
  }
  return chararr;
}




int unsetbit(int level,int **set, int idx, int value, char *str)
{
  int i,prog=0;
  for(i=0;i<9;i++)
    if (i != idx)
    {
      if (level == 0 && debug >= 3 && countbits(*set[i]) > 1 && countbits(*set[i] & ~value) == 1)
        printf("%03d. Cell %d of %s must be %s for there are no other options.\n", hintno++, i+1, str, digitstr(numstring,*set[i] & ~value));
      prog = prog || (*set[i] & value);
      *set[i] &= ~value;
    }
  return prog;
}

int countbits(int value)
{
  int i,tot=0;
  for (i=0;i<9;i++)
  {
    tot += (0x1 & value);
    value >>= 1;
  }
  return tot;
}

int possible(sudoku *mySud)
{
  int i,pos=1;
  for(i=0;pos && i<81;i++)
    pos = pos && countbits(mySud->grid[i]);
  return pos;
}

int solved(sudoku *mySud)
{
  int i,sol=1;
  for(i=0;sol && i<81;i++)
    sol = sol && (countbits(mySud->grid[i]) == 1);
  return sol;
}


int counter_hypothesize(sudoku *mySud, int n)  /* level n hypothese: kopieer mySud, veronderstel waarde vrije cel en toon onoplosbaarheid aan */
{
  int sol=0, pos=1;
  sudoku *newSud;
  int i,j;
  int cand;
  char string[256],string2[256];
  int rowidx;
  int progress = 1;

  
  maxlevel = (n>maxlevel) ? n : maxlevel;
  
  newSud = malloc(sizeof(sudoku));
  init(newSud);
  
  while(progress && !sol && pos)
  {
    while(progress && !sol && pos)
    {
      progress = 0;
      progress = rule_out(n,mySud);  /* cel altijd lid van 3 sets. cel waarin precies 1 bit op staat, schakelt zelfde bit alle 3 setgenootcellen uit */
      pos = possible(mySud);
      sol = solved(mySud);
    }
    if (debug>=2 && debug!=3)
      print_puzzle(mySud,"AFTER ITERATIVE RULE OUT");
        
    /* geen progressie meer met wegstrepen */
    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"row %d", i+1);
        progress += findsoleoption(n,mySud->row[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FIND SOLE OPTION (row)");
    }
    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"column %d", i+1);
        progress += findsoleoption(n,mySud->col[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FIND SOLE OPTION (column)");
    }
    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"block %d", i+1);
        progress += findsoleoption(n,mySud->box[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FIND SOLE OPTION (block)");
    }

    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"row %d", i+1);
        progress += stragglerelim(n,mySud->row[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER STRAGGLER ELIMINATION (row)");
    }

    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"column %d", i+1);
        progress += stragglerelim(n,mySud->col[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER STRAGGLER ELIMINATION (column)");
    }

    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        sprintf(string,"block %d", i+1);
        progress += stragglerelim(n,mySud->box[i], string);
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER STRAGGLER ELIMINATION (box)");
    }
    
    if (!progress)
    {
      for (i=0;i<9;i++)
      {
        for (j=0;j<3;j++)
        {
          rowidx = (int)(3*(int)(i/3)+j);
          sprintf(string,"row %d",i+1);
          sprintf(string2,"block %d",rowidx+1);
          progress += compulsaryblockpart(n,mySud->row[i],j,mySud->box[rowidx],i%3,string,string2);
        }
      }
      for (i=0;i<9;i++)
      {
        for (j=0;j<3;j++)
        {
          sprintf(string,"column %d",i+1);
          sprintf(string2,"block %d",(int)i/3+3*j+1);
          progress += compulsaryblockpart(n,mySud->col[i],j,mySud->box[(int)i/3+3*j],i%3+3,string,string2);
        }
      }
      for (i=0;i<9;i++)
      {
        for (j=0;j<3;j++)
        {
          rowidx = (int)(3*(int)(i/3)+j);
          sprintf(string,"block %d",i+1);
          sprintf(string2,"row %d",rowidx+1);
          progress += compulsaryblockpart(n,mySud->box[i],j,mySud->row[rowidx],i%3,string,string2);
        }
      }
      for (i=0;i<9;i++)
      {
        for (j=0;j<3;j++)
        {
          sprintf(string,"block %d",i+1);
          sprintf(string2,"column %d",3*(i%3)+j+1);
          progress += compulsaryblockpart(n,mySud->box[i],j+3,mySud->col[3*(i%3)+j],(int)floor(i/3),string,string2);
        }
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER COMPULSARY BLOCKPART (cand anywhere)");
    }
    
    if (!progress)
    {
      for(i=0;i<9;i++)
      {
        sprintf(string,"row %d", i+1);
        progress += findchain(n,mySud->row[i],string); 
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FINDCHAIN (row)");
    }

    if (!progress)
    {
      for(i=0;i<9;i++)
      {
        sprintf(string,"column %d", i+1);
        progress += findchain(n,mySud->col[i],string); 
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FINDCHAIN (col)");
    }

    if (!progress)
    {
      for(i=0;i<9;i++)
      {
        sprintf(string,"block %d", i+1);
        progress += findchain(n,mySud->box[i],string); 
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER FINDCHAIN (box)");
    }
    
    if (!progress)
    {
      progress += xwing(n,mySud,2,"sudoku");
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER XWING (box)");
    }

    /*    */
    
    pos = possible(mySud);
    sol = solved(mySud);

    if (!progress)
    {
      if (debug>=2 && debug!=3)
        printf("GOING INTO COUNTER HYPOTHESIZE\n\n");
      if (!sol && pos && n<hypo)
      {
        for (i=0;!progress && i<81;i++)
          if (countbits(mySud->grid[i]) >= 2)
          {
            cand = EEN;
            for (j=0;j<9;j++)
            {
              if (mySud->grid[i] & cand)
              {
                copy(mySud,newSud);
                newSud->grid[i] = cand;
                counter_hypothesize(newSud,n+1);
                if (!possible(newSud))
                {
                  mySud->grid[i] &= ~cand;
                  progress = 1;
                  if (debug >= 3)
                    printf("%03d. Assuming digit %s in cell %d of grid invalidates entire sudoku. Dismissed as candidate.\n", hintno++, digitstr(numstring,cand), i+1);
                  break;
                }
              }
              cand << 1;
            }
          }
      }
      if (debug>=2 && debug!=3)
        print_puzzle(mySud,"AFTER COUNTER HYPOTHESIZE");
    }

    pos = possible(mySud);
    sol = solved(mySud);
  }
  
  free(newSud);
  
  return sol;
}

void copy(sudoku *p1, sudoku *p2)
{
  int i;
  for (i=0;i<81;i++)
    p2->grid[i] = p1->grid[i];
}

void init(sudoku *mySud)
{
  int i,j,k,l;
  int c;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      for (k=0;k<3;k++)
        for (l=0;l<3;l++)
        {
          c = i*27+j*9+k*3+l;
          mySud->row[i*3+j][k*3+l] = mySud->grid+c;
          mySud->col[k*3+l][i*3+j] = mySud->grid+c;
          mySud->box[i*3+k][j*3+l] = mySud->grid+c;
          mySud->grid[c] = 0x1FF;  /* default: 9 bits up */
        }
}

void fill_in(sudoku *mySud, char *filename)
{
  FILE *fp;
  int i,j,r[9];
  fp = fopen(filename,"ra");
  if (!fp)
    { fprintf(stderr,"Error opening file\n"); exit(1);}
  for (i=0;i<9;i++)
  {
    fscanf(fp, "%d %d %d %d %d %d %d %d %d", r, r+1, r+2, r+3, r+4, r+5, r+6, r+7, r+8);
    for (j=0;j<9;j++)
      if (r[j] > 0)
        *mySud->row[i][j] = 0x1 << (r[j]-1);
  }
  fclose(fp);
}

fill_in_string(sudoku *mySud)
{
  // _4398_25_+6__425___+2____1_94+9____4_7_+3__6_8___+41_2_9__3+82_5_____+____4___5+53489_71_
  int i,j;
  char c;

  for (i=0;i<9;i++)
    for (j=0;j<9;j++)
    {
      c = sudokusolver_string[i*10+j];
      if (c != '_')
        *mySud->row[i][j] = EEN << (atoi(&c)-1);
    }
}
/*
void fill_in2()
{
  *box[0][4] = VIER;  *box[0][8] = ACHT;
  *box[1][1] = ZEVEN; *box[1][2] = EEN;   *box[1][5] = TWEE;  *box[1][6] = ZES; //   *box[1][8] = VIER;
  *box[2][0] = ZES;   *box[2][1] = NEGEN; *box[2][5] = EEN;   *box[2][6] = ZEVEN;
  *box[3][1] = NEGEN; *box[3][3] = TWEE;  *box[3][5] = ZEVEN; *box[3][7] = DRIE;  *box[3][8] = VIER;
  *box[4][2] = ZES;   *box[4][7] = TWEE;
  *box[5][0] = EEN;   *box[5][1] = ZEVEN; *box[5][5] = ACHT;
  *box[6][1] = ACHT;  *box[6][7] = TWEE;
  *box[7][0] = NEGEN; *box[7][1] = VIER;
  *box[8][6] = VIJF;  *box[8][7] = VIER;
}
*/
