//CMP104 - Programming with C++: Assignment
//Bridget Casey - 1802644

#define NOMINMAX //Avoids conflicts when including windows.h

#include<stdio.h> 
#include<stdlib.h> 
#include<ctype.h>  
#include<conio.h> 
#include<windows.h>
#include<string>
#include<array>
#include<time.h>
#include<iostream>

#define MAX_X_POSITION	77
#define SCROLL_POSITION	24

const int consoleSize_x = 80;	//Max width of the console
const int consoleSize_y = 25;	//Max height of the console

const int totalRows = 7;	//The total number of rows that can be displayed in a game
const int totalColumns = 9;	//The maximum number of columns a player can choose to be displayed in a game
int columnDifference;	//The difference between the total number of columns and the number the player has selected
bool isSpinning;	//For checking if the columns are currently spinning
bool isPlaying;		//For checking if the game is currently running
bool colourBonusOn;	//For checking if the player wants a colour bonus
bool uniqueCharsOn;	//For checking if the player wants to have unique character effects in their game
bool appliedBonusSpin;	//For checking if a free bonus spin has previously been applied

int cash = 500;		//Base cash the player begins the game with, DEFAULT: 500
const int default_cashCost = 10;	//The default cost for playing one game
int cashCost;	//The total cost of playing one game

//---Method Prototypes---
void init_Render();
void set_Colour(int fcolour, int bcolour);
void render_String(int x, int y, std::string string);
void render_Char(int x, int y, char character);
int checkUserInput(int userInput, int lowerLimit, int upperLimit);
void quitGame();
void main_Screen();
void generate_Char_Reel(char machineCharacters[][totalRows], int machineCharColours[][totalRows]);
void render_Char_Reel(int x, int y, char machineCharacters[][totalRows], int machineCharColours[][totalRows]);
void check_Win(char winningChars[][totalRows], int winningCharColours[][totalRows]);
void clear(int startX, int startY, int finalX, int finalY);

CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
HANDLE handleConsole;

void init_Render()	//Initialises the console window to begin rendering
{
	COORD consoleSize = {consoleSize_x, consoleSize_y};
	srand((unsigned)time(NULL));

	handleConsole = CreateFile(TEXT("CONOUT$"), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0L, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);

	SetConsoleScreenBufferSize(handleConsole, consoleSize);
	GetConsoleScreenBufferInfo(handleConsole, & consoleInfo);
}

void set_Colour(int foreground_colour, int background_colour)	//Sets the current colour to be rendered to the console
{
	SetConsoleTextAttribute(handleConsole, (WORD)((background_colour << 4) | foreground_colour));
}

void render_String(int x, int y, std::string string)	//Renders a basic string at a given x,y position in the console
{
	COORD cursorPosition;

	cursorPosition.X = x;
	cursorPosition.Y = y;

	SetConsoleCursorPosition(handleConsole, cursorPosition);

	std::cout << string;
}

void render_Char(int x, int y, char character)	//Renders a basic character at a given x,y position in the console
{
	COORD cursorPosition;

	cursorPosition.X = x;
	cursorPosition.Y = y;

	SetConsoleCursorPosition(handleConsole, cursorPosition);

	std::cout << character;
}

//Input validation for the options menu
int checkUserInput(int userInput, int lowerLimit, int upperLimit)
{
	std::cin >> userInput;

	//Takes in a lower and upper limit that the input should be between and compares it to the user's current input
	while (!userInput || !(userInput >= lowerLimit && userInput <= upperLimit))
	{
		clear(5, 18, consoleSize_x, consoleSize_y);
		render_String(5, 20, "Invalid input. Please try again.");
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Ignores all characters equal to the current size of the input stream
		std::cin >> userInput;
	}

	return userInput;
}

//Allows the player to quit the game
void quitGame()
{
	clear(5, 6, consoleSize_x, consoleSize_y);
	render_String(5, 8, "Are you sure you want to quit? Yes (Y + Q)/No (N + Q)");

	//If Y and Q are pressed sequentially, program halts and prints a message to the console
	if (GetAsyncKeyState('Y'))
	{
		isPlaying = false;
		clear(0, 0, consoleSize_x, consoleSize_y);

		std::cout << "\nQUTTING GAME\n" << std::endl;
	}

	//If N and Q are pressed sequentially, loops back to the main menu
	else if (GetAsyncKeyState('N'))
	{
		clear(5, 5, consoleSize_x, consoleSize_y);
		render_String(5, 5, "Continuing gameplay...");
		
		main_Screen();
	}
}

