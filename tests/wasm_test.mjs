// Prueba el hash.wasm publicado contra las herramientas del sistema (md5sum, sha*sum).
// Uso: node tests/wasm_test.mjs [archivo]
import { readFileSync } from "node:fs";
import { execFileSync } from "node:child_process";
import { randomBytes } from "node:crypto";
import { writeFileSync, rmSync } from "node:fs";

const ALGOS = [
  { nombre: "md5", mascara: 1, offset: 0, bytes: 16, comando: "md5sum" },
  { nombre: "sha1", mascara: 2, offset: 16, bytes: 20, comando: "sha1sum" },
  { nombre: "sha256", mascara: 4, offset: 36, bytes: 32, comando: "sha256sum" },
  { nombre: "sha512", mascara: 8, offset: 68, bytes: 64, comando: "sha512sum" },
];

const { instance } = await WebAssembly.instantiate(readFileSync(new URL("../web/hash/hash.wasm", import.meta.url)));
const w = instance.exports;

function hashWasm(datos) {
  const tam = w.buffer_size();
  w.hash_init(15);
  for (let i = 0; i < datos.length; i += tam) {
    const trozo = datos.subarray(i, i + tam);
    new Uint8Array(w.memory.buffer, w.buffer_ptr(), trozo.length).set(trozo);
    w.hash_update(trozo.length);
  }
  w.hash_final();
  const salida = new Uint8Array(w.memory.buffer, w.output_ptr(), 132);
  const hex = (o, n) => Buffer.from(salida.subarray(o, o + n)).toString("hex");
  return Object.fromEntries(ALGOS.map((a) => [a.nombre, hex(a.offset, a.bytes)]));
}

let ruta = process.argv[2];
const temporal = !ruta;
if (temporal) {
  ruta = "tests/random-wasm.bin";
  writeFileSync(ruta, randomBytes(3_000_000));
}

let fallos = 0;
try {
  const obtenido = hashWasm(readFileSync(ruta));
  for (const a of ALGOS) {
    const esperado = execFileSync(a.comando, [ruta]).toString().split(" ")[0];
    const ok = obtenido[a.nombre] === esperado;
    if (!ok) fallos++;
    console.log(`${ok ? "OK  " : "FAIL"} wasm ${a.nombre} = ${a.comando}`);
  }
} finally {
  if (temporal) rmSync(ruta);
}
process.exit(fallos ? 1 : 0);
