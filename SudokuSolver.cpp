#include "SudokuSolver.h"
#include <iostream>

///////////////////////////////////////////////////////////////////////////
//  Poss implementations
///////////////////////////////////////////////////////////////////////////

Poss::Poss(int x, int y, int starter) //if given a starter other than 0, creates a solved Possibility point
{									  //otherwise, all numbers are initially a possibility
		r = x; c = y;
		p[0] = starter;
		if (starter != 0)
			for (int k = 1; k < 10; k++)
				if (k != starter)
					p[k] = 0;
				else
					p[k] = k;
		else
			for (int k = 1; k < 10; k++)
				p[k] = k;
}

int Poss::nPoss() const //returns the number of possibilities for this point
{
	if (solved())
		return 1;

	int count = 0;
	for (int k = 1; k < 10; k++)
	{
		if (p[k] != 0)
			count++;
	}
	return count;
}

bool Poss::has(int val) const //checks if a number is one of the possibilities
{
	if (val < 1 || val > 9)
		return false;

	return p[val];
}

bool Poss::remove(int val) //removes a possibility from this point, solving if only one is left
{
	if (val < 1 || val > 9)
		return false;
	if (p[val] == 0 || p[0] != 0)
		return false;
	p[val] = 0;
	if (nPoss() == 1)
		for (int k = 1; k < 10; k++)
			if (p[k] != 0)
				p[0] = p[k];
	return true;
}

bool Poss::solve(int val) //directly solves for a specific value (external solution)
{
	if (solved() || !has(val))
		return false;

	for (int k = 1; k < 10; k++)
		if (k != val)
			p[k] = 0;
	p[0] = val;
	return true;
}

///////////////////////////////////////////////////////////////////////////
//  Puzzle implementations
///////////////////////////////////////////////////////////////////////////

//The possibility structure to be mentioned comes from the puzzle, where there is a 9x9 of spots to solve,
//and each has 9 possibilities, resulting in 9x9x9 possibilities.  This can be easily and compactly represented
//in 3 dimensions, and enables a more in-depth analysis by observing blocks of remaining possibilities and
//choosing spots that have the most influence on the puzzle (yet to be efectively implemented)

Puzzle::Puzzle(int arr[9][9]) //creates a 3-dimensional possibility structure to represent the puzzle
{
	save = nullptr;
	ar = new Poss**[9];
	for (int k = 0; k < 9; k++)
		ar[k] = new Poss*[9];

	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			ar[k][c] = new Poss(k, c, arr[k][c]);
}

Puzzle::Puzzle(const Puzzle& other) //creates a separate copy from another puzzle object
{
	save = nullptr;
	ar = new Poss**[9];
	for (int k = 0; k < 9; k++)
		ar[k] = new Poss*[9];

	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			ar[k][c] = new Poss(k, c, other.ar[k][c]->solution());
}

Puzzle& Puzzle::operator=(const Puzzle& other) //sets this puzzle equal to another, and takes on its save
{
	save = other.save;
	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			delete ar[k][c];

	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			ar[k][c] = new Poss(k, c, other.ar[k][c]->solution());
	return *this;
}

Puzzle::CorrectPoss::CorrectPoss() //a structure to help with checking possibilities in a row/column/block
{
	for (int k = 1; k < 10; k++)
		p[k] = k;
}

bool Puzzle::CorrectPoss::check(int val) //checks off a value or red flags a conflict
{
	if (val == 0)
		return true;
	if (val < 1 || val > 9 || p[val] == 0)
		return false;

	p[val] = 0;
	return true;
}

bool Puzzle::solutionUpdate(int r, int c) //updates all possibilities influenced by this spot
{
	bool success = false;
	for (int k = 0; k < 9; k++)
	{
		if (ar[r][k]->remove(ar[r][c]->solution()))
			success = true;
		if (ar[k][c]->remove(ar[r][c]->solution()))
			success = true;
	}

	for (int rb = (r / 3) * 3; rb < ((r / 3) * 3) + 3; rb++)
		for (int cb = (c / 3) * 3; cb < ((c / 3) * 3) + 3; cb++)
			if (ar[rb][cb]->remove(ar[r][c]->solution()))
				success = true;
	return success;
}

bool Puzzle::rowCorrect(int r) const //checks a row for correctness
{
	if (r < 0 || r > 8)
		return false;

	CorrectPoss cp;
	for (int k = 0; k < 9; k++)
		if (!cp.check(ar[r][k]->solution()) || ar[r][k]->nPoss() < 1)
			return false;
	return true;
}

