#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>

bool alphabetical(std::string a, std::string b);
void initializeLibraries(void);
void cmdWindowControl(void);
void writebackAndReinitialize(void);

static std::mutex s_mu;
static bool s_endProgram = false;
static bool s_spreadsheetWritebackFlag = false;
static bool s_CentralBranchBooksShelvedFlag = false;
static bool s_PGBranchBooksShelvedFlag = false;
static bool s_MPBranchBooksShelvedFlag = false;

/*
	class description:
	Objects of bookNode are used as storage for the title and author of a book, and as
	nodes that are part of two different binary search trees. The left leaf always comes
	alphabetically before the right leaf. There is a binary search tree for both the title
	and the author, which is the reason for the 4 bookNode pointers defined.
*/
class bookNode
{
private:
	std::string title;
	std::string authorName;
	bool deleteFlag = false;
public:

	bookNode* rightLeafTitle = NULL;
	bookNode* leftLeafTitle = NULL;
	bookNode* rightLeafAuthor = NULL;
	bookNode* leftLeafAuthor = NULL;

	void assign(std::string Title, std::string Author)
	{
		title = Title;
		authorName = Author;
	}
	std::string getTitle(void)
	{
		return title;
	}
	std::string getAuthorName(void)
	{
		return authorName;
	}

	/*
		function description:
		The delete flag is here because when a book is removed from the binary search tree,
		it isn't deleted from memory or removed from the tree (this would cause issues with
		the overlapping trees). Instead, the delete flag is set to true, and the 'removed'
		bookNode isn't written back into the .csv file that is used to create this tree.
	*/
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
	class description:
	the bookTree class is a binary search tree that contains all the functions which
	allocate, traverse, access data from, and deallocate bookNodes.

	the bookTree class also does all of the writing back to the spreadsheet .csv files.
*/
class bookTree
{
private:

	/*
		function description:
		Prints the binary search tree in post order to the .csv file, this is done during writeback.
		It is done in post order, because, if the .csv file were to become in alphabetical order then the tree
		would become a linked list, which has a slower search time then a binary search tree.

		PARAM:	bookNode* node, used to traverse the tree recursively.
		PARAM:	ofstream file, file stream of the .csv file.
		PARAM:	integer num, a random number (0 or 1) which decides whether to writeback with the
				title tree or the author tree. The goal of this is to keep the .csv file as randomized
				as possible to optimize the binary search tree.
	*/
	void printPostOrderToFile(bookNode* node, std::ofstream& file, int num)
	{
		std::string line;
		std::string title;
		std::string author;

		//base case
		if (node == NULL)
			return;

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

		//only write back if the delete flag is false
		if (!node->getDeleteFlag())
		{
			line = node->getTitle() + ',' + node->getAuthorName() + '\n';
			file << line;
		}
	}

	/*
		function description:
		Deallocates the binary search tree.

		PARAM: bookNode* node, used to traverse the binary search tree recursively.
	*/
	void postOrderDeallocation(bookNode* node)
	{
		//base case
		if (node == NULL)
			return;

		postOrderDeallocation(node->leftLeafTitle);
		postOrderDeallocation(node->rightLeafTitle);
		delete node;
	}

protected:
	bookNode* root;

	/*
		function description:
		Prints the books in alphabetical order (title or author).

		PARAM: bookNode* node, used to traverse the tree recursively.
		PARAM: string select, selection for title or author.
	*/
	void printInOrder(bookNode* node, std::string select)
	{
		std::string title;
		std::string authorName;
		std::string space;
		int spaceLength;
		int titleLength;

		//base case
		if (node == NULL)
			return;

		//creating string variable 'space' to format the output.
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

		//In order printing algorithm. 
		//Recurses along title leafs if title is selected,
		//and along author leafs if author is selected.
		if (select == "title")
		{
			printInOrder(node->leftLeafTitle, select);

			if (!node->getDeleteFlag())
			{
				std::cout << title << space << " by      " << authorName << std::endl;
			}

			printInOrder(node->rightLeafTitle, select);
		}
		else if (select == "author")
		{
			printInOrder(node->leftLeafAuthor, select);

			if (!node->getDeleteFlag())
			{
				std::cout << title << space << " by      " << authorName << std::endl;
			}

			printInOrder(node->rightLeafAuthor, select);
		}
		else
			std::cout << "incorrect selection, must be 'title' or 'author'";

	}

public:
	bookTree()
	{
		root = NULL;
	}

