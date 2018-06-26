
typedef struct adpcm_state_t {
    short valprev;
    char index;
} adpcm_state;

static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
    
static void adpcm_encoder(short* indata, char* outdata, int len, adpcm_state* state)
{
    int val = 0;
    int sign = 0;
    unsigned int delta = 0;
    int diff = 0;
    unsigned int udiff = 0;
    unsigned int vpdiff = 0;
    unsigned int outputbuffer = 0;
    char* outp = outdata;
    short* inp = indata;
    int valpred = state -> valprev;
    int index = (int)state -> index;
    unsigned int step = stepsizeTable[index];
    int bufferstep = 1;
    while(len-- > 0) 
    {
        val = *inp++;
        diff = val - valpred;
        if(diff < 0)
        {
            sign = 8;
            diff = (-diff);
        }
        else
        {
            sign = 0;
        }
        udiff = (unsigned int)diff;
        delta = 0;
        vpdiff = (step >> 3);
        if(udiff >= step)
        {
            delta = 4;
            udiff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if(udiff >= step)
        {
            delta |= 2;
            udiff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if(udiff >= step) 
        {
            delta |= 1;
            vpdiff += step;
        }
        if(sign != 0)
        {
            valpred -= vpdiff;
            if(valpred < -32768)
            {
                valpred = -32768;
            }
        }
        else
        {
            valpred += vpdiff;
            if(valpred > 32767)
            {
                valpred = 32767;
            }
        }
        delta |= sign;
        index += indexTable[delta];
        if(index < 0) index = 0;
        if(index > 88) index = 88;
        step = stepsizeTable[index];
        if(bufferstep != 0)
        {
            outputbuffer = (delta << 4);
        } 
        else 
        {
            *outp++ = (char)(delta | outputbuffer);
        }
        bufferstep = !bufferstep;
    }
    if(bufferstep == 0)
    {
        *outp++ = (char)outputbuffer;
    }
    state -> valprev = (short)valpred;
    state -> index = (char)index;
}

static void adpcm_decoder(char* indata, short* outdata, int len, adpcm_state* state)
{
    int sign = 0;
    int delta = 0;
    int vpdiff = 0;
    int inputbuffer = 0;
    short* outp = outdata;
    char* inp = indata;
    int valpred = state -> valprev;
    int index = state -> index;
    int step = stepsizeTable[index];
    int bufferstep = 0;
    for (; len > 0; --len)
    {
        if(bufferstep)
        {
            delta = inputbuffer & 0xf;
        }
        else 
        {
            inputbuffer = *inp++;
            delta = (inputbuffer >> 4) & 0xf;
        }
        bufferstep = !bufferstep;
        index += indexTable[delta];
        if(index < 0)
        {
            index = 0;
        }
        if(index > 88)
        {
            index = 88;
        }
        sign = delta & 8;
        delta = delta & 7;
        vpdiff = (delta * 2 + 1) * step / 8;
        if(delta & 4)
        {
            vpdiff += step;
        }
        if(delta & 2)
        {
            vpdiff += step>>1;
        }
        if(delta & 1)
        {
            vpdiff += step>>2;
        }
        if(sign)
        {
            valpred -= vpdiff;
        }
        else
        {
            valpred += vpdiff;
        }
        if(valpred > 32767)
        {
            valpred = 32767;
        }
        else if(valpred < -32768)
        {
            valpred = -32768;
        }
        step = stepsizeTable[index];
        *outp++ = valpred;
    }
    state -> valprev = valpred;
    state -> index = index;
}

static adpcm_state adpcmState[64] = {0};

void initAdpcmState(index)
{
    adpcmState[index].valprev = 0;
    adpcmState[index].index = 0;
}

void resetAdpcmState(int index, int stateValprev, int stateIndex)
{
    adpcmState[index].valprev = (short)stateValprev;
    adpcmState[index].index = (char)stateIndex;
}

void getAdpcmState(int index, short* stateValprev, char* stateIndex)
{
    *stateValprev = adpcmState[index].valprev;
    *stateIndex = adpcmState[index].index;
}

void decodeAdpcm(int index, char* indata, short* outdata, int len)
{
    adpcm_decoder(indata, outdata, len, adpcmState + index);
}

void encodeAdpcm(int index, short* indata, char* outdata, int len)
{
    adpcm_encoder(indata, outdata, len, adpcmState + index);
}
