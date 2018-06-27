
class G726 extends AudioCoder
{
    constructor(wasm, importObj, bitCount)
    {
        super(wasm, importObj);
        this._bitCount = bitCount;
        this._indexDe = G726._currentIndex;
        this._indexEn = G726._currentIndex + 1;
        G726._currentIndex = (G726._currentIndex + 2) & 0x3F;
        this._wasm.instance.exports._initG726State(this._indexDe, this._bitCount);
        this._wasm.instance.exports._initG726State(this._indexEn, this._bitCount);
    }

    decodeBe(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._decodeG726(this._indexDe, 0, data.byteLength, data.byteLength, 0);
        return new Int16Array(this._memory.buffer, data.byteLength, ((data.byteLength << 4) / this._bitCount) >>> 1);
    }

    encodeBe(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._encodeG726(this._indexEn, 0, data.byteLength / 2, data.byteLength, 0);
        return new Uint8Array(this._memory.buffer, data.byteLength, (data.byteLength * this._bitCount) >>> 4);
    }

    decodeLe(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._decodeG726(this._indexDe, 0, data.byteLength, data.byteLength, 1);
        return new Int16Array(this._memory.buffer, data.byteLength, ((data.byteLength << 4) / this._bitCount) >>> 1);
    }

    encodeLe(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._encodeG726(this._indexEn, 0, data.byteLength / 2, data.byteLength, 1);
        return new Uint8Array(this._memory.buffer, data.byteLength, (data.byteLength * this._bitCount) >>> 4);
    }
}

G726._currentIndex = 0;
