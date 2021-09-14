#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

bool alphabetical(string a, string b);
void initializeLibraries(void);
void cmdWindowControl(void);
void writebackAndReinitialize(void);

static mutex s_mu;
static bool s_endProgram = false;
static bool s_spreadsheetWritebackFlag = false;
static bool s_CentralBranchBooksShelvedFlag = false;
static bool s_PGBranchBooksShelvedFlag = false;
static bool s_MPBranchBooksShelvedFlag = false;

/*
	Objects of bookNode function as storage for the title and author of a book that are parts of two
	different binary search tree. The left leaf always comes alphabetically before and the right leaf 
	always comes alphabetically after. There is a binary search tree for both the title and the author, 
	although they use the same spot in memory just point in a different configuration, hence the 4 bookNode pointers defined.

	Encapsulation is used such that the program/user only has the access/ability to:
	 - (method) 	Assign the title and author.
	 - (method) 	Get the title.
	 - (method) 	Get the author's name.
	 - (method)	Set the delete flag (means it won’t be written back into the spreadsheet, or printed as part of the book tree that it is contained in).
	 - (method)	Get the delete flag.
*/
class bookNode
{
private:
	string title;
	string authorName;
	bool deleteFlag = false;
public:
	
	bookNode* rightLeafTitle = NULL;
	bookNode* leftLeafTitle = NULL;
	bookNode* rightLeafAuthor = NULL;
	bookNode* leftLeafAuthor = NULL;

	void assign(string Title, string Author)
	{
		title = Title;
		authorName = Author;
	}
	string getTitle(void)
	{
		return title;
 	}
	string getAuthorName(void)
	{
		return authorName;
	}

	//----------------------------------------------------------------------------------//
	//The delete flag is here because when a book is removed from the binary search tree,
	//it isn't actually removed (would be really confusing with the four pointers).
	//Instead, the delete flag is put up and it functions as if it is deleted until
	//the structure is written back into the spreadsheet .csv file, and the deleted
	//node is not written back in.
	//----------------------------------------------------------------------------------//
	void setDeleteFlag(void)
	{
		deleteFlag = true;
	}
	bool getDeleteFlag(void)
	{
		return deleteFlag;
	}
};

/*
	the bookTree class does all operations that allocate, traverse, access data from, and deallocate
	from a binary search tree of sudokuNodes.

	the bookTree node also does all of the writing back to the spreadsheet .csv files.

	Encapsulation is used such that the program/user only has the access/ability to:
	 - (method) 	Print the list.
	 - (method) 	Add a book node to the binary search tree.
	 - (method) 	Delete a book from the binary search tree.
	 - (method)		Writeback the data in the binary search tree into a .csv file specified by a filename (filename is stored in objects that inherit this class)

	 Encapsulation is used such that the classes that inherit this class have the access/ability to to:
	  - (property)	view the root node to the book tree.
	  - (method)	print the list in alphabetical order (by title or author)
*/
class bookTree
{
private:

	/*
	Prints the binary search tree in post order to the .csv file, this is done during writeback.
	It is done in post order, because if the .csv file becomes in alphabetical order then when it is
	initialized to the binary search tree, the binary search tree will effectively be a linked list.

	PARAM:	bookNode* node, used to traverse the tree recursively.
	PARAM:	ofstream file, file stream of the .csv file declared in a different method.
	PARAM:	integer num, a random number (0 or 1) declared in a different method which tells 
			the method whether to writeback with the title tree or the author tree. The goal
			of this is to keep the .csv file as randomized as possible, because the 
			binary search tree will be best optimized that way.
	*/
	void printPostOrderToFile(bookNode* node, ofstream &file, int num)
	{
		string line;
		string title;
		string author;

		//---------------------//
		//Base case of traversal.
		//---------------------//
		if (node == NULL)
			return;

		//---------------------------------------------------------------------------------------------//
		//Num parameter described at lines 107-110. Method is called recursively to print all the books.
		//---------------------------------------------------------------------------------------------//
		if (num == 1)
		{
			printPostOrderToFile(node->leftLeafTitle, file, num);
			printPostOrderToFile(node->rightLeafTitle, file, num);
		}
		else
		{
			printPostOrderToFile(node->leftLeafAuthor, file, num);
			printPostOrderToFile(node->rightLeafAuthor, file, num);
		}
		//------------------------------------------//
		//Only write back if the delete flag is false.
		//------------------------------------------//
		if (!node->getDeleteFlag())
		{
			line = node->getTitle() + ',' + node->getAuthorName() + '\n';
			file << line;
		}
	}

