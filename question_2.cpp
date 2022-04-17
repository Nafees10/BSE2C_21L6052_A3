#include <iostream>
#include <fstream>

#define SIZE_STEP 16

/// Length of string
int length(const char *str){
	int l = 0;
	while (str[l])
		l ++;
	return l;
}

/// to store r1, c1, r2, c2
struct WordPos{
	int r1, c1, r2, c2;
	WordPos(int r = -1, int c = -1, int rr = -1, int cc = -1){
		r1 = r;
		c1 = c;
		r2 = rr;
		c2 = cc;
	}
	bool isValid(){
		return r1 >= 0 && c1 >= 0 && r2 >= 0 && c2 >= 0;
	}
};

std::ostream& operator<<(std::ostream& stream, const WordPos pos){
	stream << '{' << pos.r1 << ',' << pos.c1 << "},{" <<
		pos.r2 << ',' << pos.c2 << "}";
	return stream;
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

/// changes character to uppercase
/// 
/// Returns: false if not an alphabet
bool toUpper(char &c){
	if (c >= 'a' && c <= 'z')
		c -= 32;
	return c >= 'A' && c <= 'Z';
}

class WordSearchGrid;

/// prototype of function that is used to check if a word exists at an address
/// the arguments passed are:
/// * the grid
/// * word
/// * cell address
typedef WordPos (*FinderFunc)(WordSearchGrid*, char*, int);

class WordSearchGrid{
private:
	/// the grid. 2D array mapped onto a 1D
	char *_grid;
	/// rows in grid
	int _rows;
	/// columns in grid
	int _cols;
	/// number of characters in grid (area)
	int _area;

	/// dummy
	char _dummy;

	/// finder functions
	FinderFunc *_finders;
	/// number of finder functions
	int _findersCount;
public:
	WordSearchGrid(){
		_grid = nullptr;
		_rows = _cols = _area = 0;

		_finders = nullptr;
		_findersCount = 0;
	}
	WordSearchGrid(const char *filename){
		_grid = nullptr;
		_rows = _cols = _area = 0;

		_finders = nullptr;
		_findersCount = 0;
		fromFile(filename);
	}
	~WordSearchGrid(){
		if (_grid)
			delete[] _grid;
		if (_finders)
			delete[] _finders;
	}
	/// rows
	int rows(){
		return _rows;
	}
	/// columns
	int cols(){
		return _cols;
	}
	/// area
	int area(){
		return _area;
	}
	/// linear address
	int linAddr(int r, int c){
		return r * _cols + c;
	}
	/// linear address to r, c
	void linAddr(int lin, int &r, int &c){
		r = lin / _cols;
		c = lin % _cols;
	}
	/// cell at address
	char &cell(int addr){
		if (addr < 0 || addr > _area){
			_dummy = 0;
			return _dummy;
		}
		return _grid[addr];
	}
	/// cell at r, c
	char &cell(int r, int c){
		return cell(linAddr(r, c));
	}
	/// Adds a finder function
	void addFinder(FinderFunc func){
		FinderFunc *newArr = new FinderFunc[_findersCount + 1];
		for (int i = 0; i < _findersCount; i ++)
			newArr[i] = _finders[i];
		newArr[_findersCount ++] = func;
		delete[] _finders;
		_finders = newArr;
	}
	/// tries finding a word.
	WordPos find(char *word){
		for (int addr = 0; addr < _area; addr ++){
			for (int finder = 0; finder < _findersCount; finder ++){
				WordPos pos = _finders[finder](this, word, addr);
				if (pos.isValid())
					return pos;
			}
		}
		return WordPos();
	}
	/// loads grid from file
	void fromFile(const char *filename){
		std::ifstream file(filename);
		if (!file){
			std::cerr << "Failed to open file " << filename << "\n";
			exit(1);
		}
		char *buffer = new char[SIZE_STEP];
		int bufferCap = SIZE_STEP;
		int bufferLen = 0;

		int lineLength = -1, currentLineLength = 0, lineCount = 0;
		while (!file.eof()){
			char c;
			file.get(c);
			if (c == '\r' || c == '\n' || file.eof()){
				if (currentLineLength == 0)
					continue; // ignore empty lines
				if (lineLength == -1)
					lineLength = currentLineLength;
				if (lineLength != currentLineLength){
					std::cerr << "Varying length lines found. File is invalid.\n";
					file.close();
					delete[] buffer;
					exit(1);
				}
				currentLineLength = 0;
				continue;
			}
			if (!toUpper(c)){
				// bad character
				std::cerr << "Non alphabet character found. File is invalid.\n";
				delete[] buffer;
				exit(1);
			}
			if (++ currentLineLength == 1)
				lineCount ++;
			// append to buffer
			if (bufferLen == bufferCap){
				// grow
				char *newBuf = new char[bufferCap + SIZE_STEP];
				for (int i = 0; i < bufferCap; i ++)
					newBuf[i] = buffer[i];
				delete[] buffer;
				buffer = newBuf;
				bufferCap += SIZE_STEP;
			}
			buffer[bufferLen ++] = c;
		}
		file.close();
		_cols = lineLength;
		_rows = lineCount;
		_area = bufferLen;
		if (_cols * _rows != _area){
			std::cerr << "Varying length lines found. File is invalid.\n";
			delete[] buffer;
			exit(1);
		}
		if (_area == bufferCap){
			_grid = buffer;
		}else{
			_grid = new char[_area];
			for (int i = 0; i < _area; i ++)
				_grid[i] = buffer[i];
			delete[] buffer;
		}
	}
	/// prints the grid
	void print(){
		for (int r = 0; r < _rows; r ++){
			const int rowOffset = r * _cols;
			for (int c = 0; c < _cols; c ++)
				std::cout << _grid[rowOffset + c] << ' ';
			std::cout << '\n';
		}
		std::cout << '\n';
	}
};

WordPos finderHorizontalL2R(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int cStart = c;
	int count = 0;
	while (count < len && c < grid->cols()){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, c ++;
	}
	if (count < len)
		return WordPos();
	return WordPos(r, cStart, r, c - 1);
}

WordPos finderHorizontalR2L(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int cStart = c;
	int count = 0;
	while (count < len && c >= 0){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, c --;
	}
	if (count < len)
		return WordPos();
	return WordPos(r, cStart, r, c + 1);
}

WordPos finderVerticalU2D(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r;
	int count = 0;
	while (count < len && r < grid->rows()){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r ++;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, c, r - 1, c);
}

WordPos finderVerticalD2U(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r;
	int count = 0;
	while (count < len && r >= 0){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r --;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, c, r + 1, c);
}

WordPos finderDiagonalUL2DR(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r, cStart = c;
	int count = 0;
	while (count < len && r < grid->rows() && c < grid->cols()){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r ++, c ++;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, cStart, r - 1, c - 1);
}

WordPos finderDiagonalDR2UL(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r, cStart = c;
	int count = 0;
	while (count < len && r >= 0 && c >= 0){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r --, c --;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, cStart, r + 1, c + 1);
}

WordPos finderDiagonalUR2DL(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r, cStart = c;
	int count = 0;
	while (count < len && r < grid->rows() && c >= 0){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r ++, c --;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, cStart, r - 1, c + 1);
}

WordPos finderDiagonalDL2UR(WordSearchGrid *grid, char *word, int addr){
	const int len = length(word);
	int r, c;
	grid->linAddr(addr, r, c);
	const int rStart = r, cStart = c;
	int count = 0;
	while (count < len && r >= 0 && c < grid->cols()){
		if (grid->cell(r, c) != word[count])
			break;
		count ++, r --, c ++;
	}
	if (count < len)
		return WordPos();
	return WordPos(rStart, cStart, r + 1, c - 1);
}

int main(int argc, char **argv){
	const char *filename = "input.txt", *outFilename = "output.txt";
	if (argc >= 1)
		filename = argv[1];
	if (argc >= 2)
		outFilename = argv[2];
	std::ofstream file(outFilename);
	if (!file){
		std::cout << "Failed to open output file " << outFilename << "\n";
		exit(1);
	}
	// get row/cols, and ignore them
	int n;
	std::cout << "enter 2 values for size, (that I will ignore anyways): ";
	std::cin >> n >> n;

	WordSearchGrid grid(filename);
	grid.addFinder(finderHorizontalL2R);
	grid.addFinder(finderHorizontalR2L);
	grid.addFinder(finderVerticalU2D);
	grid.addFinder(finderVerticalD2U);
	grid.addFinder(finderDiagonalUL2DR);
	grid.addFinder(finderDiagonalDR2UL);
	grid.addFinder(finderDiagonalUR2DL);
	grid.addFinder(finderDiagonalDL2UR);
	std::cout << "grid is:\n";
	grid.print();

	std::cout << "enter n: ";
	std::cin >> n;
	std::cin.ignore(100,'\n');

	int bufferSize = grid.cols() > grid.rows() ? grid.cols() : grid.rows();
	char *buffer = new char[bufferSize];
	while (n --){
		std::cout << "Enter word: ";
		std::cin.getline(buffer, bufferSize);
		sanitize(buffer);
		std::cout << "Looking for \"" << buffer << "\"\n";
		WordPos pos = grid.find(buffer);
		if (pos.isValid()){
			std::cout << pos << std::endl;
			file << pos << std::endl;
		}else{
			std::cout << "Not found\n";
			file << "Not found\n";
		}
	}

	delete[] buffer;
	return 0;
}