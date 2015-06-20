// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    rgbsse.h

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#ifndef __RGBSSE__
#define __RGBSSE__

#include <emmintrin.h>

/***************************************************************************
    TABLES
***************************************************************************/

extern const struct _rgbsse_statics
{
	__m128  dummy_for_alignment;
	INT16   maxbyte[8];
	INT16	alpha_mask[8];
	INT16	red_mask[8];
	INT16	green_mask[8];
	INT16	blue_mask[8];
	INT16   scale_table[256][8];
} rgbsse_statics;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbint_t
{
public:
	inline rgbint_t() { }
	inline rgbint_t(UINT32 rgb) { set_rgb(rgb); }
	inline rgbint_t(UINT32 r, UINT32 g, UINT32 b) { set_rgb(r, g, b); }
	inline rgbint_t(rgb_t& rgb) { m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128()); }

	inline void set(void* value) { m_value = *(__m128i*)value; }
	inline __m128i get() { return m_value; }
	inline void set(__m128i value) { m_value = value; }
	inline void set(rgbint_t& other) { m_value = other.m_value; }
	inline void set_rgb(UINT32 rgb) { m_value = _mm_and_si128(_mm_set1_epi32(0xff), _mm_set_epi32(0, rgb, rgb >> 8, rgb >> 16)); }
	inline void set_rgb(INT32 r, INT32 g, INT32 b) { m_value = _mm_set_epi32(0, r, g, b); }
	inline void set_rgb(rgb_t& rgb) { m_value = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128()), _mm_setzero_si128()); }

	inline rgb_t to_rgb()
	{
		__m128i anded = _mm_and_si128(m_value, _mm_set1_epi32(0x000000ff));
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(anded, anded), _mm_setzero_si128()));
	}

	inline rgb_t to_rgb_clamp()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, m_value), _mm_setzero_si128()));
	}

	inline rgb_t to_rgba()
	{
		return to_rgb();
	}

	inline rgb_t to_rgba_clamp()
	{
		return to_rgb_clamp();
	}

	inline void add(const rgbint_t& color2)
	{
		m_value = _mm_add_epi32(m_value, color2.m_value);
	}

	inline void add_imm(const INT32 imm)
	{
		__m128i temp = _mm_set1_epi32(imm);
		m_value = _mm_add_epi32(m_value, temp);
	}

	inline void add_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i temp = _mm_set_epi32(0, r, g, b);
		m_value = _mm_add_epi32(m_value, temp);
	}

	inline void sub(const rgbint_t& color2)
	{
		m_value = _mm_sub_epi32(m_value, color2.m_value);
	}

	inline void sub_imm(const INT32 imm)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void sub_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set_epi32(0, r, g, b));
	}

	inline void subr(rgbint_t& color2)
	{
		m_value = _mm_sub_epi32(color2.m_value, m_value);
	}

	inline void subr_imm(const INT32 imm)
	{
		m_value = _mm_sub_epi32(_mm_set1_epi32(imm), m_value);
	}

	inline void subr_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i temp = _mm_set_epi32(0, r, g, b);
		m_value = _mm_sub_epi32(temp, m_value);
	}

	inline void set_r(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.red_mask), _mm_set_epi32(0, value, 0, 0));
	}

	inline void set_g(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.green_mask), _mm_set_epi32(0, 0, value, 0));
	}

	inline void set_b(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.blue_mask), _mm_set_epi32(0, 0, 0, value));
	}

	inline UINT8 get_r()
	{
		return _mm_extract_epi16(m_value, 4) & 0xff;
	}

	inline UINT8 get_g()
	{
		return _mm_extract_epi16(m_value, 2) & 0xff;
	}

	inline UINT8 get_b()
	{
		return _mm_extract_epi16(m_value, 0) & 0xff;
	}

	inline UINT16 get_r32()
	{
		return _mm_extract_epi16(m_value, 4);
	}

	inline UINT16 get_g32()
	{
		return _mm_extract_epi16(m_value, 2);
	}

	inline UINT16 get_b32()
	{
		return _mm_extract_epi16(m_value, 0);
	}

	inline void mul(rgbint_t& color)
	{
		__m128i tmp1 = _mm_mul_epu32(m_value, color.m_value);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(color.m_value, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm(const INT32 imm)
	{
		__m128i immv = _mm_set1_epi32(imm);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i immv = _mm_set_epi32(0, r, g, b);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void shl(const rgbint_t& shift)
	{
		m_value = _mm_sll_epi32(m_value, shift.m_value);
	}

	inline void shl_imm(const UINT8 shift)
	{
		m_value = _mm_slli_epi32(m_value, shift);
	}

	inline void shl_imm_all(const UINT8 shift)
	{
		m_value = _mm_slli_si128(m_value, shift >> 3);
	}

	inline void shr(const rgbint_t& shift)
	{
		m_value = _mm_srl_epi32(m_value, shift.m_value);
	}

	inline void shr_imm(const UINT8 shift)
	{
		m_value = _mm_srli_epi32(m_value, shift);
	}

	inline void shr_imm_all(const UINT8 shift)
	{
		m_value = _mm_srli_si128(m_value, shift >> 3);
	}

	inline void sra(const rgbint_t& shift)
	{
		m_value = _mm_sra_epi32(m_value, shift.m_value);
	}

	inline void sra_imm(const UINT8 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
	}

	inline void or_reg(const rgbint_t& color2)
	{
		m_value = _mm_or_si128(m_value, color2.m_value);
	}

	inline void or_imm(const INT32 value)
	{
		m_value = _mm_or_si128(m_value, _mm_set1_epi32(value));
	}

	inline void and_reg(const rgbint_t& color)
	{
		m_value = _mm_and_si128(m_value, color.m_value);
	}

	inline void and_imm(const INT32 value)
	{
		m_value = _mm_and_si128(m_value, _mm_set1_epi32(value));
	}

	inline void and_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_and_si128(m_value, _mm_set_epi32(0xffffffff, r, g, b));
	}

	inline void xor_reg(const rgbint_t& color2)
	{
		m_value = _mm_xor_si128(m_value, color2.m_value);
	}

	inline void xor_imm(const INT32 value)
	{
		m_value = _mm_xor_si128(m_value, _mm_set1_epi32(value));
	}

	inline void clamp_and_clear(const rgbint_t& color, const INT32 sign)
	{
		__m128i vsign = _mm_set1_epi32(sign);
		m_value = _mm_and_si128(color.m_value, _mm_cmpeq_epi32(_mm_and_si128(color.m_value, vsign), _mm_setzero_si128()));
		vsign = _mm_srai_epi32(vsign, 1);
		vsign = _mm_xor_si128(vsign, _mm_set1_epi32(0xffffffff));
		__m128i mask = _mm_cmpgt_epi32(m_value, vsign); // m_value > vsign ? 0xffffffff : 0
		m_value = _mm_or_si128(_mm_and_si128(vsign, mask), _mm_and_si128(m_value, _mm_xor_si128(mask, _mm_set1_epi32(0xffffffff))));
	}

	inline void min(const INT32 value)
	{
		__m128i val = _mm_set1_epi32(value);
		__m128i mask = _mm_cmpgt_epi32(m_value, val); // m_value > value ? 0xffffffff : 0
		m_value = _mm_or_si128(_mm_and_si128(val, mask), _mm_and_si128(m_value, _mm_xor_si128(mask, _mm_set1_epi32(0xffffffff))));
	}

	void blend(const rgbint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbint_t& scale);
	void scale_imm_and_clamp(const INT16 scale);
	void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other, const rgbint_t& scale2);
	void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other);
	void scale_imm_add_and_clamp(const INT16 scale, const rgbint_t& other);

	void max(const rgbint_t& max);

	inline void sign_extend(const INT32 compare, const INT32 sign)
	{
		__m128i compare_vec = _mm_set1_epi32(compare);
		__m128i compare_mask = _mm_cmpeq_epi32(_mm_and_si128(m_value, compare_vec), compare_vec);
		__m128i compared = _mm_and_si128(_mm_set1_epi32(sign), compare_mask);
		m_value = _mm_or_si128(m_value, compared);
	}

	void print();

	inline void cmpeq(const rgbint_t& value)
	{
		m_value = _mm_cmpeq_epi32(m_value, value.m_value);
	}

	inline void cmpeq_imm(const UINT32 value)
	{
		m_value = _mm_cmpeq_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmpgt(const rgbint_t& value)
	{
		m_value = _mm_cmpgt_epi32(m_value, value.m_value);
	}

	inline void cmplt(const rgbint_t& value)
	{
		m_value = _mm_cmplt_epi32(m_value, value.m_value);
	}

	inline rgbint_t operator=(const rgbint_t& other)
	{
		m_value = other.m_value;
		return *this;
	}

	inline rgbint_t& operator+=(const rgbint_t& other)
	{
		m_value = _mm_add_epi32(m_value, other.m_value);
		return *this;
	}

	inline rgbint_t& operator+=(const INT32 other)
	{
		m_value = _mm_add_epi32(m_value, _mm_set1_epi32(other));
		return *this;
	}

	inline rgbint_t& operator-=(const rgbint_t& other)
	{
		m_value = _mm_sub_epi32(m_value, other.m_value);
		return *this;
	}

	inline rgbint_t& operator*=(const rgbint_t& other)
	{
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, other.m_value), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(other.m_value, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
		return *this;
	}

	inline rgbint_t& operator*=(const INT32 other)
	{
		const __m128i immv = _mm_set1_epi32(other);
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, immv), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
		return *this;
	}

	inline rgbint_t& operator>>=(const INT32 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
		return *this;
	}

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v);

