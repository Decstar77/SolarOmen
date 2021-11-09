#pragma once
#include "SolarRigidBodyMK2.h"

namespace cm::MK2
{
	class VecN {
	public:
		VecN() : N(0), data(NULL) {}
		VecN(int _N);
		VecN(const VecN& rhs);
		VecN& operator = (const VecN& rhs);
		~VecN() { delete[] data; }

		float			operator[] (const int idx) const { return data[idx]; }
		float& operator[] (const int idx) { return data[idx]; }
		const VecN& operator *= (float rhs);
		VecN			operator * (float rhs) const;
		VecN			operator + (const VecN& rhs) const;
		VecN			operator - (const VecN& rhs) const;
		const VecN& operator += (const VecN& rhs);
		const VecN& operator -= (const VecN& rhs);

		float Dot(const VecN& rhs) const;
		void Zero();

	public:
		int		N;
		float* data;
	};

	inline VecN::VecN(int _N) {
		N = _N;
		data = new float[_N];
	}

	inline VecN::VecN(const VecN& rhs) {
		N = rhs.N;
		data = new float[N];
		for (int i = 0; i < N; i++) {
			data[i] = rhs.data[i];
		}
	}

	inline VecN& VecN::operator = (const VecN& rhs) {
		delete[] data;

		N = rhs.N;
		data = new float[N];
		for (int i = 0; i < N; i++) {
			data[i] = rhs.data[i];
		}
		return *this;
	}

	inline const VecN& VecN::operator *= (float rhs) {
		for (int i = 0; i < N; i++) {
			data[i] *= rhs;
		}
		return *this;
	}

	inline VecN VecN::operator * (float rhs) const {
		VecN tmp = *this;
		tmp *= rhs;
		return tmp;
	}

	inline VecN VecN::operator + (const VecN& rhs) const {
		VecN tmp = *this;
		for (int i = 0; i < N; i++) {
			tmp.data[i] += rhs.data[i];
		}
		return tmp;
	}

	inline VecN VecN::operator - (const VecN& rhs) const {
		VecN tmp = *this;
		for (int i = 0; i < N; i++) {
			tmp.data[i] -= rhs.data[i];
		}
		return tmp;
	}

	inline const VecN& VecN::operator += (const VecN& rhs) {
		for (int i = 0; i < N; i++) {
			data[i] += rhs.data[i];
		}
		return *this;
	}

	inline const VecN& VecN::operator -= (const VecN& rhs) {
		for (int i = 0; i < N; i++) {
			data[i] -= rhs.data[i];
		}
		return *this;
	}

	inline float VecN::Dot(const VecN& rhs) const {
		float sum = 0;
		for (int i = 0; i < N; i++) {
			sum += data[i] * rhs.data[i];
		}
		return sum;
	}

	inline void VecN::Zero() {
		for (int i = 0; i < N; i++) {
			data[i] = 0.0f;
		}
	}

	class MatMN {
	public:
		MatMN() : M(0), N(0), rows(0) {}
		MatMN(int M, int N);
		MatMN(const MatMN& rhs) {
			*this = rhs;
		}
		~MatMN() { delete[] rows; }

		const MatMN& operator = (const MatMN& rhs);
		const MatMN& operator *= (float rhs);
		VecN operator * (const VecN& rhs) const;
		MatMN operator * (const MatMN& rhs) const;
		MatMN operator * (const float rhs) const;

		void Zero();
		MatMN Transpose() const;

	public:
		int		M;	// M rows
		int		N;	// N columns
		VecN* rows;
	};

	inline MatMN::MatMN(int _M, int _N) {
		M = _M;
		N = _N;
		rows = new VecN[M];
		for (int m = 0; m < M; m++) {
			rows[m] = VecN(N);
		}
	}

	inline const MatMN& MatMN::operator = (const MatMN& rhs) {
		M = rhs.M;
		N = rhs.N;
		rows = new VecN[M];
		for (int m = 0; m < M; m++) {
			rows[m] = rhs.rows[m];
		}
		return *this;
	}

	inline const MatMN& MatMN::operator *= (float rhs) {
		for (int m = 0; m < M; m++) {
			rows[m] *= rhs;
		}
		return *this;
	}

	inline VecN MatMN::operator * (const VecN& rhs) const {
		// Check that the incoming vector is of the correct dimension
		if (rhs.N != N) {
			return rhs;
		}

		VecN tmp(M);
		for (int m = 0; m < M; m++) {
			tmp[m] = rhs.Dot(rows[m]);
		}
		return tmp;
	}