//Main menu screen for navigating to playing the game, options, or quitting
void main_Screen()
{
	Sleep(500);
	set_Colour(15, 0);
	render_String(5, 3, "Cash: $" + std::to_string(cash));
	render_String(5, 6, "Press 'E' to Play, 'O' for Options, 'Q' to Quit");

	//Navigates to the quit menu
	if (GetAsyncKeyState('Q'))
	{
		quitGame();
	}

	//Navigates to the options menu
	else if (GetAsyncKeyState('O'))
	{
		int userInput = 0;
		clear(0, 0, consoleSize_x, consoleSize_y);

		render_String(5, 8, "It costs cash to play a game. Get all of a symbol in a row for x2 cash,");
		render_String(5, 9, "get two or more in a row for x1.5 cash.");
		render_String(5, 10, "Customise your game options below by pressing the corresponding numbers:");
		render_String(5, 12, "1. Change default number of columns");
		render_String(5, 13, "2. Toggle colour bonus");
		render_String(5, 14, "3. Toggle unique character effects");
		render_String(5, 15, "   ");

		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));	//Clears console input to avoid problems using std::cin and GetAsyncKeyState() concurrently

		userInput = checkUserInput(userInput, 1, 3);

		//Checks what option from 1 to 3 the player has selected
		switch (userInput)
		{
		case 1:
			render_String(5, 16, "Pick column number (3-9): ");
			userInput = checkUserInput(userInput, 3, 9);

			//Adjusts the column difference and the cost of playing a game relative to the number of columns the player has selected
			columnDifference = totalColumns - userInput;
			cashCost = default_cashCost * userInput;

			clear(5, 19, consoleSize_x, consoleSize_y);
			render_String(5, 20, "Total columns set to " + std::to_string(userInput));
			render_String(5, 21, "Press 'O' again to alter other options.");	//Resets the options menu
			break;

		case 2:
			render_String(5, 16, "Turn Colour Bonus ON/OFF (2/1): ");
			userInput = checkUserInput(userInput, 1, 2);
			clear(5, 18, consoleSize_x, consoleSize_y);

			//Changes colour bonus on or off depending on if the user inputs 2 or 1
			if (userInput == 2)
			{
				colourBonusOn = true;
				render_String(5, 20, "Colour Bonus set to ON");
			}

			else if (userInput == 1)
			{
				colourBonusOn = false;
				render_String(5, 20, "Colour Bonus set to OFF");
			}
			break;

		case 3:
			render_String(5, 16, "Turn Unique Character Effects ON/OFF (2/1): ");
			userInput = checkUserInput(userInput, 1, 2);
			clear(5, 18, consoleSize_x, consoleSize_y);

			//Changes unique character effects on or off depending on if the user inputs 2 or 1
			if (userInput == 2)
			{
				uniqueCharsOn = true;
				render_String(5, 20, "Unique Character Effects set to ON");
			}

			else if (userInput == 1)
			{
				uniqueCharsOn = false;
				render_String(5, 20, "Unique Character Effects set to OFF");
			}
			break;

		default:
			//Default response in the event something unexpected happens
			clear(5, 19, consoleSize_x, consoleSize_y);
			render_String(5, 20, "Invalid input. Please try again.");
			break;
		}
	}

	else if (GetAsyncKeyState('E'))
	{
		//Declares the character and colour arrays to be used in-game and initiates gameplay
		int machineCharColours[totalColumns][totalRows];
		char machineCharacters[totalColumns][totalRows];

		generate_Char_Reel(machineCharacters, machineCharColours);
		render_Char_Reel(10, 10, machineCharacters, machineCharColours);
	}
}

void generate_Char_Reel(char machineCharacters[][totalRows], int machineCharColours[][totalRows])
{
	char charSelection[totalRows] = {156, '$', '%', '&', '*', '!', ';'};

	//Generates two 7x9 2D arrays and populates them with a fixed set of characters and randomly selected colours
	for (int columnCount = 0; columnCount < totalColumns; ++columnCount)
	{
		for (int rowCount = 0; rowCount < totalRows; ++rowCount)
		{
			machineCharacters[columnCount][rowCount] = charSelection[rowCount];
			machineCharColours[columnCount][rowCount] = rand() % 14 + 1;
		}
	}

	//Shuffles the order of the characters in each column to provide an element of randomness to gameplay
	for (int columns = 0; columns < totalColumns; ++columns)
	{
		for (int rows = 0; rows < totalRows; ++rows)
		{
			std::swap(machineCharacters[columns][rand() % (totalRows - 1)], machineCharacters[columns][rand() % (totalRows - 1)]);
		}
	}
}

