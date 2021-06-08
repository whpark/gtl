//////////////////////////////////////////////////////////////////////
//
// matrix.ixx: TMatrix class. from mocha
//
// PWH
// 2000.05.27 
// 2008.11.01
// 2021.06.08 from mocha.
//
//////////////////////////////////////////////////////////////////////


module;

#include <array>
#include <vector>
#include <cmath>
#include <span>

#include "gtl/_config.h"
#include "gtl/_macro.h"

//#include "opencv2/core/matx.hpp"

export module gtl:matrix;
//import :concepts;

export namespace gtl {

	template < class T, size_t m, size_t n = m >
	class TMatrix {
	public:
		static_assert(m > 0 and n > 0);
		enum {
			rows = m,
			cols = n,
		};
		using value_type = T;
		using var_type = std::array<value_type, m*n>;

		var_type mat_ {};

	public:
		constexpr TMatrix() = default;
		constexpr TMatrix(TMatrix const&) = default;
		constexpr TMatrix(TMatrix &&) = default;
		constexpr TMatrix(std::initializer_list<T>&& lst) : mat_{} {
			auto pos = lst.begin();
			for (auto& v : mat_) {
				if (pos == lst.end())
					break;
				v = *pos++;
			}
		}
		constexpr TMatrix& operator = (TMatrix const&) = default;
		constexpr TMatrix& operator = (TMatrix &&) = default;

		constexpr auto operator <=> (TMatrix const&) const = default;

		constexpr static TMatrix eye() requires (m == n) {
			TMatrix M{};
			for (size_t i{}; i < m; i++)
				M(i, i) = 1.;
			return M;
		}
		constexpr static TMatrix zeros() {
			TMatrix M{};
			return M;
		}
		constexpr static TMatrix ones() {
			TMatrix M;
			for (auto& v : M.mat_) v = 1;
			return M;
		}
		constexpr static TMatrix all(value_type value) {
			TMatrix M;
			for (auto& v : M.mat_) v = value;
			return M;
		}

		constexpr value_type& operator () (size_t row, size_t col) {
			return mat_[row*cols + col];
		}
		constexpr value_type const& operator () (size_t row, size_t col) const {
			return mat_[row*cols + col];
		}
		constexpr value_type& at(size_t row, size_t col) {
			return mat_[row*cols + col];
		}
		constexpr value_type const& at(size_t row, size_t col) const {
			return mat_[row*cols + col];
		}

		constexpr TMatrix& operator += (const TMatrix& B) { for (size_t i{}; i < m*n; i++) mat_[i] += B.mat_[i]; return *this; }
		constexpr TMatrix& operator -= (const TMatrix& B) { for (size_t i{}; i < m*n; i++) mat_[i] -= B.mat_[i]; return *this; }
		constexpr TMatrix operator + (TMatrix const& B) { return TMatrix{*this} += B; }
		constexpr TMatrix operator - (TMatrix const& B) { return TMatrix{*this} -= B; }
		constexpr TMatrix operator - () const { TMatrix B{*this}; for (auto& v : B.mat_) v = -v; return B; }
		constexpr TMatrix& operator *= (value_type const& B) { for (auto& v : mat_) v *= B; return *this; }
		constexpr TMatrix& operator /= (value_type const& B) { for (auto& v : mat_) v /= B; return *this; }
		constexpr TMatrix operator * (value_type const& B) { return TMatrix{*this} *= B; }
		constexpr TMatrix operator / (value_type const& B) { return TMatrix{*this} /= B; }
		friend TMatrix operator * (value_type a, TMatrix B) {
			return B *= a;
		}
		template < class vector >
		vector operator * (vector const& pt) const {
			//static_assert(pt.size() == rows);

			vector C{};
			for (size_t row {}; row < rows; row++) {
				for (size_t col {}; col < cols; col++) {
					C.member(row) += at(row, col) * pt.member(col);
				}
			}
			return C;
		}

		template < size_t n2 >
		constexpr TMatrix<T, m, n2> operator * (TMatrix<T, n, n2> const& B) const {
			auto const& A = *this;
			TMatrix<T, m, n2> C{};
			for (size_t i{}; i < rows; i++) {
				for (size_t j{}; j < n2; j++) {
					for (size_t k {}; k < cols; k++) {
						C(i,j) += A(i,k) * B(k,j);
					}
				}
			}
			return C;
		}

		constexpr [[nodiscard]] value_type sum(void) const {
			value_type s{};
			for (auto v : mat_) s += v;
			return s;
		}

		constexpr [[nodiscard]] TMatrix<T, n, m> trans(void) const {
			TMatrix<T, n, m> C;
			for (size_t i{}; i < rows; i++) {
				for (size_t j{}; j < cols; j++) {
					C(j,i) = at(i,j);
				}
			}
		}

