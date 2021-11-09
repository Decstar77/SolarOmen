#pragma once

#include "../core/SolarCore.h"

namespace cm
{
	/*
	====================================================
	VecN
	====================================================
	*/

	// @TODO: Remove allocation
	class VecN
	{
	public:
		int32 count;
		real32* data;

	public:
		VecN() : count(0), data(NULL) {}
		VecN(int32 _N);
		VecN(const VecN& rhs);
		VecN& operator = (const VecN& rhs);
		~VecN() { delete[] data; }

		real32  operator[] (const int32 idx) const { return data[idx]; }
		real32& operator[] (const int32 idx) { return data[idx]; }
		const VecN& operator *= (real32 rhs);
		VecN		operator * (real32 rhs) const;
		VecN		operator + (const VecN& rhs) const;
		VecN		operator - (const VecN& rhs) const;
		const VecN& operator += (const VecN& rhs);
		const VecN& operator -= (const VecN& rhs);

		real32 Dot(const VecN& rhs) const;
		void Zero();
	};

	inline VecN::VecN(int32 _N)
	{
		count = _N;
		data = new real32[_N];
	}

	inline VecN::VecN(const VecN& rhs)
	{
		count = rhs.count;
		data = new real32[count];
		for (int32 i = 0; i < count; i++)
		{
			data[i] = rhs.data[i];
		}
	}

	inline VecN& VecN::operator = (const VecN& rhs)
	{
		delete[] data;

		count = rhs.count;
		data = new real32[count];
		for (int32 i = 0; i < count; i++)
		{
			data[i] = rhs.data[i];
		}

		return *this;
	}

	inline const VecN& VecN::operator *= (real32 rhs)
	{
		for (int32 i = 0; i < count; i++)
		{
			data[i] *= rhs;
		}

		return *this;
	}

	inline VecN VecN::operator * (real32 rhs) const
	{
		VecN tmp = *this;
		tmp *= rhs;

		return tmp;
	}

	inline VecN VecN::operator + (const VecN& rhs) const
	{
		VecN tmp = *this;
		for (int32 i = 0; i < count; i++)
		{
			tmp.data[i] += rhs.data[i];
		}

		return tmp;
	}

	inline VecN VecN::operator - (const VecN& rhs) const
	{
		VecN tmp = *this;
		for (int32 i = 0; i < count; i++)
		{
			tmp.data[i] -= rhs.data[i];
		}

		return tmp;
	}

	inline const VecN& VecN::operator += (const VecN& rhs)
	{
		for (int32 i = 0; i < count; i++)
		{
			data[i] += rhs.data[i];
		}

		return *this;
	}

	inline const VecN& VecN::operator -= (const VecN& rhs)
	{
		for (int32 i = 0; i < count; i++)
		{
			data[i] -= rhs.data[i];
		}

		return *this;
	}

	inline real32 VecN::Dot(const VecN& rhs) const
	{
		real32 sum = 0;
		for (int32 i = 0; i < count; i++)
		{
			sum += data[i] * rhs.data[i];
		}

		return sum;
	}

	inline void VecN::Zero()
	{
		for (int32 i = 0; i < count; i++)
		{
			data[i] = 0.0f;
		}
	}

	/*
	====================================================
	MatN
	====================================================
	*/

	class MatMN
	{
	public:
		int32 M;	// M rows
		int32 N;	// N columns
		VecN* rows;

	public:
		MatMN() : M(0), N(0), rows(nullptr) {}
		MatMN(int32 M, int32 N);
		MatMN(const MatMN& rhs) {
			*this = rhs;
		}
		~MatMN() { delete[] rows; }

		const MatMN& operator = (const MatMN& rhs);
		const MatMN& operator *= (real32 rhs);
		VecN operator * (const VecN& rhs) const;
		MatMN operator * (const MatMN& rhs) const;
		MatMN operator * (const real32 rhs) const;

		void Zero();
		MatMN Transpose() const;
	};

	inline MatMN::MatMN(int32 _M, int32 _N)
	{
		M = _M;
		N = _N;
		rows = new VecN[M];
		for (int32 m = 0; m < M; m++)
		{
			rows[m] = VecN(N);
		}
	}

	inline const MatMN& MatMN::operator = (const MatMN& rhs)
	{
		M = rhs.M;
		N = rhs.N;
		rows = new VecN[M];
		for (int32 m = 0; m < M; m++)
		{
			rows[m] = rhs.rows[m];
		}

		return *this;
	}

	inline const MatMN& MatMN::operator *= (real32 rhs)
	{
		for (int32 m = 0; m < M; m++)
		{
			rows[m] *= rhs;
		}

		return *this;
	}

	inline VecN MatMN::operator * (const VecN& rhs) const
	{
		// Check that the incoming vector is of the correct dimension
		if (rhs.count != N)
		{
			return rhs;
		}

		VecN tmp(M);
		for (int32 m = 0; m < M; m++)
		{
			tmp[m] = rhs.Dot(rows[m]);
		}

		return tmp;
	}

