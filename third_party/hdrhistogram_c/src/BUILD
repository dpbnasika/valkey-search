licenses(["notice"])
package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "hdrhistogram_c",
    srcs = [
        "hdr_encoding.c",
        "hdr_histogram.c",
        "hdr_interval_recorder.c",
        "hdr_thread.c",
        "hdr_time.c",
        "hdr_writer_reader_phaser.c",
    ],
    hdrs = [
        "hdr_atomic.h",
        "hdr_encoding.h",
        "hdr_endian.h",
        "hdr_histogram.h",
        "hdr_interval_recorder.h",
        "hdr_tests.h",
        "hdr_thread.h",
        "hdr_time.h",
        "hdr_writer_reader_phaser.h",
    ],
    copts = [
        "-include stddef.h",
    ],
    includes = ["."],
)