	/*
	Deallocates the binary search tree.

	PARAM: bookNode* node, used to traverse the binary search tree recursively.
	*/
	void postOrderDeallocation(bookNode* node)
	{
		//---------------------//
		//Base case of traversal.
		//---------------------//
		if (node == NULL)
			return;

		postOrderDeallocation(node->leftLeafTitle);
		postOrderDeallocation(node->rightLeafTitle);
		delete node;
	}

protected:
	bookNode* root;

	/*
	Prints the books in alphabetical order (title or author).

	PARAM: bookNode* node, used to traverse the tree recursively.
	PARAM: string select, selection for alphabetical traversal along title or author trees.
	*/
	void printInOrder(bookNode* node, string select)
	{
		string title;
		string authorName;
		string space;
		int spaceLength;
		int titleLength;

		//---------------------//
		//Base case of traversal.
		//---------------------//
		if (node == NULL)
			return;

		//---------------------------------------------------//
		//creating string variable 'space' to format the output.
		//---------------------------------------------------//
		title = node->getTitle();
		authorName = node->getAuthorName();
		space = "";
		titleLength = title.length();
		spaceLength = space.length();
		
		while (spaceLength + titleLength < 50)
		{
			space = space + " ";
			spaceLength = space.length();
		}

		//----------------------------------------------//
		//Actual in order printing algorithm. 
		//Recurses along title leafs if title is selected,
		//and along author leafs if author is selected.
		//----------------------------------------------//
		if (select == "title")
		{
			printInOrder(node->leftLeafTitle, select);

			if (!node->getDeleteFlag())
			{
				cout << title << space << " by      " << authorName << "\n";
			}

			printInOrder(node->rightLeafTitle, select);
		}
		else if (select == "author")
		{
			printInOrder(node->leftLeafAuthor, select);

			if (!node->getDeleteFlag())
			{
				cout << title << space << " by      " << authorName << "\n";
			}

			printInOrder(node->rightLeafAuthor, select);
		}
		else
			cout << "incorrect selection, must be 'title' or 'author'";
		
	}

public:
	bookTree()
	{
		root = NULL;
	}

	/*
	Prints the books in bookTree in either alphabetical order based off of the book title, or the authors name.

	PARAM: string select: the selection for which alphabetical order (must be 'title' or 'author')
	*/
	void printList(string select)
	{
		printInOrder(root, select);
	}

	/*
	Allocates and adds a book to the binary search tree with the properties of title and author.
	*/
	void addBook(string title, string author)
	{
		bookNode* newnode;
		bookNode* traversal;
		bookNode* previous;

		newnode = new bookNode;
		newnode->assign(title, author);

		//--------------------------------------------------------------//
		//If the binary search tree is empty, set the newnode as the root.
		//--------------------------------------------------------------//
		if (root == NULL)
		{
			root = newnode;
			return;
		}

		traversal = root;
		previous = NULL;

		//-----------------------------------------------------------------------------//
		//Traverses through the title tree and places the new book in alphabetical order.
		//-----------------------------------------------------------------------------//
		while (traversal != NULL)
		{
			if (alphabetical(newnode->getTitle(), traversal->getTitle()))
			{
				previous = traversal;
				traversal = traversal->leftLeafTitle;
			}
			else
			{
				previous = traversal;
				traversal = traversal->rightLeafTitle;
			}
		}
		if (alphabetical(newnode->getTitle(), previous->getTitle()))
			previous->leftLeafTitle = newnode;
		else
			previous->rightLeafTitle = newnode;

	//------------------------------------------------------------------------------//
	//Traverses through the author tree and places the new book in alphabetical order.
	//------------------------------------------------------------------------------//
		traversal = root;
		previous = NULL;

		while (traversal != NULL)
		{
			if (alphabetical(newnode->getAuthorName(), traversal->getAuthorName()))
			{
				previous = traversal;
				traversal = traversal->leftLeafAuthor;
			}
			else
			{
				previous = traversal;
				traversal = traversal->rightLeafAuthor;
			}
		}
		if (alphabetical(newnode->getAuthorName(), previous->getAuthorName()))
			previous->leftLeafAuthor = newnode;
		else
			previous->rightLeafAuthor = newnode;
	}

