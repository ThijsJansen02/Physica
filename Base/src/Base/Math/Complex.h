#pragma once

#include <Base/Base.h>
#include <math.h>

namespace PH::Base {


	template<typename T>
	struct Complex {
		T real;
		T imag;

		Complex<T> conjugate() {
			return { real, -imag };
		}

		Complex<T>(T real) 
			: real(real), imag(0.0f) {
		}

		Complex<T>(T real, T imag)
			: real(real), imag(imag) {
		}

		T arg() {

			DebugBreak(); //did not implement a atan function for this typename
			//atan2f(imag, real);
		}

		T modulus_squared() {
			return (*this * this->conjugate()).real;
		}

		T modulus() {
			DebugBreak(); //did not implement this function for this typename 
		}

		static Complex<T> i() { 
			return { 0.0f, 1.0f };
		}
	};

	real32 Complex<real32>::arg() {
		return atan2f(imag, real);
	}

	real64 Complex<real64>::arg() {
		return atan2(imag, real);
	}

	real32 Complex<real32>::modulus() {
		return sqrtf(modulus_squared());
	}

	real64 Complex<real64>::modulus() {
		return sqrt(modulus_squared());
	}
	
	template<typename leftT, typename rightT>
	Complex<rightT> operator*(leftT left, Complex<rightT> right) {
		return { left * right.real, left * right.imag };
	}

	template<typename leftT, typename rightT>
	Complex<leftT> operator+(Complex<leftT> left, rightT right) {
		return { left.real + right, left.imag };
	}


	template<typename leftT, typename rightT>
	Complex<leftT> operator+(leftT left, Complex<rightT>  right) {
		return right + left;
	}

	template<typename leftT, typename rightT>
	Complex<leftT> operator-(Complex<leftT> left, rightT right) {
		return left + -1.0f * right;
	}

	template<typename T>
	Complex<T> operator+(Complex<T> left, Complex<T> right) {
		return { left.real + right.real, left.imag - right.imag };
	}

	template<typename T>
	Complex<T> operator-(Complex<T> left, Complex<T> right) {
		return left - (-1.0f * right);
	}

	template<typename T>
	Complex<T> operator/(Complex<T> left, T right) {
		return { left.real / right, left.imag / right };
	}

	template<typename T>
	Complex<T> operator*(Complex<T> left, Complex<T> right) {
		return { left.real * right.real - left.imag * right.imag, left.imag * right.real + left.real * right.imag };
	}

	template<typename T>
	Complex<T> operator/(Complex<T> left, Complex<T> right) {

		T devisor = (right * right.conjugate()).real;
		return (left * right.conjugate()) / devisor;

	}


}