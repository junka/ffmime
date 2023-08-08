#!/usr/bin/env node

const ffmime = require("./index.js");

(async function main() {
  if (process.argv.length < 3) {
    console.log("no specified file");
    return;
  }
  const filename = process.argv[2];
  const ret = await ffmime.GetMimeCodecs(filename);
  console.log(ret);
})();