	/*
	Effectively deletes a book from the binary search tree (sets the delete flag as true).

	PARAM: string title, the title of the book being deleted.
	PARAM: string author, the author of the book being deleted.
	RETURN: bool, an indicator of whether a book with the given parameters
	was actually deleted (if no book matches the parameters, false is returned)
	*/
	bool deleteBook(string title, string author)
	{
		bookNode* traversal;
		string traversalTitle;
		string traversalAuthor;

		traversal = root;
		traversalTitle = traversal->getTitle();
		traversalAuthor = traversal->getAuthorName();

		//----------------------------------------------------------------------------------//
		//Traverses the title tree and stops iterating if it finds a book that 
		//matches both parameters, returns false if the book is absent from the 
		//binary search tree in the position it would be in (traversal reaches null pointer).
		//----------------------------------------------------------------------------------//
		while ((title != traversalTitle) && (author != traversalAuthor))
		{
			
			if (alphabetical(title, traversalTitle))
			{
				if (traversal->leftLeafTitle == NULL)
					return false;
				traversal = traversal->leftLeafTitle;
			}
			else
			{
				if (traversal->rightLeafTitle == NULL)
					return false;
				traversal = traversal->rightLeafTitle;
			}
				

			traversalTitle = traversal->getTitle();
			traversalAuthor = traversal->getAuthorName();
		}

		//------------------------------------------------------------------//
		//Effective deletion of the book. Actual deletion comes when the book 
		//is deallocated and isn't written back into the .csv file.
		//------------------------------------------------------------------//
		traversal->setDeleteFlag();
		return true;
	}

	/*
	Writes the books in the binary search tree back into the .csv file.

	PARAM:string filename, the filename to be written back into.
	*/
	void writeback(string filename)
	{
		bookNode* node;
		ofstream file;

		//-------------------------------------------------------------------------------//
		//Integer num is used to decide whether writeback is done in post order with title 
		//tree or author tree, increases the randomization of the .csv file.
		//-------------------------------------------------------------------------------//
		int num = rand() % 2; 
		file.open(filename);
		file.close();
		file.open(filename, ios::app);

		node = root;

		//---------------------------------------------------------------//
		//Call to printPostOrderToFile does the actual writing to the file.
		//---------------------------------------------------------------//
		printPostOrderToFile(node, file, num);
		file.close();
	}

	//--------------------------------------------------------------------------//
	//Deconstructor:
	//deallocates the book tree by calling the recursive deallocation method.
	//
	//'root = NULL' is here because when the writeback and reinitialization 
	//occurs during runtime on a static version of the object, the binary search 
	 //tree will not properly be reinitialized if root isn't null. This is because 
	//the constructor that would normally set it to NULL cannot be called.
	//--------------------------------------------------------------------------//
	~bookTree() 
	{
		bookNode* node;
		node = root;
		postOrderDeallocation(node);
		root = NULL;
	}
};

/*
	Inherits from the bookTree class, and also adds a patron name to store and keep track of
	in addition to the title and author parameters. The patron name is also included in the
	.csv file.

	Encapsulation is used such that the program/user only has the access/ability to:
	 - (parameter)	Pointer to the next patronNode object in a linked list of patronNode objects
	 - (method)		Set the patron name.
	 - (method)		Get the patron name.
	 - (method)		Write back to the .csv file, calls a similar post order print as the book tree,
				but includes the patron name.
*/
class patronNode : public bookTree
{
private:
	string name;

	/*
	Exactly the same method as the writeback for just a book tree, except it prints the name as well.
	See the printPostOrderToFile() method on line 112 for information on how the method works.
	*/
	void patronPrintPostOrderToFile(bookNode* node, ofstream& file, int num)
	{
		string line;
		string title;
		string author;

		if (node == NULL)
			return;

		if (num == 1)
		{
			patronPrintPostOrderToFile(node->leftLeafTitle, file, num);
			patronPrintPostOrderToFile(node->rightLeafTitle, file, num);
		}
		else
		{
			patronPrintPostOrderToFile(node->leftLeafAuthor, file, num);
			patronPrintPostOrderToFile(node->rightLeafAuthor, file, num);
		}
		if (!node->getDeleteFlag())
		{
			line = node->getTitle() + ',' + node->getAuthorName() + ',' + name + '\n';
			file << line;
		}
	}
public:
	patronNode* next = NULL;

