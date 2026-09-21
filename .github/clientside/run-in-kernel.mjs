#!/usr/bin/env node
// Boot a kernel in headless Chromium, untar a runner into it, run one command.
//
//   run-in-kernel.mjs <kernel-dir> <sysroot-dir> <boot.json> <runner.tar> <cmd> [args...]
//
// Exits with the command's status. stdout/stderr are relayed as they arrive.
//
// This is the browser equivalent of `docker run --entrypoint <cmd> <image>`.
// The kernel is threaded wasm and only runs in a cross-origin isolated page,
// so there is no way to drive it from Node directly: everything is served
// to a real headless Chromium, which is also the runtime students get.
//
// Needs `playwright` on the module path and a Chromium it can launch
// (`npx playwright install --with-deps chromium`, or CHROMIUM_PATH).

import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import { chromium } from "playwright";

const [kernelDir, sysrootDir, bootJson, tarball, ...argv] = process.argv.slice(2);
if (!argv.length) {
  console.error("usage: run-in-kernel.mjs <kernel-dir> <sysroot-dir> <boot.json> <runner.tar> <cmd> [args...]");
  process.exit(64);
}

const DEBUG = !!process.env.KERNEL_DEBUG;

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
    await client.boot(new URL("/sysroot.tar", location.href).href, env, boot.preload, boot.licence);
    await client.untar("/", await (await fetch("/runner.tar")).arrayBuffer());
    const sid = await client.run(argv, env, "/opt/test-runner", true);
    if (early.has(sid)) return early.get(sid);
    return await new Promise((resolve) => ended.set(sid, resolve));
  };
  window.__ready = true;
</script>`;

const server = http.createServer((req, res) => {
  const p = new URL(req.url, "http://localhost").pathname;
  if (p === "/") { res.writeHead(200, { ...ISOLATION, "Content-Type": "text/html" }); return res.end(PAGE); }
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
} finally {
  await browser.close().catch(() => {});
  server.close();
}
process.exit(status);