		constexpr value_type determinant() const requires (m == n) {
			if constexpr (m == 1) {
				return mat_[0];
			}
			else if constexpr (m == 2) {
				return mat_[0]*mat_[3] - mat_[1]*mat_[2];	// ad-bc
			}
			else if constexpr (m == 3) {
				auto M = [&,this](size_t i0, size_t i1, size_t i2) -> value_type {
					return mat_[i0]*mat_[i1]*mat_[i2];
				};
				return M(0,4,8)+M(1,5,6)+M(2,3,7)-M(0,5,7)-M(1,3,8)-M(2,4,6);
			}
			else {
				value_type d{};
				TMatrix<T, m-1, n-1> C;
				for (size_t i {}; i < m; i++) {
					if (at(0,i) == 0.0) continue;
					for (size_t j {}; j < C.rows; j++) {
						for (size_t k {}; k < C.cols; k++) {
							C(j,k) = at(j+1, (k+i+1)%cols);
						}
					}
					d += at(0,i) * C.determinant();
				}
				return d;
			}
		}

		// Routine edited at 1996. 3. 19.
		friend void ludcmp(TMatrix& A, std::array<int, m>& index);
		friend void lubksb(TMatrix& A, TMatrix<T, 1, rows>& Col, std::array<int, m>& index);

		TMatrix inv(T tiny = 0., bool* pbOK = {}) const requires (m == n) {
			if (determinant() <= tiny) {
				if (pbOK)
					*pbOK = false;
				return eye();
			}
			constexpr auto dim = rows;
			std::array<int, rows> index{};

			//	ASSERT(det() != 0.0);
			TMatrix A{*this};
			TMatrix B{};
			TMatrix<T, 1, rows> col{};

			ludcmp(A, index);

			for (size_t j {}; j < dim; j++) {
				for(size_t i {}; i < dim; i++)
					col(0,i) = .0;
				col(0,j) = 1.0;
				lubksb(A, col, index);
				for (size_t i {}; i < dim; i++)
					B(i,j) = col(0,i);
			}

			if (pbOK)
				*pbOK = true;

			return B;
		}

		/* Routine for L-U Decomposition
		* ref> Numerical Recipes
		*/

		// Routine edited at 1996. 3. 19.
		friend void ludcmp(TMatrix& A, std::array<int, m>& index) {
			constexpr T TINY = 1.0e-20;
			constexpr auto dim = A.rows;
			size_t imax{};
			T big{}, dum{}, sum{}, temp{};

			TMatrix<T, 1, dim> vv{};
			for (size_t i {}; i < dim; i++) {
				big = .0;
				for (size_t j {}; j < dim; j++)
					if ( (temp=std::abs(A(i,j))) > big ) big = temp;
				//ASSERT(big != 0.0);
				vv(0,i) = 1.0 / big;
			}
			for (size_t j {}; j < dim; j++) {
				for (size_t i {}; i < j; i++) {
					sum = A(i,j);
					for (size_t k {}; k < i; k++)
						sum -= A(i,k) * A(k,j);
					A(i,j) = sum;
				}
				big = .0;
				for (size_t i {j}; i < dim; i++) {
					sum = A(i,j);
					for (size_t k {}; k < j; k++)
						sum -= A(i,k) * A(k,j);
					A(i,j) = sum;
					if ( (dum=vv(0,i)*std::abs(sum)) >= big ) {
						big = dum;
						imax = i;
					}
				}
				if (j != imax) {
					for (size_t k {}; k < dim; k++) {
						dum = A(imax,k);
						A(imax,k) = A(j,k);
						A(j,k) = dum;
					}
					vv(0,imax) = vv(0,j);
				}
				index[j] = imax;
				if ( A(j,j) == .0) A(j,j) = TINY;
				if ( j != (dim-1) ) {
					dum = 1.0 / A(j,j);
					for (size_t i = j+1; i < dim; i++) A(i,j) *= dum;
				}
			}
		}

		/* Routine for L-U Back Substitution
		* ref> Numerical Recipes
		*/

		// Routine edited at 1996. 3. 19.
		friend void lubksb(TMatrix& A, TMatrix<T, 1, rows>& Col, std::array<int, m>& index) {
			ptrdiff_t ii = -1;
			constexpr auto dim = A.rows;
			T sum;

			for (ptrdiff_t i {}; i < dim; i++) {
				int ip = index[i];
				sum = Col(0,ip);
				Col(0,ip) = Col(0,i);
				if (ii != -1)
					for (ptrdiff_t j = ii; j < i; j++) sum -= A(i,j) * Col(0,j);
				else if (sum != 0.) ii = i;
				Col(0,i) = sum;
			}
			for (ptrdiff_t i = dim-1; i >= 0; i--) {
				sum = Col(0,i);
				for (ptrdiff_t j = i+1; j < dim; j++) sum -= A(i,j) * Col(0,j);
				Col(0,i) = sum / A(i,i);
			}
		}
	};

}