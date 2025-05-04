onmessage = (e) => {
    const type = e.data[0];
    const file = e.data[1];

    let data;

    switch (type) {
        case 'get_file_info':
            // Mount FS for files.
            if (!FS.analyzePath('/work').exists) {
                FS.mkdir('/work');
            }
            FS.mount(WORKERFS, { files: [file] }, '/work');

            Module.trimingWebM('/work/' + file.name, '/trimed_' + file.name, "0", "20.000000");

            const trimmedFileBuffer = FS.readFile('/trimed_' + file.name);
            const trimmedFile = new Blob([trimmedFileBuffer], { type: file.type });
            const blobUrl = URL.createObjectURL(trimmedFile);

            // Call the wasm module.
            const info = Module.getVideoInfo('/trimed_' + file.name);

            const keyframes = [];
            for (let i = 0; i < info.keyframes.size(); i++) {
                keyframes.push(info.keyframes.get(i).pts_time_string);
            }

            const versions = {
                libavutil:  Module.avutil_version(),
                libavcodec:  Module.avcodec_version(),
                libavformat:  Module.avformat_version(),
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
            FS.unmount('/work');
            break;
        
        case 'get_frames':
            if (!FS.analyzePath('/work').exists) {
                FS.mkdir('/work');
            }
            FS.mount(WORKERFS, { files: [file] }, '/work');

            const offset = e.data[2];
            const frames = Module.get_frames('/work/' + file.name, offset);

            // Remap frames into collection.
            const f = [];
            for (let i = 0; i < frames.frames.size(); i++) {
                f.push(frames.frames.get(i));
            }

            data = {
                ...frames,
                frames: f,
            }
            postMessage(data);

            // Cleanup mount.
            FS.unmount('/work');
            break;
    
        default:
            break;
    }

}
self.importScripts('ffprobe-wasm.js'); // Load ffprobe into worker context.