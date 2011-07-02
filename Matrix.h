#ifndef MATRIX_H
#define MATRIX_H
#include <iostream>
#include <vector> 
using namespace std;

class MatRow;

enum mkmatflag {
	ZERO = 0,
	IDENT = 1,
	RAND = 2,
	MIXED = 4,
	ONE = 8
};

class Matrix {
	vector<double> data;
	int rows, cols;
	int precision;

public:
	Matrix();
	Matrix(int r, int c, mkmatflag flag = ZERO);
	Matrix(const Matrix&, int, int, int, int);
	~Matrix();
	Matrix operator+(const Matrix&) const;
	Matrix operator-(const Matrix&) const;
	Matrix operator*(const Matrix&) const;
	Matrix operator*(double) const;
	Matrix operator/(const Matrix&) const;
	Matrix operator-() const;
	MatRow operator[](int);
	int Precision(int);
	int Precision() const;
	friend ostream& operator<<(ostream&, const Matrix&);
	double Get(int, int) const;
	void Set(int, int, double);
	double& Ref(int, int);
	double Det() const;
	Matrix Inv() const; // looks inefficient
	Matrix Gauss() const;
	Matrix Trn() const;
	int Rows() const { return rows; }
	int Cols() const { return cols; }
};

class MatRow {
	Matrix *parent;
	int row;
	bool standalone;
	double *data;
	int size;

public:
	MatRow(Matrix*, int);
	MatRow(int);
	double& operator[](int);
	MatRow operator*(double) const;
	MatRow& operator-=(const MatRow&);
	int Size() const { return size; }
	double Get(int) const;
	void Set(int, double);
	double& Ref(int);
};

#endif
