Gstreamer Camera
----------------

#### Command line examples:

```bash
# save mp4 video every 10s or 1MB
gst-launch-1.0 -e videotestsrc ! video/x-raw,width=1920,height=1080 ! queue ! x264enc ! h264parse ! splitmuxsink location=video%02d.mp4 max-size-time=10000000000 max-size-bytes=1000000

# save mp4 videos every 10s AND show preview 
gst-launch-1.0 -e videotestsrc ! video/x-raw,width=1920,height=1080 ! queue ! x264enc ! h264parse ! tee name=t ! queue ! splitmuxsink location=video%02d.mp4 max-size-time=10000000000 t. ! queue ! h264parse ! decodebin ! autovideosink sync=false

# stream h264 over rtp
gst-launch-1.0 -v v4l2src do-timestamp=true ! video/x-raw, format=\(string\)YUY2, width=\(int\)640, height=\(int\)480, framerate=\(fraction\)60/1 ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency intra-refresh=true vbv-buf-capacity=0 qp-min=21 pass=qual quantizer=24 byte-stream=true key-int-max=30 ! rtph264pay ! udpsink host=192.168.21.241 port=42146 auto-multicast=false
```
