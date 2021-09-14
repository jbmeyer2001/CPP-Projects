/*
Author:			Joseph Meyer
date updated:	September 12th, 2021
*/

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void printPuzzle(int puzzle[9][9]);
bool isSolved(int puzzle[9][9]);
bool duplicates(int array[9]);
bool isValid(int puzzle[9][9]);
int nextRow(int row_num, int column_num);
int nextColumn(int row_num, int column_num);
bool solveSudoku(int puzzle[9][9], int row_num, int column_num);

/*
Objects of sudokuNode class function as storage for information on a specific sudoku
puzzle, and as nodes in a linked list.

*** IMPORTANT ***
 - The information that fills the properties of sudokuNode comes from text files, the file
name of which is a parameter to the assignPuzzle method.
 - The reason assignPuzzle isn't part of a constructor is so that objects of sudokuNode can be
allocated before the program knows what file name to assign.
 - Instead, the properties are declared to a default value.
 - The boolean property 'puzzleAssignedFlag' is used to restrain the program from
 doing actions on properties of sudokuNode if they are not assigned.

Encapuslation is used such that the program/user only has the access/ability to:
  - (property) the pointer to the next sudokuNode, for linked list implementation.
  - (method)   assign the puzzle once (once a sudokuNode is assigned values, those values cannot be reassigned).
  - (method)   display the unsolved puzzle stored in sudokuNode.
  - (method)   display the solved puzzle stored in sudokuNode.
  - (method)   copy the solved sudoku into an array. The values at the actual memory location that stores the solved puzzle cannot be modified.
  - (method)   return the puzzle name (or file name) of the puzzle.
*/
class sudokuNode
{
private:
	//-------------------------------------------//
	//Private properties defined to default values.
	//-------------------------------------------//
	string puzzlename = "";
	bool puzzleAssignedFlag = false;
	int puzzle_unsolved[9][9] = { -1 };
	int puzzle_solved[9][9] = { -1 };

public:
	sudokuNode* next = NULL;

	/*
	Fills the properties of sudokuNode object with values that are determined from
	a text file. The text file is found by its name through the parameter 'puzzleName'
	and is formatted as nine lines of nine values (from 0-9) seperated by spaces,
	where '0' represents an empty space.

	PARAM: String puzzleName: represents the file name of a text file that contains
	a sudoku puzzle.
	*/
	void assignPuzzle(string puzzleName)
	{
		int number = -1;
		bool errFlag = false;

		//-----------------------------------------------------------------------//
		//Only allows object to be assigned if it has not been assigned previously.
		//-----------------------------------------------------------------------//
		if (!puzzleAssignedFlag)
		{
			puzzlename = puzzleName;

			//---------------------------------------------------------------//
			//If file opens properly, assigns the values from the text file to
			//both puzzle_unsolved and puzzle_solved.
			//---------------------------------------------------------------//
			string filename = puzzleName + ".txt";
			ifstream sudokuFile;
			sudokuFile.open(filename);

			if (sudokuFile.is_open())
			{
				for (int i = 0; i < 9; i++)
				{
					for (int j = 0; j < 9; j++)
					{
						sudokuFile >> number;
						puzzle_unsolved[i][j] = number;
						puzzle_solved[i][j] = number;
					}
				}
				sudokuFile.close();
			}
			else
			{
				//-------------------------------------------------------------------//
				//If file does not open properly, print an error and put up error flag.
				//-------------------------------------------------------------------//
				cout << "**Error opening file**" << endl;
				errFlag = true;
			}

			//---------------------------------------------------------------------------------------//
			//Only if there were no errors reading from the file, is puzzle_solved solved by algorithm
			//and assigned flag is put up.
			//---------------------------------------------------------------------------------------//
			if (!errFlag)
			{
				solveSudoku(puzzle_solved, 0, 0);
				puzzleAssignedFlag = true;
			}
		}
	}

	/*
	If the puzzle has been assigned, calls printPuzzle on the puzzle_unsolved array.
	*/
	void displayUnsolvedPuzzle(void)
	{
		cout << "\n";
		if (puzzleAssignedFlag)
		{
			printPuzzle(puzzle_unsolved);
		}
		else
			cout << "**Error: Program attempted to access puzzle that has not been assigned**" << endl;
	}

