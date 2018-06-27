
#define NULL 0

/*! Bitstream handler state */
typedef struct bitstream_state_s
{
	unsigned int bitstream;
	int residue;
} bitstream_state_t;

typedef struct g726_state_s g726_state_t;

typedef struct g726_state_s
{
	int rate;
	int bits_per_sample;
	int yl;
	short yu;
	short dms;
	short dml;
	short ap;
	short a[2];
	short b[6];
	short pk[2];
	short dq[6];
	short sr[2];
	int td;
	bitstream_state_t bs;
} g726_state_t;

static const int g726_16_dqlntab[4] =
{
	116, 365, 365, 116
};

static const int g726_16_witab[4] =
{
	-704, 14048, 14048, -704
};

static const int g726_16_fitab[4] =
{
	0x000, 0xE00, 0xE00, 0x000
};

static const int g726_24_dqlntab[8] =
{
	-2048, 135, 273, 373, 373, 273, 135, -2048
};

static const int g726_24_witab[8] =
{
	-128, 960, 4384, 18624, 18624, 4384, 960, -128
};

static const int g726_24_fitab[8] =
{
	0x000, 0x200, 0x400, 0xE00, 0xE00, 0x400, 0x200, 0x000
};

static const int g726_32_dqlntab[16] =
{
	-2048,   4, 135, 213, 273, 323, 373,   425,
	425, 373, 323, 273, 213, 135,   4, -2048
};

static const int g726_32_witab[16] =
{
	-384,   576,  1312,  2048,  3584,  6336, 11360, 35904,
	35904, 11360,  6336,  3584,  2048,  1312,   576,  -384
};

static const int g726_32_fitab[16] =
{
	0x000, 0x000, 0x000, 0x200, 0x200, 0x200, 0x600, 0xE00,
	0xE00, 0x600, 0x200, 0x200, 0x200, 0x000, 0x000, 0x000
};

static const int g726_40_dqlntab[32] =
{
	-2048, -66, 28, 104, 169, 224, 274, 318,
	358, 395, 429, 459, 488, 514, 539, 566,
	566, 539, 514, 488, 459, 429, 395, 358,
	318, 274, 224, 169, 104, 28, -66, -2048
};

static const int g726_40_witab[32] =
{
	448,   448,   768,  1248,  1280,  1312,  1856,  3200,
	4512,  5728,  7008,  8960, 11456, 14080, 16928, 22272,
	22272, 16928, 14080, 11456,  8960,  7008,  5728,  4512,
	3200,  1856,  1312,  1280,  1248,   768,   448,   448
};

static const int g726_40_fitab[32] =
{
	0x000, 0x000, 0x000, 0x000, 0x000, 0x200, 0x200, 0x200,
	0x200, 0x200, 0x400, 0x600, 0x800, 0xA00, 0xC00, 0xC00,
	0xC00, 0xC00, 0xA00, 0x800, 0x600, 0x400, 0x200, 0x200,
	0x200, 0x200, 0x200, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const int qtab_726_16[1] =
{
    261
};

static const int qtab_726_24[3] =
{
    8, 218, 331
};

static const int qtab_726_32[7] =
{
    -124, 80, 178, 246, 300, 349, 400
};

static const int qtab_726_40[15] =
{
    -122, -16,  68, 139, 198, 250, 298, 339,
     378, 413, 445, 475, 502, 528, 553
};

static __inline int top_bit(unsigned int bits)
{
	int res = 0;
	if (bits == 0)
	{
		return -1;
	}
	if (bits & 0xFFFF0000)
	{
		bits &= 0xFFFF0000;
		res += 16;
	}
	if (bits & 0xFF00FF00)
	{
		bits &= 0xFF00FF00;
		res += 8;
	}
	if (bits & 0xF0F0F0F0)
	{
		bits &= 0xF0F0F0F0;
		res += 4;
	}
	if (bits & 0xCCCCCCCC)
	{
		bits &= 0xCCCCCCCC;
		res += 2;
	}
	if (bits & 0xAAAAAAAA)
	{
		bits &= 0xAAAAAAAA;
		res += 1;
	}
	return res;
}


static bitstream_state_t *bitstream_init(bitstream_state_t *s)
{
	if (s == NULL)
	{
		return NULL;
	}	
	s->bitstream = 0;
	s->residue = 0;
	return s;
}

static short quantize(int d, int y, const int table[], int quantizer_states)
{
    short dqm = d >= 0 ? d : -d;
    short exp = (top_bit(dqm >> 1) + 1);
    short mant = ((dqm << 7) >> exp) & 0x7F;
    short dl = (exp << 7) + mant;
    short dln = dl - (short)(y >> 2);
    int size = (quantizer_states - 1) >> 1;
	int i = 0;
    for (; i < size && dln >= table[i]; ++i);
    if (d < 0)
    {
        return (short) ((size << 1) + 1 - i);
    }
    if (i == 0 && (quantizer_states & 1))
    {
        return (short)quantizer_states;
    }
    return (short)i;
}

static short fmult(short an, short srn)
{
	short anmag = (an > 0) ? an : ((-an) & 0x1FFF);
	short anexp = (short)(top_bit(anmag) - 5);
	short anmant = (anmag == 0) ? 32 : (anexp >= 0) ? (anmag >> anexp) : (anmag << -anexp);
	short wanexp = anexp + ((srn >> 6) & 0xF) - 13;
	short wanmant = (anmant*(srn & 0x3F) + 0x30) >> 4;
	short retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) : (wanmant >> -wanexp);
	return (((an ^ srn) < 0) ?  -retval : retval);
}

