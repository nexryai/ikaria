// vite.config.ts
import { defineConfig } from 'vite';
import dts from 'vite-plugin-dts';
import { viteStaticCopy } from 'vite-plugin-static-copy';

export default defineConfig({
  build: {
    lib: {
      entry: 'src/index.ts', // エントリーポイント
      name: 'ikaria',        // グローバル変数として公開する場合の名前（UMD用）
      fileName: 'index',
      formats: ['es'],       // type: module なので ES Modules 形式のみでOK
    },
    rollupOptions: {
      external: [/^node:/],
    },
  },
  plugins: [
    // 型定義ファイルの生成 (.d.ts)
    dts({
      rollupTypes: true
    }),
    // WASMファイルを dist にコピー
    viteStaticCopy({
      targets: [
        {
          src: 'src/ikaria-wasm.wasm',
          dest: '.' // dist直下に配置
        }
      ]
    })
  ],
});
