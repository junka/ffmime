// const require = createRequire(import.meta.url);
const path = require('path')
const ffpromise = new Promise((resolve) => {
  const ffmime = require("./ffmime-wasm.js");
  ffmime.onRuntimeInitialized = () => {
    resolve(ffmime);
  };
});

async function GetMimeCodecs(file) {
  const { FS, getmime } = await ffpromise;
  try {
    if (FS.findObject("/root") == null) {
      FS.mkdir("/root");
    }
    FS.mount(FS.filesystems.NODEFS, { root: path.dirname(file) }, "/root");
    return getmime("/root/" + path.basename(file));
  } catch (error) {
    console.log("err", error);
  } finally {
    // Cleanup mount.
    FS.unmount("/root");
  }
}

module.exports = {GetMimeCodecs};