bool Puzzle::colCorrect(int c) const //checks a column for correctness
{
	if (c < 0 || c > 8)
		return false;

	CorrectPoss cp;
	for (int k = 0; k < 9; k++)
		if (!cp.check(ar[k][c]->solution()) || ar[k][c]->nPoss() < 1)
			return false;
	return true;
}

bool Puzzle::blockCorrect(int r, int c) const //checks a block for correctness
{
	if (r < 0 || r > 8 || c < 0 || c > 8)
		return false;

	CorrectPoss cp;
	for (int rb = (r / 3) * 3; rb < ((r / 3) * 3) + 3; rb++)
		for (int cb = (c / 3) * 3; cb < ((c / 3) * 3) + 3; cb++)
			if (!cp.check(ar[rb][cb]->solution()) || ar[rb][cb]->nPoss() < 1)
				return false;
	return true;
}

bool Puzzle::puzCorrect() const //checks the puzzle for correctness
{
	for (int k = 0; k < 9; k++)
		if (!rowCorrect(k) || !colCorrect(k))
			return false;

	for (int k = 0; k < 9; k += 3)
		for (int c = 0; c < 9; c += 3)
			if (!blockCorrect(k, c))
				return false;

	return true;
}

bool Puzzle::comparePoss() //checks for limited possibilities (puts each section "in context"), by taking external possibilities into account
{
	bool success = false;

	for (int k = 0; k < 9; k++)
	{
		if (compRow(k))
			success = true;
		if (compCol(k))
			success = true;
	}

	for (int k = 0; k < 9; k += 3)
		for (int c = 0; c < 9; c += 3)
			if (compBlock(k, c))
				success = true;

	return success;
}

bool Puzzle::compRow(int r) //checks for limited possibilities in the context of a given row
{
	bool success = false;
	for (int k = 1; k < 10; k++)
	{
		int count = 0;
		for (int c = 0; c < 9; c++)
			if (ar[r][c]->solution() == k)
			{
				count = 0;
				break;
			}
			else if (ar[r][c]->has(k))
				count++;

		if (count == 1)
		{
			for (int c = 0; c < 9; c++)
				if (ar[r][c]->has(k))
					ar[r][c]->solve(k);
			success = true;
		}
	}
	return success;
}

bool Puzzle::compCol(int c) //checks for limited possibilities in the context of a given column
{
	bool success = false;
	for (int k = 1; k < 10; k++)
	{
		int count = 0;
		for (int j = 0; j < 9; j++)
			if (ar[j][c]->solution() == k)
			{
				count = 0;
				break;
			}
			else if (ar[j][c]->has(k))
				count++;

		if (count == 1)
		{
			for (int j = 0; j < 9; j++)
				if (ar[j][c]->has(k))
					ar[j][c]->solve(k);
			success = true;
		}
	}
	return success;
}

bool Puzzle::compBlock(int r, int c) //checks for limited possibilities in the context of a given block
{
	bool success = false;
	for (int k = 1; k < 10; k++)
	{
		int count = 0;
		for (int rb = (r / 3) * 3; rb < ((r / 3) * 3) + 3; rb++)
			for (int cb = (c / 3) * 3; cb < ((c / 3) * 3) + 3; cb++)
				if (ar[rb][cb]->solution() == k)
				{
					count = 0;
					break;
				}
				else if (ar[rb][cb]->has(k))
					count++;

		if (count == 1)
		{
			for (int rb = (r / 3) * 3; rb < ((r / 3) * 3) + 3; rb++)
				for (int cb = (c / 3) * 3; cb < ((c / 3) * 3) + 3; cb++)
					if (ar[r][c]->has(k))
						ar[r][c]->solve(k);
			success = true;
		}
	}
	return success;
}

bool Puzzle::forceSolve(int r, int c, int val) //forces a spot to solve to a specific value, while saving the puzzle
{
	Puzzle* temp = new Puzzle(*this);
	if (save == nullptr)
		save = temp;
	else
	{
		temp->save = save;
		save = temp;
	}
	if (ar[r][c]->solve(val))
		return true;
	else
	{
		save = temp->save;
		delete temp;
		return false;
	}
}

bool Puzzle::restoreSave() //backs up to the stored save
{
	if (save == nullptr)
		return false;
	Puzzle* temp = save;
	*this = *save;
	delete temp;
	return true;
}

