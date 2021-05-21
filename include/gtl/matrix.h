#pragma once

// TMatx.hpp: TMatx class.
//
// PWH
// 2000.05.27 
// 2008.11.01
// 2021.05.21.
//
//////////////////////////////////////////////////////////////////////

#include <array>
#include <initializer_list>
#include <compare>

#include "gtl/_config.h"


namespace gtl {
#pragma pack(push, 8)

	/// @brief Simple matrix with m cols, n rows
	template < typename T, size_t m, size_t n >
	class TMatx {
	public:
		enum : size_t {
			rows = m,
			cols = n,
		};
		using value_type = T;

		std::array<T, m*n> values_{};

	public:
		TMatx(void) {}
		TMatx(TMatx const&) = default;
		TMatx(TMatx &&) = default;

		TMatx(std::initializer_list<T>&& values) : values_(values) {}
		TMatx(T value) : values_{value} {}

		~TMatx() {}

		TMatx& operator = (TMatx const&) = default;
		TMatx& operator = (TMatx &&) = default;
		auto operator <=> (TMatx const&) = default;
		auto operator <=> (T v) {
			return *this <=> TMatx<T, m, n>(value);
		}

		TMatx& Set(std::initializer_list<T>&& values) {
			values_ = values;
		}

		// row
		std::array<T, n>& operator[] (int row) { return *std::bit_cast<std::array<T, n>*>(&values_[row*n]); }
		std::array<T, n> const & operator[] (int row) const { return *std::bit_cast<std::array<T, n>*>(&values_[row*n]); }

		// value at
		T& operator() (size_t row, size_t col) {
			return values_[row*n + col];
		}
		T const& operator() (size_t row, size_t col) const {
			return values_[row*n + col];
		}

		TMatx& operator += (const TMatx& B) { for (size_t i{}; i < values_.size(); i++) values_[i] += B.values_[i]; return *this; }
		TMatx& operator -= (const TMatx& B) { for (size_t i{}; i < values_.size(); i++) values_[i] -= B.values_[i]; return *this; }
		TMatx operator + (TMatx const& B) const { auto r{*this}; return r += B; }
		TMatx operator - (TMatx const& B) const { auto r{*this}; return r -= B; }

		template < size_t cols2 >
		TMatx operator * (TMatx<n, cols2> const& B) const {
			TMatx<T, > C
		}

		TMatx operator * (const TMatx& B);
		TMatx& operator *= (const TMatx& B);
		TMatx& operator *= (const T& a);
		TMatx operator ^ (int r);
		TMatx& operator ^= (int r);
		template <class D>
		TMatx Convolute(const TMatx<D>& B, const T& vFactor);
		T Sum(void) const;
		TMatx trans(void);
		T det(void);
		void eigen(TMatx& E_value, TMatx& E_Vector) const;
		TMatx inv(void);

		TMatx<T> Partial(int m_nM, int m_nN, int x, int y) {
			TMatx<T> M;
			M.Set(m_nM, m_nN);

			for (int i = 0; i < m_nM; i++) {
				for (int j = 0; j < m_nN; j++) {
					M(i, j) = (*this)(i+m_nM/2, j+m_nN/2);
				}
			}
			return M;
		}

		template < class Vector >
		friend TMatx<Vector> operator*(const TMatx<double>& A, const TMatx<Vector>& B);
		template < class Vector >
		friend TMatx<Vector> operator*(const TMatx<Vector>& A, const TMatx<double>& B);
		template < class Vector >
		friend Vector operator*(TMatx<double>& A, Vector& B);
		friend TMatx operator *(double a, const TMatx& B);

		friend void ludcmp(TMatx&, int*);
		friend void lubksb(TMatx&, TMatx&, int*);

		friend void jacobi(TMatx&, TMatx&, TMatx&);
		friend void eigsrt(TMatx&, TMatx&);
	};

	template <class T>
	int Mcheck(TMatx<T> A);

	template <class T> TMatx<T>::TMatx(int m, int n) {
		m_body = NULL;
		m_M = NULL;
		m_nM = m_nN = 0;
		Set(m, n);
	}

