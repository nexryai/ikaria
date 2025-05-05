import { parentPort } from 'worker_threads';
import Ikaria from './ikaria-wasm.js';


parentPort!.on('message', async msg => {
    const { type, filePath } = msg;

    const parentDir = filePath.substring(0, filePath.lastIndexOf('/'));
    const fileName = filePath.substring(filePath.lastIndexOf('/') + 1);

    let data;

    const ikaria = await Ikaria();

    switch (type) {
        case 'get_file_info':
            // Mount FS for files.
            if (!ikaria.FS.analyzePath('/work', false).exists) {
                ikaria.FS.mkdir('/work');
            }
            ikaria.FS.mount("NODEFS", {
                // mount parent directory of the file path
                root: parentDir,

            }, '/work');

            ikaria.trimingWebM('/work/' + fileName, '/trimed_' + fileName, "0", "20.000000");

            const trimmedFileBuffer = ikaria.FS.readFile('/trimed_' + fileName);

            // Call the wasm module.
            const info = ikaria.getVideoInfo('/trimed_' + fileName);

            const keyframes = [];
            // ref: https://github.com/emscripten-core/emscripten/issues/11070
            // @ts-ignore
            for (let i = 0; i < info.keyframes.size(); i++) {
                // @ts-ignore
                keyframes.push(info.keyframes.get(i).pts_time_string);
            }

            const versions = {
                libavutil:  ikaria.AVUTIL_VERSION(),
                libavcodec:  ikaria.AVCODEC_VERSION(),
                libavformat:  ikaria.AVFORMAT_VERSION(),
            };

            // Send back data response.
            data = {
                ...info,
                keyframes: keyframes,
                versions,
                trimmedFileBuffer,
            }

            console.log('Worker: File info', data);
            postMessage(data);

            // Cleanup mount.
            ikaria.FS.unmount('/work');
            break;
    
    
        default:
            break;
    }

});

// self.importScripts('ikaria-wasm.js'); // Load ffprobe into worker context.
