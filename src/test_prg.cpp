//Tanay Khandke
//gold chase p1

#include "goldchase.h"
#include "Map.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <cstdlib>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;
//struct mapboard

struct mapboard
{
  int rows;
  int cols;
  unsigned char players;
  unsigned char map[0];
};


int main()
{
  string numgold;
  ifstream mystream;
  mapboard* mb;
  string size_mapcol;

  srand(time(NULL));

  //opening map
  mystream.open("mymap.txt");
  int numrow=0;
  int numcol ;

  sem_t* sem_ptr;
//semaphore,

//first player
//sem_wait

  while (!mystream.eof())
  {

    //reading number of gold
    getline(mystream,numgold);

    while (getline(mystream, size_mapcol))
    {
      numrow++;

      numcol = size_mapcol.size();
    }



  }

  mystream.close();

  int int_numgold = atoi (numgold.c_str());
  int arrsize = numcol * numrow;


  //sem opening
  sem_ptr = sem_open("/Semaphore_open",O_CREAT | O_EXCL, S_IRUSR|S_IWUSR,1);

  //error checking of Semaphore
  if (sem_ptr == SEM_FAILED)
  {
    if (errno != EEXIST)
    {
        perror ("Opening Semaphore");
        exit(1);
    }
  }
  if(sem_ptr != SEM_FAILED)
  {
//opening shared memory
  int fd=shm_open("/tan_sem", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
  ftruncate(fd,arrsize + sizeof(mapboard));

  //mapping
  mb= (mapboard*)mmap(NULL, arrsize+ sizeof(mapboard), PROT_READ|PROT_WRITE,
  MAP_SHARED, fd, 0);
  mb->rows=numrow;
  mb->cols=numcol;

  ifstream myfile;
  myfile.open("mymap.txt");

  char ch;
  char arrayMap[arrsize];
  string ignorenum;
  getline(myfile, ignorenum);
  int i =0;

//puuting map in shared memory
  while (!myfile.get(ch).eof())
  {
	   if(ch==' ' || ch=='*')
     {
       arrayMap[i] = ch;

       i++;
     }
  }
//closing file
  myfile.close();


//read map
  int j = 0;
  while (j < arrsize ) /*j <= sizeof(arrayMap)*/
  {
    if (arrayMap[j] == ' ')
    {
      mb->map[j] = 0;

    }
    else if (arrayMap[j] == '*')
    {
      mb->map[j] = G_WALL;
    }
    j++;
  }


  Map goldMine((const unsigned char*) mb->map, mb->rows,mb->cols);

  goldMine.drawMap();
  int pos_random = 0;
  int a,b;


  //randomly placing gold

  for (a = 0; a < int_numgold-1 ; a++ )
  {
    pos_random = rand()% arrsize;
    while (mb->map[pos_random] != 0)
    {
      pos_random = rand()%arrsize;
    }
    mb->map[pos_random] = G_FOOL;
  }
  pos_random = rand()% arrsize;

  while (mb->map[pos_random] != 0)
  {
    pos_random= rand()% arrsize;
  }

  if (int_numgold > 0)
  {
    mb->map[pos_random] = G_GOLD;
  }
  goldMine.drawMap();


  while (mb->map[pos_random] != 0)
  {
    pos_random = rand()%arrsize;
  }
  mb->map[pos_random] = G_PLR0;
  goldMine.drawMap();

  char key = goldMine.getKey();

  int currpos = pos_random;

  mb->players |=G_PLR0; //turn on bit
sem_post(sem_ptr);

int flag = 0;  //found true gold
//key manipulation
  while (key != 'Q')
  {
    sem_wait(sem_ptr);

    if (key == 'h')
    {
    if ((currpos % mb->cols) != 0)
    {
        if (mb->map[currpos-1] == G_WALL)
        {
            mb->map[currpos] |= G_PLR0;
        }

        else if (mb->map[currpos-1] == G_GOLD)
        {
          //player finds true gold and reaches edge and post msg
            mb->map[currpos-1] |= G_PLR0; //turn on
            mb->map[currpos] &=~ G_PLR0; //turn off
            flag = 1;
            mb->map[currpos] &=~ G_GOLD;
            currpos= currpos-1;
            goldMine.drawMap();
            goldMine.postNotice ("Congratulations You Have found the real gold.Move to the edge of the map to exit!");
        }
        else if (mb->map[currpos-1]== G_FOOL)
        {
          //found fool gold
            mb->map[currpos-1] |= G_PLR0;
            mb->map[currpos] &=~ G_PLR0;
            currpos= currpos-1;
            goldMine.drawMap();
            goldMine.postNotice ("You have found Fool's Gold");
        }

        else
        {
            mb->map[currpos-1] |= G_PLR0;
            mb->map[currpos] &=~ G_PLR0;
            currpos= currpos-1;
        }
        goldMine.drawMap();

    }
    else if (flag == 1)
    {
        mb->map[currpos] &=~ G_PLR0;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ G_PLR0;
        break;

    }
    }
    else if (key == 'j')
    {

        if ((currpos + mb->cols )<arrsize)
        {
            if (mb->map[currpos+ mb->cols] == G_WALL)
            {
                mb->map[currpos] |= G_PLR0;
            }

            else if (mb->map[currpos + mb->cols] == G_GOLD)
            {
                mb->map[currpos+mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos+mb->cols;
                flag = 1;
                 mb->map[currpos] &=~ G_GOLD;
                goldMine.drawMap();
                goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
            }

            else if (mb->map[currpos + mb->cols] == G_FOOL)
            {
                mb->map[currpos+mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos+mb->cols;
                goldMine.drawMap();
                goldMine.postNotice ("You found Fool's Gold");
            }

            else
            {
                mb->map[currpos+mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos+mb->cols;

            }
        }
        else if (flag == 1)
    {
      //found gold but not on edge
        mb->map[currpos] &=~ G_PLR0;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ G_PLR0;
        break;

    }

        goldMine.drawMap();

    }
    else if (key == 'k')
    {

        if ((currpos-mb->cols)> 0)
        {

            if (mb->map[currpos-mb->cols] == G_WALL)
            {
                mb->map[currpos] |= G_PLR0;
            }

            else if (mb->map[currpos-mb->cols] == G_GOLD)
            {
                mb->map[currpos-mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos-mb->cols;
                flag = 1;
                mb->map[currpos] &=~ G_GOLD;
                goldMine.drawMap();
                goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
            }
            else if (mb->map[currpos-mb->cols] == G_FOOL)
            {
                mb->map[currpos-mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos-mb->cols;
                goldMine.drawMap();
                goldMine.postNotice ("You have found Fool's Gold");
            }

            else
            {
                mb->map[currpos-mb->cols] |= G_PLR0;
                mb->map[currpos] &=~ G_PLR0;
                currpos= currpos-mb->cols;
            }
        }

else if (flag == 1)
    {
        mb->map[currpos] &=~ G_PLR0;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!");
        mb->players &=~ G_PLR0;
        break;

    }


        goldMine.drawMap();
    }
    else if (key == 'l')
    {

        if (currpos%(mb->cols) != (mb->cols-1))
        {
        if (mb->map[currpos+1] == G_WALL)
        {
            mb->map[currpos] |= G_PLR0;
        }
        else if (mb->map[currpos+1] == G_GOLD)
        {
            mb->map[currpos+1] |= G_PLR0;
            mb->map[currpos] &=~ G_PLR0;
            currpos= currpos+1;
            flag = 1;
            mb->map[currpos] &=~ G_GOLD;
            goldMine.drawMap();
            goldMine.postNotice ("Congratulations You Have found the real gold.Move to the edge of the map to exit!");
        }
        else if (mb->map[currpos+1] == G_FOOL)
        {
            mb->map[currpos+1] |= G_PLR0;
            mb->map[currpos] &=~ G_PLR0;
            currpos= currpos+1;
            goldMine.drawMap();
            goldMine.postNotice ("You have found Fool's Gold");
        }

        else
        {
            mb->map[currpos+1] |= G_PLR0;
            mb->map[currpos] &=~ G_PLR0;
            currpos= currpos+1;
        }
        }
        else if (flag == 1)
        {
        mb->map[currpos] &=~ G_PLR0;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ G_PLR0;
        break;

        }

        goldMine.drawMap();
   }
   key = goldMine.getKey();

    sem_post(sem_ptr);

  }
  //turn off player 1
    mb->map[currpos] &=~ G_PLR0;
    mb->players &=~ G_PLR0;
}
//assigning other players

else
{
    sem_ptr = sem_open("/Semaphore_open",O_RDWR, S_IRUSR|S_IWUSR,1);
    int fd=shm_open("/tan_sem", O_RDWR, S_IRUSR|S_IWUSR);
    int row;
    int col;
    read(fd,&row,sizeof(int));
    read(fd,&col,sizeof(int));

//mapping
    int arrsize1 = row*col;
    mb= (mapboard*)mmap(NULL, arrsize1+ sizeof(mapboard),
    PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    Map goldMine((unsigned const char*) mb->map, row,col);
    unsigned char currentPlayer = 0;

    //assign players
    if (!(mb->players & G_PLR0))
    {
        currentPlayer = G_PLR0;
    }
    else if (!(mb->players & G_PLR1))
    {
        currentPlayer = G_PLR1;
    }
    else if (!(mb->players & G_PLR2))
    {
        currentPlayer = G_PLR2;
    }
    else if (!(mb->players & G_PLR3))
    {
        currentPlayer = G_PLR3;
    }
    else if (!(mb->players & G_PLR4))
    {
        currentPlayer = G_PLR4;
    }

    int pos_random = rand() % arrsize;


    while (mb->map[pos_random] != 0)
    {
        pos_random = rand()%arrsize;
    }
    mb->map[pos_random] |= currentPlayer;
    goldMine.drawMap();


    int currpos = pos_random;
    int key;

    //turn on current player
  mb->players |= currentPlayer;


int flag = 0;


  while (key != 'Q')
  {
    sem_wait(sem_ptr);

    if (key == 'h')
    {
    if (currpos % mb->cols != 0)
    {
        if (mb->map[currpos-1] == G_WALL)
        {
            mb->map[currpos] |= currentPlayer;
        }

        else if (mb->map[currpos-1] == G_GOLD)
        {

            mb->map[currpos-1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos-1;
            flag = 1;
            mb->map[currpos] &=~ G_GOLD;
            goldMine.drawMap();
            goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
        }
        else if (mb->map[currpos-1]== G_FOOL)
        {
            mb->map[currpos-1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos-1;
            goldMine.drawMap();
            goldMine.postNotice ("You have won Fool's Gold");
        }

        else
        {
            mb->map[currpos-1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos-1;
        }
        }
         else if (flag == 1)
        {
        mb->map[currpos] &=~ currentPlayer;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ currentPlayer;
        break;

        }
        goldMine.drawMap();

    }
    else if (key == 'j')
    {

        if ((currpos + mb->cols )<arrsize)
        {
            if (mb->map[currpos+ mb->cols] == G_WALL)
            {
                mb->map[currpos] |= currentPlayer;
            }

            else if (mb->map[currpos + mb->cols] == G_GOLD)
            {
                mb->map[currpos+mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;

                currpos= currpos+mb->cols;
                flag = 1;
            mb->map[currpos] &=~ G_GOLD;
                goldMine.drawMap();
                goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
            }

            else if (mb->map[currpos + mb->cols] == G_FOOL)
            {
                mb->map[currpos+mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;
                currpos= currpos+mb->cols;
                goldMine.drawMap();
                goldMine.postNotice ("You found Fool's Gold");
            }

            else
            {
                mb->map[currpos+mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;
                currpos= currpos+mb->cols;

            }
        }
         else if (flag == 1)
        {
        mb->map[currpos] &=~ currentPlayer;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ currentPlayer;
        break;

        }



        goldMine.drawMap();

    }
    else if (key == 'k')
    {

        if ((currpos-mb->cols)> 0)
        {

            if (mb->map[currpos-mb->cols] == G_WALL)
            {
                mb->map[currpos] |= currentPlayer;
            }

            else if (mb->map[currpos-mb->cols] == G_GOLD)
            {
                mb->map[currpos-mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;
                currpos= currpos-mb->cols;
                flag = 1;
            mb->map[currpos] &=~ G_GOLD;
                goldMine.drawMap();
                goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
            }
            else if (mb->map[currpos-mb->cols] == G_FOOL)
            {
                mb->map[currpos-mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;
                currpos= currpos-mb->cols;
                goldMine.drawMap();
                goldMine.postNotice ("You have found Fool's Gold");
            }

            else
            {
                mb->map[currpos-mb->cols] |= currentPlayer;
                mb->map[currpos] &=~ currentPlayer;
                currpos= currpos-mb->cols;
            }
        }
 else if (flag == 1)
        {
        mb->map[currpos] &=~ currentPlayer;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ currentPlayer;
        break;

        }

        goldMine.drawMap();
    }
    else if (key == 'l')
    {

        if (currpos%(mb->cols) != (mb->cols - 1))
        {
        if (mb->map[currpos+1] == G_WALL)
        {
            mb->map[currpos] |= currentPlayer;
        }
        else if (mb->map[currpos+1] == G_GOLD)
        {
            mb->map[currpos+1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos+1;
            flag = 1;
            mb->map[currpos] &=~ G_GOLD;
            goldMine.drawMap();
            goldMine.postNotice ("Congratulations You Have found the real gold. Move to the edge of the map to exit!");
        }
        else if (mb->map[currpos+1] == G_FOOL)
        {
            mb->map[currpos+1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos+1;
            goldMine.drawMap();
            goldMine.postNotice ("You have found Fool's Gold");
        }

        else
        {
            mb->map[currpos+1] |= currentPlayer;
            mb->map[currpos] &=~ currentPlayer;
            currpos= currpos+1;
        }
}
 else if (flag == 1)
        {
        mb->map[currpos] &=~ currentPlayer;
        goldMine.drawMap();
        goldMine.postNotice ("Congrats! You have won!!!");
        mb->players &=~ currentPlayer;
        break;

        }
        goldMine.drawMap();
   }

    sem_post(sem_ptr);
    key = goldMine.getKey();

  }
  //turn off current player
  mb->map[currpos] &=~ currentPlayer;
  mb->players &=~ currentPlayer;
}
//unlinking semaphore and shared memory
if (mb->players == 0)//no player playing
{
  shm_unlink("/tan_sem");
  sem_close(sem_ptr);
  sem_unlink("/Semaphore_open");
}
  return 0;
}