	template <class T> TMatx<T>& TMatx<T>::Set(int m, int n) {
		Destroy();

		if (m && n) {
			m_nM = m;
			m_nN = n;

			m_body = new T[m_nM*m_nN];
			m_M = new T*[m_nM];
			T* pt = m_body;
			int i;
			for (i = 0; i < m_nM; i++, pt += m_nN)
				m_M[i] = pt;

		}
		return *this;
	}

	template <class T> TMatx<T>::TMatx(int m, int n, T f) {
		m_body = NULL;
		m_M = NULL;
		m_nM = m_nN = 0;
		Set(m, n, f);
	}

	template <class T> TMatx<T>& TMatx<T>::Set(int m, int n, T f) {
		Destroy();
		int i, j;

		Set(m, n);

		for (i = 0; i < m_nM; i++) {
			for (j = 0; j < m_nN; j++)
				m_M[i][j] = f;
		}

		return *this;
	}

	template <class T> TMatx<T>::TMatx(const TMatx<T>& B) {
		m_body = NULL;
		m_M = NULL;
		m_nM = m_nN = 0;
		Set(B);
	}

	template <class T> TMatx<T>& TMatx<T>::Set(const TMatx<T>& B) {
		Destroy();

		Set(B.M(), B.N());
		if (m_body && B.m_body)
			memcpy(m_body, B.m_body, m_nM*m_nN*sizeof(T));

		return *this;
	}

	template <class T> TMatx<T>::~TMatx(void) {
		Destroy();
	}

	template <class T> void TMatx<T>::Destroy(void) {
		if (m_M)
			delete [] m_M;
		if (m_body)
			delete [] m_body;

		m_body = NULL;
		m_M = NULL;
		m_nM = m_nN = 0;
	}

	template <class T> TMatx<T>& TMatx<T>::operator = (const TMatx<T>& B) {
		int i, j;

		if (this == &B)
			return *this;

	//	ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		Set(B.m_nM, B.m_nN);

		for (i = 0; i < m_nM; i++) {
			for (j = 0; j < m_nN; j++)
				m_M[i][j] = B.m_M[i][j];//memcpy(m_M[i], B.m_M[i], m_nN*sizeof(T));
		}

		return *this;
	}

	template <class T> int TMatx<T>::operator == (const TMatx<T>& B) {
		int i;
		if ( (m_nM != B.m_nM) || (m_nN != B.m_nN) ) return 0;
		for (i = 0; i < m_nM; i++)
			if (!memcmp(m_M[i], B.m_M[i], m_nN)) return 0;
		return 1;
	}

	template <class T> int TMatx<T>::operator != (const TMatx<T>& B) {
		int i;
		if ( (m_nM == B.m_nM) && (m_nN == B.m_nN) )
			for (i = 0; i < m_nM; i++)
				if (memcmp(m_M[i], B.m_M[i], m_nN)) return 1;
		else return 1;

		return 0;
	}

	template <class T> TMatx<T> TMatx<T>::operator + (const TMatx<T>& B) {
		int i, j;

		ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		TMatx<T> C(m_nM, m_nN);
		for (i = 0; i < m_nM; i++)
			for (j = 0; j < m_nN; j++)
				C.m_M[i][j] = m_M[i][j] + B.m_M[i][j];

		return C;
	}

	template <class T> TMatx<T>& TMatx<T>::operator += (const TMatx<T>& B) {
		int i, j;

		ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		for (i = 0; i < m_nM; i++)
			for (j = 0; j < m_nN; j++)
				m_M[i][j] += B.m_M[i][j];

		return *this;
	}

	template <class T> TMatx<T> TMatx<T>::operator - (const TMatx<T>& B) {
		int i, j;

		ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		TMatx<T> C(m_nM, m_nN);
		for (i = 0; i < m_nM; i++)
			for (j = 0; j < m_nN; j++)
				C.m_M[i][j] = m_M[i][j] - B.m_M[i][j];

		return C;
	}