	void setName(string Name)
	{
		name = Name;
	}
	string getName(void)
	{
		return name;
	}
	/*
	Similar method as used for just the book tree class on line 377, except this method
	does not have to do any file io, as it is being called by another method that inherits this 
	class. Doing the same file opens and closes would result in loss of data.
	*/
	void patronWriteback(string filename, ofstream& file)
	{
		bookNode* node;
		int num = rand() % 2;
	
		node = root;

		patronPrintPostOrderToFile(node, file, num);
	}

};

/*
	the patronList class does all operations that allocate, traverse, access data from, and deallocate 
	from a linked list of patronNodes.

	Encapsulation is used such that the program/user only has the access/ability to:
	- (method)		Print the list.
	- (method)		Check out a book.
	- (method)		Check in a book (return).
	- (method)		Write back the data in the linked list to a .csv file containing data of checked out books.

*/
class patronList
{
private:
	patronNode* head;
	string writebackFilename;

public:
	patronList(string WritebackFilename)
	{
		writebackFilename = WritebackFilename;
		head = NULL;
	}

	/*
	Prints the checked out books by traversing the linked list and printing the patron (book tree) in each node.

	PARAM: string select, selects whether to print alphabetically according to title or author.
	*/
	void printList(string select)
	{
		patronNode* traversal;
		traversal = head;

		//---------------//
		//Prints head node.
		//---------------//
		cout << traversal->getName() << " has the following books checked out:" << endl;
		traversal->printList(select);
		cout << endl;

		if (traversal->next == NULL)
			return;

		//-------------------------//
		//Prints the remaining nodes.
		//-------------------------//
		do
		{
			traversal = traversal->next;
			cout << traversal->getName() << " has the following books checked out:" << endl;
			traversal->printList(select);
			cout << endl;
		} while (traversal->next != NULL);
	}

	/*
	Checks out a book (adds to the patron list) by traversing through the linked list to see whether the patron who is 
	checking a book out is already in the system. If they are in the system, the book is added to their binary search tree. 
	If they are not in the system, a patron node is allocated and assigned the parameters.

	PARAM: string title, title of the book being checked out.
	PARAM: string author, author of the book being checked out.
	PARAM: patronName, name of the patron checking the book out.
	*/
	void checkOut(string title, string author, string patronName)
	{
		patronNode* patron;
		patronNode* newnode;

		//---------------------------------------------------//
		//If linked list is empty, add a new node to the head.
		//---------------------------------------------------//
		if (head == NULL)
		{
			newnode = new patronNode;
			newnode->setName(patronName);
			newnode->addBook(title, author);
			head = newnode;
			return;
		}

		//--------------------------------------------------------------------------------//
		//If linked list is empty, iterate through the list trying to find the patrons name.
		//--------------------------------------------------------------------------------//
		patron = head;
		while (patron->next != NULL)
		{
			if (patronName == patron->getName())
			{
				patron->addBook(title, author);
				return;
			}
			patron = patron->next;
		}
		if (patronName == patron->getName())
		{
			patron->addBook(title, author);
			return;
		}

		//-------------------------------------------------------------------------------------------------------//
		//If the method hasn't returned yet it means the patron was not in the list, so they are added to the list.
		//-------------------------------------------------------------------------------------------------------//
		newnode = new patronNode;
		newnode->setName(patronName);
		newnode->addBook(title, author);
		patron->next = newnode;
	}

	/*
	checks in a book (removes from the patron list) by traversing through all users and thier respective binary search trees trying 
	to find the book (defined by the parameters title and author). Returns true and sets the delete flag if it is found, 
	returns false if a checked out book is not found matching the parameters.

	PARAM: string title, title of the book to check back in.
	PARAM: string author, author of the book to check back in.
	RETURN: bool indicates whether a book was found and deleted or not.
	*/
	bool checkIn(string title, string author)
	{
		patronNode* patron;
		patron = head;

		while (!patron->deleteBook(title, author))
		{
			patron = patron->next;
			if (patron == NULL)
				return false;
		}
		return true;
	}

