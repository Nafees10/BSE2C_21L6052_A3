#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#define SIZE_STEP 16

/// used in calculating ideal size
#define SIZE_MULTIPLIER 1.5

/// max number of iterations before it gives up on
/// finding the best grid
#define MAX_ITERATIONS 100000

#define EMPTY ' '

/// shuffle array
void shuffle(int *array, int n){
	for (int i = 0; i < n; i ++){
		for (int j = 1; j < n; j ++){
			if (rand() % 2){
				const int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
	}
}

/// Length of string
int length(const char *str){
	int l = 0;
	while (str[l])
		l ++;
	return l;
}

/// Makes a new copy of string
/// Returns: new string
char *stringCopy(const char* str){
	const int len = length(str);
	char *ret = new char[len + 1];
	for (int i = 0; i < len; i ++)
		ret[i] = str[i];
	ret[len] = 0;
	return ret;
}

/// Reverses a string, into a new one
/// Returns: new string
char *stringReverseNew(const char *str){
	const int lastIndex = length(str) - 1;
	char *ret = new char[lastIndex + 2];
	int i = 0;
	while (i <= lastIndex){
		ret[i] = str[lastIndex - i];
		i ++;
	}
	ret[lastIndex + 1] = 0;
	return ret;
}

/// sanitizes word (makes it uppercase, removes non alphabets)
void sanitize(char* str){
	if (str == nullptr)
		return;
	int len = length(str);
	// first make it all uppercase
	for (int i = 0; i < len; i ++){
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] -= 32;
	}
	// now remove non alphabets
	int i = 0, shift = 0;
	while (i + shift < len){
		if (str[i + shift] >= 'A' && str[i + shift] <= 'Z'){
			str[i] = str[i + shift];
			i ++;
		}else{
			shift ++;
		}
	}
	str[i] = 0;
}

char getRandomAlphabet(){
	return (rand() % 26) + 'A';
}

/// a linked list based stack  
/// adapted from:
/// https://github.com/Nafees10/utils/blob/master/source/utils/ds.d#L411
class StackInt{
	struct StackIntItem{
		int item;
		StackIntItem *ptr;
	};
	StackIntItem *_ptr;
	int _count;
public:
	/// constructor
	StackInt(){
		_ptr = nullptr;
		_count = 0;
	}
	/// copy constructor
	StackInt(const StackInt& from){
		_count = from._count;
		if (!from._ptr)
			return;
		// ptr. ptr everywhere
		_ptr = new StackIntItem();
		_ptr->item = from._ptr->item;
		_ptr->ptr = from._ptr->ptr;
		StackIntItem *ptr = _ptr->ptr, *prevPtr = _ptr;
		while (ptr != nullptr){
			prevPtr->ptr = new StackIntItem();
			prevPtr->ptr->item = ptr->item;
			prevPtr->ptr->ptr = ptr->ptr;
			ptr = prevPtr->ptr->ptr;
			prevPtr = prevPtr->ptr;
		}
	}
	~StackInt(){
		while (_ptr){
			StackIntItem *nextPtr = _ptr->ptr;
			delete _ptr;
			_ptr = nextPtr;
		}
	}
	/// if stack is empty
	bool isEmpty(){
		return _ptr == NULL; // or could've done _count == 0
	}
	/// pushes data to stack
	void push(int data){
		StackIntItem *newItem = new StackIntItem;
		newItem->item = data;
		newItem->ptr = _ptr;
		_ptr = newItem;
		_count ++;
	}
	/// Returns: whats at top of stack
	int peek(){
		if (_ptr)
			return _ptr->item;
		return -1;
	}
	/// Returns: whats at tope of stack and pops it
	int pop(){
		if (!_ptr)
			return -1;
		int ret = _ptr->item;
		StackIntItem *prevPtr = _ptr;
		_ptr = _ptr->ptr;
		_count --;
		delete prevPtr;
		return ret;
	}
	/// Returns: number of items on stack
	int count(){
		return _count;
	}
};

/// for storing words, in descending order of length
class WordList{
private:
	char **_words;
	int _capacity;
	int _count;
	/// increases capacity of array
	void _increaseSize(){
		char **newList = new char*[_capacity + SIZE_STEP];
		for (int i = 0; i < _capacity; i ++)
			newList[i] = _words[i];
		for (int i = 0; i < SIZE_STEP; i ++)
			newList[_capacity + i] = nullptr;
		_capacity += SIZE_STEP;
		delete[] _words;
		_words = newList;
	}
public:
	/// constructor
	WordList(){
		_words = nullptr;
		_count = 0;
		_capacity = 0;
	}
	/// constructor, read words from newline-separated file
	WordList(const char *filename){
		_words = nullptr;
		_count = 0;
		_capacity = 0;
		fromFile(filename);
	}
	/// copy constructor
	WordList(const WordList& from){
		_capacity = from._count;
		_count = from._count;
		_words = new char*[_count];
		for (int i = 0; i < _count; i ++)
			_words[i] = stringCopy(from._words[i]);
	}
	~WordList(){
		for (int i = 0; i < _count; i ++)
			delete[] _words[i];
		delete[] _words;
	}
	/// Returns: number of words
	int count(){
		return _count;
	}
	/// Returns: word at index
	char *get(int index){
		if (index < 0 || index >= _count)
			return nullptr;
		return _words[index];
	}
	/// Returns: word at index
	char *operator[](int index){
		return get(index);
	}
	/// Adds a new word, while keeping the order
	void add(char *word){
		if (word == nullptr || length(word) == 0)
			return;
		int len = length(word);
		// find insertion index
		int index = 0;
		while (index < _count && length(_words[index]) >= len)
			index ++;
		if (_count == _capacity)
			_increaseSize();
		if (index == _count){
			_words[_count ++] = word;
			return;
		}
		// shift
		for (int i = _count; i > index; i --)
			_words[i] = _words[i - 1];
		// place
		_words[index] = word;
		_count ++;
	}
	/// add words from newline-separated file
	void fromFile(const char *filename){
		std::ifstream file(filename);
		if (!file){
			std::cerr << "Failed to open file\nbyebye\n";
			exit(1);
			return;
		}
		char buffer[100];
		while (!file.eof()){
			file.getline(buffer, 100);
			sanitize(buffer);
			add(stringCopy(buffer));
		}
		file.close();
	}
};

/// Wordsearch grid (square)
class Grid{
private:
	char *_grid; // grid
	int _gridSize; // size of grid (sqrt this to get _len)
	char _dummy; // dummy
	int _len; // length (grid is a square)
	int _charCount; /// count of characters in the grid
	bool _locked; /// if this can be modified
public:
	/// Constructor
	Grid(){
		_grid = nullptr;
		_gridSize = 0;
		_len = 0;
		_charCount = -1;
		_locked = false;
	}
	/// copy constructor
	Grid(const Grid& from){
		_len = from._len;
		if (_len){
			_gridSize = _len * _len;
			_grid = new char[_gridSize];
			for (int addr = 0; addr < _gridSize; addr ++)
				_grid[addr] = from._grid[addr];
		}else{
			_grid = nullptr;
			_gridSize = 0;
		}
		_charCount = from._charCount;
		_locked = from._locked;
	}
	/// Constructor. creates empty grid of length
	Grid(int length){
		_len = length;
		_gridSize = _len * _len;
		_grid = new char[_gridSize];
		_charCount = 0;
		_locked = false;
		for (int addr = 0; addr < _gridSize; addr ++)
			_grid[addr] = EMPTY;
	}
	~Grid(){
		if (_grid != nullptr)
			delete[] _grid;
	}
	/// fills empty cells with random alphabets.
	/// Locks the grid from further changes
	void finalize(){
		charCount();
		for (int addr = 0; addr < _gridSize; addr ++)
			if (_grid[addr] == EMPTY)
				_grid[addr] = getRandomAlphabet();
	}
	int linAddr(int x, int y){
		return x + (y * _len);
	}
	void linAddr(int linAddr, int &x, int &y){
		y = linAddr / _len;
		x = linAddr % _len;
	}
	/// Size of grid (length square)
	int size(){
		return _gridSize;
	}
	/// Length of grid
	int length(){
		return _len;
	}
	/// Number of characters in grid
	int charCount(){
		// if already calculated, return that
		if (_charCount >= 0 || _locked)
			return _charCount;
		_charCount = 0;
		for (int addr = 0; addr < _gridSize; addr ++)
			_charCount += _grid[addr] != EMPTY;
		return _charCount;
	}
	/// Returns: cell
	char &cell(int addr){
		if (addr < 0 || addr >= _gridSize){
			std::cerr << "out of bound access detected " << addr << '\n';
			return _dummy;
		}
		if (_locked){
			_dummy = _grid[addr];
			return _dummy;
		}
		_charCount = -1;
		return _grid[addr];
	}
	/// Returns: cell
	char &cell(int x, int y){
		return cell(linAddr(x, y));
	}
	/// undoes last action from a history stack
	/// Returns: false if history seems wrong. true if undid
	bool undo(StackInt *history){
		if (history->count() - 1 < history->peek())
			return false;
		int count = history->pop();
		while (count --)
			cell(history->pop()) = EMPTY;
		return true;
	}
	/// if a cell is empty
	bool isEmpty(int addr){
		return addr >= 0 && addr < _gridSize && _grid[addr] == EMPTY;
	}
	/// if a cell is empty
	bool isEmpty(int x, int y){
		return isEmpty(linAddr(x, y));
	}
	/// prints to stdout
	void print(){
		// print col numbers
		std::cout << "  ";
		for (int x = 0; x < _len; x ++){
			if (x < 10)
				std::cout << ' ';
			std::cout << x;
		}
		for (int addr = 0; addr < _gridSize; addr ++){
			if (addr % _len == 0){
				std::cout << '\n';
				if (addr / _len < 10)
					std::cout << ' ';
				std::cout << addr / _len << ' ';
			}
			std::cout << _grid[addr] << ' ';
		}
		std::cout << '\n';
	}
	/// writes to a file
	/// Returns: true if done, false if errored
	bool toFile(const char *filename){
		std::ofstream file(filename);
		if (!file){
			std::cerr << "Failed to open file " << filename << '\n';
			return false;
		}
		for (int addr = 0; addr < _gridSize; addr ++){
			if (addr && addr % _len == 0)
				file << '\n';
			file << _grid[addr];
		}
		file.close();
		return true;
	}
};

/// a function that tries to place a word, on a grid, at coordinates
/// and returns whether it was placed or not
/// 
/// the function receives:
/// * grid
/// * history stack (should push each address it alters on grid), and at end
///		should push the number of alterations done
/// * CString of the word to place
/// * address to place at
typedef bool (*WordPlacerFunc)(Grid*, StackInt*, char*, char*, int);

// Grid generator
class GridGen{
private:
	/// list of words to insert
	WordList *_words;
	/// sum of number of characters in all words
	int _charCount;
	/// length of the grid that will be generated next
	int _gridLen;

	/// history of alterations to grid
	StackInt *_history;
	/// current grid that is being worked on
	Grid *_grid;
	/// amount of words that have been placed by each placer
	int *_placerWCount;
	/// best grid so far
	Grid *_bestGrid;
	/// best grid's score. 0 is best. higher is bad
	int _bestGridScore;

	/// array of grid placers
	WordPlacerFunc *_placers;
	/// number of placers
	int _placersCount;

	/// order in which placers are read
	int *_placersOrder;
	/// order in which addresses are read
	int *_addrOrder;

	/// max iteratons
	int _maxIterations;
	/// current iterations
	int _iterations;
	/// number of grids that were generated
	int _bestGridCandidates;

	/// Randomizes placers and addresses orders.
	/// _grid must exist before this
	void _randomize(){
		srand(time(nullptr));
		if (_placersOrder)
			delete[] _placersOrder;
		if (_addrOrder)
			delete[] _addrOrder;
		_placersOrder = new int[_placersCount];
		for (int i = 0; i < _placersCount; i ++)
			_placersOrder[i] = i;
		
		const int gridSize = _grid->size();
		_addrOrder = new int[gridSize];
		for (int i = 0; i < _grid->size(); i ++)
			_addrOrder[i] = i;

		shuffle(_placersOrder, _placersCount);
		shuffle(_addrOrder, gridSize);
	}

	int _getScore(){
		const int totalWords = _words->count();
		const float mean = totalWords / (float)_placersCount;
		float diffSum = 0;
		for (int i = 0; i < _placersCount; i ++)
			diffSum += abs(mean - _placerWCount[i]);
		// diffSum closer to 0 = best. farther from 0 is worst
		int score = diffSum * 1000;
		score -= _charCount - _grid->charCount();
		return score;
	}

	/// generates
	bool _generate(int wordInd){
		char *word = _words->get(wordInd);
		char *wordRev = stringReverseNew(word);
		const bool isLastWord = wordInd + 1 == _words->count();
		const int addrEnd = _grid->size();
		for (int addrI = 0; addrI < addrEnd; addrI ++){
			const int addr = _addrOrder[addrI];
			for (int placerI = 0; placerI < _placersCount; placerI ++){
				const int placer = _placersOrder[placerI];
				// try the placer on every cell
				if (_placers[placer](_grid, _history, word, wordRev, addr)){
					_placerWCount[placer] ++;
					_bestGridCandidates ++;
					if (isLastWord){
						const int thisScore = _getScore();
						if (thisScore < _bestGridScore || !_bestGrid){
							if (_bestGrid)
								delete _bestGrid;
							_bestGrid = new Grid(*_grid);
							_bestGridScore = thisScore;
						}
					}else{
						// try all placers on next word
						_generate(wordInd + 1);
					}
					// undo
					_placerWCount[placer] --;
					_grid->undo(_history);
				}
				// if this placer didnt do anything, try next one
			}
			if (++_iterations > _maxIterations && _bestGrid){
				delete[] wordRev;
				return true;
			}
		}
		delete[] wordRev;
		return _bestGrid != nullptr;
	}
public:
	GridGen(WordList *words, int maxIter = MAX_ITERATIONS){
		if (words == nullptr || words->count() == 0){
			std::cerr << "you passed bad words.\n"
				<< "prepare for some s e g f a u l t s\n";
		}
		_maxIterations = maxIter;
		_words = words;
		_history = nullptr;
		_grid = nullptr;
		_gridLen = -1;
		_placers = nullptr;
		_placerWCount = nullptr;
		_placersCount = 0;
		_bestGrid = nullptr;
		_placersOrder = nullptr;
		_addrOrder = nullptr;
		gridLen();
	}
	~GridGen(){
		if (_placers != nullptr)
			delete[] _placers;
		if (_bestGrid != nullptr)
			delete _bestGrid;
		if (_placersOrder != nullptr)
			delete _placersOrder;
		if (_addrOrder != nullptr)
			delete _addrOrder;
	}
	/// attempts to generate the best possible grid
	/// 
	/// Returns: true if a grid was generated
	bool generate(){
		_grid = new Grid(_gridLen);
		_history = new StackInt;
		_placerWCount = new int[_placersCount];
		for (int i = 0; i < _placersCount; i ++)
			_placerWCount[i] = 0;

		if (_bestGrid)
			delete _bestGrid;
		_bestGrid = nullptr;
		_bestGridScore = 0;
		_iterations = 0;
		_bestGridCandidates = 0;

		_randomize();

		bool found = _generate(0);

		delete _grid;
		delete _history;
		delete[] _placerWCount;
		_grid = nullptr;
		_history = nullptr;
		_placerWCount = nullptr;

		if (!found)
			_gridLen = _gridLen * SIZE_MULTIPLIER;
		return found;
	}
	/// Returns: best grid or nullptr
	Grid *bestGrid(){
		return _bestGrid;
	}
	/// Returns: score of best grid. zero is perfect, higher score is worse
	int bestGridScore(){
		return _bestGridScore;
	}
	/// Returns: number of grids that were generated
	int bestGridCandidates(){
		return _bestGridCandidates;
	}
	/// adds a new grid placer
	void addPlacer(WordPlacerFunc func){
		WordPlacerFunc *newArr = new WordPlacerFunc[_placersCount + 1];
		for (int i = 0; i < _placersCount; i ++)
			newArr[i] = _placers[i];
		newArr[_placersCount ++] = func;
		delete[] _placers;
		_placers = newArr;
	}
	/// Returns: ideal length for a grid to store words
	int gridLen(){
		if (_gridLen > 0)
			return _gridLen;
		if (!_words || _words->count() == 0)
			return -1;
		// count the number of chars needed in square
		_charCount = 0;
		for (int i = 0; i < _words->count(); i++)
			_charCount += length(_words->get(i));
		// now need a square big enough to hold all these.
		float idealLen = sqrt(_charCount);
		if (idealLen < length(_words->get(0)))
			_gridLen =  length(_words->get(0)) * SIZE_MULTIPLIER;
		else
			_gridLen = idealLen * SIZE_MULTIPLIER;
		return _gridLen;
	}
};

bool placerHorizontalL2R(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	int len = length(word);
	bool ret = true;
	int x, y;
	grid->linAddr(addr, x, y);
	if (x + len >= grid->length())
		return false;
	for (int i = 0; ret && i < len && x < grid->length(); x ++, i ++)
		ret = grid->isEmpty(x, y) || grid->cell(x, y) == word[i];
	if (!ret)
		return false;
	// place
	grid->linAddr(addr, x, y);
	int alterCount = 0;
	for (int i = 0; i < len && x < grid->length(); x ++, i ++){
		if (!grid->isEmpty(x, y))
			continue;
		grid->cell(x, y) = word[i];
		history->push(grid->linAddr(x, y));
		alterCount ++;
	}
	history->push(alterCount);
	return true;
}

bool placeHorizontalR2L(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	return  placerHorizontalL2R(grid, history, rev, rev, addr);
}

bool placerVerticalU2D(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	int len = length(word);
	bool ret = true;
	int x, y;
	grid->linAddr(addr, x, y);
	if (y + len >= grid->length())
		return false;
	for (int i = 0; ret && i < len && y < grid->length(); y ++, i ++)
		ret = grid->isEmpty(x, y) || grid->cell(x, y) == word[i];
	if (!ret)
		return false;
	// place
	grid->linAddr(addr, x, y);
	int alterCount = 0;
	for (int i = 0; i < len && y < grid->length(); y ++, i ++){
		if (!grid->isEmpty(x, y))
			continue;
		grid->cell(x, y) = word[i];
		history->push(grid->linAddr(x, y));
		alterCount ++;
	}
	history->push(alterCount);
	return true;
}

bool placerVerticalD2U(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	return placerVerticalU2D(grid, history, rev, rev, addr);
}

bool placerDiagonalUL2DR(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	int len = length(word);
	bool ret = true;
	int x, y;
	grid->linAddr(addr, x, y);
	if (y + len >= grid->length() || x + len >= grid->length())
		return false;
	for (int i = 0; ret && i < len && x < grid->length() && y < grid->length(); x ++, y ++, i ++)
		ret = grid->isEmpty(x, y) || grid->cell(x, y) == word[i];
	if (!ret)
		return false;
	// place
	grid->linAddr(addr, x, y);
	int alterCount = 0;
	for (int i = 0; i < len && x < grid->length() && y < grid->length(); x ++, y ++, i ++){
		if (!grid->isEmpty(x, y))
			continue;
		grid->cell(x, y) = word[i];
		history->push(grid->linAddr(x, y));
		alterCount ++;
	}
	history->push(alterCount);
	return true;
}

bool placerDiagonalDR2UL(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	return placerDiagonalUL2DR(grid, history, rev, rev, addr);
}

bool placerDiagonalUR2DL(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	int len = length(word);
	bool ret = true;
	int x, y;
	grid->linAddr(addr, x, y);
	if (y + len >= grid->length() || x < len + 1)
		return false;
	for (int i = 0; ret && i < len && x > 0 && y < grid->length(); x --, y ++, i ++)
		ret = grid->isEmpty(x, y) || grid->cell(x, y) == word[i];
	if (!ret)
		return false;
	// place
	grid->linAddr(addr, x, y);
	int alterCount = 0;
	for (int i = 0; i < len && x > 0 && y < grid->length(); x --, y ++, i ++){
		if (!grid->isEmpty(x, y))
			continue;
		grid->cell(x, y) = word[i];
		history->push(grid->linAddr(x, y));
		alterCount ++;
	}
	history->push(alterCount);
	return true;
}

bool placerDiagonalDL2UR(Grid *grid, StackInt *history, char *word, char *rev, int addr){
	return placerDiagonalUR2DL(grid, history, rev, rev, addr);
}

int main(int argc, char **argv){
	const char *filename = "input.txt", *outFilename = "output.txt";
	if (argc >= 1)
		filename = argv[1];
	if (argc >= 2)
		outFilename = argv[2];
	srand(time(nullptr));
	WordList *words = new WordList(filename);
	GridGen generator(words);
	generator.addPlacer(placerHorizontalL2R);
	generator.addPlacer(placeHorizontalR2L);
	generator.addPlacer(placerVerticalU2D);
	generator.addPlacer(placerVerticalD2U);
	generator.addPlacer(placerDiagonalUL2DR);
	generator.addPlacer(placerDiagonalDR2UL);
	generator.addPlacer(placerDiagonalUR2DL);
	generator.addPlacer(placerDiagonalDL2UR);
	if (!generator.generate()){
		std::cerr << "Failed to generate grid. Adjust SIZE_MULTIPLIER\n";
		delete words;
		exit(1);
	}

	Grid *grid = generator.bestGrid();
	if (grid != nullptr){
		std::cout << generator.bestGridCandidates() << " grids were generated\n";
		std::cout << "best one had a score of " << generator.bestGridScore() << "\n";
		grid->print();
		std::cout << "final grid:\n";
		grid->finalize();
		grid->print();
		if (!grid->toFile(outFilename))
			exit(1);
	}
	delete words;
	return 0;
}
