#include <stdint.h>

#define R_UNS_INF  0b00000110
#define R_POS_INF  0b00000100
#define R_NEG_INF  0b00000010
#define R_NOT_INF  0b00000000
#define R_NAN      0b00001000
#define R_OVERFLOW 0b00000001
#define R_DIM_MISMATCH 0b0001000

#include <stdio.h>

namespace ril
{
	char __rint_flags;//0|0|0|dim_mismatch|nan|posinf|neginf|overflow
}

template<uint32_t size>
//last bit of last byte least significant
class __rint
{
private:
	char val[size];
public:
	__rint() {};
	template <typename inptype> __rint(inptype value)
	{
		set(value);
	}

	__rint<size> copy() const
	{
		//__rint<size> r();
		//for (int n = size; n;)
		//{
		//	--n;
		//	r.val[n] = val[n];
		//}
		//return r;
		return *this;
	}
	const __rint<size>& set(__rint<size>arg)
	{
		//for (int n = size; n;)
		//{
		//	--n;
		//	val[n] = arg.val[n];
		//}
		//return *this;
		return (*this = arg);
	}
	template <typename inptype> const __rint<size>& set(inptype value)
	{
		for (int n = size; n;)
		{
			--n;
			val[n] = 0;
		}
		for (int n = size; n && value;)
		{
			--n;
			val[n] = value & 0xff;
			value >>= 8;
		}
		return *this;
	}

	void print()
	{
		for (int n = 0; n < size; n++)
		{
			printf("%02hhx|",(char)val[n]);
		}
		printf("\n");
	}
	bool _bool();
	int _int()
	{
		return _int32();
	}
	int8_t _int8()
	{
		return (int8_t)val[size-1];
	}
	int16_t _int16()
	{
		return (int16_t)val[size-2] + (int16_t)val[size-1];
	}
	int32_t _int32()
	{
		return (int32_t)val[size-1] + (int32_t)val[size-2] * 256 + (int32_t)val[size-3] * 256 * 256 + (int32_t)val[size-4] * 256 * 256 * 256;
	}
	int64_t _int64();

