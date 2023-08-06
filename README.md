

For MSE, we usually need a mime type and codec string like
```
video/mp4; codecs="avc1.64001E, mp4a.40.2"
audio/mp4; codecs=mp4a.40.42
video/webm; codecs="vp8, vorbis"
```
see More [web format codec](https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter).
see [Wiki](https://wiki.whatwg.org/wiki/video_type_parameters)

---

mime supported types
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


How to know the the mime type and codecs.
just run 
```
make mime
./ffmime mediafile
```

wasm
```
docker build -f Dockerfile --output dist .
```
wasm js file is built to dist directory