	template <class T> TMatx<T>& TMatx<T>::operator -= (const TMatx<T>& B) {
		int i, j;

		ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		for (i = 0; i < m_nM; i++)
			for (j = 0; j < m_nN; j++)
				m_M[i][j] -= B.m_M[i][j];

		return *this;
	}

	template <class T> TMatx<T> TMatx<T>::operator * (const TMatx<T>& B) {
		int i, j, k;

		if ( (B.m_nM == 1) && (B.m_nN == 1) ) {
			TMatx<T> C(m_nM, m_nN);
			for (i = 0; i < m_nM; i++)
				for (j = 0; j < m_nN; j++)
					C.m_M[i][j] *= B.m_M[0][0];
			return C;
		}

		ASSERT(m_nN == B.m_nM);

		TMatx<T> C(m_nM, B.m_nN, 0.0);
		for (i = 0; i < C.m_nM; i++)
			for (j = 0; j < C.m_nN; j++)
				for (k = 0; k < m_nN; k++)
					C.m_M[i][j] += m_M[i][k] * B.m_M[k][j];

		return C;
	}

	template <class T> TMatx<T>& TMatx<T>::operator *= (const TMatx<T>& B) {
		int i, j, k;

		ASSERT( (m_nM == B.m_nM) && (m_nN == B.m_nN) );

		TMatx<T> C(m_nM, B.m_nN, 0.0);
		for (i = 0; i < C.m_nM; i++)
			for (j = 0; j < C.m_nN; j++)
				for (k = 0; k < m_nN; k++)
					C.m_M[i][j] += m_M[i][k] * B.m_M[k][j];

		*this = C;
		return *this;
	}

	template <class T> TMatx<T> TMatx<T>::operator ^ (int r) {
		int i;

		ASSERT(m_nM == m_nN);

		TMatx<T> C(m_nM, m_nN, 0.0);

		if (r == 0) {
			for (i = 0; i < m_nM; i++)
				C.m_M[i][i] = 1.0;
			return C;
		}

		if (r < 0) {
			r = -r;
			C = inv();
		}

		for (i = 0; i < r-1; i++)
			C = C * *this;

		return C;
	}

	template <class T> TMatx<T>& TMatx<T>::operator ^= (int r) {
		return *this = (*this)^r;
	}

	#if (_MSC_VER >= _MSC_VER_VS2005)
	template <class T> template < class D>
	#else
	template <class T, class D>
	#endif
	TMatx<T> TMatx<T>::Convolute(const TMatx<D>& B, const T& vFactor) {
		TMatx<T> C(m_nM, m_nN, vFactor-vFactor);

		int sh_i = B.M()/2, sh_j = B.N()/2;
		for (int i = 0; i < C.m_nM; i++) {
			for (int j = 0; j < C.m_nN; j++) {

				for (int k = 0; k < B.M(); k++) {
					for (int l = 0; l < B.N(); l++) {
						C(i, j) += GetSafeValue(i-sh_i+k, j-sh_j+l) * B(k, l);
					}
				}

			}
		}

		C *= vFactor;

		return C;
	}

	template <class T> T TMatx<T>::Sum(void) const {
		T v = (*this)(0, 0);
		for (int i = 0; i < m_nM; i++) {
			for (int j = 0; j < m_nN; j++) {
				v += (*this)(i, j);
			}
		}

		v -= (*this)(0, 0);
		return v;
	}

	template <class T> TMatx<T> TMatx<T>::trans(void) {
		int i, j;

		TMatx<T> C(m_nN, m_nM);
		for (i = 0; i < m_nN; i++)
			for (j = 0; j < m_nM; j++)
				C.m_M[i][j] = m_M[j][i];

		return C;
	}