void render_Char_Reel(int x, int y, char machineCharacters[][totalRows], int machineCharColours[][totalRows])
{
	clear(0, 0, consoleSize_x, consoleSize_y);

	//Checks that no free bonus spin has been applied and deducts cash accordingly
	if (!appliedBonusSpin)
	{
		cash -= cashCost;
	}

	//If the player has no or negative cash left, cash is reset to zero and they automatically lose the game
	if (cash <= 0)
	{
		cash = 0;
		clear(0, 0, consoleSize_x, consoleSize_y);
		std::cout << "\nNo cash left! YOU LOSE!\n" << std::endl;
		isPlaying = false;
	}

	if (cash > 0)
	{
		appliedBonusSpin = false;
		int xOffset = 0, yOffset = 0;
		int columnSpeed = 25;	//How fast the columns rotate, DEFAULT: 25
		int columnsCycleCounter = 0;
		int currentColumns = 0;	//The number of columns currently left to render on the screen
		isSpinning = true;

		render_String(5, 3, "Cash: $" + std::to_string(cash));
		render_String(5, 4, "Press 'S' to stop the current cycle and move to the next column.");
		render_String(5, 5, "This will happen automatically after a period of time.");

		while (isSpinning)
		{
			for (int columnCount = currentColumns; columnCount < (totalColumns - columnDifference); ++columnCount)
			{
				//(totalColumns - columnDifference) gives the current number of columns the player has selected
				//The default is 3 columns
				for (int rowCount = 0; rowCount < totalRows; ++rowCount)
				{
					if (colourBonusOn)
					{
						if (rowCount == 3)	//The centre row is given a white background to illustrate that this is the winning row
						{
							set_Colour(machineCharColours[columnCount][rowCount], 15);
						}

						else //All other rows are given black backgrounds and their corresponding foreground colours from the colour array
						{
							set_Colour(machineCharColours[columnCount][rowCount], 0);
						}
					}

					//Renders each character at an incrementing x and y position with an offset to make them evenly spaced out
					render_Char(x + xOffset * 4, y + yOffset * 2, machineCharacters[columnCount][rowCount]);

					//Once all characters have been rendered once, checks the cursor is not on the first row
					if (rowCount != 0)
					{
						//Begins swapping the positions of each character with the character behind it in a given column
						//This gives the illusion the columns are reels that are spinning backwards
						std::swap(machineCharacters[columnCount][rowCount - 1], machineCharacters[columnCount][rowCount]);

						if (colourBonusOn)
						{
							//Also swaps the colours in the same pattern to give the impression the characters and colours are moving as one object
							std::swap(machineCharColours[columnCount][rowCount - 1], machineCharColours[columnCount][rowCount]);
						}

						else
						{
							set_Colour(15, 0);
						}
					}

					++yOffset;	//Increments the y offset to begin rendering the next column
					Sleep(columnSpeed);
				}

				yOffset = 0;	//Resets y offset to rerender characters in the same positions
				++columnsCycleCounter; //Increments and keeps track of how many times 

				//Once the column cycle counter reaches a certain number or the user presses 'S', it skips to the next column
				if (columnsCycleCounter == 50 || GetAsyncKeyState('S'))
				{
					columnsCycleCounter = 0;
					++currentColumns;

					set_Colour(15, 0);
					render_String(5, 24, "Moving to Column " + std::to_string(currentColumns + 1));
				}

				//Ensures that columns that have been skipped are no longer being rerendered to the screen
				if (columnCount == (totalColumns - columnDifference) - 1)
				{
					xOffset = currentColumns;
				}

				else
				{
					++xOffset;
				}
			}

			//Once all columns have been rendered and rerendered, breaks out of the loop and stops spinning
			if (currentColumns == (totalColumns - columnDifference))
			{
				clear(5, 23, consoleSize_x, consoleSize_y);
				isSpinning = false;
				break;
			}
		}

		//Passes the character and colour arrays to check_Win to check if the player has won anything
		check_Win(machineCharacters, machineCharColours);
	}
}

