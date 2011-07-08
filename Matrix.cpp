#include "Matrix.h"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cfloat>
#include <cmath>
#ifdef CLUSTER
	#include <omp.h>
	#include <iostream>
	using namespace std;
#endif

Matrix::Matrix() {
	rows = 0;
	cols = 0;
	precision = 5;
}

Matrix::Matrix(int r, int c, mkmatflag flag) {
	srand(time(NULL));
	rows = r;
	cols = c;
	precision = 5;

	for (r = 0; r < rows; r++)
		for (c = 0; c < cols; c++)
			data.push_back(0.0);

	// if ident is set, set matrix to identity matrix
	if (flag & IDENT) {
		for (r = 0; r < rows; r++)
			for (c = 0; c < cols; c++)
				data[r*cols+c] = (r == c) ? 1 : 0;
		return;
	}

	// if random positive or negative numbers
	if (flag & MIXED) {
		for (r = 0; r < rows; r++)
			for (c = 0; c < cols; c++)
				data[r*cols+c] = rand() % 200 - 100;
		return;
	}

	// if random positive numbers which may sum to one
	if (flag & (RAND | ONE))
		for (r = 0; r < rows; r++)
			for (c = 0; c < cols; c++)
				data[r*cols+c] = rand() % 100;

	// if all rows sum to one
	if (!(flag & ONE))
		return;

	for (r = 0; r < rows; r++) {
		double sum = 0;
		for (c = 0; c < cols; c++)
			sum += data[r*cols+c];
		for (c = 0; c < cols; c++)
			data[r*cols+c] /= sum;
	}
}

Matrix::Matrix(const Matrix &M, int x, int r, int y, int c) {
	int maxx = x + r;
	int maxy = y + c;
	rows = r;
	cols = c;
	precision = M.Precision();
	for (int i = x; i < maxx; i++)
		for (int j = y; j < maxy; j++)
			data.push_back(M.Get(i, j));
}

Matrix::~Matrix() {
	data.clear();
	rows = 0;
	cols = 0;
}

// recursive function to find the determinant of a matrix
double Matrix::Det() const {
	if (rows != cols)
		throw "Matrix determinant: Non-square matrix";
	if (rows == 1)
		return (data[0]);
	if (rows == 2)
		return (data[0]*data[3] - data[1]*data[2]);
	double det = 0;
	int col;
	#pragma omp parallel for shared(det) private(col) schedule(dynamic)
	for (col = 0; col < cols; col++) {
		Matrix M(rows-1, cols-1);
		for (int c = 0, skip = 0; c < M.Cols(); c++, skip++) {
			if (c == col) skip++;
			for (int r = 0; r < M.Rows(); r++)
				M[r][c] = data[(r+1)*cols+skip]; 
		}
		#pragma omp atomic
		det += (col % 2 ? -1 : 1) * M.Det();
	}
	return det;
}

Matrix Matrix::Trn() const {
	Matrix ret(cols, rows);
	for (int r = 0; r < rows; r++)
		for (int c = 0; c < cols; c++)
			ret[c][r] = data[r*cols+c];
	return ret;
}

// recursive function to find the inverse of a matrix
Matrix Matrix::Inv() const {
	try {
		if (rows != cols) throw "Matrix inversion: Non-square matrix";
		if (rows == 0) throw "Matrix inversion: 0 size matrix";
		if (rows < 3) {
			double det = Det();
			if (det == 0) throw "Matrix inversion: Determinant is zero";
			if (rows == 1) {
				Matrix ret(rows, cols);
				ret.Set(0, 0, 1.0/det);
				return ret;
			}
			if (rows == 2) {
				Matrix ret(rows, cols);
				double det_ = 1.0 / det;
				ret.Set(0, 0, data[3]);
				ret.Set(0, 1, -data[1]);
				ret.Set(1, 0, -data[2]);
				ret.Set(1, 1, data[0]);
				ret = ret * det_;
				return ret;
			}
		}
		Matrix A(*this, 0, rows - rows/2, 0, cols - cols/2);
		Matrix B(*this, 0, rows - rows/2, cols - cols/2, cols/2);
		Matrix C(*this, rows - rows/2, rows/2, 0, cols - cols/2);
		Matrix D(*this, rows - rows/2, rows/2, cols - cols/2, cols/2);
		Matrix Ai = A.Inv();
		Matrix retA, retB, retC, retD;
		Matrix ret(rows, cols);
		//cout << A << endl << B << endl << C << endl << D << endl;
		#pragma omp parallel
		{
			#pragma omp sections
			{
				#pragma omp section
				retA = Ai + (Ai * B * 
					(D - C * Ai * B).Inv() * C * Ai);
				#pragma omp section
				retB = -Ai * B * (D - C * Ai * B).Inv();
				#pragma omp section
				retC = -(D - C * Ai * B).Inv() * C * Ai;
				#pragma omp section
				retD = (D - C * Ai * B).Inv();
			}
		}
		//cout << retA << endl << retB << endl << retC << endl << retD << endl;
		for (int r = 0; r < retA.Rows(); r++)
			for (int c = 0; c < retA.Cols(); c++)
				ret.Set(r, c, retA[r][c]);
		for (int r = 0; r < retB.Rows(); r++)
			for (int c = 0; c < retB.Cols(); c++)
				ret.Set(r, c + retA.Cols(), retB[r][c]);
		for (int r = 0; r < retC.Rows(); r++)
			for (int c = 0; c < retC.Cols(); c++)
				ret.Set(r + retA.Rows(), c, retC[r][c]);
		for (int r = 0; r < retD.Rows(); r++)
			for (int c = 0; c < retD.Cols(); c++)
				ret.Set(r + retA.Rows(), c + retA.Cols(), retD[r][c]);
		return ret;
	} catch (...) {
		throw;
	}
}

