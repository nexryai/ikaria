import Ikaria from './ikaria-wasm.js';

console.log('Worker: Starting worker');

self.onmessage = async (e: MessageEvent) => {
    try {
        console.log('Worker: Received message', e.data);
        const type = e.data[0];
        const file = e.data[1];

        const ikaria = await Ikaria();

        switch (type) {
            case 'get_video_info':
                // Mount FS for files.
                if (!ikaria.FS.analyzePath('/work', false).exists) {
                    ikaria.FS.mkdir('/work');
                }

                ikaria.FS.mount(ikaria.FS.filesystems.WORKERFS, { files: [file] }, '/work');

                // Call the wasm module.
                const info = ikaria.getVideoInfo('/work/' + file.name);

                const keyframes = [];
                // ref: https://github.com/emscripten-core/emscripten/issues/11070
                // @ts-ignore
                for (let i = 0; i < info.keyframes.size(); i++) {
                    // @ts-ignore
                    keyframes.push(info.keyframes.get(i).ptsTimeString);
                }

                // Send back data response.
                const data = {
                    ...info,
                    keyframes: keyframes,
                }

                console.log('Worker: File info', data);
                postMessage(data);

                // Cleanup mount.
                ikaria.FS.unmount('/work');
                break;
        
            case 'trim_video':
                const startSec = e.data[2];
                const endSec = e.data[3];
                const tweakTimestamp = e.data[4];

                // Mount FS for files.
                if (!ikaria.FS.analyzePath('/work', false).exists) {
                    ikaria.FS.mkdir('/work');
                }

                ikaria.FS.mount(ikaria.FS.filesystems.WORKERFS, { files: [file] }, '/work');

                ikaria.trimingWebM('/work/' + file.name, '/trimed_' + file.name, startSec, endSec, tweakTimestamp);

                const trimmedFileBuffer = ikaria.FS.readFile('/trimed_' + file.name);
                const trimmedFile = new Blob([trimmedFileBuffer], { type: file.type });
                const blobUrl = URL.createObjectURL(trimmedFile);

                postMessage(blobUrl);

                // Cleanup mount.
                ikaria.FS.unmount('/work');
                break;
            
            default:
                break;
        }
    } catch (error: any) {
        postMessage({ error: error.message });
    }
}