	/*
	If the puzzle has been assigned, calls printPuzzle on the puzzle_solved array.
	*/
	void displaySolvedPuzzle(void)
	{
		cout << "\n";
		if (puzzleAssignedFlag)
		{
			printPuzzle(puzzle_solved);
		}
		else
			cout << "**Error: Program attempted to access puzzle that has not been assigned**" << endl;
	}

	/*
	If the puzzle has been assigned, copies the values from puzzle_solved into parameter puzzle.

	PARAM: 9x9 integer array puzzle[9][9]: representation of solved puzzle copied to it.
	*/
	void getSolvedPuzzle(int puzzle[9][9])
	{
		if (puzzleAssignedFlag)
		{
			for (int i = 0; i < 9; i++)
			{
				for (int j = 0; j < 9; j++)
				{
					puzzle[i][j] = puzzle_solved[i][j];
				}
			}
		}
		else
			cout << "**Error: Program attempted to access puzzle that has not been assigned**" << endl;
	}

	/*
	If the puzzle has been assigned, returns the puzzlename.

	RETURN: string representing name of puzzle.
	*/
	string getPuzzleName(void)
	{
		if (puzzleAssignedFlag)
		{
			return puzzlename;
		}
		else
		{
			cout << "**Error: Program attempted to access puzzle that has not been assigned**" << endl;
			return "";
		}

	}
};

/*
The sudokuList class does all operations that allocate, traverse, access data from, and deallocate
from a linked list of sudokuNodes.

The goal with the sudokuList class was to cover up as much as possible, so that any actions done
relating to the actual puzzles in the program are done through calls to methods of sudokuList.

*** IMPORTANT ***
There is a text file FilenameList.txt that is hardcoded in this class which has the file names
of all the sudoku text files that need to be added to the linked list.

Encapuslation is used such that the program/user only has the access/ability to:
 - (method) add a sudoku puzzle to the linked list (creates its own text file, and adds its name to FilenameList.txt).
 - (method) display a the file names of all the sudoku puzzles in the list.
 - (method) display a solved sudoku selected by the user (calls displaySolvedPuzzle() from sudokuNode object).
 - (method) display an unsolved sudoku selected by the user (calls displayUnsolvedPuzzle() from sudokuNode object).
 - (method) checks a users solution against a sudoku currently in the list.
*/
class sudokuList
{
private:
	sudokuNode* head;

	/*
	Finds whether a puzzle with a given name is in the linked list.

	PARAM: string puzzleName: the name of the puzzle being searched for.
	RETURN: sudokuNode pointer, the address of the puzzle being searched for if found, or NULL if no puzzle was found with the provided name.
	*/
	sudokuNode* findPuzzle(string puzzleName)
	{
		sudokuNode* traversal = head;

		//--------------------------------------------------------------------------------------------------------------//
		//Checks each node in the linked list to see if the puzzleName parameter matches the puzzleName of the node
		//and returns a pointer to that node if it does. Returns the NULL pointer if a node with puzzleName was not found.
		//--------------------------------------------------------------------------------------------------------------//
		if (puzzleName == traversal->getPuzzleName())
		{
			return traversal;
		}
		else
		{
			do
			{
				traversal = traversal->next;
				if (puzzleName == traversal->getPuzzleName())
				{
					return traversal;
				}
			} while (traversal->next != NULL);
		}
		return NULL;
	}

public:
	/*
	SudokuList constructor:
	Creates the head node for the list, and fills in the linked list with all the
	puzzles specified in the FilenameList.txt file.
	*/
	sudokuList(void)
	{
		string puzzleName;
		ifstream sudokuListFile;

		//------------------------------------------------------------------------//
		//Defines the head node pointer, allocates the head node,
		//and assigns it the puzzle specified by the first line in FilenameList.txt.
		//------------------------------------------------------------------------//
		sudokuNode* newNode;
		sudokuNode* traversalNode;
		head = new sudokuNode;

		sudokuListFile.open("FilenameList.txt");
		getline(sudokuListFile, puzzleName);

		head->assignPuzzle(puzzleName);
		traversalNode = head;

		//------------------------------------------------------//
		//Iterates through all lines in the FilenameList.txt file, 
		//allocating and assigning nodes of sudokuNode.
		//------------------------------------------------------//
		while (getline(sudokuListFile, puzzleName))
		{
			newNode = new sudokuNode;
			newNode->assignPuzzle(puzzleName);
			traversalNode->next = newNode;
			traversalNode = newNode;
		}
	}