	/*
		function description:
		Prints the books in bookTree in either alphabetical order based off of the book title, or the authors name.

		PARAM: string select: the selection for which alphabetical order (must be 'title' or 'author')
	*/
	void printList(std::string select)
	{
		printInOrder(root, select);
	}

	void addBook(std::string title, std::string author)
	{
		bookNode* newnode;
		bookNode* traversal;
		bookNode* previous;

		newnode = new bookNode;
		newnode->assign(title, author);

		//If the binary search tree is empty, set the newnode as the root.
		if (root == NULL)
		{
			root = newnode;
			return;
		}

		traversal = root;
		previous = NULL;

		//Traverses through the title tree and places the new book in alphabetical order.
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

		//Traverses through the author tree and places the new book in alphabetical order.
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
		function description:
		Effectively deletes a book from the binary search tree (sets the delete flag as true).

		PARAM: string title, the title of the book being deleted.
		PARAM: string author, the author of the book being deleted.
		RETURN: bool, an indicator of whether a book with the given parameters
		was actually deleted (if no book matches the parameters, false is returned)
	*/
	bool deleteBook(std::string title, std::string author)
	{
		bookNode* traversal;
		std::string traversalTitle;
		std::string traversalAuthor;

		traversal = root;
		traversalTitle = traversal->getTitle();
		traversalAuthor = traversal->getAuthorName();

		//Traverses the title tree and stops iterating if a book is found which matches the paramters,
		//returns false if the book is absent from the binary search tree
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

		//Effective deletion of the book. Actual removal from memory comes 
		//when the book is deallocated but isn't written back into the .csv file.  
		traversal->setDeleteFlag();
		return true;
	}

	/*
		function description:
		Writes the books in the binary search tree back into the .csv file.

		PARAM:string filename, the filename to be written back into.
	*/
	void writeback(std::string filename)
	{
		bookNode* node;
		std::ofstream file;

		//Integer num is used to decide whether writeback is done in post order with title 
		//tree or author tree, which increases the randomization of the .csv file.
		int num = rand() % 2;
		file.open(filename);
		file.close();
		file.open(filename, std::ios::app);

		node = root;

		//Call to printPostOrderToFile does the actual writing to the file.
		printPostOrderToFile(node, file, num);
		file.close();
	}

	/*
			funciton definition:
			deallocates the book tree by calling the recursive deallocation function.
	*/
	~bookTree()
	{
		bookNode* node;
		node = root;
		postOrderDeallocation(node);
		root = NULL;
	}
};

/*
	class description:
	Inherits from the bookTree class, also keeps track of a patron name in addition to
	the title and author parameters. The patron name is also included in the .csv file.
*/
class patronNode : public bookTree
{
private:
	std::string name;

	/*
		function description:
		Exactly the same function as the writeback for just a book tree,
		except it prints the patron name in addition to the title and author.
	*/
	void patronPrintPostOrderToFile(bookNode* node, std::ofstream& file, int num)
	{
		std::string line;
		std::string title;
		std::string author;

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

	void setName(std::string Name)
	{
		name = Name;
	}
	std::string getName(void)
	{
		return name;
	}

	/*
		Similar function as used for just the book tree class, except the file open
		and closing is already done in the other function.
	*/
	void patronWriteback(std::string filename, std::ofstream& file)
	{
		bookNode* node;
		int num = rand() % 2;

		node = root;

		patronPrintPostOrderToFile(node, file, num);
	}

};

/*
	class description:
	the patronList class does all operations that allocate, traverse, access data from, and deallocate
	from a linked list of patronNodes.
*/
class patronList
{
private:
	patronNode* head;
	std::string writebackFilename;

public:
	patronList(std::string WritebackFilename)
	{
		writebackFilename = WritebackFilename;
		head = NULL;
	}

	/*
		function description:
		Prints the checked out books by traversing the linked list and printing the patron (book tree) in each node.

		PARAM: string select, selects whether to print alphabetically according to title or author.
	*/
	void printList(std::string select)
	{
		patronNode* traversal;
		traversal = head;

		//Prints head node.
		std::cout << traversal->getName() << " has the following books checked out:" << std::endl;
		traversal->printList(select);
		std::cout << std::endl;

		if (traversal->next == NULL)
			return;

		//Prints the remaining nodes.
		do
		{
			traversal = traversal->next;
			std::cout << traversal->getName() << " has the following books checked out:" << std::endl;
			traversal->printList(select);
			std::cout << std::endl;
		} while (traversal->next != NULL);
	}