static __inline short predictor_zero(g726_state_t *s)
{
	int sezi = fmult(s->b[0] >> 2, s->dq[0]);
	for (int i = 1;  i < 6;  i++)
	{
		sezi += fmult(s->b[i] >> 2, s->dq[i]);
	}
	return (short)sezi;
}

static __inline short predictor_pole(g726_state_t *s)
{
	return (fmult(s->a[1] >> 2, s->sr[1]) + fmult(s->a[0] >> 2, s->sr[0]));
}


static int step_size(g726_state_t *s)
{
	if (s->ap >= 256)
	{
		return s->yu;
	}
	int y = s->yl >> 6;
	int dif = s->yu - y;
	int al = s->ap >> 2;
	if (dif > 0)
	{
		y += (dif*al) >> 6;
	}
	else if (dif < 0)
	{
		y += (dif*al + 0x3F) >> 6;
	}
	return y;
}

static short reconstruct(int sign, int dqln, int y)
{
	short dql = (short)(dqln + (y >> 2));
	if (dql < 0)
	{
		return ((sign)  ?  -0x8000  :  0);
	}
	short dex = (dql >> 7) & 15;
	short dqt = 128 + (dql & 127);
	short dq = (dqt << 7) >> (14 - dex);
	return ((sign) ? (dq - 0x8000) : dq);
}

static void update(g726_state_t *s, int y, int wi, int fi, int dq, int sr, int dqsez)
{
	short exp = 0;	
	short a2p = 0;
	short pk0 = (dqsez < 0) ? 1 : 0;
	short mag = (short) (dq & 0x7FFF);
	short ylint = (short) (s->yl >> 15);
	short ylfrac = (short) ((s->yl >> 10) & 0x1F);
	short thr = (ylint > 9)  ?  (31 << 10)  :  ((32 + ylfrac) << ylint);
	short dqthr = (thr + (thr >> 1)) >> 1;
	int tr = s->td && mag > dqthr ? 1 : 0;
	s->yu = (short) (y + ((wi - y) >> 5));
	if (s->yu < 544)
	{
		s->yu = 544;
	}
	else if (s->yu > 5120)
	{
		s->yu = 5120;
	}
	s->yl += s->yu + ((-s->yl) >> 6);
	if (tr)
	{
		s->a[0] = 0;
		s->a[1] = 0;
		s->b[0] = 0;
		s->b[1] = 0;
		s->b[2] = 0;
		s->b[3] = 0;
		s->b[4] = 0;
		s->b[5] = 0;
	}
	else
	{
		short pks1 = pk0 ^ s->pk[0];
		a2p = s->a[1] - (s->a[1] >> 7);
		if (dqsez != 0)
		{
			short fa1 = (pks1) ? s->a[0] : -s->a[0];
			if (fa1 < -8191)
			{
				a2p -= 0x100;
			}
			else if (fa1 > 8191)
			{
				a2p += 0xFF;
			}
			else
			{
				a2p += fa1 >> 5;
			}
			if (pk0 ^ s->pk[1])
			{
				if (a2p <= -12160)
				{
					a2p = -12288;
				}
				else if (a2p >= 12416)
				{
					a2p = 12288;
				}
				else
				{
					a2p -= 0x80;
				}
			}
			else if (a2p <= -12416)
			{
				a2p = -12288;
			}
			else if (a2p >= 12160)
			{
				a2p = 12288;
			}
			else
			{
				a2p += 0x80;
			}
		}
		s->a[1] = a2p;
		s->a[0] -= s->a[0] >> 8;
		if (dqsez != 0)
		{
			if (pks1 == 0)
			{
				s->a[0] += 192;
			}
			else
			{
				s->a[0] -= 192;
			}
		}
		short a1ul = 15360 - a2p;
		if (s->a[0] < -a1ul)
		{
			s->a[0] = -a1ul;
		}
		else if (s->a[0] > a1ul)
		{
			s->a[0] = a1ul;
		}

		for (int i = 0; i < 6; i++)
		{
			s->b[i] -= s->b[i] >> ((s->bits_per_sample == 5)  ?  9  :  8);
			if (dq & 0x7FFF)
			{
				if ((dq ^ s->dq[i]) >= 0)
				{
					s->b[i] += 128;
				}
				else
				{
					s->b[i] -= 128;
				}
			}
		}
	}

	for (int i = 5;  i > 0;  i--)
	{
		s->dq[i] = s->dq[i - 1];
	}
	if (mag == 0)
	{
		s->dq[0] = (dq >= 0)  ?  0x20  :  0xFC20;
	}
	else
	{
		exp = (short) (top_bit(mag) + 1);
		s->dq[0] = (dq >= 0) ? ((exp << 6) + ((mag << 6) >> exp)) : ((exp << 6) + ((mag << 6) >> exp) - 0x400);
	}

	s->sr[1] = s->sr[0];
	if (sr == 0)
	{
		s->sr[0] = 0x20;
	}
	else if (sr > 0)
	{
		exp = (short) (top_bit(sr) + 1);
		s->sr[0] = (short) ((exp << 6) + ((sr << 6) >> exp));
	}
	else if (sr > -32768)
	{
		mag = (short) -sr;
		exp = (short) (top_bit(mag) + 1);
		s->sr[0] =  (exp << 6) + ((mag << 6) >> exp) - 0x400;
	}
	else
	{
		s->sr[0] = (short) 0xFC20;
	}
	s->pk[1] = s->pk[0];
	s->pk[0] = pk0;
	s->td = !tr && a2p < -11776 ? 1 : 0;
	s->dms += ((short) fi - s->dms) >> 5;
	s->dml += (((short) (fi << 2) - s->dml) >> 7);
	int dms_abs = (s->dms << 2) - s->dml;
	if (tr)
	{
		s->ap = 256;
	}
	else if (y < 1536)
	{
		s->ap += (0x200 - s->ap) >> 4;
	}
	else if (s->td)
	{
		s->ap += (0x200 - s->ap) >> 4;
	}
	else if ((dms_abs >= 0 ? dms_abs : -dms_abs) >= (s->dml >> 3))
	{
		s->ap += (0x200 - s->ap) >> 4;
	}
	else
	{
		s->ap += (-s->ap) >> 4;
	}	
}

