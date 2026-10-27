#!/usr/bin/env node
// Boot a kernel in headless Chromium, untar a runner into it, run one command.
//
//   run-in-kernel.mjs [--extract <kernel-path> <host-path>]... \
//     <kernel-dir> <sysroot-dir> <boot.json> <runner.tar> <cmd> [args...]
//
// Exits with the command's status. stdout/stderr are relayed as they arrive.
//
// --extract copies a file out of the kernel once the command has succeeded:
// the browser equivalent of `docker cp`. It is how a binary built by the
// sysroot's own compiler gets back out to be shipped.
//
// This is the browser equivalent of `docker run --entrypoint <cmd> <image>`.
// The kernel is threaded wasm and only runs in a cross-origin isolated page,
// so there is no way to drive it from Node directly: everything is served
// to a real headless Chromium, which is also the runtime students get.
//
// Needs `playwright` on the module path and a Chromium it can launch
// (`npx playwright install --with-deps chromium`, or CHROMIUM_PATH).
//
// A published sysroot carries its wasm binaries as stubs that name the real
// bytes by absolute path under /test-runners/, which the kernel fetches from
// this origin on first use. Those requests are passed through to where the
// website serves them (TEST_RUNNERS_BASE overrides), so what runs here is
// what students get. A raw, unstubbed sysroot never asks.

import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import { chromium } from "playwright";

const args = process.argv.slice(2);
const extracts = [];
while (args[0] === "--extract") {
  const [, from, to] = args.splice(0, 3);
  if (!from || !to) break;
  extracts.push([from, to]);
}
const [kernelDir, sysrootDir, bootJson, tarball, ...argv] = args;
if (!argv.length) {
  console.error(
    "usage: run-in-kernel.mjs [--extract <kernel-path> <host-path>]... " +
      "<kernel-dir> <sysroot-dir> <boot.json> <runner.tar> <cmd> [args...]",
  );
  process.exit(64);
}

const DEBUG = !!process.env.KERNEL_DEBUG;
const TEST_RUNNERS_BASE = process.env.TEST_RUNNERS_BASE ?? "https://exercism.org/test-runners";

const files = {
  "/kernel/kernel.js": [path.join(kernelDir, "kernel.js"), "text/javascript"],
  "/kernel/kernel_bg.wasm": [path.join(kernelDir, "kernel_bg.wasm"), "application/wasm"],
  "/kernel/kernel_client.mjs": [path.join(kernelDir, "kernel_client.mjs"), "text/javascript"],
  "/sysroot.tar": [path.join(sysrootDir, "sysroot.tar"), "application/x-tar"],
  "/boot.json": [bootJson, "application/json"],
  "/runner.tar": [tarball, "application/x-tar"],
};
for (const [p] of Object.values(files)) {
  if (!fs.existsSync(p)) { console.error(`missing: ${p}`); process.exit(66); }
}

// The kernel is threaded wasm: the page must be cross-origin isolated, which
// means every response carries COOP/COEP. Same pair the website sends.
const ISOLATION = {
  "Cross-Origin-Opener-Policy": "same-origin",
  "Cross-Origin-Embedder-Policy": "credentialless",
};

const PAGE = `<!doctype html><meta charset="utf-8"><title>kernel</title>
<script type="module">
  import { KernelClient } from "/kernel/kernel_client.mjs";
  const dec = new TextDecoder();
  const worker = new Worker("/kernel/kernel.js", { type: "module", name: "kernel" });
  const client = new KernelClient({
    endpoint: worker,
    handlers: {
      streamOut: (_s, d) => window.__out(dec.decode(d)),
      streamErr: (_s, d) => window.__err(dec.decode(d)),
      streamIn: () => undefined,
      streamClosed: () => {},
      // Newer kernels insist on these. Nothing a test runner runs draws.
      createCanvas: () => { throw new Error("no canvas in a test run"); },
      destroyCanvas: () => {},
    },
  });
  // run() resolves at spawn with a session id; SessionEnded carries the
  // exit status. Mirrors what the website's Kernel.ts does.
  const ended = new Map();   // sid -> resolve
  const early = new Map();   // sid -> result, when the event beat run()
  client.processEvents((e) => {
    window.__event(JSON.stringify(e));
    if (e.tag !== "SessionEnded") return;
    const status = e.result.tag === "Ok" ? (e.result.value ?? 0) : 1;
    if (ended.has(e.sid)) ended.get(e.sid)(status); else early.set(e.sid, status);
  });
  window.__run = async (argv) => {
    const boot = await (await fetch("/boot.json")).json();
    const env = Object.entries(boot.env).map(([k, v]) => k + "=" + v);
    // Newer kernels call the list precompile; older ones, preload.
    await client.boot(new URL("/sysroot.tar", location.href).href, env, boot.precompile ?? boot.preload, boot.licence);
    await client.untar("/", await (await fetch("/runner.tar")).arrayBuffer());
    const sid = await client.run(argv, env, "/opt/test-runner", true);
    if (early.has(sid)) return early.get(sid);
    return await new Promise((resolve) => ended.set(sid, resolve));
  };
  // Binary-safe: the bytes cross back to Node as base64, not decoded text.
  window.__read = async (path) => {
    const bytes = new Uint8Array(await client.readFile(path));
    let binary = "";
    for (let i = 0; i < bytes.length; i += 0x8000) {
      binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
    }
    return btoa(binary);
  };
  window.__ready = true;
</script>`;

