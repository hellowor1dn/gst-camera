#include <gst/gst.h>

static gboolean on_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    auto loop = (GMainLoop *) data;
    g_print("Got %s msg\n", GST_MESSAGE_TYPE_NAME (msg));

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream.\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;

            gst_message_parse_error(msg, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);

            g_main_loop_quit(loop);
            break;
        }
//        case GST_MESSAGE_STATE_CHANGED: {
//            GstState previous, next, pending;
//            gst_message_parse_state_changed(msg, &previous, &next, &pending);
//
//            break;
//        }
        default:
            /* unhandled message */
            break;
    }

    /* we want to be notified again the next time there is a message
     * on the bus, so returning TRUE (FALSE means we want to stop watching
     * for messages on the bus and our callback should not be called again)
     */
    return TRUE;
}

int main(int argc, char *argv[]) {
    // check args
    if (argc != 3) {
        g_printerr("Usage: %s <v4l2 dev filename> <save location template>\n", argv[0]);
        return -1;
    }

    // init gstreamer
    gst_init(&argc, &argv);

    // init loop
    auto loop = g_main_loop_new(NULL, FALSE);

    // create pipeline
    auto pipeline = gst_pipeline_new("camera-recorder");
    if (!pipeline) {
        g_printerr("Cannot create pipeline element.\n");
        return -1;
    }

    // region create elements
    // region create video source
    // v4l2 source
    auto v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    auto video_filter = gst_element_factory_make("capsfilter", "video_filter");
    auto jpegdec = gst_element_factory_make("jpegdec", "jpegdec");
    if (!v4l2src || !video_filter || !jpegdec) {
        g_printerr("Cannot create stream element.\n");
        return -1;
    }

    // set input device
    g_object_set(G_OBJECT (v4l2src), "device", argv[1], NULL);

    // set video caps
    auto video_caps = gst_caps_new_simple("image/jpeg",
                                          "width", G_TYPE_INT, 2592,
                                          "height", G_TYPE_INT, 1944,
                                          NULL);
    g_object_set(G_OBJECT(video_filter), "caps", video_caps, NULL);
    gst_caps_unref(video_caps);
    // endregion

    // region create mpeg encoder
    auto encode_queue = gst_element_factory_make("queue", "encode_queue");
    auto x264enc = gst_element_factory_make("x264enc", "x264enc");
    auto h264_filter = gst_element_factory_make("capsfilter", "h264_filter");
    auto h264parse = gst_element_factory_make("h264parse", "h264parse");
    if (!encode_queue || !x264enc || !h264_filter || !h264parse) {
        g_printerr("Cannot create stream element.\n");
        return -1;
    }

    // Maximal distance between two key-frames
    g_object_set(G_OBJECT (x264enc), "key-int-max", 10, NULL);

    // set encode caps
    auto encode_caps = gst_caps_new_simple("video/x-h264",
                                           "profile", G_TYPE_STRING, "high",
                                           NULL);
    g_object_set(G_OBJECT(h264_filter), "caps", encode_caps, NULL);
    gst_caps_unref(encode_caps);
    // endregion

    // region split and write files
    auto splitmuxsink = gst_element_factory_make("splitmuxsink", "splitmuxsink");
    if (!splitmuxsink) {
        g_printerr("Cannot create stream element.\n");
        return -1;
    }

    // set file location
    g_object_set(G_OBJECT (splitmuxsink), "location", argv[2], NULL);

    // split every 5 minutes
    g_object_set(G_OBJECT (splitmuxsink), "max-size-time", 10000000000, NULL);
//    g_object_set(G_OBJECT (splitmuxsink), "max-size-time", 3e+11, NULL);
    g_object_set(G_OBJECT (splitmuxsink), "send-keyframe-requests", TRUE, NULL);
    // endregion
    // endregion

    // region set up pipeline
    // add elements to pipeline
    gst_bin_add_many(GST_BIN (pipeline),
                     v4l2src,
                     video_filter,
                     jpegdec,
                     encode_queue,
                     x264enc,
                     h264_filter,
                     h264parse,
                     splitmuxsink,
                     NULL);

    // link source
    if (!gst_element_link_many(v4l2src,
                               video_filter,
                               jpegdec,
                               encode_queue,
                               x264enc,
                               h264_filter,
                               h264parse,
                               splitmuxsink,
                               NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // add a message handler
    auto bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
    auto bus_watch_id = gst_bus_add_watch(bus, on_bus_call, loop);
    gst_object_unref(bus);
    // endregion

    // start playing
    g_print("Now playing\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // run main loop
    g_print("Running...\n");
    g_main_loop_run(loop);

    // clean up
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT (pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);

    return 0;
}