// Interfaz del verificador. El cálculo ocurre en hash.wasm (C compilado a WebAssembly).

const ALGORITMOS = [
  { id: "md5", nombre: "MD5", bit: 1, offset: 0, bytes: 16 },
  { id: "sha1", nombre: "SHA-1", bit: 2, offset: 16, bytes: 20 },
  { id: "sha256", nombre: "SHA-256", bit: 4, offset: 36, bytes: 32 },
  { id: "sha512", nombre: "SHA-512", bit: 8, offset: 68, bytes: 64 },
];

const hex = (u8) => Array.from(u8, (b) => b.toString(16).padStart(2, "0")).join("");
const tamano = (n) =>
  n < 1024 ? `${n} B` : n < 1048576 ? `${(n / 1024).toFixed(1)} KB` : n < 1073741824 ? `${(n / 1048576).toFixed(1)} MB` : `${(n / 1073741824).toFixed(2)} GB`;
const escapar = (s) => s.replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]);

const PLANTILLA = `
  <section class="hx-card hx-entrada">
    <label class="hx-drop" id="hx-drop">
      <svg width="44" height="44" viewBox="0 0 24 24" fill="none" aria-hidden="true">
        <path d="M12 3l7 3v5c0 4.4-3 8.3-7 9.5-4-1.2-7-5.1-7-9.5V6z" stroke="currentColor" stroke-width="1.5"/>
        <path d="M9 12l2 2 4-4" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/>
      </svg>
      <strong>Arrastra uno o varios archivos aquí</strong>
      <span class="hx-muted">o haz clic para elegirlos · cualquier tamaño</span>
      <input id="hx-archivos" type="file" multiple class="hx-sr" />
    </label>
    <div class="hx-opciones">
      <fieldset class="hx-algs">
        <legend>Algoritmos</legend>
        ${ALGORITMOS.map(
          (a) => `<label><input type="checkbox" id="hx-${a.id}" value="${a.bit}" ${a.id === "sha256" || a.id === "md5" ? "checked" : ""}/> ${a.nombre}</label>`,
        ).join("")}
      </fieldset>
      <label class="hx-esperado">
        <span>Hash publicado por el proveedor (opcional)</span>
        <input id="hx-esperado" type="text" spellcheck="false" autocomplete="off" placeholder="Pega aquí el SHA-256 o MD5 para comparar" />
      </label>
    </div>
    <p class="hx-muted hx-nota">🔒 Los archivos se procesan en tu navegador con WebAssembly. No se suben a ningún servidor.</p>
  </section>
  <section id="hx-resultados" class="hx-resultados" aria-live="polite"></section>
  <section class="hx-info">
    <div class="hx-card"><h3>¿Para qué sirve?</h3><p class="hx-muted">Confirma que un instalador, respaldo o documento es idéntico al original y no fue alterado en la descarga.</p></div>
    <div class="hx-card"><h3>Hecho en C</h3><p class="hx-muted">MD5, SHA-1, SHA-256 y SHA-512 escritos en C, compilados a WebAssembly y probados contra las herramientas del sistema.</p></div>
    <div class="hx-card"><h3>¿Cuál usar?</h3><p class="hx-muted">SHA-256 o SHA-512 para seguridad. MD5 y SHA-1 solo para comparar con sumas antiguas.</p></div>
  </section>`;