#define G726_DECODER_DEF(bitCount, codeValue, codeWith, dqWhth) \
static short g726_##bitCount##_decoder(g726_state_t *s, unsigned char code) \
{ \
	code &= codeValue; \
	short sezi = predictor_zero(s); \
	short sei = sezi + predictor_pole(s); \
	int y = step_size(s); \
	short dq = reconstruct(code & codeWith, g726_##bitCount##_dqlntab[code], y); \
	short se = sei >> 1; \
	short sr = (dq < 0) ? (se - (dq & dqWhth)) : (se + dq); \
	short dqsez = sr + (sezi >> 1) - se; \
	update(s, y, g726_##bitCount##_witab[code], g726_##bitCount##_fitab[code], dq, sr, dqsez); \
	return (sr << 2); \
}

G726_DECODER_DEF(16, 0x03, 2, 0x3FFF)
G726_DECODER_DEF(24, 0x07, 4, 0x3FFF)
G726_DECODER_DEF(32, 0x0F, 8, 0x3FFF)
G726_DECODER_DEF(40, 0x1F, 0x10, 0x7FFF)

#define G726_ENCODER_DEF(bitCount, iWith, qWith, dqWhth) \
static unsigned char g726_##bitCount##_encoder(g726_state_t *s, short amp) \
{ \
    short sezi = predictor_zero(s); \
    short sei = sezi + predictor_pole(s); \
    short se = sei >> 1; \
    short d = amp - se; \
    int y = step_size(s); \
    short i = quantize(d, y, qtab_726_##bitCount, qWith); \
    short dq = reconstruct(i & iWith, g726_##bitCount##_dqlntab[i], y); \
    short sr = (dq < 0) ? (se - (dq & dqWhth)) : (se + dq); \
    short dqsez = sr + (sezi >> 1) - se; \
    update(s, y, g726_##bitCount##_witab[i], g726_##bitCount##_fitab[i], dq, sr, dqsez); \
    return (unsigned char)i; \
}

G726_ENCODER_DEF(16, 2, 4, 0x3FFF)
G726_ENCODER_DEF(24, 4, 7, 0x3FFF)
G726_ENCODER_DEF(32, 8, 15, 0x3FFF)
G726_ENCODER_DEF(40, 0x10, 31, 0x7FFF)

static g726_state_t* g726_init(g726_state_t *s, int bit_rate)
{
	if (bit_rate != 16000  &&  bit_rate != 24000  &&  bit_rate != 32000  &&  bit_rate != 40000)
	{
		return NULL;
	}
	s->yl = 34816;
	s->yu = 544;
	s->dms = 0;
	s->dml = 0;
	s->ap = 0;
	s->rate = bit_rate;

	for (int i = 0; i < 2; i++)
	{
		s->a[i] = 0;
		s->pk[i] = 0;
		s->sr[i] = 32;
	}
	for (int i = 0; i < 6; i++)
	{
		s->b[i] = 0;
		s->dq[i] = 32;
	}
	s->td = 0;
	switch (bit_rate)
	{
		case 16000:
			s->bits_per_sample = 2;
			break;
		case 24000:
			s->bits_per_sample = 3;
			break;
		case 32000:
			s->bits_per_sample = 4;
			break;
		case 40000:
			s->bits_per_sample = 5;
			break;
	}
	bitstream_init(&s->bs);
	return s;
}

#define G726_DECODE_DATA_DEF(bitCount) \
static int g726_decode##bitCount(g726_state_t *s, short amp[], unsigned char g726_data[], int g726_bytes) \
{ \
	int samples = 0; \
	for (int i = 0; ; ) \
	{ \
		if (s->bs.residue < s->bits_per_sample) \
		{ \
			if (i >= g726_bytes) \
				break; \
			s->bs.bitstream = (s->bs.bitstream << 8) | g726_data[i++]; \
			s->bs.residue += 8; \
		} \
		unsigned char code = (unsigned char) ((s->bs.bitstream >> (s->bs.residue - s->bits_per_sample)) & ((1 << s->bits_per_sample) - 1)); \
		s->bs.residue -= s->bits_per_sample; \
		int sl = g726_##bitCount##_decoder(s, code); \
		amp[samples++] = (short)sl; \
	} \
	return samples; \
}

G726_DECODE_DATA_DEF(16)
G726_DECODE_DATA_DEF(24)
G726_DECODE_DATA_DEF(32)
G726_DECODE_DATA_DEF(40)

#define G726_ENCODE_DATA_DEF(bitCount) \
static int g726_encode##bitCount(g726_state_t *s, unsigned char g726_data[], short amp[], int len) \
{ \
    int g726_bytes = 0; \
    short sl = 0; \
    unsigned char code = 0; \
    for (int i = 0;  i < len;  i++) \
    { \
		sl = amp[i] >> 2; \
        code =  g726_##bitCount##_encoder(s, sl); \
		s->bs.bitstream = (s->bs.bitstream << s->bits_per_sample) | code; \
		s->bs.residue += s->bits_per_sample; \
		if (s->bs.residue >= 8) \
		{ \
			g726_data[g726_bytes++] = (unsigned char) ((s->bs.bitstream >> (s->bs.residue - 8)) & 0xFF); \
			s->bs.residue -= 8; \
		} \
    } \
    return g726_bytes; \
}

G726_ENCODE_DATA_DEF(16)
G726_ENCODE_DATA_DEF(24)
G726_ENCODE_DATA_DEF(32)
G726_ENCODE_DATA_DEF(40)

static g726_state_t g726_state[64] = {0};

void initG726State(int index, int bitCount)
{
	g726_init(g726_state + index, 8000 * bitCount);
}

void decodeG726(int index, unsigned char* inData, int len, short* outData, int type)
{
	if(type == 1)
	{
        for(int i = 0; i != len; ++i)
        {
            inData[i] = ((inData[i] & 0x0F) << 4) + ((inData[i] >> 4) & 0x0F);
        }
	}
	int bitCount = (g726_state + index) -> bits_per_sample;
	switch(bitCount)
	{
		case 2:
		g726_decode16(g726_state + index, outData, inData, len);
		break;	
		case 3:
		g726_decode24(g726_state + index, outData, inData, len);
		break;	
		case 4:
		g726_decode32(g726_state + index, outData, inData, len);
		break;	
		case 5:
		g726_decode40(g726_state + index, outData, inData, len);
		break;	
	}
}

void encodeG726(int index, short* inData, int len, unsigned char* outData, int type)
{
	int bitCount = (g726_state + index) -> bits_per_sample;
	int outLen = 0;
	switch(bitCount)
	{
		case 2:
		outLen = g726_encode16(g726_state + index, outData, inData, len);
		break;
		case 3:
		outLen = g726_encode24(g726_state + index, outData, inData, len);
		break;
		case 4:
		outLen = g726_encode32(g726_state + index, outData, inData, len);
		break;
		case 5:
		outLen = g726_encode40(g726_state + index, outData, inData, len);
		break;
	}
	if(type == 1)
	{
		char* buffer = (char*)outData;
        for(int i = 0; i != (outLen << 1); ++i)
        {
            buffer[i] = ((buffer[i] & 0x0F) << 4) + ((buffer[i] >> 4) & 0x0F);
        }
	}
}
