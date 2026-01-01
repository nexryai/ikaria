import { defineConfig } from 'vite';
import dts from 'vite-plugin-dts';
import { viteStaticCopy } from 'vite-plugin-static-copy';

export default defineConfig({
  base: './',
  build: {
    lib: {
      entry: 'src/index.ts',
      name: 'ikaria',
      fileName: 'index',
      formats: ['es'],
    },
    rollupOptions: {
      external: [/^node:/],
    },
    minify: 'terser'
  },
  worker: {
    format: 'es',
  },
  plugins: [
    dts({
      rollupTypes: true
    }),
    viteStaticCopy({
      targets: [
        {
          src: 'src/ikaria.wasm',
          dest: '.'
        }
      ]
    })
  ],
});