	/*
		function description:
		Checks out a book (adds to the patron list) by traversing through the linked list to see whether the patron who is
		checking a book out is already in the system. If they are in the system, the book is added to their binary search tree.
		If they are not in the system, a patron node is allocated and assigned the parameters.

		PARAM: string title, title of the book being checked out.
		PARAM: string author, author of the book being checked out.
		PARAM: patronName, name of the patron checking the book out.
	*/
	void checkOut(std::string title, std::string author, std::string patronName)
	{
		patronNode* patron;
		patronNode* newnode;

		//If linked list is empty, add a new node to the head.
		if (head == NULL)
		{
			newnode = new patronNode;
			newnode->setName(patronName);
			newnode->addBook(title, author);
			head = newnode;
			return;
		}

		//If linked list is not empty, iterate through the list trying to find the patrons name.
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

		//If the function hasn't returned yet it means the patron was not in the list, so they are added to the list.
		newnode = new patronNode;
		newnode->setName(patronName);
		newnode->addBook(title, author);
		patron->next = newnode;
	}

	/*
		function description:
		checks in a book (removes from the patron list) by traversing through all users and thier respective binary search trees trying
		to find the book (defined by the parameters title and author). Returns true and sets the delete flag if it is found,
		returns false if a checked out book is not found matching the parameters.

		PARAM: string title, title of the book to check back in.
		PARAM: string author, author of the book to check back in.
		RETURN: bool indicates whether a book was found and deleted or not.
	*/
	bool checkIn(std::string title, std::string author)
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
		function description:
		writes back the list to the .csv file containing data on the checked out books.

