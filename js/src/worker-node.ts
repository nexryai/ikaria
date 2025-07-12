import { parentPort } from 'worker_threads';
import Ikaria from './ikaria-wasm.js';

if (!parentPort) {
    throw new Error('This module must be run as a worker thread');
}

parentPort.on('message', async (message: any) => {
    try {
        const { type, filePath, startSec, endSec, tweakTimestamp, resizeWidth, resizeHeight } = message;
        const parentDir = filePath.substring(0, filePath.lastIndexOf('/'));
        const fileName = filePath.substring(filePath.lastIndexOf('/') + 1);

        const ikaria = await Ikaria();

        if (!ikaria.FS.analyzePath('/work', false).exists) {
            ikaria.FS.mkdir('/work');
        }
        ikaria.FS.mount(ikaria.FS.filesystems.NODEFS, {
            root: parentDir,
        }, '/work');

        switch (type) {
            case 'get_video_info':
                // Call the wasm module.
                const info = ikaria.getVideoInfo('/work/' + fileName);

                const keyframes = [];
                // ref: https://github.com/emscripten-core/emscripten/issues/11070
                // @ts-ignore
                for (let i = 0; i < info.keyframes.size(); i++) {
                    // @ts-ignore
                    keyframes.push(info.keyframes.get(i).ptsTimeString);
                }

                const versions = {
                    libavutil: ikaria.AVUTIL_VERSION(),
                    libavcodec: ikaria.AVCODEC_VERSION(),
                    libavformat: ikaria.AVFORMAT_VERSION(),
                };

                // Send back data response.
                const data = {
                    ...info,
                    keyframes,
                    versions
                };

                console.log('Worker: File info', data);
                parentPort?.postMessage(data);
                break;

            case 'trim_video':
                ikaria.trimingWebM('/work/' + fileName, '/trimed_' + fileName, startSec, endSec, tweakTimestamp);
                const trimmedFileBuffer = ikaria.FS.readFile('/trimed_' + fileName);
                parentPort?.postMessage(trimmedFileBuffer);
                break;

            case 'reseize_image':
                ikaria.resizeImageToWebP('/work/' + fileName, `/${fileName}2.webp`, resizeWidth, resizeHeight);
                const resizedImageBuffer = ikaria.FS.readFile(`/${fileName}2.webp`);
                parentPort?.postMessage(resizedImageBuffer);
                break;

            default:
                break;
        }

        // Cleanup mount
        ikaria.FS.unmount('/work');
    } catch (error: any) {
        parentPort?.postMessage({ error: error.message });
    }
});

// self.importScripts('ikaria-wasm.js'); // Load ffprobe into worker context.