	/*
	Adds a sudoku inputted by the user to the linked list, creates a text file representation
	of the puzzle, and adds the file name of that puzzle to the FilenameList.txt.
	*/
	void addSudoku(void)
	{
		string line;
		string filename;
		ofstream sudokuListFile;
		ofstream sudokuFile;
		sudokuNode* traversal;
		int length;
		int puzzle[9][9];
		int puzzleCheckSolved[9][9];
		const char* digit;

		int j; //Being used outside of the for loop iterating through it, so must be defined before (line 365).

		//------------------------------------------------------------------------------------------------//
		//Recieves input from the user for both the name of the sudoku, and the values in the sudoku.
		//Inludes error handling for if the sudoku file is a duplicate name, and if invalid lines are given.
		//------------------------------------------------------------------------------------------------//
		cout << "          Enter sudoku file name:" << endl;
	get_sudoku_name:
		getline(cin, filename, '\n');
		if (findPuzzle(filename) != NULL)
		{
			cout << "          System already has a sudoku with that name, please put in a different file name:" << endl;
			goto get_sudoku_name;
		}

		cout << "          NOTE: all lines of sudoku puzzle must be entered as 9 integer values seperated by spaces, '0' represents an empty square." << endl;
		cout << "          Enter sudoku puzzle:" << endl;

		for (int i = 0; i < 9; i++)
		{
		read_line_addSudoku:
			getline(cin, line, '\n');
			for (int j = 0; j < 9; j++)
			{
				length = line.length();
				if (length != 17)
				{
					cout << "          Entry invalid length, try again:" << endl;
					goto read_line_addSudoku;
				}

				digit = &line[j * 2];
				try
				{
					puzzle[i][j] = stoi(digit);
					puzzleCheckSolved[i][j] = stoi(digit);
				}
				catch (invalid_argument)
				{
					cout << "          Invalid entry, try again:" << endl;
					goto read_line_addSudoku;
				}
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------//
		//Checks to see whether the puzzle is acutally solveable, and only adds the puzzle to the system if it is.
		//
		//Additionally, note that solveSudoku calls 'puzzleCheckSolved' instead of 'puzzle' because if puzzle is called and 
		//solved correctly, a solved version would be copied into the text file and linked list when it should be an unsolved version.
		//--------------------------------------------------------------------------------------------------------------------------//
		if (!solveSudoku(puzzleCheckSolved, 0, 0))
		{
			cout << "**ERROR: Sudoku entered does not have a solution, it has not been added to the system**" << endl;
		}
		else
		{
			//--------------------------------------------------------------------------------------------------------//
			//Creates a sudoku file with the file name given by the user that contains the unsolved puzzle they entered.
			//--------------------------------------------------------------------------------------------------------//
			sudokuFile.open(filename + ".txt");
			if (sudokuFile.is_open())
			{
				for (int i = 0; i < 9; i++)
				{
					for (j = 0; j < 8; j++)
					{
						sudokuFile << puzzle[i][j] << " ";
					}
					sudokuFile << puzzle[i][j] << endl;
				}
				sudokuFile.close();
			}

			//---------------------------------------------------//
			//Appends the file name into the FilenameList.txt file.
			//---------------------------------------------------//
			sudokuListFile.open("FilenameList.txt", ios::app);
			if (sudokuListFile.is_open())
			{
				sudokuListFile << "\n" << filename;
				sudokuListFile.close();
			}

			//-----------------------------------------------------------------------------//
			//Allocates and assigns a new sudokuNode with the information given by the user,
			//and traverses through the linked list to add it to the end.
			//-----------------------------------------------------------------------------//
			traversal = head;
			while (traversal->next != NULL)
			{
				traversal = traversal->next;
			}
			traversal->next = new sudokuNode;
			traversal = traversal->next;
			traversal->assignPuzzle(filename);

			cout << "           " << filename << " has been successfully added!" << endl;
		}
	}

	/*
	Displays the names of all puzzles in the linked list.
	*/
	void displayPuzzleNames(void)
	{
		sudokuNode* traversal = head;
		string name;

		while (traversal != NULL)
		{
			name = traversal->getPuzzleName();
			cout << name << endl;
			traversal = traversal->next;
		}
	}

	/*
	Displays the unsolved representation of a puzzle in the linked list specified by the parameter puzzleName.
	*/
	void displayUnsolved(string puzzleName)
	{
		sudokuNode* sudoku;
		sudoku = findPuzzle(puzzleName);
		if (sudoku != NULL)
		{
			sudoku->displayUnsolvedPuzzle();
		}
		else
		{
			cout << endl << "**Sudoku with that name was not found**" << endl;
		}
	}


	/*
	Displays the solved representation of a puzzle in the linked list specified by the parameter puzzleName.
	*/
	void displaySolved(string puzzleName)
	{
		sudokuNode* sudoku;
		sudoku = findPuzzle(puzzleName);
		if (sudoku != NULL)
		{
			sudoku->displaySolvedPuzzle();
		}
		else
		{
			cout << endl << "**Sudoku with that name was not found**" << endl;
		}
	}

	/*
	Checks a user entered sudoku solution against the actual solution of a sudoku specified
	by the puzzleName parameter, and informs the user whether they solved the puzzle correctly.
	*/
	void checkSudoku(string puzzleName)
	{
		sudokuNode* sudoku;
		int puzzle[9][9];
		int length;
		const char* digit;
		string line;
		bool wrongFlag = false;


		sudoku = findPuzzle(puzzleName);
		sudoku->getSolvedPuzzle(puzzle);

		cout << "          NOTE: all lines of sudoku puzzle must be entered as 9 integer values seperated by spaces, '0' represents an empty square." << endl;
		cout << "          Enter sudoku puzzle:" << endl;

		//--------------------------------------------------------------------------------------------//
		//Reads the inputs from the user and converts them to integers to be checked against the array,
		//has error handling for if the input is invalid.
		//--------------------------------------------------------------------------------------------//
		for (int i = 0; i < 9; i++)
		{
		read_line_checkSudoku:
			getline(cin, line);
			for (int j = 0; j < 9; j++)
			{
				length = line.length();
				if (length != 17)
				{
					cout << "          Entry invalid length, try again:" << endl;
					goto read_line_checkSudoku;
				}
				digit = &line[j * 2];

				try
				{
					if (puzzle[i][j] != stoi(digit))
					{
						wrongFlag = true;
					}
				}
				catch (invalid_argument)
				{
					cout << "          Invalid entry, try again:" << endl;
					goto read_line_checkSudoku;
				}
			}
		}

		//---------------------------------------------------------------------------//
		//Does not inform the user whether the puzzle they entered was incorrect until
		//every line has been entered. If the wrongFlag stays down, it means every 
		//entry was correct, and the user is informed.
		//---------------------------------------------------------------------------//
		if (!wrongFlag)
		{
			cout << "          Congragulations, that was the correct solution!" << endl;
		}
		else
			cout << "          Unfortunately, that was the incorrect solution." << endl;
	}

	/*
	SudokuList deconstructor:
	Deallocates all the memory in the linked list.
	*/
	~sudokuList(void)
	{
		sudokuNode* traversal;
		sudokuNode* next;

		traversal = head;
		next = traversal->next;

		while (traversal->next != NULL)
		{
			delete traversal;
			traversal = next;
			next = traversal->next;
		}
		delete traversal;
	}
};

int main(void)
{
	string select;
	char select_char;
	string name;
	sudokuList list;

	//------------------------------------------------------------------------------------------------------------//
	//Start:
	//user inputs what action they would like to do, program uses switch statment and goto to branch to that location.
	//------------------------------------------------------------------------------------------------------------//
start:
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	cout << "          Add an unsolved sudoku:                                    (enter 1)" << endl;
	cout << "          Solve a sudoku currently in the system:                    (enter 2)" << endl;
	cout << "          View a solved sudoku:                                      (enter 3)" << endl;
	cout << "          Quit the application:                                      (enter 4)" << endl;
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

	//-----------------------------------------------------------------------------------------------//
	//Program needs '\n' as a delimiter or else an invalid entry could send the program into an 
	//infinite loop because it won't pause for the user to input a new value before reading using cin.
	//
	//I know there are other ways to solve this issue.
	//But, the following logic allows me the ease of use of getline, with a switch statement.
	//-----------------------------------------------------------------------------------------------//
start_input:
	getline(cin, select, '\n');
	if (select.length() != 1)
	{
		cout << "          Invalid entry, try again:" << endl;
		goto start_input;
	}
	select_char = select[0]; //Because a switch statement can't use strings.

	switch (select_char)
	{
	case '1':
		goto add_sudoku;
	case '2':
		goto solve_sudoku;
	case '3':
		goto view_solved_sudoku;
	case '4':
		goto quit;
	default:
		cout << "          invalid entry, try again:" << endl;
		goto start_input;
	}




	//-------------------------------------------------------------------//
	//Add sudoku:
	//Runs the addSudoku method in the list object, then returns to start.
	//-------------------------------------------------------------------//
add_sudoku:
	list.addSudoku();
	goto start;




	//--------------------------------------------------------------------------------------------------------------//
	//Solve sudoku:
	//User selects which sudoku they would like to solve from a list displayed by the displayPuzzleNames method,
	//then the displayUnsolved method is called to show what the unsolved version is before branching to check_sudoku.
	//--------------------------------------------------------------------------------------------------------------//
solve_sudoku:
	cout << "          Type the name of the sudoku you would like to solve from the following list:" << endl;
	list.displayPuzzleNames();
	cout << endl;
	getline(cin, name, '\n');
	list.displayUnsolved(name);
	cout << "\n\n";
	goto check_sudoku;




	//-------------------------------------------------------------------------------------------//
	//View solved solution:
	//Displays all sudokus in the system, the user enters which one they would like to see solved, 
	//then program branches to display_solved_sudoku.
	//-------------------------------------------------------------------------------------------//
view_solved_sudoku:
	cout << "          Type the value of the sudoku you would like view solved from the following list:" << endl;
	list.displayPuzzleNames();
	cout << endl;
	getline(cin, name, '\n');
	goto display_solved_sudoku;




	//-----------------------------------------------------------------------------------//
	//Check sudoku:
	//Runs checkSudoku method and asks user what to do next, and branches to that location.
	//-----------------------------------------------------------------------------------//
check_sudoku:
	list.checkSudoku(name);
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	cout << "          Return to the start:                                   (enter 1)" << endl;
	cout << "          Quit the application:                                  (enter 2)" << endl;
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

	//---------------------------------------------------------------------------------------------------//
		//Program needs '\n' as a delimiter or else an invalid entry could send the program into an 
		//infinite loop because it won't pause for the user to input a new value before reading using cin.
		//
		//I know there are other ways to solve this issue.
		//But, the following logic allows me the ease of use of getline, with a switch statement.
		//-----------------------------------------------------------------------------------------------//
check_sudoku_input:
	getline(cin, select, '\n');
	if (select.length() != 1)
	{
		cout << "          Invalid entry, try again:" << endl;
		goto check_sudoku_input;
	}
	select_char = select[0]; //Because a switch statement can't use strings.

	switch (select_char)
	{
	case '1':
		goto start;
	case '2':
		goto quit;
	default:
		cout << "          Invalid entry, try again:" << endl;
		goto check_sudoku_input;
	}




	//---------------------------------------------------------------------------------//
	//Display solved sudoku:
	//runs displaySolved method and asks user what to do next, branches to that location.
	//---------------------------------------------------------------------------------//
display_solved_sudoku:
	list.displaySolved(name);
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	cout << "          Return to the start?                                   (enter 1)" << endl;
	cout << "          Quit the application?                                  (enter 2)" << endl;
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

	//------------------------------------------------------------------------------------------------------------------//
		//Program needs '\n' as a delimiter or else an invalid entry could send the program into an 
		//infinite loop because it won't pause for the user to input a new value before reading using cin.
		//
		//I know there are ways to change the delimiter of cin.
		//But, the following logic allows me to get the ease of use of getline (because the delimiter is set in the function)
		//with a switch statement
		//------------------------------------------------------------------------------------------------------------------//
display_solved_sudoku_input:
	getline(cin, select, '\n');
	if (select.length() > 1)
	{
		cout << "          Invalid entry, try again:" << endl;
		goto display_solved_sudoku_input;
	}
	select_char = select[0];

	switch (select_char)
	{
	case '1':
		goto start;
	case '2':
		goto quit;
	default:
		cout << "          Invalid entry, try again:" << endl;
		goto start;
	}




quit:
	cout << "          Thanks for playing!" << endl;
	return 0;
}

/*
Checks a 9x9 array that represents a sudoku puzzle to see whether it is solved or not.

PARAM: 9x9 integer array puzzle[9][9]: array representing a sudoku puzzle.
RETURN: boolean variable: true if the sudoku is solved and zero if it is not.
*/
bool isSolved(int puzzle[9][9])
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (puzzle[i][j] == 0)
				return false;
		}
	}
	return true;
}