// fetch() has already undone any Content-Encoding, so the bytes go out plain.
// Immutable, as upstream: the kernel fetches a stubbed file every time it is
// opened, and a test suite's links open the same libraries over and over.
async function passThrough(p, res) {
  const upstream = await fetch(TEST_RUNNERS_BASE + p.slice("/test-runners".length));
  if (!upstream.ok) { res.writeHead(upstream.status, ISOLATION); return res.end(); }
  const bytes = Buffer.from(await upstream.arrayBuffer());
  res.writeHead(200, {
    ...ISOLATION,
    "Content-Type": upstream.headers.get("content-type") ?? "application/octet-stream",
    "Content-Length": bytes.length,
    "Cache-Control": "public, max-age=31536000, immutable",
  });
  res.end(bytes);
}

const server = http.createServer((req, res) => {
  const p = new URL(req.url, "http://localhost").pathname;
  if (p === "/") { res.writeHead(200, { ...ISOLATION, "Content-Type": "text/html" }); return res.end(PAGE); }
  if (p.startsWith("/test-runners/")) {
    return passThrough(p, res).catch((e) => { console.error("[proxy]", p, e.message); res.writeHead(502, ISOLATION); res.end(); });
  }
  const entry = files[p];
  if (!entry) { res.writeHead(404, ISOLATION); return res.end(); }
  res.writeHead(200, { ...ISOLATION, "Content-Type": entry[1] });
  fs.createReadStream(entry[0]).pipe(res);
});
await new Promise((r) => server.listen(0, "127.0.0.1", r));
const origin = `http://127.0.0.1:${server.address().port}`;

const browser = await chromium.launch({ executablePath: process.env.CHROMIUM_PATH, args: ["--no-sandbox"] });
const page = await browser.newPage();
await page.exposeFunction("__out", (s) => process.stdout.write(s));
await page.exposeFunction("__err", (s) => process.stderr.write(s));
await page.exposeFunction("__event", (s) => { if (DEBUG) console.error("[event]", s); });
page.on("pageerror", (e) => console.error("[page]", e.message));
page.on("console", (m) => { if (m.type() === "error") console.error("[console]", m.text()); });

let status = 1;
try {
  await page.goto(origin, { waitUntil: "load" });
  if (!(await page.evaluate(() => crossOriginIsolated))) throw new Error("page is not cross-origin isolated");
  await page.waitForFunction(() => window.__ready, null, { timeout: 30_000 });
  const raw = await page.evaluate((a) => window.__run(a), argv).then(
    (s) => s,
    (e) => { console.error("[kernel]", e.message); return 1; },
  );
  if (DEBUG) console.error("[run returned]", JSON.stringify(raw));
  status = raw ?? 0;
  if (status === 0) {
    for (const [from, to] of extracts) {
      const bytes = Buffer.from(await page.evaluate((p) => window.__read(p), from), "base64");
      fs.mkdirSync(path.dirname(path.resolve(to)), { recursive: true });
      fs.writeFileSync(to, bytes);
      console.error(`[extract] ${from} -> ${to} (${bytes.length} bytes)`);
    }
  }
} finally {
  await browser.close().catch(() => {});
  server.close();
}
process.exit(status);
