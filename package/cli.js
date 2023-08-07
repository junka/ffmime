const GetMimeCodecs = require('./index');

(async function main() {
  const filename = process.argv[2];
  // const filename = "/Users/admin/proj/github/ffmime/test.mp4";
  const ret = await GetMimeCodecs(filename);
  console.log(ret);
})();