/*
Checks an array with 9 values to see whether there are duplicates.

PARAM: integer array array[9]: array representing a line, column, or box in a sudoku puzzle.
RETURN: boolean value: true if there are duplicates, false if there are no duplicates.
*/
bool duplicates(int array[9])
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 8; j > i; j--)
		{
			//----------------------------------------------------------------------//
			//Index i starts at the beginning, and for each iteration of index i,
			//index j starts at the end and moves towards the beginning until it 
			//reaches i. This ensures that every index is checked against every other,
			//but never against itself.
			//
			//Indices filled with 0 (empty spaces) are ignored.
			//----------------------------------------------------------------------//
			if ((array[i] != 0) && (array[j] != 0) && (array[i] == array[j]))
				return true;
		}
	}
	return false;
}

/*
Checks a 9x9 array that represents a sudoku puzzle to ensure it is valid.
It first checks to see if there are any invalid numbers, any number less than zero or more than nine.
Then, it checks each row, column, and box to see if there are any duplicates.

PARAM: 9x9 integer array puzzle[9][9]: array representing a sudoku puzzle.
RETURN: boolean variable: true if the sudoku is valid, and zero if it is not.
*/
bool isValid(int puzzle[9][9])
{
	int checkArray[9];
	//--------------------------------------------------------//
	//Check to see if there are any negatives or numbers over 9.
	//--------------------------------------------------------//
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if ((puzzle[i][j] > 9) || (puzzle[i][j] < 0))
			{
				return false;
			}
		}
	}
	//--------------------------------------------//
	//Check all rows to see if they have duplicates.
	//--------------------------------------------//
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			checkArray[j] = puzzle[i][j];
		}
		if (duplicates(checkArray))
		{
			return false;
		}
	}
	//-----------------------------------------------//
	//Check all columns to see if they have duplicates.
	//-----------------------------------------------//
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			checkArray[j] = puzzle[j][i];
		}
		if (duplicates(checkArray))
		{
			return false;
		}
	}


	//---------------------------------------------//
	//Check all boxes to see if they have duplicates.
	//---------------------------------------------//
	for (int i = 0; i <= 6; i = i + 3)
	{
		for (int j = 0; j <= 6; j = j + 3)
		{
			//---------------------------------------------------------------------------------//
			//Variables i and j determine which box is being checked for duplicates by selecting
			//the first index of each box within the sudoku array. (0,0) for box one, (0,3)
			//for box two, (0,6) for box 3, (3,0) for box four, etc.
			//---------------------------------------------------------------------------------//
			for (int k = 0; k < 3; k++)
			{
				for (int l = 0; l < 3; l++) {
					//-------------------------------------------------------------------------------------------------------------------------//
					//Variables k and l determine the indices within each box, and therefore, the values copied into checkArray[9] to be checked.
					//
					//For example, the indices of box 5 are: (3,3), (3,4), (3,5), (4,3), (4,4), (4,5), (5,3), (5,4), (5,5).
					//-------------------------------------------------------------------------------------------------------------------------//
					checkArray[(k * 3) + l] = puzzle[k + i][l + j];
				}
			}
			if (duplicates(checkArray))
			{
				return false;
			}
		}
	}

	return true;
}