		PARAM: string filename, the file to write back to.
	*/
	void fullListWriteback(std::string filename)
	{
		patronNode* traversal;
		traversal = head;

		//---------------------------------------------------//
		//Opens and closes the file to delete before appending.
		//---------------------------------------------------//
		std::ofstream file;
		file.open(filename);
		file.close();
		file.open(filename, std::ios::app);

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

	/*
		function description:
		deallocates the patron list.
	*/
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
	class description:
	Library class inherits the booktree class, and also controls a linked list
	of book nodes by using their rightLeafTitle bookNode* as 'next node'.
*/
class Library : public bookTree
{
private:
	std::string writebackFilename;
	bookNode* shelvingBacklogHead;
	int linkedListLength;
public:
	Library(std::string WritebackFilename)
	{
		writebackFilename = WritebackFilename;
	}

	/*
		function description:
		Returns a book. The book is added to a linked list of book nodes first, until
		five books have accumulated, then they are shelved into the book tree.

		PARAM: string title, title of the book being returned.
		PARAM: string author, author of the book being returned.
	*/
	void bookReturned(std::string title, std::string author)
	{
		bookNode* newnode;
		bookNode* traverse;
		bookNode* nextnode;
		newnode = new bookNode;

		newnode->assign(title, author);

		//If the linked list is empty, add the new node to the head.
		if (shelvingBacklogHead == NULL)
		{
			shelvingBacklogHead = newnode;
			linkedListLength = 1;
			return;
		}

		traverse = shelvingBacklogHead;
		//Using rightLeafTitle pointer like a next pointer so that the same class can
		//be used in to form a tree and a linked list.
		while (traverse->rightLeafTitle != NULL)
		{
			traverse = traverse->rightLeafTitle;
		}
		traverse->rightLeafTitle = newnode;
		linkedListLength++;

		//Books are shelved once five have been returned to the same library.
		if (linkedListLength == 5)
		{
			//So that the user can be informed that books were shelved and at which library.
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

			//Once books have been added to the tree, deallocate list.
			traverse = shelvingBacklogHead;
			for (int i = 1; i <= 5; i++)
			{
				nextnode = traverse->rightLeafTitle;
				delete traverse;
				traverse = nextnode;

			}

			//Head is set to null once books have been deallocated.
			shelvingBacklogHead = NULL;
		}
	}

	/*
		function description:
		If Library is being deconstructed, be sure to add the backlog of books
		to the library even if there aren't five, and deallocate the list.
		Also calls a writeback of the binary search tree which represents it using the
		writebackFilename parameter.
	*/
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
static std::string s_checkedOutFileName = "CheckedOut.csv";
static std::string s_CentralBranchFileName = "CentralBranch.csv";
static std::string s_PGBranchFileName = "PointGreyBranch.csv";
static std::string s_MPBranchFileName = "MountPleasantBranch.csv";

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

	std::thread t1(writebackAndReinitialize);
	cmdWindowControl();
	s_endProgram = true;
	t1.join();
}

/*
Function that returns 1 if parameter a comes before b in alphabetical order,
used to traverse binary search trees.
*/
bool alphabetical(std::string a, std::string b)
{
	return a < b;
}

/*
Initializes the objects of library, and the checked out books by reading
all of the data from the .csv files to dynamic memory.
*/
void initializeLibraries(void)
{
	std::string line, word, title, author, patronName;

	std::ifstream checkedOutStream, CentralBranchStream, PGBranchStream, MPBranchStream;

	checkedOutStream.open(s_checkedOutFileName);
	CentralBranchStream.open(s_CentralBranchFileName);
	PGBranchStream.open(s_PGBranchFileName);
	MPBranchStream.open(s_MPBranchFileName);

	//--------------------------------------------------------------------------//
	//Adding all the books in the CheckedOut file to the static patronList object.
	//--------------------------------------------------------------------------//
	while (getline(checkedOutStream, line))
	{
		std::stringstream ss(line);
		getline(ss, word, ',');
		title = word;
		getline(ss, word, ',');
		author = word;
		getline(ss, word, ',');
		patronName = word;
		s_checkedOutObject.checkOut(title, author, patronName);
	}

	//--------------------------------------------------------------------//
	//Adding all the books in CentralBranch file to a static library object.
	//--------------------------------------------------------------------//
	while (getline(CentralBranchStream, line))
	{
		std::stringstream ss(line);
		getline(ss, word, ',');
		title = word;
		getline(ss, word, ',');
		author = word;
		s_CentralBranchObject.addBook(title, author);
	}

	//---------------------------------------------------------------------//
	//Adding all the books in PointGreyBranch file to a static library object.
	//---------------------------------------------------------------------//
	while (getline(PGBranchStream, line))
	{
		std::stringstream ss(line);
		getline(ss, word, ',');
		title = word;
		getline(ss, word, ',');
		author = word;
		s_PGBranchObject.addBook(title, author);
	}

	//--------------------------------------------------------------------------//
	//Adding all the books in MountPleasantBranch file to a static library object.
	//--------------------------------------------------------------------------//
	while (getline(MPBranchStream, line))
	{
		std::stringstream ss(line);
		getline(ss, word, ',');
		title = word;
		getline(ss, word, ',');
		author = word;
		s_MPBranchObject.addBook(title, author);
	}

	checkedOutStream.close();
	CentralBranchStream.close();
	PGBranchStream.close();
	MPBranchStream.close();
}

/*
Used in a separate thread from the main function, writes back the dynamic instances of every book back into
their respective files. The objects are then reinitialized. This action cleans up the deleted books by not
writing back books with true delete flags. If the solution were scaled up to the size of an actual library
and was being accessed by multiple computers, this writeback would be important.
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
	std::string select, entered, title, author, patron;
	char selectChar;

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
		std::cout << "\n\n\n";
		std::cout << "          ***Spreadsheets have been updated***" << std::endl;
		std::cout << "\n\n\n";
		s_spreadsheetWritebackFlag = false;
	}
	if (s_CentralBranchBooksShelvedFlag)
	{
		std::cout << "\n\n\n";
		std::cout << "          ***Returned books have been shelved at the central branch***" << std::endl;
		std::cout << "\n\n\n";
		s_CentralBranchBooksShelvedFlag = false;
	}
	if (s_PGBranchBooksShelvedFlag)
	{
		std::cout << "\n\n\n";
		std::cout << "          ***Returned books have been shelved at the Point Grey branch***" << std::endl;
		std::cout << "\n\n\n";
		s_PGBranchBooksShelvedFlag = false;
	}
	if (s_MPBranchBooksShelvedFlag)
	{
		std::cout << "\n\n\n";
		std::cout << "          ***Returned books have been shelved at the Mount Pleasant branch***" << std::endl;
		std::cout << "\n\n\n";
		s_MPBranchBooksShelvedFlag = false;
	}
	std::cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	std::cout << "          Check out book:                                            (enter 1)" << std::endl;
	std::cout << "          view catalogue:                                            (enter 2)" << std::endl;
	std::cout << "          Return book:                                               (enter 3)" << std::endl;
	std::cout << "          Quit the application:                                      (enter 4)" << std::endl;
	std::cout << "          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

	//-----------------------------------------------------------------------------------------------//
	//start_input:
	//recieves input from the user then sends the to either check out a book, view the catalogue,
	//or return a book.
	//-----------------------------------------------------------------------------------------------//
start_input:
	getline(std::cin, select, '\n');
	if (select.length() != 1)
	{
		std::cout << "          Invalid entry, try again:" << std::endl;
		goto start_input;
	}
	selectChar = select[0]; //Because a switch statement can't use strings.

	switch (selectChar)
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
		std::cout << "          invalid entry, try again:" << std::endl;
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
	std::cout << "          Enter the title, author, then the patron checking the book out." << std::endl;
	std::cout << "title: ";
	getline(std::cin, entered, '\n');
	title = entered;
	std::cout << "author: ";
	getline(std::cin, entered, '\n');
	author = entered;
	std::cout << "patron: ";
	getline(std::cin, entered, '\n');
	patron = entered;
	std::cout << std::endl;

	s_mu.lock();
	if (s_CentralBranchObject.deleteBook(title, author))
	{
		s_checkedOutObject.checkOut(title, author, patron);
		std::cout << title << " by " << author << " checked out from the Central branch by " << patron << "." << std::endl;
	}
	else if (s_PGBranchObject.deleteBook(title, author))
	{
		s_checkedOutObject.checkOut(title, author, patron);
		std::cout << title << " by " << author << " checked out from the Point Grey branch by " << patron << "." << std::endl;
	}
	else if (s_MPBranchObject.deleteBook(title, author))
	{
		s_checkedOutObject.checkOut(title, author, patron);
		std::cout << title << " by " << author << " checked out from the Mount Pleasant branch by " << patron << "." << std::endl;
	}
	else
	{
		std::cout << "          The book entered is not available at any of our libraries";
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
	std::cout << "          Enter whether the catalogue should be in alphabetical order of 'title' or 'author':" << std::endl;
view_catalogue_entry:
	getline(std::cin, entered, '\n');
	if ((entered != "title") && (entered != "author"))
	{
		std::cout << "          Invalid entry, please enter 'title' or 'author':" << std::endl;
		goto view_catalogue_entry;
	}
	s_mu.lock();
	std::cout << "\n\n";
	std::cout << "The books at the Central branch are:" << std::endl;
	s_CentralBranchObject.printList(entered);
	std::cout << "\n\n";
	std::cout << "The books at the Point Grey branch are:" << std::endl;
	s_PGBranchObject.printList(entered);
	std::cout << "\n\n";
	std::cout << "The books at the Mount Pleasant branch are:" << std::endl;
	s_PGBranchObject.printList(entered);
	s_mu.unlock();
	goto start; //Return to start.



	//------------------------------------------------------------------//
	//return_book:
	//prints the patrons and what books they have checked out,
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
	std::cout << "          The following patrons have the following books checked out." << std::endl;
	s_checkedOutObject.printList("title");
	std::cout << "          Enter the title, author, and library being returned to." << std::endl;
return_book_entry:
	std::cout << "title: ";
	getline(std::cin, entered, '\n');
	title = entered;
	std::cout << "author: ";
	getline(std::cin, entered, '\n');
	author = entered;

	s_mu.lock();
	bool temp = !s_checkedOutObject.checkIn(title, author);
	s_mu.unlock();

	if (temp)
	{
		std::cout << "          Invalid book entry, please try again:" << std::endl;
		goto return_book_entry;
	}

book_entry_library:
	std::cout << "enter 'Central', 'Point Grey', or 'Mount Pleasant' to select the library:";
	getline(std::cin, entered, '\n');
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
	std::cout << title << " by " << author << " returned to " << entered << " branch." << std::endl;
	goto start;//return to start.
}