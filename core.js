// Thin bridge to core.wasm. Calls are sync once loaded, so the shared C buffer can't race.
const enc = new TextEncoder(), dec = new TextDecoder();

export const core = WebAssembly.instantiateStreaming(fetch('/core.wasm')).then(({ instance: { exports: c } }) => {
  const mem = () => new Uint8Array(c.memory.buffer, c.buf(), c.buf_size());
  return {
    // encodeInto truncates at the buffer end; the header sits at the top so that's fine.
    runAt(url, code) {
      const m = mem(), u = enc.encodeInto(url, m).written;
      return c.run_at(u, enc.encodeInto(code, m.subarray(u)).written);
    },
    name(code) {
      const m = mem(), off = c.script_name(enc.encodeInto(code, m).written);
      return dec.decode(m.subarray(off, off + c.result_len()));
    },
  };
});