void Puzzle::solve() //main solving function; prioritizes direct solutions, then indirect, and finally resorts to manual selections
{
	bool success = false;
	while (!solved())
	{
		success = false;
		printOut();
		if (!puzCorrect())
		{
			std::cout << "Puzzle invalid!" << std::endl << std::endl;
			if (!restoreSave())
				exit(1);
			else
				continue;
		}
		for (int k = 0; k < 9; k++)
			for (int c = 0; c < 9; c++)
				if (ar[k][c]->solved())
					if (solutionUpdate(k, c))
						success = true;

		if (!success)
			if (!comparePoss()) //TODO: add alignment(or just box-sharing) pseudo-solutions, and rewrite comparations to update/refine possibilities
			{
				int level = -1; int row = -1; int col = -1;
				std::cout << "Solution unreachable.  Possibility Map:" << std::endl;
				printPossMap();
				std::cout << "Possibility Levels are as follows:" << std::endl;
				printPossLevels();
				while (1)
				{
					row = getRow(true);

					if (row == -1)
						if (restoreSave())
							break;
						else
						{
							std::cout << "No previous guess.  Resuming current guess." << std::endl;
							row = getRow(false);
						}

					col = getCol();
					level = getChoice();
					row--; col--;
					if (forceSolve(row, col, level))
					{
						std::cout << "Guessing " << (row + 1) << ", " << (col + 1) << " is " << level << "." << std::endl;
						break;
					}
					else
						std::cout << "Invalid guess.  Please try again." << std::endl;
				}
			}

	}
}

bool Puzzle::solved() const //checks itself to see if it was solved
{
	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			if (!ar[k][c]->solved())
				return false;
	return true;
}

void Puzzle::printOut() const //prints out the puzzle as it is
{
	for (int k = 0; k < 9; k++)
	{
		for (int c = 0; c < 9; c++)
			std::cout << ar[k][c]->solution() << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void Puzzle::printPossMap() const //prints out 2-D rendering of the Poss map
{
	for (int k = 0; k < 9; k++)
	{
		for (int c = 0; c < 9; c++)
			std::cout << ar[k][c]->nPoss() << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void Puzzle::printPossLevels() const //prints out each "z-level" of the 3-D map, or specific value's possibility
{
	for (int k = 1; k < 10; k++)
	{
		std::cout << "Level " << k << std::endl;
		for (int r = 0; r < 9; r++)
		{
			for (int c = 0; c < 9; c++)
				if (ar[r][c]->has(k))
					std::cout << "1" << " ";
				else
					std::cout << "0" << " ";
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

Puzzle::~Puzzle() //cleans up the puzzle
{
	for (int k = 0; k < 9; k++)
		for (int c = 0; c < 9; c++)
			delete ar[k][c];

	for (int k = 0; k < 9; k++)
		delete[] ar[k];
	delete[] ar;
}

///////////////////////////////////////////////////////////////////////////
//  Independent function implementations
///////////////////////////////////////////////////////////////////////////

void sSolve(int arr[9][9]) //adds a finalizing output to the puzzle's outputs
{
	Puzzle sud(arr);
	sud.solve();

	if (sud.solved())
		std::cout << "Solved" << std::endl << std::endl;
	else
		std::cout << "Not solved" << std::endl << std::endl;
	std::cout << "Final readout:" << std::endl;
	sud.printOut();

	std::cout << "Change:" << std::endl;
	for (int k = 0; k < 9; k++)
	{
		for (int c = 0; c < 9; c++)
			std::cout << sud.solutionRet(k, c) - arr[k][c] << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;


}

void checkLegality(int arr[9][9]) //external check for validity
{
	Puzzle sud(arr);
	if (sud.puzCorrect())
		std::cout << "Puzzle is valid" << std::endl;
	else
		std::cout << "Puzzle is invalid" << std::endl;
}

int getRow(bool previous)
{
	while (1)
	{
		int r = -1;
		std::cout << "Row choice";
		if (previous)
			std::cout << " (Enter - 1 to go back a guess)";
		std::cout << ": ";
		std::cin >> r;
		if ((r == -1 && previous) || (r >= 1 && r <= 9))
			return r;
		std::cout << "Incorrect row.  Please choose again." << std::endl;
	}
}

int getCol()
{
	while (1)
	{
		int c = -1;
		std::cout << "Column choice: ";
		std::cin >> c;
		if (c >= 1 && c <= 9)
			return c;
		std::cout << "Incorrect column.  Please choose again." << std::endl;
	}
}

int getChoice()
{
	while (1)
	{
		int lev = -1;
		std::cout << "Level (Solution) Choice: ";
		std::cin >> lev;
		if (lev >= 1 && lev <= 9)
			return lev;
		std::cout << "Incorrect level.  Please choose again." << std::endl;
	}
}