typedef struct Matrix Matrix;
struct Matrix {
	unsigned int r, c;
	double **d;
};

Matrix *newmat(unsigned int, unsigned int);
void delmat(Matrix *);
void printmat(Matrix *);
Matrix *scanmat(void);

/*
 * Matrix arithmetic
 */
void ident(Matrix *);
void matmul(Matrix *, Matrix *);
void matmulr(Matrix *, Matrix *);
double determinant(Matrix *);
void adjoint(Matrix *, Matrix *);
double invertmat(Matrix *, Matrix *);