	/*
	writes back the list to the .csv file containing data on the checked out books.

	PARAM: string filename, the file to write back to.
	*/
	void fullListWriteback(string filename)
	{
		patronNode* traversal;
		traversal = head;
		
		//---------------------------------------------------//
		//Opens and closes the file to delete before appending.
		//---------------------------------------------------//
		ofstream file;
		file.open(filename);
		file.close();
		file.open(filename, ios::app);

		//--------------------//
		//Linked list traversal.
		//--------------------//
		while (traversal->next != NULL)
		{
			traversal->patronWriteback(filename, file);
			traversal = traversal->next;
		}
		traversal->patronWriteback(filename, file);
		file.close();
	}

	//-------------------------------------------------------------------------//
	//Deconstructor:
	//deallocates the patron list.
	//'head = NULL' is here because when the writeback and reinitialization 
	//occurs during runtime on a static version of the object, the binary search 
	//tree will not properly be reinitialized if root isn't null. This is because 
	//the constructor that would normally set it to NULL cannot be called.
	//-------------------------------------------------------------------------//
	~patronList()
	{
		fullListWriteback(writebackFilename);

		patronNode* traversal;
		patronNode* next;

		traversal = head;
		next = traversal->next;

		while (traversal->next != NULL)
		{
			delete traversal;
			traversal = next;
			next = traversal->next;
		}
		delete traversal;

		head = NULL;
	}
};

/*
	Library class inherits the booktree class, and also controls a linked list 
	of book nodes by using their rightLeafTitle bookNode* as 'next node'.

	Encapsulation is used such that the program/user only has the access/ability to:
	- (method)		Return a book (not added to the booktree until books are returned to be shelved).

*/
class Library : public bookTree
{
private:
	string writebackFilename;
	bookNode* shelvingBacklogHead;
	int linkedListLength;
public:
	Library(string WritebackFilename)
	{
		writebackFilename = WritebackFilename;
	}

	/*
	Returns a book. The book is added to a linked list of book nodes first, until
	five books have accumulated, then they are shelved into the book tree.

	PARAM: string title, title of the book being returned.
	PARAM: string author, author of the book being returned.
	*/
	void bookReturned(string title, string author)
	{
		bookNode* newnode;
		bookNode* traverse;
		bookNode* nextnode;
		newnode = new bookNode;

		newnode->assign(title, author);

		//---------------------------------------------------------//
		//If the linked list is empty, add the new node to the head.
		//---------------------------------------------------------//
		if (shelvingBacklogHead == NULL)
		{
			shelvingBacklogHead = newnode;
			linkedListLength = 1;
			return;
		}

		traverse = shelvingBacklogHead;
		//---------------------------------------------------------------------------//
		//Using rightLeafTitle pointer like a next pointer so that the same class can
		//be used in to form a tree and a linked list.
		//---------------------------------------------------------------------------//
		while (traverse->rightLeafTitle != NULL)
		{
			traverse = traverse->rightLeafTitle;
		}
		traverse->rightLeafTitle = newnode;
		linkedListLength++;

		//-----------------------------------------------------------------//
		//Books are shelved once five have been returned to the same library.
		//-----------------------------------------------------------------//
		if (linkedListLength == 5)
		{
			//----------------------------------------------------------------------------//
			//So that the user can be informed that books were shelved and at which library.
			//----------------------------------------------------------------------------//
			if (writebackFilename == "CentralBransh.csv")
			{
				s_CentralBranchBooksShelvedFlag = true;
			}
			else if (writebackFilename == "PointGreyBranch.csv")
			{
				s_PGBranchBooksShelvedFlag = true;
			}
			else
			{
				s_MPBranchBooksShelvedFlag = true;
			}
			
			traverse = shelvingBacklogHead;

			for (int i = 1; i <= 5; i++)
			{
				addBook(traverse->getTitle(), traverse->getAuthorName());
				traverse = traverse->rightLeafTitle;
			}

			//------------------------------------------------------//
			//Once books have been added to the tree, deallocate list.
			//------------------------------------------------------//
			traverse = shelvingBacklogHead;
			for (int i = 1; i <= 5; i++)
			{
				nextnode = traverse->rightLeafTitle;
				delete traverse;
				traverse = nextnode;

			}

			//---------------------------------------------------//
			//Head is set to null once books have been deallocated.
			//---------------------------------------------------//
			shelvingBacklogHead = NULL;
		}
	}

