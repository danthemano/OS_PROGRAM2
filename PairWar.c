#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

//creates an array of 52 cards
int deck_size = 52;
int deck[deck_size];

//a bool value to determine if the winner of a round is found
bool isWon = false;

/* Structure for player so we can keep track of their cards */
typedef struct player {
	int id;
	int card1;
	int card2;
}

//creates the 3 players for the game
player player1;
player player2;
player player3;

void isWinner(player player)
{
	int i;
	for(i = 0; i < 3; i++)
	{
		printf("PLAYER %d: Exits round", (player.id + i) % 4);
	}
	
	//resets the cards of each player
	player1.card1 = 0;
	player1.card2 = 0;
	player2.card1 = 0;
	player2.card2 = 0;
	player3.card1 = 0;
	player3.card2 = 0;
	
	isWon = true;
	
}

//shuffles the deck of cards
void shuffle(int deck)
{
	//resets the size of the deck
	deck_size = 52;
	
	int i, j, temp;
	for(i = 0; i < 52; i++) {
		j = i + rand() / (RAND_MAX / (52 - i) + 1);
		temp = cards[i];
		cards[i] = cards[j];
		cards[j] = temp;
	}
}

//prints out the content of the deck
void displayDeck() 
{
	printf("DECK: ");
	int i;
	for(i = 0; i < DECK_SIZE; i++)
		printf("%d ", deck[i]);
	printf("\n");	
}

//takes an integer and returns the face card value
string intToString(int i)
{
	switch(i)
	{
		case 11: return "Jack";
		case 12: return "Queen";
		case 13: return "King";		
	}
}

void getSecondCard(player player)
{
	int card2 = getCard();
	
	//assigns a second card to the card spot that is open and displays the hand
	if(player.card1 == 0)
	{
		printf("PLAYER %d: Hand %d", player.id, player.card2);
		player.card1 = card2;
		printf("PLAYER %d: Draws %d", player.id, player.card1);
	}	
	else
	{
		printf("PLAYER %d: Hand %d", player.id, player.card1);
		player.card2 = card2;
		printf("PLAYER %d: Draws %d", player.id, player.card2);
	}

	//checks to see if the player won
	if(player.card1 == player.card2)
	{
		showHand(player);
		printf("PLAYER %d: Wins", player.id);
		
		//initiates the win sequence
		isWinner(player);	
	}
	
	else
	{
		//decides at random what card gets discarded
		int discard = rand() % 2;
		
		//displays the discarded card and sets it to 0
		switch(discard)
		{
			case 0: 
				printf("PLAYER %d: Discards %d", player.id, player.card2);
				player.card2 = 0;
				break;
			case 1:
				printf("PLAYER %d: Discards %d", player.id, player.card1);
				player.card1 = 0;
		}
		
		//if it is the last player of the round, displays the deck
		if(player.id == 3)
		displayDeck();
	}
	
}

//puts a card on the bottom of the deck
void returnCardToDeck(int card) {
	deck[deck_size] = card;
	deck_size++;
}

//returns the top card of the deck and moves each card one space up the array
int getCard()
{
	int card = deck[0];
	
	int i;
	for(i = 0; i < deck_size - 1; i++)
		deck[i] = deck[i + 1];
		
	deck_size--;
	return card;
}

void startRound() 
{
	shuffle(deck);
	
	
	
	//deals the first card to each player
	player1.card1 = getCard();
	player2.card1 = getCard();
	player3.card1 = getCard();
	
	//keeps track of the current player that is dealt a card
	int currentPlayer = 1;
	
	while(!isWon)
	{
		switch(currentPlayer)
		{
			case 1:
				getSecondCard(player1);
				break;
			case 2:
				getSecondCard(player2);
				break;
			case 3:
				getSecondCard(player3);
		}
		
		currentPlayer = currentPlayer + 1 % 3;
	}
}
