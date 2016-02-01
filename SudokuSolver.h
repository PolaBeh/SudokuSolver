#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

class Poss
{
public:
	Poss(int x, int y, int starter);
	bool solved() const { return p[0]; }
	int nPoss() const;
	bool has(int val) const;
	int solution() const { return p[0]; }
	bool remove(int val);
	bool solve(int val);
private:
	int r, c;
	int p[10];
};

class Puzzle
{
public:
	Puzzle(int arr[9][9]);
	Puzzle(const Puzzle& other);
	Puzzle& operator=(const Puzzle& other);
	~Puzzle();
	void solve();
	bool solved() const;
	bool puzCorrect() const;
	void printOut() const;
	void printPossMap() const;
	void printPossLevels() const;
	int solutionRet(int r, int c) const { return ar[r][c]->solution(); }
private:
	struct CorrectPoss
	{
		CorrectPoss();
		bool check(int val);
		int p[10];
	};
	Poss*** ar;
	Puzzle* save;
	bool solutionUpdate(int r, int c);
	bool forceSolve(int r, int c, int val);
	bool restoreSave();
	bool guess();
	bool comparePoss();
	bool compRow(int r);
	bool compCol(int c);
	bool compBlock(int r, int c);
	bool rowCorrect(int r) const;
	bool colCorrect(int c) const;
	bool blockCorrect(int r, int c) const;
};

void sSolve(int arr[9][9]);
void checkLegality(int arr[9][9]);
int getRow(bool previous);
int getCol();
int getChoice();

#endif