	template <class T> T TMatx<T>::det() {
		int dim;
		if (m_nM != m_nN) {return (T)0.0;}
		dim = m_nM;
		T d;
		d = (T)0.0;
		if (dim <= 3) {
			int i, j;
			for (i = 0; i < dim; i++) {
				T s;
				for (s = 1.0, j = 0; j < dim; j++)
					s *= m_M[j][(i+j)%dim];
				d += s;
				for (s = 1.0, j = 0; j < dim; j++)
					s *= m_M[j][(dim+i-j)%dim];
				d -= s;
			}
		} else {
			int i, j, k;
			TMatx<T> C(m_nM-1, m_nN-1);
			for (i = 0; i < dim; i++) {
				if (m_M[0][i] == 0.0) continue;
				for (j = 0; j < C.m_nM; j++) {
					for (k = 0; k < C.m_nN; k++) {
						C[j][k] = m_M[j+1][ (k+i+1) % m_nN ];
					}
				}
				d += m_M[0][i] * C.det();
			}
		}
		return d;
	}

	/*
	template <class T> T TMatx<T>::detNew(void) const {
		int i, j, dim, level;
		T d, s;

		if (m_nM != m_nN) { return 0.0; }

		d = 0.0;
		dim = m_nM;

		if (dim == 1) return m_M[0][0];
		if (dim < 1) return 0.0;

		int sign = 0;
		int* index = new int [dim];
		for (i = 0; i < dim; i++) {
			for (j = i; j < dim; j++) {
				for (k = j; k < dim; k++) {
					for (l = k; l < dim; l++) {
						;
					}
				}
			}
		}

		delete [] index;

		return d;
	}
	*/
	/* The Routine for TMatx Inversion
	 *      By L-U Decomposition And Back-Substitution
	 */

	// Routine edited at 1996. 3. 19.

	template <class T> TMatx<T> TMatx<T>::inv(void) {
		int dim, i, j;
		int *index;

	//	ASSERT(det() != 0.0);
		dim = m_nN;
		TMatx<T> A(*this);
		TMatx<T> B(dim, dim, 0.0);
		TMatx<T> col(1, dim, 0.0);
		index = new int[dim];
		for (i = 0; i < dim; i++)
			index[i] = 0;

		ludcmp(A, index);

		for (j = 0; j < dim; j++) {
			for(i = 0; i < dim; i++) col.m_M[0][i] = .0;
			col.m_M[0][j] = 1.0;
			lubksb(A, col, index);
			for (i = 0; i < dim; i++) B.m_M[i][j] = col.m_M[0][i];
		}

		delete [] index;
		return B;
	}

	template <class T> void TMatx<T>::eigen(TMatx<T>& E_value, TMatx<T>& E_Vector) const {

		TMatx<T> A(*this);

		jacobi(A, E_value, E_Vector);

		eigsrt(E_value, E_Vector);
	}

	/* Routine for L-U Decomposition
	 * ref> Numerical Recipes
	 */

	// Routine edited at 1996. 3. 19.
	#define TINY (1.0e-20)
	template <class T> void ludcmp(TMatx<T>& A, int *index) {
		int imax, i, j, k, dim;
		T big, dum, sum, temp;

		dim = A.m_nN;
		TMatx<T> vv(1, dim, 0);
		for (i = 0; i < dim; i++) {
			big = .0;
			for (j = 0; j < dim; j++)
				if ( (temp=fabs(A.m_M[i][j])) > big ) big = temp;
			ASSERT(big != 0.0);
			vv.m_M[0][i] = 1.0 / big;
		}
		for (j = 0; j < dim; j++) {
			for (i = 0; i < j; i++) {
				sum = A.m_M[i][j];
				for (k = 0; k < i; k++) sum -= A.m_M[i][k] * A.m_M[k][j];
				A.m_M[i][j] = sum;
			}
			big = .0;
			for (i = j; i < dim; i++) {
				sum = A.m_M[i][j];
				for (k = 0; k < j; k++) sum -= A.m_M[i][k] * A.m_M[k][j];
				A.m_M[i][j] = sum;
				if ( (dum=vv.m_M[0][i]*fabs(sum)) >= big ) {
					big = dum;
					imax = i;
				}
			}
			if (j != imax) {
				for (k = 0; k < dim; k++) {
					dum = A.m_M[imax][k];
					A.m_M[imax][k] = A.m_M[j][k];
					A.m_M[j][k] = dum;
				}
				vv.m_M[0][imax] = vv.m_M[0][j];
			}
			index[j] = imax;
			if ( A.m_M[j][j] == .0) A.m_M[j][j] = TINY;
			if ( j != (dim-1) ) {
				dum = 1.0 / (A.m_M[j][j]);
				for (i = j+1; i < dim; i++) A.m_M[i][j] *= dum;
			}
		}
	}
	#undef TINY

