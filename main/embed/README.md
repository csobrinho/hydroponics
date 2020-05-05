## Convert png to RGB565 binary format.

```bash
ffmpeg -vcodec png -i image.png -vcodec rawvideo -f rawvideo -pix_fmt rgb565 image.bin
```