protected:
	__m128i	m_value;

private:
	rgbint_t(__m128i value);
};

class rgbaint_t : public rgbint_t
{
public:
	inline rgbaint_t() { }
	inline rgbaint_t(UINT32 rgba) { m_value = _mm_and_si128(_mm_set1_epi32(0xff), _mm_set_epi32(rgba >> 24, rgba >> 16, rgba >> 8, rgba)); }
	inline rgbaint_t(UINT32 a, UINT32 r, UINT32 g, UINT32 b) { set_rgba(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgba) { m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgba), _mm_setzero_si128()); }

	inline void set(rgbaint_t& other) { m_value = other.m_value; }
	inline void set(__m128i value) { m_value = value; }
	inline void set_rgba(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_value = _mm_set_epi32(a, r, g, b);
	}

	inline void set_a(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.alpha_mask), _mm_set_epi32(value, 0, 0, 0));
	}

	inline UINT8 get_a()
	{
		return _mm_extract_epi16(m_value, 6) & 0xff;
	}

	inline UINT16 get_a32()
	{
		return _mm_extract_epi16(m_value, 6);
	}

	inline void and_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_and_si128(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i temp = _mm_set_epi32(a, r, g, b);
		m_value = _mm_add_epi32(m_value, temp);
	}

	inline void sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i temp = _mm_set_epi32(a, r, g, b);
		m_value = _mm_sub_epi32(m_value, temp);
	}

	inline void subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i temp = _mm_set_epi32(a, r, g, b);
		m_value = _mm_sub_epi32(temp, m_value);
	}

	inline void mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i immv = _mm_set_epi32(a, r, g, b);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void merge_alpha(rgbaint_t& alpha)
	{
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 7), 7);
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 6), 6);
	}
};

#endif /* __RGBSSE__ */