	/* Routine for L-U Back Substitution
	 * ref> Numerical Recipes
	 */

	// Routine edited at 1996. 3. 19.
	template <class T> void lubksb(TMatx<T>& A, TMatx<T>& Col, int *index) {
		int ii = -1;
		int i, ip, j;
		int dim;
		T sum;

		dim = A.m_nN;
		for (i = 0; i < dim; i++) {
			ip = index[i];
			sum = Col.m_M[0][ip];
			Col.m_M[0][ip] = Col.m_M[0][i];
			if (ii != -1)
				for (j = ii; j < i; j++) sum -= A.m_M[i][j] * Col.m_M[0][j];
			else if (sum) ii = i;
			Col.m_M[0][i] = sum;
		}
		for (i = dim-1; i >= 0; i--) {
			sum = Col.m_M[0][i];
			for (j = i+1; j < dim; j++) sum -= A.m_M[i][j] * Col.m_M[0][j];
			Col.m_M[0][i] = sum / A.m_M[i][i];
		}
	}

	/* Eigen values & Eigen Vectors of Real Symmetric matrix A[m_nN][m_nN]
	 * 1994. 9. 7.
	 */

	// Routine edited at 1996. 3. 19.

	#define Rotate(A, i, j, k, l) \
		{\
			g = A[i][j];\
			h = A[k][l];\
			A[i][j] = g - s*(h + g*tau);\
			A[k][l] = h + s*(g - h*tau);\
		}\


	#define Iter 50
	template <class T> void jacobi(TMatx<T>& A, TMatx<T>& d, TMatx<T>& V) {
		int iq, ip, i, j, dim;
		T tresh, theta, tau, t, sm, s, h, g, c;

		dim = A.m_nN;
		TMatx<T> b(1, dim, 0.0);
		TMatx<T> z(1, dim, 0.0);

		for (ip = 0; ip < dim; ip++) {
			for (iq = 0; iq < dim; iq++) V.m_M[ip][iq] = .0;
			V.m_M[ip][ip] = 1.0;
		}
		for (ip = 0; ip < dim; ip++) {
			b.m_M[0][ip] = d.m_M[0][ip] = A.m_M[ip][ip];
			z.m_M[0][ip] = .0;
		}

		for (i = 0; i < Iter; i++) {
			sm = .0;

			for (ip = 0; ip < dim-1; ip++)
				for (iq = ip+1; iq < dim; iq++)
					sm += fabs(A.m_M[ip][iq]);
			if (sm == .0)
				return;

			if (i < 3) tresh = 0.2 * sm / (dim*dim);
			else tresh = .0;

			for (ip = 0; ip < dim-1; ip++) {
				for (iq = ip+1; iq < dim; iq ++) {
					g = 100.0 * fabs(A.m_M[ip][iq]);
					if ( (i > 3) && (fabs(d.m_M[0][ip])+g == fabs(d.m_M[0][ip]))
						&& (fabs(d.m_M[0][iq])+g == fabs(d.m_M[0][iq])) )
						A.m_M[ip][iq] = .0;
					else
					if (fabs(A.m_M[ip][iq]) > tresh) {
						h = d.m_M[0][iq] - d.m_M[0][ip];
						if ( fabs(h) + g == fabs(h) ) t = A.m_M[ip][iq]/h;
						else {
							theta = 0.5 * h / A.m_M[ip][iq];
							t = 1.0 / ( fabs(theta) + sqrt(1.0 + theta*theta) );
							if (theta < .0) t = -t;
						}
						c = 1.0 / sqrt(1 + t*t);
						s = t*c;
						tau = s / (1.0 + c);
						h = t * A.m_M[ip][iq];
						z.m_M[0][ip] -= h;
						z.m_M[0][iq] += h;
						d.m_M[0][ip] -= h;
						d.m_M[0][iq] += h;
						A.m_M[ip][iq] = .0;

						for (j = 0; j < ip; j++) Rotate(A.m_M, j, ip, j, iq);
						for (j = ip + 1; j < iq; j++) Rotate(A.m_M, ip, j, j, iq);
						for (j = iq + 1; j < dim; j++) Rotate(A.m_M, ip, j, iq, j);
						for (j = 0; j < dim; j++) Rotate(V.m_M, j, ip, j, iq);
					}
				}
			}
			for (ip = 0; ip < dim; ip++) {
				b.m_M[0][ip] += z.m_M[0][ip];
				d.m_M[0][ip] = b.m_M[0][ip];
				z.m_M[0][ip] =.0;
			}
		}
		ASSERT(false);
		// !! Too many iterations in routine JACOBI
	}
	#undef Iter
	#undef Rotate

