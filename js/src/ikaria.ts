import Ikaria from './ikaria-wasm.js';


self.onmessage = async (e: MessageEvent) => {
    const type = e.data[0];
    const file = e.data[1];

    let data;

    const ikaria = await Ikaria();

    switch (type) {
        case 'get_file_info':
            // Mount FS for files.
            if (!ikaria.FS.analyzePath('/work', false).exists) {
                ikaria.FS.mkdir('/work');
            }
            ikaria.FS.mount("WORKERFS", { files: [file] }, '/work');

            ikaria.trimingWebM('/work/' + file.name, '/trimed_' + file.name, "0", "20.000000");

            const trimmedFileBuffer = ikaria.FS.readFile('/trimed_' + file.name);
            const trimmedFile = new Blob([trimmedFileBuffer], { type: file.type });
            const blobUrl = URL.createObjectURL(trimmedFile);

            // Call the wasm module.
            const info = ikaria.getVideoInfo('/trimed_' + file.name);

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
                blobUrl
            }

            console.log('Worker: File info', data);
            postMessage(data);

            // Cleanup mount.
            ikaria.FS.unmount('/work');
            break;
    
    
        default:
            break;
    }

}

// self.importScripts('ikaria-wasm.js'); // Load ffprobe into worker context.
