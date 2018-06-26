
class G711
{
    constructor(wasm, memory)
    {
        this._wasm = wasm;
        this._memory = new Uint8Array(memory.buffer);
    }

    decodeA(data)
    {
        let len = this._doCode(data, this._wasm.instance.exports._decodeG711a, data.byteLength);
        return new Int16Array(this._memory.buffer, 10240, len);
    }

    decodeU(data)
    {
        let len = this._doCode(data, this._wasm.instance.exports._decodeG711u, data.byteLength);
        return new Int16Array(this._memory.buffer, 10240, len);
    }

    encodeA(data)
    {
        let len = this._doCode(data, this._wasm.instance.exports._encodeG711a, data.byteLength / 2);
        return new Uint8Array(this._memory.buffer, 10240, len);
    }

    encodeU(data)
    {
        let len = this._doCode(data, this._wasm.instance.exports._encodeG711u, data.byteLength / 2);
        return new Uint8Array(this._memory.buffer, 10240, len);
    }

    _doCode(data, len, callback)
    {
        this._memory.set(new Uint8Array(data.buffer));
        return callback(10240, 0, len);
    }
}
