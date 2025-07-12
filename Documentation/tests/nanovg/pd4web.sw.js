const CACHE_NAME = 'webpatch-cache-v1';
const FILES_TO_CACHE = [
  'index.html',
  'index.pd',
  'pd4web.js',
  'pd4web.gui.js',
  'pd4web.threads.js',
  'pd4web.style.css',
  'pd4web.wasm',
  'pd4web.data',
  'icon-192.png',
  'icon-512.png'
];

self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(CACHE_NAME).then(cache => cache.addAll(FILES_TO_CACHE))
  );
});

self.addEventListener('fetch', event => {
  event.respondWith(
    caches.match(event.request).then(response =>
      response || fetch(event.request)
    )
  );
});