void check_Win(char winningChars[][totalRows], int winningCharColours[][totalRows])
{
	int colourBonus = 0;	//Logs the occurances of matching colours
	int dollarSign = 0;		//Logs the occurances of dollar signs in a row
	int exclamationMark = 0;	//Logs the occurances of exclamation marks in a row
	int amperSand = 0;		//Logs the occurances of ampersands in a row
	int charWin = 0;	//To check how many characters in a row are the same overall
	int column2 = 0;	//The current second column to be compared to the first column
	bool noUniqueChars = false;

	for (int column1 = 0; column1 < (totalColumns - columnDifference); ++column1)
	{
		++column2;

		if (column2 != (totalColumns - columnDifference))
		{
			//Checking for characters on the centre row
			if (winningChars[column1][2] == winningChars[column2][2])
			{
				//Checks how many characters in a row match and increments charWin for each pair
				++charWin;

				if (uniqueCharsOn)
				{
					//Checks the occurances of each unique character, so long as the player has selected unique character effects
					if (winningChars[column1][2] == '$' && winningChars[column2][2] == '$')
					{
						++dollarSign;
					}

					else if (winningChars[column1][2] == '!' && winningChars[column2][2] == '!')
					{
						++exclamationMark;
					}

					else if (winningChars[column1][2] == '&' && winningChars[column2][2] == '&')
					{
						++amperSand;
					}
				}
			}

			if (colourBonusOn)
			{
				//Checks how many colours in a row match, and increments colourBonus
				if (winningCharColours[column1][2] == winningCharColours[column2][2])
				{
					++colourBonus;
				}
			}
		}
	}

	//If none of the unique characters are in a row, set noUniqueChars to true
	if((dollarSign + 1 != (totalColumns - columnDifference)) && (exclamationMark + 1 != (totalColumns - columnDifference)) && (amperSand + 1 != (totalColumns - columnDifference)))
	{
		noUniqueChars = true;
	}

	set_Colour(15, 0);

	if (charWin + 1 == (totalColumns - columnDifference))
	{
		//If the player has got all of one type of character in a row
		if (uniqueCharsOn && !noUniqueChars)
		{
			//If unique character effects is on, and all of one type is in a row, displays the corresponding message for each type
			if (dollarSign + 1 == (totalColumns - columnDifference))
			{
				render_String(5, 23, "All DOLLAR SIGNS in a row! Triple cash bonus of " + std::to_string(cashCost * 3) + " cash!");
				cash += cashCost * 3;
			}

			if (exclamationMark + 1 == (totalColumns - columnDifference))
			{
				render_String(5, 23, "All EXCLAMATION MARKS in a row! You win NOTHING!");
			}

			if (amperSand + 1 == (totalColumns - columnDifference))
			{
				render_String(5, 23, "All AMPERSANDS in a row! You win " + std::to_string(cashCost * 2) + " cash and a FREE SPIN!");
				cash += cashCost * 2;
				appliedBonusSpin = true;	//Player is not charged for next spin when they go to play again
			}
		}

		//If all the characters in the winning row are a match, *AND* they are not dollar signs, ampersands, or exclamation marks, *OR* unique character effects is off, displays generic win message and awards a double cash reward
		else if(!uniqueCharsOn || (uniqueCharsOn && noUniqueChars))
		{
			render_String(5, 23, "All characters in a row! You have won " + std::to_string(cashCost * 2) + " cash!");
			cash += cashCost * 2;
		}
	}

	else if (charWin >= 1)
	{
		//If the player has got two or more, but not all of one character type in a row, even if they are unique characters, award 1.5 times the amount of cash originally bet
		render_String(5, 23, "Two or more characters in a row! You have won " + std::to_string(cashCost * 1.5) + " cash!");
		cash += cashCost * 1.5;
	}

	else
	{
		//If no characters match, player wins nothing and displays a losing message
		render_String(5, 23, "No matching characters. You lose.");
	}

	if (colourBonus + 1 == (totalColumns - columnDifference))
	{
		//If all of one colour in a row, award a cash bonus equal to half of the original bet cash
		render_String(5, 24, "All colours in a row! You have won an extra " + std::to_string(cashCost / 2) + " cash!");
		cash += cashCost / 2;
	}

	else if (colourBonus >= 1)
	{
		//If two or more, but not all of one colour are in a row, award a cash bonus equal to one quarter of the original bet cash
		render_String(5, 24, "Two or more colours in a row! You have won an extra " + std::to_string(cashCost / 4) + " cash!");
		cash += cashCost / 4;
	}

	else
	{
		//If no matching colours or colour bonus is turned off, display appropriate message
		if (colourBonusOn)
		{
			render_String(5, 24, "No matching colours in a row. No bonus.");
		}
		
		else
		{
			render_String(5, 24, "Colour bonus off. No bonus applied.");
		}
	}
}

//Clears the console from a given starting x and y position up until an end x and y position by filling the selected region with blank spaces
void clear(int startX, int startY, int finalX, int finalY)
{
	for (int counter_y = startY; counter_y < finalY; ++counter_y)
	{
		for (int counter_x = startX; counter_x < finalX; ++counter_x)
		{
			render_Char(counter_x, counter_y, ' ');
		}
	}
}

int main()
{
	//Setting up the game for first time run
	init_Render();
	clear(0, 0, consoleSize_x, consoleSize_y);

	render_String(5, 5, "Welcome to Fruit Machine Simulator!");

	//Setting default values
	columnDifference = 6;
	cashCost = default_cashCost * (totalColumns - columnDifference);
	appliedBonusSpin = false;
	uniqueCharsOn = true;
	colourBonusOn = true;
	isPlaying = true;

	//Play game!
	while (isPlaying)
	{
		main_Screen();
		Sleep(100);
	}

	return 0;
}