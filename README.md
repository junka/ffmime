
### About mime codecs
For MSE, we usually need a mime type and codec string like
```
video/mp4; codecs="avc1.64001E, mp4a.40.2"
audio/mp4; codecs=mp4a.40.42
video/webm; codecs="vp8, vorbis"
```
see More [web format codec](https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter).
see [Wiki](https://wiki.whatwg.org/wiki/video_type_parameters)

---

### ffmime supported types
```
text/
image/
audio/
video/
```
codecs supported types
```
av01.P.LLT.DD[.M.CCC.cp.tc.mc.F]
cccc[.pp]* (Generic ISO BMFF)
mp4v.oo[.V] (MPEG-4 video)
mp4a.oo[.A] (MPEG-4 audio)
avc1[.PPCCLL] (AVC video)
hev1.P.T.Ll.Bb (HEVC video)
and other no spec using codec name
```

### How to use a simple command line
How to know the the mime type and codecs.
For those with ffmpeg library installed and want to build from source
```
make mime
./ffmime mediafile
```
For those want to run npx
```
npx ffmime mediafile
```

### wasm
```
npm install ffmime
```
Then in the code, like in electron

```
import {GetMimeCodecs} from 'ffmime'
const mimeType = await GetMimeCodecs(filename)
console log(mimeType)
```

or run npx like above.


To build wasm from source,
```
docker build -f Dockerfile --output package .
```
wasm js files are built to package directory and then you can publish and import the package.
Only muxer and demuxer enabled in ffmpeg, so we could have a minified wasm binary which is less than 2MB size. This is much more reasonable than a full ffmpeg when we only need a probe for the mimetype and check with MediaSource.isTypeSupported.

It is really fast enough for demuxing and probe, so put it in a worker is not that reasonable.

### note
some format could have a side effect. 
For example, a `mkv` format could output `video/x-matroska; codecs="avc1.640029, mp4a.40.2"` which is not supported by chrome, but you can replace `x-matroska` with `mp4` to make it supported.
However, even the mimetype is `video/mp4`, if the container is fragmented, the media file will not be decoded right by MSE.

A `mp3` format could output `audio/mpeg; codecs=85` which `85` is the codec id. For use in browser, just strip the codecs. `audio/mpeg` will be supported by the browser.

Raw `pcm` can hardly probed without any hint. so it will return empty string.