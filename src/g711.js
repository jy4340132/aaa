
class G711 extends AudioCoder
{
    constructor(wasm, importObj)
    {
        super(wasm, importObj);
    }

    decodeA(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._decodeG711a(data.byteLength, 0, data.byteLength);
        return new Int16Array(this._memory.buffer, data.byteLength, data.byteLength);
    }

    decodeU(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._decodeG711u(data.byteLength, 0, data.byteLength);
        return new Int16Array(this._memory.buffer, data.byteLength, data.byteLength);
    }

    encodeA(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._encodeG711a(data.byteLength, 0, data.byteLength >>> 1);
        return new Uint8Array(this._memory.buffer, data.byteLength, data.byteLength >>> 1);
    }

    encodeU(data)
    {
        this._copyToMemory(data);
        this._wasm.instance.exports._encodeG711u(data.byteLength, 0, data.byteLength >>> 1);
        return new Uint8Array(this._memory.buffer, data.byteLength, data.byteLength >>> 1);
    }
}
