#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


//Struct to reprsent players
typedef struct player
{
    int id;
    int card1;
    int card2;
}player;

//Critical section protection via mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Creates a shared deck of 52 cards
int deckSize;
int deck[52];

//Notification variable for winner
int winner;

//Creates semaphore
sem_t dealerSemaphore;

//Logging file for results
FILE *file;

//Creates three players for the game
player player1;
player player2;
player player3;

//Draw card from deck
int getCard()
{
    int i;
    int card;

    //sets card to top card of the deck
    card = deck[0];

    //Moves elements of deck to the left one at a time
    for(i = 0; i < deckSize - 1; i++)
        deck[i] = deck[i + 1];

    //decrements the deck size
    deckSize--;

    //Returns top card of the deck
    return card;
}

//Puts the returned card to the bottom of the deck
void returnCard(int card)
{
    deck[deckSize] = card;
    deckSize++;
}

//Displays the shuffled deck to the console
void displayDeck()
{
    int i;

    printf("Deck: ");

    for(i = 0; i < deckSize; i++)
        printf("%d ", deck[i]);
    printf("\n");
}

//Sends deck info to the log file
void logDeck()
{
    int i;

    fprintf(file, "Deck: ");

    for(i = 0; i < deckSize; i++)
        fprintf(file, "%d ", deck[i]);
    fprintf(file, "\n");
}

//Dealer thread handled
void *handleDealer(void *args)
{
    //Creates temporary variables for this function
    int i;
    int j;
    int tmp;


    fprintf(file, "DEALER: Shuffle\n");

    //Shuffles deck
    for(i = 0; i < 52; i++)
    {
        j = i + rand() / (RAND_MAX / (52 - i) + 1);
        tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
    //Beginning round, no winner, shuffled deck contains 52 cards
    deckSize = 52;
    winner = -1;

    displayDeck();

    //Dealer deals 1 card to each player
    player1.card1 = getCard();
    player2.card1 = getCard();
    player3.card1 = getCard();

    //Unlocks the semaphore to allow each player to play
    sem_post(&dealerSemaphore);
    sem_post(&dealerSemaphore);
    sem_post(&dealerSemaphore);
}

//Players threads handled
void *handlePlayers(void *args)
{
    player *currentPlayer;
    int win;

    //Sets current player to player passed in from pthhread_create
    currentPlayer = (player *)(args);
    win = -1;

    //Locks the semaphore
    sem_wait(&dealerSemaphore);

    //Loops until a player wins
    while(1)
    {
        pthread_mutex_lock(&mutex);

        //Stop if somebody won
        if(winner != -1)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        //Displays card left in the players hand
        if(currentPlayer->card1 != -1)
        {
            fprintf(file, "PLAYER %d: hand %d\n", currentPlayer->id, currentPlayer->card1);
        }
        else
        {
            fprintf(file, "PLAYER %d: hand %d\n", currentPlayer->id, currentPlayer->card2);
        }

        //Get a new card for card1 or card2 depending on which card they dont currently have
        if(currentPlayer->card1 == -1)
        {
            currentPlayer->card1 = getCard();
            fprintf(file, "PLAYER %d: draws %d\n", currentPlayer->id, currentPlayer->card1);
        }
        else
        {
            currentPlayer->card2 = getCard();
            fprintf(file, "PLAYER %d: draws %d\n", currentPlayer->id, currentPlayer->card2);
        }

        //Displays both cards
        fprintf(file, "PLAYER %d: hand %d %d\n", currentPlayer->id, currentPlayer->card1, currentPlayer->card2);

        printf("PLAYER %d: hand %d %d\n", currentPlayer->id, currentPlayer->card1, currentPlayer->card2);

        //Checks to see if player won
        if(currentPlayer->card1 == currentPlayer->card2)
        {
            //Stop if player won
            printf("PLAYER %d WIN: yes\n", currentPlayer->id);
            winner = currentPlayer->id;
            pthread_mutex_unlock(&mutex);
            break;
        }

        printf("PLAYER %d WIN: no\n", currentPlayer->id);

        //If hand didnt win, discard random card
        if(rand() % 2 == 0)
        {
            fprintf(file, "PLAYER %d: discards %d\n", currentPlayer->id, currentPlayer->card1);
            returnCard(currentPlayer->card1);
            currentPlayer->card1 = -1;
        }
        else
        {
            fprintf(file, "PLAYER %d: discards %d\n", currentPlayer->id, currentPlayer->card2);
            returnCard(currentPlayer->card2);
            currentPlayer->card2 = -1;
        }

        //Displays card left in hand
        if(currentPlayer->card1 != -1)
        {
            fprintf(file, "PLAYER %d: hand %d\n", currentPlayer->id, currentPlayer->card1);
        }
        else
        {
            fprintf(file, "PLAYER %d: hand %d\n", currentPlayer->id, currentPlayer->card2);
        }

        //Display updated deck
        displayDeck();
        logDeck();
        pthread_mutex_unlock(&mutex);
    }

    //Exit round
    if(winner == currentPlayer->id)
    {
        fprintf(file, "PLAYER %d: wins and exits round\n", currentPlayer->id);
    }
    else
    {
        fprintf(file, "PLAYER %d: exits round\n", currentPlayer->id);
    }

    return (void *)NULL;
}


int main(int argc, char *argv[])
{
    int round = 0;
    int i;
    int j;

    pthread_t player1_thread;
    pthread_t player2_thread;
    pthread_t player3_thread;
    pthread_t dealer_thread;

    srand((unsigned)time(NULL));

    //Initialise deck
    deckSize = 0;

    for(i = 0; i < 4; i++)
    {
        for(j = 1; j <= 13; j++)
            deck[deckSize++] = j;
    }

    //Initializes the player attributes
    player1.id = 1;
    player1.card1 = -1;
    player1.card2 = -1;

    player2.id = 2;
    player2.card1 = -1;
    player2.card2 = -1;

    player3.id = 3;
    player3.card1 = -1;
    player3.card2 = -1;


    file = fopen("log.txt", "w");

    //Plays 3 rounds
    for(round = 0; round < 3; round++)
    {
        fprintf(file, "ROUND %d\n", (round + 1));
        printf("ROUND %d\n", (round + 1));

        //Initialize semaphore
        sem_init(&dealerSemaphore, 0, 1);

        //Create player threads
        pthread_create(&player1_thread, NULL, &handlePlayers, &player1);
        pthread_create(&player2_thread, NULL, &handlePlayers, &player2);
        pthread_create(&player3_thread, NULL, &handlePlayers, &player3);

        //Create dealer thread
        pthread_create(&dealer_thread, NULL, &handleDealer, NULL);

        //Wait for each thread to finish before starting another round
        pthread_join(player1_thread, NULL);
        pthread_join(player2_thread, NULL);
        pthread_join(player3_thread, NULL);
        pthread_join(dealer_thread, NULL);

        fprintf(file, "\n");
        printf("\n");
    }

    fclose(file);

    return 0;
}