	//--------------------------------------------------------------------//
	//Deconstructor:
	//If Library is being deconstructed, be sure to add the backlog of books
	//to the library even if there aren't five, and deallocate the list.
	//Also calls a writeback of the binary search tree which represents it using the 
	//writebackFilename parameter.
	//--------------------------------------------------------------------//
	~Library()
	{
		bookNode* traverse;
		bookNode* nextnode;
		traverse = shelvingBacklogHead;
		while (traverse != NULL)
		{
			addBook(traverse->getTitle(), traverse->getAuthorName());
			traverse = traverse->rightLeafTitle;
		}
		traverse = shelvingBacklogHead;
		while (traverse != NULL)
		{
			nextnode = traverse->rightLeafTitle;
			delete traverse;
			traverse = nextnode;
		}

		writeback(writebackFilename);
		shelvingBacklogHead = NULL;
	}
};

/*
Static variables, to be used by multiple threads.
*/
static string s_checkedOutFileName = "CheckedOut.csv";
static string s_CentralBranchFileName = "CentralBranch.csv";
static string s_PGBranchFileName = "PointGreyBranch.csv";
static string s_MPBranchFileName = "MountPleasantBranch.csv";

static patronList s_checkedOutObject(s_checkedOutFileName);
static Library s_CentralBranchObject(s_CentralBranchFileName);
static Library s_PGBranchObject(s_PGBranchFileName);
static Library s_MPBranchObject(s_MPBranchFileName);


int main(void)
{
	//--------------------------------------------------------------------------------//
	//Initialize, create writeback thread, then call the comand window control function.
	//When control is passed back to main from the control window funciton, the static
	//variable s_endProgram indicates to the writeback thread that it is time to stop.
	//--------------------------------------------------------------------------------//
	initializeLibraries();

	thread t1(writebackAndReinitialize);
	cmdWindowControl();
	s_endProgram = true;
	t1.join();
}

/*
Function that returns 1 if parameter a comes before b in alphabetical order,
used to traverse binary search trees.
*/
bool alphabetical(string a, string b)
{
	return a < b;
}

/*
Initializes the objects of library, and the checked out books by reading
all of the data from the .csv files to dynamic memory.
*/
void initializeLibraries(void)
{
	string line;
	string word;
	string title;
	string author;
	string patronName;

	ifstream checkedOut_ifstream;
	ifstream CentralBranch_ifstream;
	ifstream PGBranch_ifstream;
	ifstream MPBranch_ifstream;

	checkedOut_ifstream.open(s_checkedOutFileName);
	CentralBranch_ifstream.open(s_CentralBranchFileName);
	PGBranch_ifstream.open(s_PGBranchFileName);
	MPBranch_ifstream.open(s_MPBranchFileName);

	//--------------------------------------------------------------------------//
	//Adding all the books in the CheckedOut file to the static patronList object.
	//--------------------------------------------------------------------------//
	if (checkedOut_ifstream.is_open())
	{
		while (getline(checkedOut_ifstream, line))
		{
			stringstream ss(line);
			getline(ss, word, ',');
			title = word;
			getline(ss, word, ',');
			author = word;
			getline(ss, word, ',');
			patronName = word;
			s_checkedOutObject.checkOut(title, author, patronName);
		}
	}
	else
	{
		cout << "**ERROR opening file**" << endl;
	}
	

	//--------------------------------------------------------------------//
	//Adding all the books in CentralBranch file to a static library object.
	//--------------------------------------------------------------------//
	if (CentralBranch_ifstream.is_open())
	{
		while (getline(CentralBranch_ifstream, line))
		{
			stringstream ss(line);
			getline(ss, word, ',');
			title = word;
			getline(ss, word, ',');
			author = word;
			s_CentralBranchObject.addBook(title, author);
		}
	}
	else
	{
		cout << "**ERROR opening file**" << endl;
	}

	//---------------------------------------------------------------------//
	//Adding all the books in PointGreyBranch file to a static library object.
	//---------------------------------------------------------------------//
	if (PGBranch_ifstream.is_open())
	{
		while (getline(PGBranch_ifstream, line))
		{
			stringstream ss(line);
			getline(ss, word, ',');
			title = word;
			getline(ss, word, ',');
			author = word;
			s_PGBranchObject.addBook(title, author);
		}
	}
	else
	{
		cout << "**ERROR opening file**" << endl;
	}

	//--------------------------------------------------------------------------//
	//Adding all the books in MountPleasantBranch file to a static library object.
	//--------------------------------------------------------------------------//
	if (MPBranch_ifstream.is_open())
	{
		while (getline(MPBranch_ifstream, line))
		{
			stringstream ss(line);
			getline(ss, word, ',');
			title = word;
			getline(ss, word, ',');
			author = word;
			s_MPBranchObject.addBook(title, author);
		}
	}
	else
	{
		cout << "**ERROR opening file**" << endl;
	}
	

	checkedOut_ifstream.close();
	CentralBranch_ifstream.close();
	PGBranch_ifstream.close();
	MPBranch_ifstream.close();
}