	inline MatMN MatMN::operator * (const MatMN& rhs) const
	{
		// Check that the incoming matrix of the correct dimension
		if (rhs.M != N && rhs.N != M)
		{
			return rhs;
		}

		MatMN tranposedRHS = rhs.Transpose();

		MatMN tmp(M, rhs.N);
		for (int32 m = 0; m < M; m++)
		{
			for (int32 n = 0; n < rhs.N; n++)
			{
				tmp.rows[m][n] = rows[m].Dot(tranposedRHS.rows[n]);
			}
		}

		return tmp;
	}

	inline MatMN MatMN::operator * (const real32 rhs) const
	{
		MatMN tmp = *this;
		for (int32 m = 0; m < M; m++)
		{
			for (int32 n = 0; n < N; n++)
			{
				tmp.rows[m][n] *= rhs;
			}
		}

		return tmp;
	}

	inline void MatMN::Zero()
	{
		for (int32 m = 0; m < M; m++)
		{
			rows[m].Zero();
		}
	}

	inline MatMN MatMN::Transpose() const
	{

		MatMN tmp(N, M);
		for (int32 m = 0; m < M; m++)
		{
			for (int32 n = 0; n < N; n++)
			{
				tmp.rows[n][m] = rows[m][n];
			}
		}

		return tmp;
	}

	/*
	====================================================
	MatN
	====================================================
	*/

	class MatN
	{
	public:
		int32 numDimensions;
		VecN* rows;

	public:
		MatN() : numDimensions(0), rows(nullptr) {}
		MatN(int32 N);
		MatN(const MatN& rhs)
		{
			*this = rhs;
		}
		MatN(const MatMN& rhs)
		{
			*this = rhs;
		}
		~MatN() { delete[] rows; }

		const MatN& operator = (const MatN& rhs);
		const MatN& operator = (const MatMN& rhs);

		void Identity();
		void Zero();
		void Transpose();

		void operator *= (real32 rhs);
		VecN operator * (const VecN& rhs);
		MatN operator * (const MatN& rhs);

	};

	inline MatN::MatN(int32 N)
	{
		numDimensions = N;
		rows = new VecN[N];
		for (int32 i = 0; i < N; i++)
		{
			rows[i] = VecN(N);
		}
	}

	inline const MatN& MatN::operator = (const MatN& rhs)
	{
		numDimensions = rhs.numDimensions;
		rows = new VecN[numDimensions];
		for (int32 i = 0; i < numDimensions; i++)
		{
			rows[i] = rhs.rows[i];
		}

		return *this;
	}

	inline const MatN& MatN::operator = (const MatMN& rhs)
	{
		if (rhs.M != rhs.N)
		{
			return *this;
		}

		numDimensions = rhs.N;
		rows = new VecN[numDimensions];
		for (int32 i = 0; i < numDimensions; i++)
		{
			rows[i] = rhs.rows[i];
		}

		return *this;
	}

	inline void MatN::Zero()
	{
		for (int32 i = 0; i < numDimensions; i++)
		{
			rows[i].Zero();
		}
	}

	inline void MatN::Identity()
	{
		for (int32 i = 0; i < numDimensions; i++)
		{
			rows[i].Zero();
			rows[i][i] = 1.0f;
		}
	}

	inline void MatN::Transpose()
	{
		MatN tmp(numDimensions);

		for (int32 i = 0; i < numDimensions; i++)
		{
			for (int32 j = 0; j < numDimensions; j++)
			{
				tmp.rows[i][j] = rows[j][i];
			}
		}

		*this = tmp;
	}

	inline void MatN::operator *= (real32 rhs)
	{
		for (int32 i = 0; i < numDimensions; i++)
		{
			rows[i] *= rhs;
		}
	}

	inline VecN MatN::operator * (const VecN& rhs)
	{
		VecN tmp(numDimensions);

		for (int32 i = 0; i < numDimensions; i++)
		{
			tmp[i] = rows[i].Dot(rhs);
		}

		return tmp;
	}

	inline MatN MatN::operator * (const MatN& rhs)
	{
		MatN tmp(numDimensions);
		tmp.Zero();

		for (int32 i = 0; i < numDimensions; i++)
		{
			for (int32 j = 0; j < numDimensions; j++)
			{
				tmp.rows[i][j] += rows[i][j] * rhs.rows[j][i];
			}
		}

		return tmp;
	}

	inline VecN GaussSeidel(const MatN& A, const VecN& b)
	{
		int32 N = b.count;
		VecN x(N);
		x.Zero();

		for (int32 iter = 0; iter < N; iter++)
		{
			for (int32 i = 0; i < N; i++)
			{
				real32 dx = (b[i] - A.rows[i].Dot(x)) / A.rows[i][i];
				if (dx * 0.0f == dx * 0.0f)
				{
					x[i] = x[i] + dx;
				}
			}
		}

		return x;
	}

}