	/* Sorting the Eigen values & Eigen vectors
	 *   from Jacobi() in Ascending Order
	 * 1994. 9. 7. -> 1995. 3. 28.
	 */

	template <class T> void eigsrt(TMatx<T>& d, TMatx<T>& V) {
		int i, j, k;
		T p;
		int dim;

		dim = V.m_nN;
		for (i = 0; i < dim - 1; i++) {
			p = d.m_M[0][k=i];

			for (j = i + 1; j < dim; j ++)
				if (d.m_M[0][j] >= p) p = d.m_M[0][k=j];

			if (k != i) {
				d.m_M[0][k] = d.m_M[0][i];
				d.m_M[0][i] = p;
				for (j = 0; j < dim; j++) {
					p = V.m_M[j][i];
					V.m_M[j][i] = V.m_M[j][k];
					V.m_M[j][k] = p;
				}
			}
		}
	}

	template <class T> int Mcheck(TMatx<T> A) {
		int i, j, dim;
		T d1;

		if (A.m_nN != A.m_nM) {
			return 0;
		}

		dim = A.m_nN;

		for (i = 0; i < dim; i++)
			for (j = 0; j < dim; j++) {
				if (i == j) d1 = 1;
				else d1 = 0;
				if (A.m_M[i][j] != d1) return 0;
			}

		return 1;
	}

	template<class T> TMatx<T> operator * (double a, const TMatx<T>& B) {
		TMatx<T> c = B;
		for (int i = 0; i < c.m_nM; i++) {
			for (int j = 0; j < c.m_nN; j++)
				c[i][j] *= a;
		}
		return c;
	}

	template<class T> TMatx<T>& TMatx<T>::operator *= (const T& a) {
		for (int i = 0; i < m_nM; i++) {
			for (int j = 0; j < m_nN; j++)
				m_M[i][j] *= a;
		}
		return *this;
	}


	#include <math.h>
	typedef TMatx<double> DMatrix;

	template < class Vector >
	TMatx<Vector> operator*(const DMatrix& A, const TMatx<Vector>& B) {
		int i, j, k;

		if ( (A.m_nM == 1) && (A.m_nN == 1) ) {
			TMatx<Vector> C(B.m_nM, B.m_nN);
			for (i = 0; i < B.m_nM; i++)
				for (j = 0; j < B.m_nN; j++)
					C.m_M[i][j] = A.m_M[0][0] * B.m_M[i][j];
		}
		if ( (B.m_nM == 1) && (B.m_nN == 1) ) {
			TMatx<Vector> C(A.m_nM, A.m_nN);
			for (i = 0; i < A.m_nM; i++)
				for (j = 0; j < A.m_nN; j++)
					C.m_M[i][j] = A.m_M[i][j] * B.m_M[0][0];
			return C;
		}

		ASSERT(A.m_nN == B.m_nM);

		TMatx<Vector> C(A.m_nM, B.m_nN, Vector(0.0, 0.0, 0.0));
		for (i = 0; i < C.m_nM; i++)
			for (j = 0; j < C.m_nN; j++)
				for (k = 0; k < A.m_nN; k++)
					C.m_M[i][j] += A.m_M[i][k] * B.m_M[k][j];
		return C;
	}