/*
Used in a separate thread from the main function, writes back the dynamic instances of every book back into
their respective files. The objects are then reinitialized. This action cleans up the deleted books by not 
writing back books with true delete flags. While this function isn't particularly useful for this program, 
if the solution were scaled up to the size of an actual and was being accessed by multiple computers actually
manage a library system, this writeback would be important.
people, this would be necessary.
*/
void writebackAndReinitialize(void)
{
	int i = 0;
	while (true)
	{
		//----------------------------------------------------------------//
		//checks to see if the program should end, and return if it should.
		//----------------------------------------------------------------//
		while (i < 5 * 60)
		{
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(1));
			i++;
			if (s_endProgram)
				return;
		}

		//--------------------------------------------------------------------------//
		//Need to lock so that the the program doesn't try to access books while they
		//are being written back.
		//--------------------------------------------------------------------------//
		s_mu.lock();
		//-----------------------------------//
		//Writeback, reinitialize, then unlock.
		//-----------------------------------//
		s_checkedOutObject.~patronList();
		s_CentralBranchObject.~Library();
		s_PGBranchObject.~Library();
		s_MPBranchObject.~Library();
		initializeLibraries();
		s_mu.unlock();
		s_spreadsheetWritebackFlag = true;
		i = 0;
	}
}

/*
Flow of case statements and getlines that control the system based on inputs from the user.
*/
void cmdWindowControl(void)
{
	string select;
	string entered;
	string title;
	string author;
	string patron;
	char select_char;
	
	//---------------------------------------------------------------------//
	//When starting, or returning to start, check if any flags are up.
	//
	//Start:
	//User input what action they would like the program to complete,
	//and program uses a switch statement and goto to branch to that location.
	//Quitting the application returns to the main sequence.
	//---------------------------------------------------------------------//
start:
	if (s_spreadsheetWritebackFlag)
	{
		cout << "\n\n\n";
		cout << "          ***Spreadsheets have been updated***" << endl;
		cout << "\n\n\n";
		s_spreadsheetWritebackFlag = false;
	}
	if (s_CentralBranchBooksShelvedFlag)
	{
		cout << "\n\n\n";
		cout << "          ***Returned books have been shelved at the central branch***" << endl;
		cout << "\n\n\n";
		s_CentralBranchBooksShelvedFlag = false;
	}
	if (s_PGBranchBooksShelvedFlag)
	{
		cout << "\n\n\n";
		cout << "          ***Returned books have been shelved at the Point Grey branch***" << endl;
		cout << "\n\n\n";
		s_PGBranchBooksShelvedFlag = false;
	}
	if (s_MPBranchBooksShelvedFlag)
	{
		cout << "\n\n\n";
		cout << "          ***Returned books have been shelved at the Mount Pleasant branch***" << endl;
		cout << "\n\n\n";
		s_MPBranchBooksShelvedFlag = false;
	}
	cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	cout << "          Check out book:                                            (enter 1)" << endl;
	cout << "          View catalogue:                                            (enter 2)" << endl;
	cout << "          Return book:                                               (enter 3)" << endl;
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
		goto check_out;
	case '2':
		goto view_catalogue;
	case '3':
		goto return_book;
	case '4':
		return;
	default:
		cout << "          invalid entry, try again:" << endl;
		goto start_input;
	}



	//----------------------------------------------------------------------//
	//check_out:
	//The user inputs the title, author, and the patron checking the book out.
	//The program tries each library to see if that book is available,
	//and if it is the book is checked out by the patron specified.
	//
	//Need to ensure that s_mu.lock() and unlock() are used when accessing
	//data in objects that could be written back and reinitialized.
	//----------------------------------------------------------------------//