/*
Prints the sudoku puzzle with spaces between every value, and an endline after every row.

PARAM: 9x9 integer array puzzle[9][9]: array representing a sudoku puzzle.
*/
void printPuzzle(int puzzle[9][9])
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << puzzle[i][j] << " ";
		}
		cout << endl;
	}
}

/*
Used by the recursive sudoku algorithm to decide what the row index is for the next recursive call.

PARAM: integer row_num: index of the current row.
PARAM: integer column_num, index of the current column.
RETURN: integer, the row index for the next recursive call.
*/
int nextRow(int row_num, int column_num)
{
	//------------------------------------------------------------------------------//
	//Only if the current function call is on the last column should the row increase.
	//------------------------------------------------------------------------------//
	if (column_num == 8)
	{
		row_num++;
		return row_num;
	}
	return row_num;
}

/*
Used by the recursive sudoku algorithm to decide what the column index is for the next recursive call.

PARAM: integer row_num: index of the current row.
PARAM: integer column_num: index of the current column.
RETURN: integer, the column index for the next recursive call.
*/
int nextColumn(int row_num, int column_num)
{
	//----------------------------------------------------------------------------------------------//
	//If the current function call is on the last column, reset to the first column (of the next row).
	//Increment the column in every other case to continue traversing a row.
	//----------------------------------------------------------------------------------------------//
	if (column_num == 8)
	{
		return  0;
	}
	else
	{
		column_num++;
		return column_num;
	}
}

