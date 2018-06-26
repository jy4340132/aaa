
class G726
{
    constructor(wasm, memory, bitCount)
    {
        this._wasm = wasm;
        this._memory = new Uint8Array(memory.buffer);
        this._bitCount = bitCount;
        this._indexDe = G726._currentIndex;
        this._indexEn = G726._currentIndex + 1;
        G726._currentIndex = (G726._currentIndex + 2) & 0x3F;
        this._wasm.instance.exports._initG726State(this._indexDe, this._bitCount);
        this._wasm.instance.exports._initG726State(this._indexEn, this._bitCount);
    }

    decode(data)
    {
        this._memory.set(new Uint8Array(data.buffer, data.byteOffset, data.byteLength));
        this._wasm.instance.exports._decodeG726(this._indexDe, 0, data.byteLength, 10240, 10236);
        let outLenBuffer = new Uint32Array(this._memory.buffer, 10236);
        return new Int16Array(this._memory.buffer, 10240, outLenBuffer[0]);
    }

    encode(data)
    {
        this._memory.set(new Uint8Array(data.buffer, data.byteOffset, data.byteLength));
        this._wasm.instance.exports._encodeG726(this._indexEn, 0, data.byteLength / 2, 10240, 10236);
        let outLenBuffer =  new Uint32Array(this._memory.buffer, 10236);
        return new Uint8Array(this._memory.buffer, 10240, outLenBuffer[0]);
    }
}

G726._currentIndex = 0;