check_out:
	cout << "          Enter the title, author, then the patron checking the book out." << endl;
	cout << "title: ";
	getline(cin, entered, '\n');
	title = entered;
	cout << "author: ";
	getline(cin, entered, '\n');
	author = entered;
	cout << "patron: ";
	getline(cin, entered, '\n');
	patron = entered;
	cout << endl;

	s_mu.lock();
	if (s_CentralBranchObject.deleteBook(title, author))
	{
		s_checkedOutObject.checkOut(title, author, patron);
		cout << title << " by " << author << " checked out from the Central branch by " << patron << "." << endl;
	}
	else if (s_PGBranchObject.deleteBook(title, author))
	{
		s_checkedOutObject.checkOut(title, author, patron);
		cout << title << " by " << author << " checked out from the Point Grey branch by " << patron << "." << endl;
	}
	else if(s_MPBranchObject.deleteBook(title, author)) 
	{
		s_checkedOutObject.checkOut(title, author, patron);
		cout << title << " by " << author << " checked out from the Mount Pleasant branch by " << patron << "." << endl;
	}
	else
	{
		cout << "          The book entered is not available at any of our libraries";
	}
	s_mu.unlock();
	goto start; //Return to start.


	//------------------------------------------------------------------//
	//view_catalogue:
	//Program prints the contents of every library in alphabetical order
	//according to title or author depending on what the user chooses.
	//
	//Need to ensure that s_mu.lock() and unlock() are used when accessing
	//data in objects that could be written back and reinitialized.
	//------------------------------------------------------------------//
view_catalogue:
	cout << "          Enter whether the catalogue should be in alphabetical order of 'title' or 'author':" << endl;
view_catalogue_entry:
	getline(cin, entered, '\n');
	if ((entered != "title") && (entered != "author"))
	{
		cout << "          Invalid entry, please enter 'title' or 'author':" << endl;
		goto view_catalogue_entry;
	}
	s_mu.lock();
	cout << "\n\n";
	cout << "The books at the Central branch are:" << endl;
	s_CentralBranchObject.printList(entered);
	cout << "\n\n";
	cout << "The books at the Point Grey branch are:" << endl;
	s_PGBranchObject.printList(entered);
	cout << "\n\n";
	cout << "The books at the Mount Pleasant branch are:" << endl;
	s_PGBranchObject.printList(entered);
	s_mu.unlock();
	goto start; //Return to start.



	//------------------------------------------------------------------//
	//return_book:
	//prints the patrons and what books the have checked out,
	//then the user inputs what title and author is being returned.
	//if the book entered is valid, the user also chooses which 
	//library the book is being returned to. The book is then
	//removed from the patron list object, and added to the proper
	//library object.
	//
	//Need to ensure that s_mu.lock() and unlock() are used when accessing
	//data in objects that could be written back and reinitialized.
	//------------------------------------------------------------------//
return_book:
	cout << "          The following patrons have the following books checked out." << endl;
	s_checkedOutObject.printList("title");
	cout << "          Enter the title, author, and library being returned to." << endl;
return_book_entry:
	cout << "title: ";
	getline(cin, entered, '\n');
	title = entered;
	cout << "author: ";
	getline(cin, entered, '\n');
	author = entered;

	s_mu.lock();
	bool temp = !s_checkedOutObject.checkIn(title, author);
	s_mu.unlock();

	if (temp)
	{
		cout << "          Invalid book entry, please try again:" << endl;
		goto return_book_entry;
	}

book_entry_library:
	cout << "enter 'Central', 'Point Grey', or 'Mount Pleasant' to select the library:";
	getline(cin, entered, '\n');
	if ((entered != "Central") && (entered != "Point Grey") && (entered != "Mount Pleasant"))
		goto book_entry_library;
	
	s_mu.lock();
	if (entered == "Central")
		s_CentralBranchObject.bookReturned(title, author);
	else if (entered == "Point Grey")
		s_PGBranchObject.bookReturned(title, author);
	else
		s_MPBranchObject.bookReturned(title, author);
	s_mu.unlock();
	cout << title << " by " << author << " returned to " << entered << " branch." << endl;
	goto start;//return to start.
}