	inline MatMN MatMN::operator * (const MatMN& rhs) const {
		// Check that the incoming matrix of the correct dimension
		if (rhs.M != N && rhs.N != M) {
			return rhs;
		}

		MatMN tranposedRHS = rhs.Transpose();

		MatMN tmp(M, rhs.N);
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < rhs.N; n++) {
				tmp.rows[m][n] = rows[m].Dot(tranposedRHS.rows[n]);
			}
		}
		return tmp;
	}

	inline MatMN MatMN::operator * (const float rhs) const {
		MatMN tmp = *this;
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				tmp.rows[m][n] *= rhs;
			}
		}
		return tmp;
	}

	inline void MatMN::Zero() {
		for (int m = 0; m < M; m++) {
			rows[m].Zero();
		}
	}

	inline MatMN MatMN::Transpose() const {
		MatMN tmp(N, M);
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				tmp.rows[n][m] = rows[m][n];
			}
		}
		return tmp;
	}

	class MatN {
	public:
		MatN() : numDimensions(0) {}
		MatN(int N);
		MatN(const MatN& rhs) {
			*this = rhs;
		}
		MatN(const MatMN& rhs) {
			*this = rhs;
		}
		~MatN() { delete[] rows; }

		const MatN& operator = (const MatN& rhs);
		const MatN& operator = (const MatMN& rhs);

		void Identity();
		void Zero();
		void Transpose();

		void operator *= (float rhs);
		VecN operator * (const VecN& rhs);
		MatN operator * (const MatN& rhs);

	public:
		int		numDimensions;
		VecN* rows;
	};

	inline MatN::MatN(int N) {
		numDimensions = N;
		rows = new VecN[N];
		for (int i = 0; i < N; i++) {
			rows[i] = VecN(N);
		}
	}

	inline const MatN& MatN::operator = (const MatN& rhs) {
		numDimensions = rhs.numDimensions;
		rows = new VecN[numDimensions];
		for (int i = 0; i < numDimensions; i++) {
			rows[i] = rhs.rows[i];
		}
		return *this;
	}

	inline const MatN& MatN::operator = (const MatMN& rhs) {
		if (rhs.M != rhs.N) {
			return *this;
		}

		numDimensions = rhs.N;
		rows = new VecN[numDimensions];
		for (int i = 0; i < numDimensions; i++) {
			rows[i] = rhs.rows[i];
		}
		return *this;
	}

	inline void MatN::Zero() {
		for (int i = 0; i < numDimensions; i++) {
			rows[i].Zero();
		}
	}

	inline void MatN::Identity() {
		for (int i = 0; i < numDimensions; i++) {
			rows[i].Zero();
			rows[i][i] = 1.0f;
		}
	}

	inline void MatN::Transpose() {
		MatN tmp(numDimensions);

		for (int i = 0; i < numDimensions; i++) {
			for (int j = 0; j < numDimensions; j++) {
				tmp.rows[i][j] = rows[j][i];
			}
		}

		*this = tmp;
	}

	inline void MatN::operator *= (float rhs) {
		for (int i = 0; i < numDimensions; i++) {
			rows[i] *= rhs;
		}
	}

	inline VecN MatN::operator * (const VecN& rhs) {
		VecN tmp(numDimensions);

		for (int i = 0; i < numDimensions; i++) {
			tmp[i] = rows[i].Dot(rhs);
		}

		return tmp;
	}

	inline MatN MatN::operator * (const MatN& rhs) {
		MatN tmp(numDimensions);
		tmp.Zero();

		for (int i = 0; i < numDimensions; i++) {
			for (int j = 0; j < numDimensions; j++) {
				tmp.rows[i][j] += rows[i][j] * rhs.rows[j][i];
			}
		}

		return tmp;
	}

	enum class ConstraintType
	{
		DISTANCE,
	};

	class Constraint
	{
	public:
		ConstraintType type;

		RigidBody* bodyA;
		RigidBody* bodyB;

		Vec3f anchorA; // The anchor location in bodyA's space
		//Vec3f axisA; // The axis direction in bodyA's space

		Vec3f anchorB; // The anchor location in bodyB's space
		//Vec3f axisB; // The axis direction in bodyB's space				

		MatMN GetInverseMassMatrix() const;
		VecN GetVelocities() const;
		void ApplyImpulses(const VecN& impulses);
	};

	class ConstraintDistance : public Constraint
	{
	public:
		ConstraintDistance() : Constraint(),
			m_cachedLambda(1),
			m_Jacobian(1, 12)
		{
			m_cachedLambda.Zero();
			//m_baumgarte = 0.0f;
		}

		MatMN m_Jacobian;
		VecN m_cachedLambda;
		//float m_baumgarte;

		void PreSolve(const real32 dt_sec);
		void Solve();
		void PostSolve();
	};

}