/*
Copyright(c) 2021-2022 jvde.github@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>
#include <complex>

#include "../Library/Common.h"

namespace FFT
{
	static int log2(int x)
	{
		int y = 0;
		while (x >>= 1) y++;
		return y;
	}

	static int rev(int x,int logN)
	{
		static const int rev4[] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

		int y = 0, j;

		// 4 bits in one go
		for(j = 0; j < logN >> 2; j++)
		{
			y <<= 4; y |= rev4[x & 15]; x >>= 4;
		}

		// remaning bits
		for(int i = j << 2; i < logN; i++)
		{
			y <<= 1; y |= (x & 1); x >>= 1;
		}

		return y;
	}

	template <typename T>
	static void calcOmega(std::vector<std::complex<T>> &Omega,int N)
	{
		Omega.resize(N);

		for (int s = 0; s < N; s++)
			Omega[s] = std::polar(T(1), T(-2.0*PI) * T(s) / T(N));
	}


	// Radix-2 FFT, standard algorithm with inner loops reversed and twiddle factors pre-computed
	template <typename T>
	void fft(std::vector<std::complex<T>> &x)
	{
		std::complex<T> t;

		static std::vector<std::complex<T>> Omega;
		static int N = 0, logN;

		if(N != x.size())
		{
			N = (int)x.size();
			logN = log2(N);
			calcOmega(Omega,N);
		}

		int m = 2, m2 = 1;
		int w, r = N;

		for(int s = 0; s < logN; s++)
		{
			w = 0;
			r >>= 1;

			for(int j = 0; j < m2; j++)
			{
				const std::complex<T> &o = Omega[w];

				for(int k = 0; k < N; k += m)
				{
					t = o * x[k + j + m2];

					x[k + j + m2] = x[k + j] - t;
					x[k + j] += t;
				}

				w += r;
			}

			m2 = m;
			m <<= 1;
		}
	}
}