Matrix Matrix::operator+ (const Matrix &R) const {
	if (cols != R.Cols() || rows != R.Rows())
		throw "Matrix addition: Size mismatch";
	Matrix ret(rows, cols);
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < cols; col++)
			ret[row][col] = data[row*cols+col] + R.Get(row, col);
	return ret;
}

Matrix Matrix::operator- (const Matrix &R) const {
	if (cols != R.Cols() || rows != R.Rows())
		throw "Matrix subtraction: Size mismatch";
	Matrix ret(rows, cols);
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < cols; col++)
			ret[row][col] = data[row*cols+col] - R.Get(row, col);
	return ret;
}

Matrix Matrix::operator* (const Matrix &R) const {
	if (cols != R.Rows())
		throw "Matrix multiplication: Size mismatch";
	Matrix ret(rows, R.Cols());
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < R.Cols(); col++) {
			ret.Set(row, col, 0);
			for (int i = 0; i < cols; i++)
				ret[row][col] = ret[row][col] + 
					(data[row*cols+i] * R.Get(i, col));
		}
	return ret;
}

Matrix Matrix::operator* (double n) const {
	Matrix ret(rows, cols);
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < cols; col++)
			ret.Set(row, col, data[row*cols+col] * n);
	return ret;
}

Matrix Matrix::operator- () const {
	return (*this) * -1.0;
}

Matrix Matrix::operator/ (const Matrix &R) const {
	Matrix ret(rows, cols);
	try {
		ret = R.Inv() * (*this);
	} catch (...) {
		throw;
	}
	return ret;
}

MatRow Matrix::operator[] (int row) {
	return MatRow(this, row);
}

ostream& operator<< (ostream &out, const Matrix &M) {
	out.precision(M.Precision());
	if (M.Cols())
		for (int r = 0; r < M.Rows(); r++) {
			for (int c = 0; c < M.Cols(); c++) {
				out << M.Get(r, c) << "\t";
			}
			out << "\n";
		}
	return out;
}

int Matrix::Precision() const {
	return precision;
}

int Matrix::Precision(int i) {
	int old = precision;
	precision = i;
	return old;
}

double Matrix::Get(int row, int col) const {
	if (row >= rows || row < 0)
		throw "Matrix.Get: Invalid row";
	if (col >= cols || col < 0)
		throw "Matrix.Get: Invalid column";
	return data[row*cols+col];
}

double& Matrix::Ref(int row, int col) {
	if (row >= rows || row < 0)
		throw "Matrix.Ref: Invalid row";
	if (col >= cols || col < 0)
		throw "Matrix.Ref: Invalid column";
	return data[row*cols+col];
}

void Matrix::Set(int row, int col, double val) {
	if (row >= rows || row < 0)
		throw "Matrix.Set: Invalid row";
	if (col >= cols || col < 0)
		throw "Matrix.Set: Invalid column";
	//cout << *this << endl;
	data[row*cols+col] = val;
}

Matrix Matrix::Gauss() const {
	Matrix A = *this;
	int i = 0;
	int j = 0;
	int m = A.Rows();
	int n = A.Cols();

	// Gauss elimination
	while ((i < m) && (j < n)) {
		int maxi = i;
		for (int k = i + 1; k < m; k++) {
			if (abs(A[k][j]) > abs(A[maxi][j]))
				maxi = k;
		}
		if (A[maxi][j] != 0) {
			MatRow tmp(n);
			tmp = A[i];
			A[i] = A[maxi];
			A[maxi] = tmp;
			double div = A[i][j];
			for (int k = 0; k < n; k++)
				A[i][k] /= div;
			for (int k = i+1; k < m; k++)
				A[k] -= A[i] * A[k][j];
			i++;
		}
		j++;
	}

	// back substitution
	MatRow x(n);
	for (i = m-2; i >= 0; i--) {
		for (j = n-2; j > i; j--) {
			if (j < m) {
				x = A[j] * A[i][j];
				A[i] -= x;
			}
		}
	}

	return A;
}

MatRow::MatRow(Matrix *p, int r) { 
	parent = p;
	row = r;
	standalone = false;
	size = p->Cols();
}

MatRow::MatRow(int s) {
	size = s;
	data = new double[size];
	standalone = true;
}

double& MatRow::operator[](int c) {
	return Ref(c);
}

MatRow MatRow::operator*(double d) const {
	MatRow ret(size);
	if (!standalone)
		for (int i = 0; i < size; i++)
			ret[i] = d * parent->Get(row, i);
	else
		for (int i = 0; i < size; i++)
			ret[i] = d * data[i];
	return ret;
}

MatRow& MatRow::operator-=(const MatRow &r) {
	int i = 0;
	while ((i < size) && (i < r.Size())) {
		this->Ref(i) -= r.Get(i);
		i++;
	}
	return *this;
}

double MatRow::Get(int c) const {
	if (!standalone)
		return parent->Get(row, c);
	if ((c < size) && (c >= 0))
		return data[c];
	throw "Matrix indexing error";
}

void MatRow::Set(int c, double d) {
	if (!standalone)
		parent->Set(row, c, d);
	if ((c < size) && (c >= 0))
		data[c] = d;
	else
		throw "Matrix indexing error";
}

double& MatRow::Ref(int c) {
	if (!standalone)
		return parent->Ref(row, c);
	if ((c < size) && (c >= 0))
		return data[c];
	throw "Matrix indexing error";
}