	const __rint<size>& _inc()
	{
		for (int n = size; n;)
		{
			--n;
			for (char bit = 0x01; bit; bit <<= 1)
			{
				if ((~val[n]) & bit)
				{
					val[n] |= bit;
					return *this;
				}
				else
				{
					val[n] &= ~bit;
				}
			}
		}
		for (int n = size - 1; n; --n)
			val[n] = 0x00;
		return *this;
	}
	const __rint<size>& _dec()
	{
		for (int n = size; n;)
		{
			--n;
			for (char bit = 0x01; bit; bit <<= 1)
			{
				if (val[n] & bit)
				{
					val[n] &= ~bit;
					return *this;
				}
				else
				{
					val[n] |= bit;
				}
			}
		}
		for (int n = size - 1; n; --n)
			val[n] = 0xff;
		return *this;
	}
	const __rint<size>& _addto(__rint<size>arg)
	{
		char carrybit = 0x00;
		char tempcarrybit = 0x00;

		for (int n = size; n;)
		{
			--n;
			//printf(" -%d\n", n);
			tempcarrybit = 0x01 & ((val[n] & arg.val[n]) | (val[n] & carrybit) | (carrybit & arg.val[n]));
			//val[n] = (val[n] & 0xfe) | (bit & (val[n] ^ arg.val[n] ^ carrybit));
			val[n] ^= 0x01 & (arg.val[n] ^ carrybit);
			//printf("  |%02hhx|%02hhx|%02hhx\n", 0x01, carrybit, val[n]);
			carrybit = tempcarrybit;
			for (char bit = 0x02; bit; bit <<= 1)
			{
				carrybit <<= 1;
				
				tempcarrybit = bit & ((val[n] & arg.val[n]) | (val[n] & carrybit) | (carrybit & arg.val[n]));
				//val[n] = (val[n] & ~bit) | (bit & (val[n] ^ arg.val[n] ^ carrybit));
				val[n] ^= bit & (arg.val[n] ^ carrybit);
				//printf("  |%02hhx|%02hhx|%02hhx\n",bit,carrybit, val[n]);
				carrybit = tempcarrybit;
			}
			carrybit >>= 7;
		}
		return *this;
	}
	const __rint<size>& _subto(__rint<size>arg)
	{
		arg._negto();
		_addto(arg);
		/*char carrybit = 0x00;
		char tempcarrybit = 0x00;

		for (int n = size - 1; n; --n)
		{
			tempcarrybit = 0x01 & ((val[n] & arg.val[n]) | (val[n] & carrybit) | (carrybit & arg.val[n]));
			val[n] = (val[n] & 0xfe) | (val[n] ^ arg.val[n] ^ carrybit);
			carrybit = tempcarrybit;
			for (char bit = 0x02; bit; bit <<= 1)
			{
				carrybit <<= 1;
				tempcarrybit = bit & ((val[n] & arg.val[n]) | (val[n] & carrybit) | (carrybit & arg.val[n]));
				val[n] ^= bit & (arg.val[n] ^ carrybit);
				carrybit = tempcarrybit;
			}
			carrybit >>= 7;
		}*/
	}
	__rint<size> _add(__rint<size>arg) const
	{
		return arg._addto(*this);
	}
	__rint<size> _sub(__rint<size>arg) const
	{
		return arg._subto(*this);
	}
	__rint<size> _mul(__rint<size>arg) const
	{
		__rint<size>prod;
		__rint<size>_this = *this;

		for (; arg._bool(); arg >> 1)
		{
			_this << 1;
			if (arg.val[0] & 0x01)
				prod.addto(_this);
		}
		return prod;
	}
	__rint<size> _div(__rint<size>) const;
	__rint<size> _pow(__rint<size>) const;
	const __rint<size>& _multo(__rint<size>arg)
	{
		return (*this = this->_mul(arg));
	}
	const __rint<size>& _divto(__rint<size>arg);
	const __rint<size>& _powto(__rint<size>arg);
	const __rint<size>& _bitandto(__rint<size> arg)
	{
		for (int n = size - 1; n; --n)
			val[n] &= arg.val[n];
		return *this;
	}
	const __rint<size>& _bitorto(__rint<size>arg)
	{
		for (int n = size - 1; n; --n)
			val[n] |= arg.val[n];
		return *this;
	}
	const __rint<size>& _bitxorto(__rint<size>arg)
	{
		for (int n = size - 1; n; --n)
			val[n] ^= arg.val[n];
		return *this;
	}
	const __rint<size>& _bitnotto()
	{
		for (int n = size; n;)
		{
			--n;
			val[n] = ~val[n];
		}
	}
	__rint<size> _bitand(__rint<size>arg) const
	{
		return arg._bitandto(*this);
	}
	__rint<size> _bitor(__rint<size>arg) const
	{
		return arg._bitorto(*this);
	}
	__rint<size> _bitxor(__rint<size>arg) const
	{
		return arg._bitxorto(*this);
	}
	__rint<size> _bitnot() const
	{
		__rint<size> n = *this;
		return n._bitnotto();
	}
	__rint<size> _leftshift(__rint<size>arg) const
	{
		__rint<size> l = *this;
		return l._leftshiftto(arg);
	}
	__rint<size>& _rightshift(__rint<size>arg) const
	{
		__rint<size> r = *this;
		return r._rightshiftto(arg);
	}
	const __rint<size> _leftshiftto(__rint<size>arg)
	{
		if (arg.val[0] & 0x80)//negative arg
		{
			return _rightshiftto(arg._negto());
		}
		uint32_t byte = (arg._int32() / 8);
		uint32_t carbyte = size - byte;
		byte = byte;
		uint8_t bit = (arg._int8() % 8);
		uint8_t carbit = 8 - bit;


		for (uint32_t n = 0; n<carbyte; ++n)
		{
			puts("iter");
			printf("%02hhx <- %02hhx\n",val[n],val[n+byte]);
			val[n] = val[n + byte];
		}
		for (uint32_t n = carbyte; n < size; ++n)
		{
			puts("iter2");
			val[n] = 0x00;
		}

		for (uint32_t n = 0; n<size-1; ++n)
		{
			//printf("(%02hhx)", val[n]);
			val[n] <<= bit;
			//printf("%02hhx|",val[n]);
			val[n] &= val[n + 1] >> carbit;
		}
		//printf("(%02hhx)", val[size - 1]);
		val[size - 1] <<= bit;
		//printf("%02hhx\n", val[size - 1]);

		return *this;
	}
	const __rint<size>& _rightshiftto(__rint<size>arg)
	{
		if (arg.val[0] & 0x80)//negative arg
		{
			return _rightshiftto(arg._negto());
		}
		uint32_t byte = size - (arg._int32() / 8);
		if (byte >= size)
		{
			return set(0);
		}
		uint8_t bit = (arg._int8() % 8);
		uint8_t carbit = 8 - bit;

		for (uint32_t n = size; n>byte;)
		{
			--n;
			val[n] = val[n - byte];
		}

		for (uint32_t n = size-1; n; --n)
		{
			val[n] >>= bit;
			val[n] &= val[n - 1] << carbit;
		}
		val[0] >>= bit;

		return *this;
	}
	bool _gr(__rint<size>) const;
	bool _le(__rint<size>) const;
	bool _eq(__rint<size>) const;
	bool _greq(__rint<size>) const;
	bool _leeq(__rint<size>) const;
	__rint<size> _neg() const { return (this->_bitnot()). _inc(); }
	const __rint<size>& _negto() { _bitnotto(); return _inc(); }


	bool abs();
};

typedef __rint<sizeof(int)> rint;
typedef __rint<1> rint8;
typedef __rint<2> rint16;
typedef __rint<3> rint32;
typedef __rint<4> rint64;
typedef __rint<8> rint128;

#include <stdio.h>
int main()
{
	printf("%d\n", sizeof(rint));
	rint a(5);
	a.print();
	rint b(6);
	b.print();
	rint c(a._add(b));
	c.print();
	rint d(a);
	d._inc();
	d._dec();
	d.print();
	rint e(b._neg());
	e.print();
	rint f(b._add(e));
	f.print();
	rint g(a._leftshift(8));
	g.print();
	puts("");
	a.print();
	b.print();
	c.print();
	d.print();
	e.print();
	f.print();
	g.print();
}