	template < class Vector >
	TMatx<Vector> operator*(const TMatx<Vector>& A, const DMatrix& B) {
		int i, j, k;

		if ( (A.m_nM == 1) && (A.m_nN == 1) ) {
			TMatx<Vector> C(B.m_nM, B.m_nN);
			for (i = 0; i < B.m_nM; i++)
				for (j = 0; j < B.m_nN; j++)
					C.m_M[i][j] = A.m_M[0][0] * B.m_M[i][j];
		}
		if ( (B.m_nM == 1) && (B.m_nN == 1) ) {
			TMatx<Vector> C(A.m_nM, A.m_nN);
			for (i = 0; i < A.m_nM; i++)
				for (j = 0; j < A.m_nN; j++)
					C.m_M[i][j] = A.m_M[i][j] * B.m_M[0][0];
			return C;
		}

		ASSERT(A.m_nN == B.m_nM);

		TMatx<Vector> C(A.m_nM, B.m_nN, Vector(0.0, 0.0, 0.0));
		for (i = 0; i < C.m_nM; i++)
			for (j = 0; j < C.m_nN; j++)
				for (k = 0; k < A.m_nN; k++)
					C.m_M[i][j] += A.m_M[i][k] * B.m_M[k][j];
		return C;
	}

	/*
	template < class Vector >
	Vector operator*(DMatrix& A, Vector& B) {
		Vector C(B);
		Vector D(0, 0, 0, 0);
		int t;
		if (A.m_nM == 3) { t = 3; D[3] = 0.0; C.PreSet(); }
		else if (A.m_nM == 4) t = 4;
		for (int i = 0; i < t; i++)
			for (int j = 0; j < t; j++)
				D[i] += A[i][j] * B[j];
		return D;

	}
	*/

	DMatrix MRotate3_x(double);
	DMatrix MRotate3_y(double);
	DMatrix MRotate3_z(double);

	DMatrix MRotate4_x(double);
	DMatrix MRotate4_y(double);
	DMatrix MRotate4_z(double);

	// modified at 1995.3.28.
	inline DMatrix MRotate3_x(double theta) {
		DMatrix mat(3, 3, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[1][1] = mat[2][2] = c;
		mat[1][2] = -s;
		mat[2][1] = s;
		mat[0][0] = 1.0;

		return mat;
	}

	inline DMatrix MRotate3_y(double theta) {
		DMatrix mat(3, 3, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[0][0] = mat[2][2] = c;
		mat[0][2] = s;
		mat[2][0] = -s;
		mat[1][1] = 1.0;

		return mat;
	}

	inline DMatrix MRotate3_z(double theta) {
		DMatrix mat(3, 3, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[0][0] = mat[1][1] = c;
		mat[0][1] = -s;
		mat[1][0] = s;
		mat[2][2] = 1.0;

		return mat;
	}

	inline DMatrix MRotate4_x(double theta) {
		DMatrix mat(4, 4, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[1][1] = mat[2][2] = c;
		mat[1][2] = -s;
		mat[2][1] = s;
		mat[0][0] = 1.0;

		mat[3][3] = 1.0;

		return mat;
	}

	inline DMatrix MRotate4_y(double theta) {
		DMatrix mat(4, 4, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[0][0] = mat[2][2] = c;
		mat[0][2] = s;
		mat[2][0] = -s;
		mat[1][1] = 1.0;

		mat[3][3] = 1.0;

		return mat;
	}

	inline DMatrix MRotate4_z(double theta) {
		DMatrix mat(4, 4, 0.0);
		double c, s;

		c = cos(theta); s = sin(theta);
		mat[0][0] = mat[1][1] = c;
		mat[0][1] = -s;
		mat[1][0] = s;
		mat[2][2] = 1.0;

		mat[3][3] = 1.0;

		return mat;
	}

#pragma pack(pop)
}