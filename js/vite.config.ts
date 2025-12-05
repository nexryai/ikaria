import { defineConfig } from 'vite';
import dts from 'vite-plugin-dts';
import { viteStaticCopy } from 'vite-plugin-static-copy';

export default defineConfig({
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
  },
  worker: {
    format: 'es'
  },
  plugins: [
    dts({
      rollupTypes: true
    }),
    viteStaticCopy({
      targets: [
        {
          src: 'src/ikaria-wasm.wasm',
          dest: '.'
        }
      ]
    })
  ],
});
