const esbuild = require('esbuild');
const fs = require('fs');
const path = require('path');

const forceInlineWorkerPlugin = {
  name: 'force-inline-worker',
  setup(build) {
    build.onLoad({ filter: /\.js$/ }, async (args) => {
      if (args.path.includes('bundle.js')) return;

      let contents = await fs.promises.readFile(args.path, 'utf8');
      const workerPattern = /assets\/worker-browser-[\w-]+\.js/g;
      const matches = contents.match(workerPattern);

      if (!matches) return null;

      for (const match of matches) {
        const workerFullPath = path.resolve(path.dirname(args.path), match);

        if (fs.existsSync(workerFullPath)) {
          console.log(`Inlining Worker: ${match}`);

          try {
            const workerResult = await esbuild.build({
              entryPoints: [workerFullPath],
              bundle: true,
              minify: true,
              format: 'esm',
              target: 'es2022',
              write: false,
            });

            const workerCode = workerResult.outputFiles[0].text;
            const base64 = Buffer.from(workerCode).toString('base64');
            const dataUrl = `data:application/javascript;base64,${base64}`;

            contents = contents.replace(match, dataUrl);
          } catch (e) {
            console.error(`Failed to bundle worker ${match}:`, e);
          }
        }
      }

      return {
        contents,
        loader: 'js',
      };
    });
  },
};

esbuild.build({
  entryPoints: ['dist/index.js'],
  bundle: true,
  outfile: 'dist/index.bundle.js',
  format: 'esm',
  minify: true,
  plugins: [forceInlineWorkerPlugin],
  target: 'es2022',
  logLevel: 'info',
}).catch(() => process.exit(1));
