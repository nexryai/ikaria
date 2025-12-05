import Ikaria from './ikaria-wasm.js';

console.log('Worker: Starting worker');

self.onmessage = async (e) => {
    try {
        console.log('Worker: Received message', e.data);
        const type = e.data[0];
        const file = e.data[1];

        const ikaria = await Ikaria();

        switch (type) {
            case 'read_file':

            case 'read_dir':

            case 'remux_to_dash':
                console.log("Starting....")
                ikaria.remuxToDash(file, `/out/manifest.mpd`)
                console.log("Done!");
                postMessage({ result: "ok" });

            default:
                break;
        }
    } catch (error) {
        postMessage({ error: error.message });
    }
}

export default self;
