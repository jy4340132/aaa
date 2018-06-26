
#define	SIGN_BIT 0x80
#define	QUANT_MASK 0xf
#define	NSEGS 8
#define	SEG_SHIFT 4
#define	SEG_MASK 0x70
#define	BIAS 0x84

static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short* table, int size)
{
	for (int i = 0; i < size; ++i) 
    {
		if (val <= *table++)
        {
            return i;
        }
	}
	return size;
}

static int alaw2linear(unsigned char a_val)
{
	a_val ^= 0x55;
	int t = (a_val & QUANT_MASK) << 4;
	int seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) 
	{
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

static int ulaw2linear(unsigned char u_val)
{
	u_val = ~u_val;
	int t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}


static unsigned char linear2alaw(int pcm_val)	/* 2's complement (16-bit range) */
{
	int mask = 0;
	if (pcm_val >= 0) 
    {
		mask = 0xD5;
	} 
    else 
    {
		mask = 0x55;
		pcm_val = -pcm_val - 8;
	}
	int seg = search(pcm_val, seg_end, 8);
	if (seg >= 8)
    {
		return (0x7F ^ mask);
    }
	else 
    {
		unsigned char aval = seg << SEG_SHIFT;
		if (seg < 2)
        {
			aval |= (pcm_val >> 4) & QUANT_MASK;
        }
		else
        {
			aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
        }
		return (aval ^ mask);
	}
}

static unsigned char linear2ulaw(int pcm_val)
{
	int mask = 0;
	if (pcm_val < 0) 
    {
		pcm_val = BIAS - pcm_val;
		mask = 0x7F;
	} 
    else 
    {
		pcm_val += BIAS;
		mask = 0xFF;
	}
	int seg = search(pcm_val, seg_end, 8);
	if (seg >= 8)
    {
		return (0x7F ^ mask);
    }
	else 
    {
		unsigned char uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
		return (uval ^ mask);
	}
}

int decodeG711a(short* amp, unsigned char* g711a_data, int g711a_bytes)
{
	int samples = 0;
	unsigned char code = 0;
	int sl = 0;
	for(int i = 0; ; )
	{
		if(i >= g711a_bytes)
        {
			break;
        }
		code = g711a_data[++i];
		sl = alaw2linear(code);
		amp[++samples] = (short)sl;
	}
	return samples;
}

int decodeG711u(short* amp, unsigned char* g711u_data, int g711u_bytes)
{
	int samples = 0;
	unsigned char code = 0;
	int sl = 0;
	for(int i = 0; ; )
	{
		if(i >= g711u_bytes)
        {
			break;
        }
		code = g711u_data[++i];
		sl = ulaw2linear(code);
		amp[++samples] = (short)sl;
	}
	return samples;
}

int encodeG711a(unsigned char* g711_data, short* amp, int len)
{
    for (int i = 0; i < len; ++i)
	{
        g711_data[i] = linear2alaw(amp[i]);
    }
    return len;
}

int encodeG711u(unsigned char* g711_data, short* amp, int len)
{
    for (int i = 0; i < len; ++i)
	{
        g711_data[i] = linear2ulaw(amp[i]);
    }
    return len;
}
