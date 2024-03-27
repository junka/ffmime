
### About MIME codecs
When we use MSE (Media Source Extension) for developing a media player, the api **MediaSource.isTypeSupported** needs extra information about the media file.
We will need a MIME codec string like
```
video/mp4; codecs="avc1.64001E, mp4a.40.2"
audio/mp4; codecs=mp4a.40.42
video/webm; codecs="vp8, vorbis"
```


see More [web format codec](https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter).
see [Wiki](https://wiki.whatwg.org/wiki/video_type_parameters)

It consists of the MIME type part and codecs string part.

---

### MIME type supported
Media files would have text(VTT subtitle), image, audio and video files, so it is supposed to recognize them like below。
```
text/
image/
audio/
video/
```
And codecs string like below.
```
av01.P.LLT.DD[.M.CCC.cp.tc.mc.F]
cccc[.pp]* (Generic ISO BMFF)
mp4v.oo[.V] (MPEG-4 video)
mp4a.oo[.A] (MPEG-4 audio)
avc1[.PPCCLL] (AVC video)
hev1.P.T.Ll.Bb (HEVC video)
and other no spec using codec name
```

### How to use
How to know the the mime type and codecs.
For those who have installed nodejs, you may run npx like below.
```
npx ffmime <mediafile>
```

For those who have installed the ffmpeg libraries and want to build from source, run
```
git clone https://github.com/junka/ffmime.git
cd ffmime
make mime
./ffmime mediafile
```

### wasm
We can use it in javascript code.
```
npm install ffmime
```
Then in the code, like in electron we can get the MIME type and codecs string. 
The returned string can be used for the MSE API **MediaSource.isTypeSupported()**.

```
import {GetMimeCodecs} from 'ffmime'
const mimeType = await GetMimeCodecs(filename)
console log(mimeType)
```


To build wasm from source,
```
docker build -f Dockerfile --output package .
```
wasm js files are built into package directory and then you can publish and import the package.
Only muxer and demuxer configured as enabled in ffmpeg, so we could have a minified wasm binary which is less than 2MB size. This is much more reasonable than a full ffmpeg. Here we only need a probe for the mimetype and codecs string.

### Extra note
Some format could have a side effect. 
- For example, a `mkv` format could output `video/x-matroska; codecs="avc1.640029, mp4a.40.2"` which is not supported by Chrome, but you can replace `x-matroska` with `mp4` to make it pass the check of the Chrome.

- However, when the mimetype is `video/mp4`, if the mp4 container box is **NOT** **fragmented**, the media file would not be decoded right by MSE.

- A `mp3` format could output `audio/mpeg; codecs=85` which `85` is the codec id. For use in browser, just strip the codecs. `audio/mpeg` will be supported by the browser.

- Raw `pcm` can hardly probed without any hint. so it will return empty string.