export async function iniciar({ raiz, cargarWasm }) {
  raiz.innerHTML = PLANTILLA;
  const { instance } = await WebAssembly.instantiate(await cargarWasm());
  const w = instance.exports;
  const memoria = () => new Uint8Array(w.memory.buffer);
  const TAM = w.buffer_size();

  const $ = (sel) => raiz.querySelector(sel);
  const resultados = $("#hx-resultados");
  const esperado = $("#hx-esperado");
  let ultimos = [];

  async function calcular(archivo, mascara, onProgreso) {
    w.hash_init(mascara);
    const ptr = w.buffer_ptr();
    for (let pos = 0; pos < archivo.size; pos += TAM) {
      const trozo = new Uint8Array(await archivo.slice(pos, pos + TAM).arrayBuffer());
      memoria().set(trozo, ptr);
      w.hash_update(trozo.length);
      onProgreso(Math.min(1, (pos + trozo.length) / archivo.size));
    }
    if (archivo.size === 0) onProgreso(1);
    w.hash_final();
    const out = w.output_ptr();
    return ALGORITMOS.filter((a) => mascara & a.bit).map((a) => ({
      ...a,
      valor: hex(memoria().slice(out + a.offset, out + a.offset + a.bytes)),
    }));
  }

  function pintar() {
    const objetivo = esperado.value.trim().toLowerCase();
    for (const fila of resultados.querySelectorAll("[data-hash]")) {
      const coincide = objetivo && fila.dataset.hash === objetivo;
      fila.classList.toggle("hx-coincide", Boolean(coincide));
    }
    for (const tarjeta of resultados.querySelectorAll(".hx-archivo")) {
      const estado = tarjeta.querySelector(".hx-estado");
      if (!objetivo || !estado) continue;
      const hay = tarjeta.querySelector(".hx-coincide");
      estado.textContent = hay ? "✓ Coincide con el hash publicado" : "✕ No coincide con el hash publicado";
      estado.className = `hx-estado ${hay ? "hx-ok" : "hx-bad"}`;
    }
    if (!objetivo) for (const e of resultados.querySelectorAll(".hx-estado")) e.textContent = "";
  }

  async function procesar(archivos) {
    const mascara = ALGORITMOS.reduce((m, a) => m | ($(`#hx-${a.id}`).checked ? a.bit : 0), 0) || 4;
    resultados.innerHTML = "";
    ultimos = [];
    for (const archivo of archivos) {
      const tarjeta = document.createElement("article");
      tarjeta.className = "hx-card hx-archivo";
      tarjeta.innerHTML = `
        <header><div><strong class="hx-nombre">${escapar(archivo.name)}</strong><span class="hx-muted"> · ${tamano(archivo.size)}</span></div>
        <span class="hx-estado"></span></header>
        <div class="hx-barra"><span></span></div><dl></dl>`;
      resultados.append(tarjeta);
      const barra = tarjeta.querySelector(".hx-barra span");
      const inicio = performance.now();
      const hashes = await calcular(archivo, mascara, (p) => (barra.style.width = `${Math.round(p * 100)}%`));
      const segundos = (performance.now() - inicio) / 1000;
      tarjeta.querySelector(".hx-barra").remove();
      tarjeta.querySelector("dl").innerHTML = hashes
        .map(
          (h) => `<div data-hash="${h.valor}"><dt>${h.nombre}</dt><dd><code>${h.valor}</code>
            <button type="button" class="hx-copiar" data-valor="${h.valor}">Copiar</button></dd></div>`,
        )
        .join("");
      tarjeta.querySelector("header .hx-muted").textContent += ` · ${segundos < 0.01 ? "<0.01" : segundos.toFixed(2)} s`;
      ultimos.push({ archivo: archivo.name, hashes });
    }
    pintar();
  }

  const input = $("#hx-archivos");
  const zona = $("#hx-drop");
  input.addEventListener("change", () => {
    if (input.files.length) procesar([...input.files]);
    input.value = "";
  });
  zona.addEventListener("dragover", (e) => {
    e.preventDefault();
    zona.classList.add("hx-activo");
  });
  zona.addEventListener("dragleave", () => zona.classList.remove("hx-activo"));
  zona.addEventListener("drop", (e) => {
    e.preventDefault();
    zona.classList.remove("hx-activo");
    if (e.dataTransfer.files.length) procesar([...e.dataTransfer.files]);
  });
  esperado.addEventListener("input", pintar);
  resultados.addEventListener("click", async (e) => {
    const boton = e.target.closest(".hx-copiar");
    if (!boton) return;
    try {
      await navigator.clipboard.writeText(boton.dataset.valor);
      boton.textContent = "Copiado";
    } catch {
      const rango = document.createRange();
      rango.selectNodeContents(boton.previousElementSibling);
      getSelection().removeAllRanges();
      getSelection().addRange(rango);
      boton.textContent = "Selecciónalo";
    }
    setTimeout(() => (boton.textContent = "Copiar"), 1500);
  });

  // Estado inicial: muestra el resultado de un archivo de ejemplo para que se vea cómo funciona.
  const ejemplo = new File([new TextEncoder().encode("Total Security SH · verificador de archivos\n")], "ejemplo.txt", { type: "text/plain" });
  await procesar([ejemplo]);
  return { procesar };
}