/*
Utilizes a recursive algorithm to be able to solve any sudoku puzzle.

Because of how arrays are passed to functions in C++ as pointers, editing
the array in any recursive call of solveSudoku will edit the same memory location
as the origional call.

PARAM: 9x9 integer array puzzle[9][9]: array representing a sudoku puzzle.
PARAM: integer row_num, index representing the row of the array index to be filled (0 for first call).
PARAM: integer column_num, index representing the column of the array index to be filled (0 for first call).
RETURN: boolean variable, tells user whether the sudoku puzzle was solved or not.
*/
bool solveSudoku(int puzzle[9][9], int row_num, int column_num)
{
	//--------------------------------------------------------------------------------------//
	// Initialize variables and determine the indices to be called in the next recursive call.
	//--------------------------------------------------------------------------------------//
	int next_row, next_column;
	next_row = nextRow(row_num, column_num);
	next_column = nextColumn(row_num, column_num);

	//-----------------------------------------------------------------------//
	//Base case - if the sudoku is solved, return true back through every call.
	//-----------------------------------------------------------------------//
	if (isSolved(puzzle))
		return true;

	//--------------------------------------------------------------------//
	//If the current call of the function lands on an already filled square, 
	//'skip' it by calling the next index without editing the sudoku array.
	//--------------------------------------------------------------------//
	if (puzzle[row_num][column_num] != 0)
		return solveSudoku(puzzle, next_row, next_column);

	//--------------------------------------------------------------------------------------------------------//
	//If the sudoku is not solved, and the current call isn't on a filled space it is time to 'guess and check'.
	//
	//The guess: looping through the numbers 1-9.
	//
	//First check: checking whether the guess creates a sudoku that is still valid.
	//If it does not create a valid sudoku, 'guess' a different value.
	//
	//Second check: if the guess finds a valid sudoku puzzle, the second check creates another recursive
	//call of the function using next_row and next_column to move on to the next space.
	// - If true is returned, that means that the sudoku puzzle[9][9] is solved so true is returned.
	// - If false is returned, continue by 'guessing' a different value.
	//-------------------------------------------------------------------------------------------------------//
	for (int i = 1; i <= 9; i++)
	{
		puzzle[row_num][column_num] = i;
		if (isValid(puzzle))
		{
			if (solveSudoku(puzzle, next_row, next_column))
				return true;
		}
	}
	//-------------------------------------------------------------------------------------------------------------------//
	//If every value 1-9 has been tried and they have all resulted in invalid puzzles
	//it means that a previous recursive call of solveSudoku is incorrect (or the sudoku is unsolveable).
	//
	//In either of those cases, false needs to be returned to indicate the current puzzle is invalid, and to keep guessing.
	//
	//It is also important to remember to 'clean up' an invalid puzzle by setting all empty spaces to zero
	//so that they don't interfere with previous recursive calls by acting as filled spaces.
	//
	//NOTE: this also functions as a base case if the sudoku puzzle is unsolveable,
	//false is returned back through all recursive calls.
	//-------------------------------------------------------------------------------------------------------------------//
	puzzle[row_num][column_num] = 0;
	return false;
}
